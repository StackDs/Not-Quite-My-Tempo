#pragma once

#include <chrono>

namespace ntq {
namespace linux_metrics {
namespace detail {

std::chrono::nanoseconds monotonic_now();

} // namespace detail
} // namespace linux_metrics
} // namespace ntq
