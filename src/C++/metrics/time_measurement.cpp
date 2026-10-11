#include "metrics/time_measurement.hpp"

#include <cerrno>
#include <cstddef>
#include <stdexcept>
#include <sys/resource.h>
#include <sys/wait.h>
#include <system_error>
#include <time.h>
#include <type_traits>
#include <unistd.h>

namespace ntq {
namespace {

std::chrono::nanoseconds monotonic_now() {
  timespec timestamp{};
  if (clock_gettime(CLOCK_MONOTONIC, &timestamp) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "No fue posible leer CLOCK_MONOTONIC");
  }

  return std::chrono::seconds{timestamp.tv_sec} +
         std::chrono::nanoseconds{timestamp.tv_nsec};
}

bool write_all(int descriptor, const void *data, std::size_t size) {
  const auto *bytes = static_cast<const char *>(data);
  std::size_t written = 0;
  while (written < size) {
    const ssize_t result = write(descriptor, bytes + written, size - written);
    if (result == -1) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
    if (result == 0) {
      return false;
    }
    written += static_cast<std::size_t>(result);
  }
  return true;
}

bool read_all(int descriptor, void *data, std::size_t size, int &error) {
  auto *bytes = static_cast<char *>(data);
  std::size_t received = 0;
  while (received < size) {
    const ssize_t result = read(descriptor, bytes + received, size - received);
    if (result == -1) {
      if (errno == EINTR) {
        continue;
      }
      error = errno;
      return false;
    }
    if (result == 0) {
      error = 0;
      return false;
    }
    received += static_cast<std::size_t>(result);
  }
  return true;
}

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

// Consulta la resolución reportada para el reloj monotónico de Linux.
std::chrono::nanoseconds TimeMeasurement::reported_clock_resolution() {
  timespec resolution{};
  if (clock_getres(CLOCK_MONOTONIC, &resolution) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "No fue posible consultar la resolución del reloj");
  }

  return std::chrono::seconds{resolution.tv_sec} +
         std::chrono::nanoseconds{resolution.tv_nsec};
}

TimeMeasurement::TimeMeasurement(long max_involuntary_context_switches)
    : max_involuntary_context_switches_(max_involuntary_context_switches) {
  if (max_involuntary_context_switches_ < 0) {
    throw std::invalid_argument(
        "El máximo de cambios involuntarios no puede ser negativo");
  }
}

TimeMeasurement TimeMeasurement::measure_in_isolated_process(
    const std::function<void()> &algorithm,
    long max_involuntary_context_switches) {
  if (!algorithm) {
    throw std::invalid_argument("El algoritmo a medir no puede estar vacío");
  }
  if (max_involuntary_context_switches < 0) {
    throw std::invalid_argument(
        "El máximo de cambios involuntarios no puede ser negativo");
  }

  int result_pipe[2];
  if (pipe(result_pipe) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "No fue posible crear el canal de resultados");
  }

  const pid_t child = fork();
  if (child == -1) {
    const int error = errno;
    close(result_pipe[0]);
    close(result_pipe[1]);
    throw std::system_error(error, std::generic_category(),
                            "No fue posible crear el proceso de medición");
  }

  if (child == 0) {
    close(result_pipe[0]);
    try {
      TimeMeasurement measurement(max_involuntary_context_switches);
      measurement.start();
      algorithm();
      measurement.stop();
      measurement.elapsed();
      const ElapsedMeasurements result = measurement.elapsed_nanoseconds_;
      const bool sent = write_all(result_pipe[1], &result, sizeof(result));
      close(result_pipe[1]);
      _exit(sent ? 0 : 125);
    } catch (...) {
      close(result_pipe[1]);
      _exit(126);
    }
  }

  close(result_pipe[1]);
  ElapsedMeasurements elapsed_nanoseconds{};
  int read_error = 0;
  const bool received = read_all(result_pipe[0], &elapsed_nanoseconds,
                                 sizeof(elapsed_nanoseconds), read_error);
  close(result_pipe[0]);

  int child_status = 0;
  pid_t waited;
  do {
    waited = waitpid(child, &child_status, 0);
  } while (waited == -1 && errno == EINTR);
  if (waited == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "No fue posible esperar al proceso de medición");
  }

  if (!received) {
    if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == 125) {
      throw std::system_error(EPIPE, std::generic_category(),
                              "No fue posible enviar los resultados medidos");
    }
    if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) {
      throw std::runtime_error(
          "El algoritmo o la medición falló en el proceso aislado");
    }
    throw std::system_error(read_error == 0 ? EIO : read_error,
                            std::generic_category(),
                            "No fue posible recibir los resultados medidos");
  }
  if (!WIFEXITED(child_status) || WEXITSTATUS(child_status) != 0) {
    throw std::runtime_error(
        "El proceso aislado terminó con error después de medir");
  }

  TimeMeasurement measurement(max_involuntary_context_switches);
  measurement.elapsed_nanoseconds_ = elapsed_nanoseconds;
  measurement.has_stopped_ = true;
  measurement.has_elapsed_ = true;
  return measurement;
}

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

  const std::chrono::nanoseconds wall_before = monotonic_now();
  const ResourceSnapshot resources = capture_resources();
  const std::chrono::nanoseconds wall_after = monotonic_now();

  resources_start_ = resources;
  wall_start_ = wall_before + (wall_after - wall_before) / 2;
  is_running_ = true;
}

// Detiene la medicion de tiempo y captura los recursos finales del proceso.
void TimeMeasurement::stop() {
  if (!is_running_) {
    throw std::logic_error(
        "No se puede detener la medición de tiempo antes de iniciarla");
  }

  // Se usa el punto medio del intervalo que rodea el snapshot para aproximar
  // mejor el instante en que se capturaron los recursos.
  const std::chrono::nanoseconds wall_before = monotonic_now();
  const ResourceSnapshot resources = capture_resources();
  const std::chrono::nanoseconds wall_after = monotonic_now();

  resources_end_ = resources;
  wall_end_ = wall_before + (wall_after - wall_before) / 2;
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

  const auto wall_duration = wall_end_ - wall_start_;

  const long user_seconds =
      resources_end_.user_seconds - resources_start_.user_seconds;
  const long user_microseconds =
      resources_end_.user_microseconds - resources_start_.user_microseconds;
  const long system_seconds =
      resources_end_.system_seconds - resources_start_.system_seconds;
  const long system_microseconds =
      resources_end_.system_microseconds - resources_start_.system_microseconds;

  const long involuntary_context_switches =
      resources_end_.involuntary_context_switches -
      resources_start_.involuntary_context_switches;

  elapsed_nanoseconds_ = {
      static_cast<double>(wall_duration.count()),
      static_cast<double>(to_nanoseconds(user_seconds, user_microseconds)),
      static_cast<double>(to_nanoseconds(system_seconds, system_microseconds)),
      resources_end_.voluntary_context_switches -
          resources_start_.voluntary_context_switches,
      involuntary_context_switches,
      involuntary_context_switches > max_involuntary_context_switches_,
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
      elapsed_nanoseconds_.suspicious,
  };
}

} // namespace ntq
