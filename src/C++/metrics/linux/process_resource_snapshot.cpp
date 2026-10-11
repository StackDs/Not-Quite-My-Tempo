#include "metrics/time_measurement.hpp"

#include <cerrno>
#include <sys/resource.h>
#include <system_error>

namespace ntq {

// Captura en una sola llamada de getrusage los deltas/contadores del proceso.
TimeMeasurement::ResourceSnapshot TimeMeasurement::capture_resources() {
  rusage usage{};
  if (getrusage(RUSAGE_SELF, &usage) == -1) {
    throw std::system_error(
        errno, std::generic_category(),
        "No fue posible capturar el uso de recursos del proceso");
  }

  return {
      static_cast<long>(usage.ru_utime.tv_sec),
      static_cast<long>(usage.ru_utime.tv_usec),
      static_cast<long>(usage.ru_stime.tv_sec),
      static_cast<long>(usage.ru_stime.tv_usec),
      usage.ru_nvcsw,
      usage.ru_nivcsw,
      static_cast<long>(usage.ru_maxrss),
      usage.ru_minflt,
      usage.ru_majflt,
  };
}

} // namespace ntq
