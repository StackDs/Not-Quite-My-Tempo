#pragma once

#include "metrics/hardware_measurements.hpp"

namespace ntq {
namespace linux_metrics {

void calculate_hardware_derived_metrics(HardwareMeasurements &measurements);

} // namespace linux_metrics
} // namespace ntq
