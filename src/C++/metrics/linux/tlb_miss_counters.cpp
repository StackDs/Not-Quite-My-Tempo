#include "linux/tlb_miss_counters.hpp"

#include <cerrno>
#include <cstdint>
#include <limits>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace ntq {
namespace linux_metrics {
namespace {

struct CounterRead {
  std::uint64_t value;
  std::uint64_t time_enabled;
  std::uint64_t time_running;
};

int open_cache_event(unsigned operation) {
#if defined(__NR_perf_event_open)
  perf_event_attr attributes{};
  attributes.type = PERF_TYPE_HW_CACHE;
  attributes.size = sizeof(attributes);
  attributes.config = static_cast<std::uint64_t>(PERF_COUNT_HW_CACHE_DTLB) |
                      (static_cast<std::uint64_t>(operation) << 8) |
                      (static_cast<std::uint64_t>(
                           PERF_COUNT_HW_CACHE_RESULT_MISS)
                       << 16);
  attributes.disabled = 1;
  attributes.inherit = 1;
  attributes.inherit_stat = 1;
  attributes.exclude_kernel = 1;
  attributes.exclude_hv = 1;
  attributes.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED |
                           PERF_FORMAT_TOTAL_TIME_RUNNING;

  return static_cast<int>(syscall(__NR_perf_event_open, &attributes, 0, -1, -1,
                                  PERF_FLAG_FD_CLOEXEC));
#else
  (void)operation;
  errno = ENOSYS;
  return -1;
#endif
}

} // namespace

MemoryMetricStatus status_from_perf_errno(int error) {
  if (error == EACCES || error == EPERM) {
    return MemoryMetricStatus::permission_denied;
  }
  if (error == EINVAL || error == ENOENT || error == ENOSYS ||
      error == EOPNOTSUPP) {
    return MemoryMetricStatus::unsupported;
  }
  return MemoryMetricStatus::read_error;
}

TlbMissCounters::~TlbMissCounters() {
  close_event(read_misses_);
  close_event(write_misses_);
}

TlbMissCounters::TlbMissCounters(TlbMissCounters &&other) noexcept
    : read_misses_(other.read_misses_), write_misses_(other.write_misses_) {
  other.read_misses_.descriptor = -1;
  other.read_misses_.status = MemoryMetricStatus::not_measured;
  other.write_misses_.descriptor = -1;
  other.write_misses_.status = MemoryMetricStatus::not_measured;
}

TlbMissCounters &TlbMissCounters::operator=(TlbMissCounters &&other) noexcept {
  if (this != &other) {
    close_event(read_misses_);
    close_event(write_misses_);
    read_misses_ = other.read_misses_;
    write_misses_ = other.write_misses_;
    other.read_misses_.descriptor = -1;
    other.read_misses_.status = MemoryMetricStatus::not_measured;
    other.write_misses_.descriptor = -1;
    other.write_misses_.status = MemoryMetricStatus::not_measured;
  }
  return *this;
}

void TlbMissCounters::close_event(EventCounter &event) noexcept {
  if (event.descriptor >= 0) {
    close(event.descriptor);
    event.descriptor = -1;
  }
}

void TlbMissCounters::start_event(EventCounter &event,
                                  unsigned operation) noexcept {
  close_event(event);
  event.value = 0;
  event.status = MemoryMetricStatus::not_measured;

  event.descriptor = open_cache_event(operation);
  if (event.descriptor == -1) {
    event.status = status_from_perf_errno(errno);
    return;
  }
  if (ioctl(event.descriptor, PERF_EVENT_IOC_RESET, 0) == -1 ||
      ioctl(event.descriptor, PERF_EVENT_IOC_ENABLE, 0) == -1) {
    event.status = status_from_perf_errno(errno);
    close_event(event);
    return;
  }
  event.status = MemoryMetricStatus::available;
}

void TlbMissCounters::stop_event(EventCounter &event) noexcept {
  if (event.descriptor < 0) {
    return;
  }

  if (ioctl(event.descriptor, PERF_EVENT_IOC_DISABLE, 0) == -1) {
    event.status = status_from_perf_errno(errno);
    close_event(event);
    return;
  }

  CounterRead reading{};
  ssize_t bytes_read;
  do {
    bytes_read = read(event.descriptor, &reading, sizeof(reading));
  } while (bytes_read == -1 && errno == EINTR);

  if (bytes_read != static_cast<ssize_t>(sizeof(reading)) ||
      reading.time_enabled == 0 || reading.time_running == 0 ||
      reading.time_running > reading.time_enabled) {
    event.status = bytes_read == -1 ? status_from_perf_errno(errno)
                                    : MemoryMetricStatus::read_error;
    close_event(event);
    return;
  }

  const long double scaled =
      static_cast<long double>(reading.value) * reading.time_enabled /
      reading.time_running;
  if (scaled > static_cast<long double>(
                   std::numeric_limits<std::uint64_t>::max())) {
    event.status = MemoryMetricStatus::read_error;
  } else {
    event.value = static_cast<std::uint64_t>(scaled + 0.5L);
    event.status = MemoryMetricStatus::available;
  }
  close_event(event);
}

void TlbMissCounters::start() noexcept {
  start_event(read_misses_, PERF_COUNT_HW_CACHE_OP_READ);
  start_event(write_misses_, PERF_COUNT_HW_CACHE_OP_WRITE);
}

void TlbMissCounters::stop() noexcept {
  stop_event(read_misses_);
  stop_event(write_misses_);
}

void TlbMissCounters::apply(MemoryMeasurements &measurements) const noexcept {
  measurements.data_tlb_read_misses = read_misses_.value;
  measurements.data_tlb_read_status = read_misses_.status;
  measurements.data_tlb_write_misses = write_misses_.value;
  measurements.data_tlb_write_status = write_misses_.status;
}

} // namespace linux_metrics
} // namespace ntq
