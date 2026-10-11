#include "linux/perf_event_group.hpp"

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <limits>
#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <utility>

namespace ntq {
namespace linux_metrics {
namespace {

struct GroupReadHeader {
  std::uint64_t count;
  std::uint64_t time_enabled;
  std::uint64_t time_running;
};

struct GroupReadValue {
  std::uint64_t value;
  std::uint64_t id;
};

} // namespace

PerfEventStatus perf_status_from_errno(int error) {
  if (error == EACCES || error == EPERM) {
    return PerfEventStatus::permission_denied;
  }
  if (error == EINVAL || error == ENOENT || error == ENOSYS ||
      error == EOPNOTSUPP) {
    return PerfEventStatus::unsupported;
  }
  return PerfEventStatus::read_error;
}

PerfEventSpec make_hardware_event(std::uint64_t config) {
  return {PERF_TYPE_HARDWARE, config};
}

PerfEventSpec make_cache_event(unsigned cache, unsigned operation,
                               unsigned result) {
  const std::uint64_t config = static_cast<std::uint64_t>(cache) |
                               (static_cast<std::uint64_t>(operation) << 8) |
                               (static_cast<std::uint64_t>(result) << 16);
  return {PERF_TYPE_HW_CACHE, config};
}

PerfEventGroup::PerfEventGroup(std::vector<PerfEventSpec> events)
    : specs_(std::move(events)), descriptors_(specs_.size(), -1),
      ids_(specs_.size(), 0), results_(specs_.size()),
      read_buffer_(3 + specs_.size() * 2, 0) {}

PerfEventGroup::~PerfEventGroup() { close_descriptors(); }

PerfEventGroup::PerfEventGroup(PerfEventGroup &&other) noexcept
    : specs_(std::move(other.specs_)),
      descriptors_(std::move(other.descriptors_)), ids_(std::move(other.ids_)),
      results_(std::move(other.results_)),
      read_buffer_(std::move(other.read_buffer_)),
      leader_descriptor_(other.leader_descriptor_),
      is_prepared_(other.is_prepared_), is_running_(other.is_running_) {
  other.descriptors_.clear();
  other.leader_descriptor_ = -1;
  other.is_prepared_ = false;
  other.is_running_ = false;
}

PerfEventGroup &PerfEventGroup::operator=(PerfEventGroup &&other) noexcept {
  if (this != &other) {
    close_descriptors();
    specs_ = std::move(other.specs_);
    descriptors_ = std::move(other.descriptors_);
    ids_ = std::move(other.ids_);
    results_ = std::move(other.results_);
    read_buffer_ = std::move(other.read_buffer_);
    leader_descriptor_ = other.leader_descriptor_;
    is_prepared_ = other.is_prepared_;
    is_running_ = other.is_running_;
    other.descriptors_.clear();
    other.leader_descriptor_ = -1;
    other.is_prepared_ = false;
    other.is_running_ = false;
  }
  return *this;
}

void PerfEventGroup::close_descriptors() noexcept {
  for (int &descriptor : descriptors_) {
    if (descriptor >= 0) {
      close(descriptor);
      descriptor = -1;
    }
  }
  leader_descriptor_ = -1;
  is_prepared_ = false;
  is_running_ = false;
}

void PerfEventGroup::mark_group_unavailable(PerfEventStatus status) noexcept {
  for (auto &result : results_) {
    result.value = 0;
    result.status = status;
  }
  close_descriptors();
}

void PerfEventGroup::prepare() noexcept {
  close_descriptors();
  std::fill(ids_.begin(), ids_.end(), 0);
  for (auto &result : results_) {
    result = PerfEventResult{};
  }
  if (specs_.empty()) {
    return;
  }

#if !defined(__NR_perf_event_open)
  mark_group_unavailable(PerfEventStatus::unsupported);
  is_prepared_ = true;
#else
  for (std::size_t index = 0; index < specs_.size(); ++index) {
    perf_event_attr attributes{};
    attributes.type = specs_[index].type;
    attributes.size = sizeof(attributes);
    attributes.config = specs_[index].config;
    attributes.disabled = index == 0;
    attributes.inherit = 1;
    attributes.inherit_stat = 1;
    attributes.exclude_kernel = 1;
    attributes.exclude_hv = 1;
    if (index == 0) {
      attributes.read_format = PERF_FORMAT_GROUP | PERF_FORMAT_ID |
                               PERF_FORMAT_TOTAL_TIME_ENABLED |
                               PERF_FORMAT_TOTAL_TIME_RUNNING;
    }

    const int group_fd = index == 0 ? -1 : leader_descriptor_;
    const int descriptor = static_cast<int>(syscall(
        __NR_perf_event_open, &attributes, 0, -1, group_fd,
        PERF_FLAG_FD_CLOEXEC));
    if (descriptor == -1) {
      mark_group_unavailable(perf_status_from_errno(errno));
      is_prepared_ = true;
      return;
    }
    descriptors_[index] = descriptor;
    if (index == 0) {
      leader_descriptor_ = descriptor;
    }
    if (ioctl(descriptor, PERF_EVENT_IOC_ID, &ids_[index]) == -1) {
      mark_group_unavailable(perf_status_from_errno(errno));
      is_prepared_ = true;
      return;
    }
  }

  is_prepared_ = true;
#endif
}

void PerfEventGroup::start() noexcept {
  if (!is_prepared_ || leader_descriptor_ < 0) {
    return;
  }
  if (ioctl(leader_descriptor_, PERF_EVENT_IOC_RESET,
            PERF_IOC_FLAG_GROUP) == -1 ||
      ioctl(leader_descriptor_, PERF_EVENT_IOC_ENABLE,
            PERF_IOC_FLAG_GROUP) == -1) {
    mark_group_unavailable(perf_status_from_errno(errno));
    return;
  }
  is_running_ = true;
}

void PerfEventGroup::stop() noexcept {
  if (!is_running_ || leader_descriptor_ < 0) {
    return;
  }
  if (ioctl(leader_descriptor_, PERF_EVENT_IOC_DISABLE,
            PERF_IOC_FLAG_GROUP) == -1) {
    mark_group_unavailable(perf_status_from_errno(errno));
    return;
  }

  std::fill(read_buffer_.begin(), read_buffer_.end(), 0);
  const std::size_t expected_bytes = read_buffer_.size() * sizeof(std::uint64_t);
  ssize_t bytes_read;
  do {
    bytes_read = read(leader_descriptor_, read_buffer_.data(), expected_bytes);
  } while (bytes_read == -1 && errno == EINTR);
  if (bytes_read != static_cast<ssize_t>(expected_bytes)) {
    mark_group_unavailable(bytes_read == -1
                               ? perf_status_from_errno(errno)
                               : PerfEventStatus::read_error);
    return;
  }

  const GroupReadHeader header{read_buffer_[0], read_buffer_[1],
                               read_buffer_[2]};
  if (header.time_running == 0) {
    mark_group_unavailable(PerfEventStatus::not_measured);
    return;
  }
  if (header.count != specs_.size() || header.time_enabled == 0 ||
      header.time_running > header.time_enabled) {
    mark_group_unavailable(PerfEventStatus::read_error);
    return;
  }

  for (std::size_t value_index = 0; value_index < header.count; ++value_index) {
    const std::size_t offset = 3 + value_index * 2;
    const GroupReadValue reading{read_buffer_[offset],
                                 read_buffer_[offset + 1]};
    std::size_t event_index = 0;
    while (event_index < ids_.size() && ids_[event_index] != reading.id) {
      ++event_index;
    }
    if (event_index == ids_.size()) {
      mark_group_unavailable(PerfEventStatus::read_error);
      return;
    }

    const long double scaled =
        static_cast<long double>(reading.value) * header.time_enabled /
        header.time_running;
    if (scaled > static_cast<long double>(
                     std::numeric_limits<std::uint64_t>::max())) {
      mark_group_unavailable(PerfEventStatus::read_error);
      return;
    }
    results_[event_index].value =
        static_cast<std::uint64_t>(scaled + 0.5L);
    results_[event_index].status = PerfEventStatus::available;
  }

  close_descriptors();
}

const std::vector<PerfEventResult> &PerfEventGroup::results() const noexcept {
  return results_;
}

} // namespace linux_metrics
} // namespace ntq
