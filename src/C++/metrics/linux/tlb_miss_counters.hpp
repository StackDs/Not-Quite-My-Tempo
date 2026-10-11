#pragma once

#include "linux/perf_event_group.hpp"
#include "metrics/time_measurement.hpp"

namespace ntq {
namespace linux_metrics {

MemoryMetricStatus status_from_perf_errno(int error);

class TlbMissCounters {
public:
  TlbMissCounters();
  ~TlbMissCounters() = default;

  TlbMissCounters(const TlbMissCounters &) = delete;
  TlbMissCounters &operator=(const TlbMissCounters &) = delete;
  TlbMissCounters(TlbMissCounters &&other) noexcept;
  TlbMissCounters &operator=(TlbMissCounters &&other) noexcept;

  void prepare() noexcept;
  void start() noexcept;
  void stop() noexcept;
  void apply(MemoryMeasurements &measurements) const noexcept;

private:
  PerfEventGroup events_;
};

} // namespace linux_metrics
} // namespace ntq
