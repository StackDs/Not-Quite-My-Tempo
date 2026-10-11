#include "metrics/time_measurement.hpp"

#include "linux/hardware_counters.hpp"
#include "linux/hardware_measurement_calculations.hpp"
#include "linux/memory_helpers.hpp"
#include "linux/monotonic_clock.hpp"
#include "linux/tlb_miss_counters.hpp"
#include "time_conversion.hpp"

#include <stdexcept>
#include <utility>

namespace ntq {

TimeMeasurement::TimeMeasurement(long max_involuntary_context_switches)
    : max_involuntary_context_switches_(max_involuntary_context_switches) {
  // Un umbral negativo no tiene significado como máximo permitido.
  if (max_involuntary_context_switches_ < 0) {
    throw std::invalid_argument(
        "El máximo de cambios involuntarios no puede ser negativo");
  }
}

TimeMeasurement::~TimeMeasurement() = default;

TimeMeasurement::TimeMeasurement(TimeMeasurement &&other) noexcept
    : wall_start_(other.wall_start_), wall_end_(other.wall_end_),
      resources_start_(other.resources_start_),
      resources_end_(other.resources_end_),
      elapsed_nanoseconds_(other.elapsed_nanoseconds_),
      memory_measurements_(other.memory_measurements_),
      hardware_measurements_(other.hardware_measurements_),
      hardware_counters_(std::move(other.hardware_counters_)),
      tlb_counters_(std::move(other.tlb_counters_)),
      max_involuntary_context_switches_(
          other.max_involuntary_context_switches_),
      is_running_(other.is_running_), has_stopped_(other.has_stopped_),
      has_elapsed_(other.has_elapsed_) {
  // La propiedad de los contadores perf y del estado de medición pasa al destino.
  other.is_running_ = false;
  other.has_stopped_ = false;
  other.has_elapsed_ = false;
}

TimeMeasurement &TimeMeasurement::operator=(TimeMeasurement &&other) noexcept {
  if (this != &other) {
    wall_start_ = other.wall_start_;
    wall_end_ = other.wall_end_;
    resources_start_ = other.resources_start_;
    resources_end_ = other.resources_end_;
    elapsed_nanoseconds_ = other.elapsed_nanoseconds_;
    memory_measurements_ = other.memory_measurements_;
    hardware_measurements_ = other.hardware_measurements_;
    hardware_counters_ = std::move(other.hardware_counters_);
    tlb_counters_ = std::move(other.tlb_counters_);
    max_involuntary_context_switches_ = other.max_involuntary_context_switches_;
    is_running_ = other.is_running_;
    has_stopped_ = other.has_stopped_;
    has_elapsed_ = other.has_elapsed_;

    other.is_running_ = false;
    other.has_stopped_ = false;
    other.has_elapsed_ = false;
  }
  return *this;
}

void TimeMeasurement::start() {
  is_running_ = false;
  has_stopped_ = false;
  has_elapsed_ = false;

  // Abrir los eventos antes del snapshot reduce el costo instrumental dentro de
  // la sección cuyos recursos se comparan.
  memory_measurements_ = MemoryMeasurements{};
  hardware_measurements_ = HardwareMeasurements{};
  tlb_counters_.reset(new linux_metrics::TlbMissCounters());
  hardware_counters_.reset(new linux_metrics::HardwareCounters());
  tlb_counters_->prepare();
  hardware_counters_->prepare();

  const std::chrono::nanoseconds wall_before =
      linux_metrics::detail::monotonic_now();
  const ResourceSnapshot resources = capture_resources();
  tlb_counters_->start();
  hardware_counters_->start();
  const std::chrono::nanoseconds wall_after =
      linux_metrics::detail::monotonic_now();

  resources_start_ = resources;
  wall_start_ = wall_before + (wall_after - wall_before) / 2;
  is_running_ = true;
}

void TimeMeasurement::stop() {
  if (!is_running_) {
    throw std::logic_error(
        "No se puede detener la medición de tiempo antes de iniciarla");
  }

  // Detiene los contadores de hardware antes de capturar el snapshot final.
  const std::chrono::nanoseconds wall_before =
      linux_metrics::detail::monotonic_now();
  if (tlb_counters_) {
    tlb_counters_->stop();
    tlb_counters_->apply(memory_measurements_);
  }
  if (hardware_counters_) {
    hardware_counters_->stop();
    hardware_counters_->apply(hardware_measurements_);
  }
  const ResourceSnapshot resources = capture_resources();
  const std::chrono::nanoseconds wall_after =
      linux_metrics::detail::monotonic_now();

  resources_end_ = resources;
  wall_end_ = wall_before + (wall_after - wall_before) / 2;
  is_running_ = false;
  has_stopped_ = true;
  has_elapsed_ = false;
}

void TimeMeasurement::elapsed() {
  if (is_running_ || !has_stopped_) {
    throw std::logic_error("No se puede calcular el tiempo transcurrido antes "
                           "de detener la medición de tiempo");
  }

  // Calcula deltas de tiempo, recursos y la regla de sospecha sin alterar los
  // snapshots originales.
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

  if (resources_end_.peak_rss_kib < 0 ||
      !linux_metrics::peak_rss_kib_to_bytes(
          static_cast<std::uint64_t>(resources_end_.peak_rss_kib),
          memory_measurements_.peak_rss_bytes)) {
    memory_measurements_.peak_rss_bytes = 0;
    memory_measurements_.peak_rss_status = MemoryMetricStatus::read_error;
  } else {
    memory_measurements_.peak_rss_status = MemoryMetricStatus::available;
  }
  memory_measurements_.minor_page_faults = static_cast<std::uint64_t>(
      resources_end_.minor_page_faults - resources_start_.minor_page_faults);
  memory_measurements_.major_page_faults = static_cast<std::uint64_t>(
      resources_end_.major_page_faults - resources_start_.major_page_faults);

  const bool suspicious =
      involuntary_context_switches > max_involuntary_context_switches_;
  elapsed_nanoseconds_ = {
      static_cast<double>(wall_duration.count()),
      static_cast<double>(metrics_detail::cpu_time_delta_nanoseconds(
          user_seconds, user_microseconds)),
      static_cast<double>(metrics_detail::cpu_time_delta_nanoseconds(
          system_seconds, system_microseconds)),
      resources_end_.voluntary_context_switches -
          resources_start_.voluntary_context_switches,
      involuntary_context_switches,
      suspicious,
      max_involuntary_context_switches_,
      suspicious ? SuspicionReason::involuntary_context_switches_exceeded
                 : SuspicionReason::none,
  };
  linux_metrics::calculate_hardware_derived_metrics(hardware_measurements_);
  has_elapsed_ = true;
}

ElapsedMeasurements TimeMeasurement::obtain_elapsed(TimeUnit unit) const {
  if (!has_elapsed_) {
    throw std::logic_error(
        "No se puede obtener el tiempo transcurrido antes de calcularlo");
  }

  // La conversión solo afecta los tiempos; los contadores conservan sus unidades.
  const double divisor = metrics_detail::nanoseconds_per_unit(unit);
  return {
      elapsed_nanoseconds_.wall_clock / divisor,
      elapsed_nanoseconds_.cpu_user / divisor,
      elapsed_nanoseconds_.cpu_system / divisor,
      elapsed_nanoseconds_.voluntary_context_switches,
      elapsed_nanoseconds_.involuntary_context_switches,
      elapsed_nanoseconds_.suspicious,
      elapsed_nanoseconds_.involuntary_context_switch_threshold,
      elapsed_nanoseconds_.suspicious_reason,
  };
}

MemoryMeasurements TimeMeasurement::obtain_memory() const {
  if (!has_elapsed_) {
    throw std::logic_error(
        "No se pueden obtener métricas de memoria antes de calcularlas");
  }
  return memory_measurements_;
}

HardwareMeasurements TimeMeasurement::obtain_hardware() const {
  if (!has_elapsed_) {
    throw std::logic_error(
        "No se pueden obtener métricas de hardware antes de calcularlas");
  }
  return hardware_measurements_;
}

} // namespace ntq
