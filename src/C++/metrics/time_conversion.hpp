#pragma once

#include "metrics/time_measurement.hpp"

namespace ntq {
namespace metrics_detail {

long double cpu_time_delta_nanoseconds(long seconds, long microseconds);
double nanoseconds_per_unit(TimeUnit unit);

} // namespace metrics_detail
} // namespace ntq
