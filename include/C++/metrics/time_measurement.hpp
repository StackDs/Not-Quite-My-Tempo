#pragma once

#include <chrono>

namespace ntq {

// Enum que representa las unidades de tiempo que se pueden utilizar para medir
// el tiempo transcurrido.
enum class TimeUnit {
  nanoseconds,
  microseconds,
  milliseconds,
  seconds,
  minutes,
  hours
};

// Estructura que representa las mediciones de tiempo transcurrido.
struct ElapsedMeasurements {
  double wall_clock{0.0};
  double cpu_user{0.0};
  double cpu_system{0.0};
  long voluntary_context_switches{0};
  long involuntary_context_switches{0};
};

// Clase que permite medir el tiempo transcurrido y las métricas de recursos.
class TimeMeasurement {
public:
  void start();
  void stop();
  void elapsed();
  ElapsedMeasurements obtain_elapsed(TimeUnit unit) const;

  /*Se usa un reloj monotonico para medir el tiempo transcurrido, lo que
  garantiza que las mediciones no se vean afectadas por cambios en el reloj
  del sistema */
private:
  using Clock = std::chrono::steady_clock;

  // Estructura que representa una instantánea de los recursos del sistema.
  struct ResourceSnapshot {
    long user_seconds{0};
    long user_microseconds{0};
    long system_seconds{0};
    long system_microseconds{0};
    long voluntary_context_switches{0};
    long involuntary_context_switches{0};
  };

  // Funcion que captura un snapshot de los recursos del sistema, incluyendo el
  // tiempo de CPU y los cambios de contexto.
  static ResourceSnapshot capture_resources();

  Clock::time_point wall_start_{};
  Clock::time_point wall_end_{};
  ResourceSnapshot resources_start_{};
  ResourceSnapshot resources_end_{};
  ElapsedMeasurements elapsed_nanoseconds_{};
  bool is_running_{false};
  bool has_stopped_{false};
  bool has_elapsed_{false};
};

} // namespace ntq
