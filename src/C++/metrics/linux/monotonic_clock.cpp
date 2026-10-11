#include "linux/monotonic_clock.hpp"

#include "metrics/time_measurement.hpp"

#include <cerrno>
#include <system_error>
#include <time.h>

namespace ntq {
namespace linux_metrics {
namespace detail {

// Lee el mismo reloj monotónico que se usa para consultar su resolución.
std::chrono::nanoseconds monotonic_now() {
  timespec timestamp{};
  if (clock_gettime(CLOCK_MONOTONIC, &timestamp) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "No fue posible leer CLOCK_MONOTONIC");
  }
  return std::chrono::seconds{timestamp.tv_sec} +
         std::chrono::nanoseconds{timestamp.tv_nsec};
}

} // namespace detail
} // namespace linux_metrics

// Informa la resolución declarada por el kernel para CLOCK_MONOTONIC.
std::chrono::nanoseconds TimeMeasurement::reported_clock_resolution() {
  timespec resolution{};
  if (clock_getres(CLOCK_MONOTONIC, &resolution) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "No fue posible consultar la resolución del reloj");
  }
  return std::chrono::seconds{resolution.tv_sec} +
         std::chrono::nanoseconds{resolution.tv_nsec};
}

} // namespace ntq
