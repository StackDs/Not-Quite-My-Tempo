#include "linux/isolated_process.hpp"

#include "metrics/time_measurement.hpp"

#include <cerrno>
#include <cstddef>
#include <stdexcept>
#include <system_error>
#include <type_traits>
#include <sys/wait.h>
#include <unistd.h>

namespace ntq {
namespace linux_metrics {
namespace detail {
namespace {

static_assert(std::is_trivially_copyable<IsolatedMeasurementPayload>::value,
              "El resultado aislado debe poder transferirse por pipe");

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

} // namespace
} // namespace detail
} // namespace linux_metrics

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
      measurement.memory_measurements_.peak_rss_scope =
          PeakRssScope::isolated_measurement_process;
      const linux_metrics::detail::IsolatedMeasurementPayload result{
          measurement.elapsed_nanoseconds_, measurement.memory_measurements_};
      const bool sent = linux_metrics::detail::write_all(
          result_pipe[1], &result, sizeof(result));
      close(result_pipe[1]);
      _exit(sent ? 0 : 125);
    } catch (...) {
      close(result_pipe[1]);
      _exit(126);
    }
  }

  close(result_pipe[1]);
  linux_metrics::detail::IsolatedMeasurementPayload result{};
  int read_error = 0;
  const bool received = linux_metrics::detail::read_all(
      result_pipe[0], &result, sizeof(result), read_error);
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
  measurement.elapsed_nanoseconds_ = result.elapsed;
  measurement.memory_measurements_ = result.memory;
  measurement.has_stopped_ = true;
  measurement.has_elapsed_ = true;
  return measurement;
}

} // namespace ntq
