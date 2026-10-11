#pragma once

#include "linux/perf_event_group.hpp"
#include "metrics/hardware_measurements.hpp"

namespace ntq {
namespace linux_metrics {

class HardwareCounters {
public:
  HardwareCounters();

  void prepare() noexcept;
  void start() noexcept;
  void stop() noexcept;
  void apply(HardwareMeasurements &measurements) const noexcept;

private:
  PerfEventGroup cycles_and_instructions_;
  PerfEventGroup l1d_reads_;
  PerfEventGroup llc_reads_;
  PerfEventGroup branch_misses_;
};

} // namespace linux_metrics
} // namespace ntq
