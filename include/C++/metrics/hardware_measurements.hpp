#pragma once

#include <cstdint>

namespace ntq {

enum class HardwareMetricStatus {
  available,
  unsupported,
  permission_denied,
  read_error,
  not_measured,
  invalid_denominator
};

struct HardwareMeasurements {
  std::uint64_t cpu_cycles{0};
  std::uint64_t instructions_retired{0};
  double ipc{0.0};
  HardwareMetricStatus cpu_cycles_status{HardwareMetricStatus::not_measured};
  HardwareMetricStatus instructions_retired_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus ipc_status{HardwareMetricStatus::not_measured};

  std::uint64_t l1d_read_accesses{0};
  std::uint64_t l1d_read_misses{0};
  double l1d_read_miss_percent{0.0};
  HardwareMetricStatus l1d_read_accesses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus l1d_read_misses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus l1d_read_miss_percent_status{
      HardwareMetricStatus::not_measured};

  std::uint64_t llc_read_accesses{0};
  std::uint64_t llc_read_misses{0};
  double llc_read_miss_percent{0.0};
  HardwareMetricStatus llc_read_accesses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus llc_read_misses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus llc_read_miss_percent_status{
      HardwareMetricStatus::not_measured};

  std::uint64_t branch_mispredictions{0};
  HardwareMetricStatus branch_mispredictions_status{
      HardwareMetricStatus::not_measured};
};

} // namespace ntq
