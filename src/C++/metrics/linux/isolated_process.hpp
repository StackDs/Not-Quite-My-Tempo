#pragma once

#include "metrics/time_measurement.hpp"

namespace ntq {
namespace linux_metrics {
namespace detail {

struct IsolatedMeasurementPayload {
  ElapsedMeasurements elapsed;
  MemoryMeasurements memory;
};

} // namespace detail
} // namespace linux_metrics
} // namespace ntq
