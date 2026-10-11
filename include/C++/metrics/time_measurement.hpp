#pragma once

#include "metrics/hardware_measurements.hpp"

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>

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

// Regla concreta que activó la marca de sospecha de una muestra.
enum class SuspicionReason {
  none,
  involuntary_context_switches_exceeded
};

// Estructura que representa las mediciones de tiempo transcurrido.
struct ElapsedMeasurements {
  double wall_clock{0.0};
  double cpu_user{0.0};
  double cpu_system{0.0};
  long voluntary_context_switches{0};
  long involuntary_context_switches{0};
  bool suspicious{false};
  long involuntary_context_switch_threshold{0};
  SuspicionReason suspicious_reason{SuspicionReason::none};
};

enum class MemoryMetricStatus {
  available,
  unsupported,
  permission_denied,
  read_error,
  not_measured
};

enum class PeakRssScope { process_lifetime, isolated_measurement_process };

struct MemoryMeasurements {
  std::uint64_t peak_rss_bytes{0}; // ru_maxrss de Linux convertido a bytes.
  std::uint64_t minor_page_faults{0}; // Delta de ru_minflt.
  std::uint64_t major_page_faults{0}; // Delta de ru_majflt.
  std::uint64_t data_tlb_read_misses{0}; // Fallos DTLB en lecturas.
  std::uint64_t data_tlb_write_misses{0}; // Fallos DTLB en escrituras.
  MemoryMetricStatus peak_rss_status{MemoryMetricStatus::not_measured};
  MemoryMetricStatus data_tlb_read_status{MemoryMetricStatus::not_measured};
  MemoryMetricStatus data_tlb_write_status{MemoryMetricStatus::not_measured};
  PeakRssScope peak_rss_scope{PeakRssScope::process_lifetime};
};

namespace linux_metrics {
class HardwareCounters;
class TlbMissCounters;
} // namespace linux_metrics

// Clase que permite medir el tiempo transcurrido y las métricas de recursos.
class TimeMeasurement {
public:
  // El umbral es el máximo de cambios involuntarios permitidos antes de marcar
  // el resultado como sospechoso. El valor predeterminado estricto es cero.
  explicit TimeMeasurement(long max_involuntary_context_switches = 0);
  ~TimeMeasurement();

  TimeMeasurement(const TimeMeasurement &) = delete;
  TimeMeasurement &operator=(const TimeMeasurement &) = delete;
  TimeMeasurement(TimeMeasurement &&other) noexcept;
  TimeMeasurement &operator=(TimeMeasurement &&other) noexcept;

  // Devuelve la resolución reportada por Linux para CLOCK_MONOTONIC, en
  // nanosegundos. El mismo reloj se usa para medir el wall-clock.
  static std::chrono::nanoseconds reported_clock_resolution();

  // Ejecuta el algoritmo en un proceso hijo y devuelve sus métricas. El
  // lanzador debe ser monohilo al invocar este método; los cambios en memoria
  // hechos por el hijo no se reflejan en el proceso padre.
  static TimeMeasurement measure_in_isolated_process(
      const std::function<void()> &algorithm,
      long max_involuntary_context_switches = 0);

  void start();
  void stop();
  void elapsed();
  ElapsedMeasurements obtain_elapsed(TimeUnit unit) const;
  MemoryMeasurements obtain_memory() const;
  HardwareMeasurements obtain_hardware() const;

private:
  // Estructura que representa una instantánea de los recursos del sistema.
  struct ResourceSnapshot {
    long user_seconds{0};
    long user_microseconds{0};
    long system_seconds{0};
    long system_microseconds{0};
    long voluntary_context_switches{0};
    long involuntary_context_switches{0};
    long peak_rss_kib{0};
    long minor_page_faults{0};
    long major_page_faults{0};
  };

  // Funcion que captura un snapshot de los recursos del sistema, incluyendo el
  // tiempo de CPU y los cambios de contexto.
  static ResourceSnapshot capture_resources();

  std::chrono::nanoseconds wall_start_{};
  std::chrono::nanoseconds wall_end_{};
  ResourceSnapshot resources_start_{};
  ResourceSnapshot resources_end_{};
  ElapsedMeasurements elapsed_nanoseconds_{};
  MemoryMeasurements memory_measurements_{};
  HardwareMeasurements hardware_measurements_{};
  std::unique_ptr<linux_metrics::HardwareCounters> hardware_counters_;
  std::unique_ptr<linux_metrics::TlbMissCounters> tlb_counters_;
  long max_involuntary_context_switches_{0};
  bool is_running_{false};
  bool has_stopped_{false};
  bool has_elapsed_{false};
};

} // namespace ntq
