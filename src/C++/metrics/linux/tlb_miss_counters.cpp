#include "linux/tlb_miss_counters.hpp"

#include <linux/perf_event.h>
#include <utility>

namespace ntq {
namespace linux_metrics {
namespace {

MemoryMetricStatus memory_status(PerfEventStatus status) {
  switch (status) {
  case PerfEventStatus::available:
    return MemoryMetricStatus::available;
  case PerfEventStatus::unsupported:
    return MemoryMetricStatus::unsupported;
  case PerfEventStatus::permission_denied:
    return MemoryMetricStatus::permission_denied;
  case PerfEventStatus::read_error:
    return MemoryMetricStatus::read_error;
  case PerfEventStatus::not_measured:
    return MemoryMetricStatus::not_measured;
  }
  return MemoryMetricStatus::read_error;
}

} // namespace

MemoryMetricStatus status_from_perf_errno(int error) {
  return memory_status(perf_status_from_errno(error));
}

TlbMissCounters::TlbMissCounters()
    : events_({make_cache_event(PERF_COUNT_HW_CACHE_DTLB,
                                PERF_COUNT_HW_CACHE_OP_READ,
                                PERF_COUNT_HW_CACHE_RESULT_MISS),
               make_cache_event(PERF_COUNT_HW_CACHE_DTLB,
                                PERF_COUNT_HW_CACHE_OP_WRITE,
                                PERF_COUNT_HW_CACHE_RESULT_MISS)}) {}

TlbMissCounters::TlbMissCounters(TlbMissCounters &&other) noexcept
    : events_(std::move(other.events_)) {}

TlbMissCounters &
TlbMissCounters::operator=(TlbMissCounters &&other) noexcept {
  events_ = std::move(other.events_);
  return *this;
}

void TlbMissCounters::prepare() noexcept { events_.prepare(); }

void TlbMissCounters::start() noexcept { events_.start(); }

void TlbMissCounters::stop() noexcept { events_.stop(); }

void TlbMissCounters::apply(MemoryMeasurements &measurements) const noexcept {
  const auto &results = events_.results();
  if (results.size() != 2) {
    measurements.data_tlb_read_status = MemoryMetricStatus::not_measured;
    measurements.data_tlb_write_status = MemoryMetricStatus::not_measured;
    return;
  }
  measurements.data_tlb_read_misses = results[0].value;
  measurements.data_tlb_read_status = memory_status(results[0].status);
  measurements.data_tlb_write_misses = results[1].value;
  measurements.data_tlb_write_status = memory_status(results[1].status);
}

} // namespace linux_metrics
} // namespace ntq
