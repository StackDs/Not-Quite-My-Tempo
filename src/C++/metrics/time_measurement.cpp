#include "metrics/time_measurement.hpp"

#include <cerrno>
#include <stdexcept>
#include <sys/resource.h>
#include <system_error>

namespace ntq {
namespace {

// Funcion auxliar para convertir segundos y microsegundos a nanosegundos
long double to_nanoseconds(long seconds, long microseconds) {
  return (static_cast<long double>(seconds) * 1'000'000.0L +
          static_cast<long double>(microseconds)) *
         1'000.0L;
}

// Funcion de conversión de unidades de tiempo
double nanoseconds_per_unit(TimeUnit unit) {
  switch (unit) {
  case TimeUnit::nanoseconds:
    return 1.0;
  case TimeUnit::microseconds:
    return 1'000.0;
  case TimeUnit::milliseconds:
    return 1'000'000.0;
  case TimeUnit::seconds:
    return 1'000'000'000.0;
  case TimeUnit::minutes:
    return 60'000'000'000.0;
  case TimeUnit::hours:
    return 3'600'000'000'000.0;
  }
  throw std::invalid_argument("Unidad de tiempo no válida");
}

} // namespace

// Capturamos los recursos del proceso utilizando getrusage.
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
  };
}

// Inicia la medicion de tiempo y captura los recursos iniciales del proceso.
void TimeMeasurement::start() {
  is_running_ = false;
  has_stopped_ = false;
  has_elapsed_ = false;

  const ResourceSnapshot resources = capture_resources();
  const Clock::time_point wall_time = Clock::now();

  resources_start_ = resources;
  wall_start_ = wall_time;
  is_running_ = true;
}

// Detiene la medicion de tiempo y captura los recursos finales del proceso.
void TimeMeasurement::stop() {
  if (!is_running_) {
    throw std::logic_error(
        "No se puede detener la medición de tiempo antes de iniciarla");
  }

  // Capturamos los recursos finales y el tiempo final
  const ResourceSnapshot resources = capture_resources();
  const Clock::time_point wall_time = Clock::now();

  resources_end_ = resources;
  wall_end_ = wall_time;
  is_running_ = false;
  has_stopped_ = true;
  has_elapsed_ = false;
}

// Calcula el tiempo transcurrido y las métricas de recursos.
void TimeMeasurement::elapsed() {
  if (is_running_ || !has_stopped_) {
    throw std::logic_error("No se puede calcular el tiempo transcurrido antes "
                           "de detener la medición de tiempo");
  }

  const auto wall_duration =
      std::chrono::duration<long double, std::nano>(wall_end_ - wall_start_);

  const long user_seconds =
      resources_end_.user_seconds - resources_start_.user_seconds;
  const long user_microseconds =
      resources_end_.user_microseconds - resources_start_.user_microseconds;
  const long system_seconds =
      resources_end_.system_seconds - resources_start_.system_seconds;
  const long system_microseconds =
      resources_end_.system_microseconds - resources_start_.system_microseconds;

  elapsed_nanoseconds_ = {
      static_cast<double>(wall_duration.count()),
      static_cast<double>(to_nanoseconds(user_seconds, user_microseconds)),
      static_cast<double>(to_nanoseconds(system_seconds, system_microseconds)),
      resources_end_.voluntary_context_switches -
          resources_start_.voluntary_context_switches,
      resources_end_.involuntary_context_switches -
          resources_start_.involuntary_context_switches,
  };
  has_elapsed_ = true;
}

ElapsedMeasurements TimeMeasurement::obtain_elapsed(TimeUnit unit) const {
  if (!has_elapsed_) {
    throw std::logic_error(
        "No se puede obtener el tiempo transcurrido antes de calcularlo");
  }

  // Calculamos el divisor para convertir los nanosegundos a la unidad deseada
  const double divisor = nanoseconds_per_unit(unit);

  // Devolvemos la estructura con las mediciones
  return {
      elapsed_nanoseconds_.wall_clock / divisor,
      elapsed_nanoseconds_.cpu_user / divisor,
      elapsed_nanoseconds_.cpu_system / divisor,
      elapsed_nanoseconds_.voluntary_context_switches,
      elapsed_nanoseconds_.involuntary_context_switches,
  };
}

} // namespace ntq
