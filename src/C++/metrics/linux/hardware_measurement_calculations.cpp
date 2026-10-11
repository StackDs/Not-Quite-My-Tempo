#include "linux/hardware_measurement_calculations.hpp"

namespace ntq {
namespace linux_metrics {
namespace {

HardwareMetricStatus combined_status(HardwareMetricStatus first,
                                     HardwareMetricStatus second) {
  if (first != HardwareMetricStatus::available) {
    return first;
  }
  return second;
}

} // namespace

void calculate_hardware_derived_metrics(HardwareMeasurements &measurements) {
  // Las razones solo son válidas cuando todos sus contadores están disponibles
  // y el denominador es distinto de cero.
  if (measurements.cpu_cycles_status != HardwareMetricStatus::available ||
      measurements.instructions_retired_status !=
          HardwareMetricStatus::available) {
    measurements.ipc_status = combined_status(
        measurements.cpu_cycles_status,
        measurements.instructions_retired_status);
  } else if (measurements.cpu_cycles == 0) {
    measurements.ipc_status = HardwareMetricStatus::invalid_denominator;
  } else {
    measurements.ipc = static_cast<double>(measurements.instructions_retired) /
                       measurements.cpu_cycles;
    measurements.ipc_status = HardwareMetricStatus::available;
  }

  if (measurements.l1d_read_accesses_status !=
          HardwareMetricStatus::available ||
      measurements.l1d_read_misses_status != HardwareMetricStatus::available) {
    measurements.l1d_read_miss_percent_status = combined_status(
        measurements.l1d_read_accesses_status,
        measurements.l1d_read_misses_status);
  } else if (measurements.l1d_read_accesses == 0) {
    measurements.l1d_read_miss_percent_status =
        HardwareMetricStatus::invalid_denominator;
  } else {
    measurements.l1d_read_miss_percent =
        100.0 * static_cast<double>(measurements.l1d_read_misses) /
        measurements.l1d_read_accesses;
    measurements.l1d_read_miss_percent_status =
        HardwareMetricStatus::available;
  }

  if (measurements.llc_read_accesses_status !=
          HardwareMetricStatus::available ||
      measurements.llc_read_misses_status != HardwareMetricStatus::available) {
    measurements.llc_read_miss_percent_status = combined_status(
        measurements.llc_read_accesses_status,
        measurements.llc_read_misses_status);
  } else if (measurements.llc_read_accesses == 0) {
    measurements.llc_read_miss_percent_status =
        HardwareMetricStatus::invalid_denominator;
  } else {
    measurements.llc_read_miss_percent =
        100.0 * static_cast<double>(measurements.llc_read_misses) /
        measurements.llc_read_accesses;
    measurements.llc_read_miss_percent_status =
        HardwareMetricStatus::available;
  }
}

} // namespace linux_metrics
} // namespace ntq
