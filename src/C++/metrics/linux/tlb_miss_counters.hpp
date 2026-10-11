#pragma once

#include "metrics/time_measurement.hpp"

namespace ntq {
namespace linux_metrics {

MemoryMetricStatus status_from_perf_errno(int error);

class TlbMissCounters {
public:
  TlbMissCounters() = default;
  ~TlbMissCounters();

  TlbMissCounters(const TlbMissCounters &) = delete;
  TlbMissCounters &operator=(const TlbMissCounters &) = delete;
  TlbMissCounters(TlbMissCounters &&other) noexcept;
  TlbMissCounters &operator=(TlbMissCounters &&other) noexcept;

  void start() noexcept;
  void stop() noexcept;
  void apply(MemoryMeasurements &measurements) const noexcept;

private:
  struct EventCounter {
    int descriptor{-1};
    std::uint64_t value{0};
    MemoryMetricStatus status{MemoryMetricStatus::not_measured};
  };

  static void close_event(EventCounter &event) noexcept;
  static void start_event(EventCounter &event, unsigned operation) noexcept;
  static void stop_event(EventCounter &event) noexcept;

  EventCounter read_misses_{};
  EventCounter write_misses_{};
};

} // namespace linux_metrics
} // namespace ntq
