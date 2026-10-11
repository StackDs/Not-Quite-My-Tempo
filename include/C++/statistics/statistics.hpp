#pragma once

#include "metrics/hardware_measurements.hpp"
#include "metrics/time_measurement.hpp"
#include "system/system_info.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ntq {
namespace statistics {

// Estado global de un resumen o de un cálculo derivado.
enum class StatisticsStatus {
  available,
  insufficient_samples,
  low_reliability,
  invalid_input,
  invalid_denominator,
  unavailable
};

// Estado de la estimación de un intervalo de confianza.
enum class ConfidenceIntervalStatus {
  available,
  low_sample_count,
  insufficient_samples
};

enum class ConfidenceIntervalMethod { percentile_bootstrap };

// Indica si el tamaño de muestra respalda la interpretación de una cola.
enum class TailReliabilityStatus { reliable, low_sample_count, unavailable };

// Identificador estable de cada campo numérico que se puede resumir.
enum class MetricId {
  wall_clock,
  cpu_user,
  cpu_system,
  voluntary_context_switches,
  involuntary_context_switches,
  peak_rss_bytes,
  minor_page_faults,
  major_page_faults,
  data_tlb_read_misses,
  data_tlb_write_misses,
  cpu_cycles,
  instructions_retired,
  ipc,
  l1d_read_accesses,
  l1d_read_misses,
  l1d_read_miss_percent,
  llc_read_accesses,
  llc_read_misses,
  llc_read_miss_percent,
  branch_mispredictions
};

// Disponibilidad de un valor de medición; un estado no disponible no se trata
// como un valor numérico cero.
enum class MetricValueStatus {
  available,
  unsupported,
  permission_denied,
  read_error,
  not_measured,
  invalid_denominator,
  invalid_value
};

// Motivo por el que una observación no entra en los resúmenes finales.
enum class SampleExclusionReason {
  warmup,
  suspicious_context_switches,
  iqr_outlier,
  invalid_wall_clock
};

// Política de emparejamiento para comparar dos variantes.
enum class PairingPolicy { automatic, require_paired, unpaired };
enum class PairingUsed { paired, unpaired, unavailable };

// Una observación medida. group_key es opaca para el módulo; repetition_index
// determina qué ejecuciones se apartan como warm-up y pair_id permite comparar
// ejecuciones equivalentes de variantes distintas.
struct BenchmarkSample {
  std::string sample_id; // Identificador único para auditar inclusiones/exclusiones.
  // Clave opaca definida por quien llama; puede representar parámetros, índices,
  // tamaño u otra dimensión de agrupación.
  std::string group_key;
  std::string variant_key; // Algoritmo o versión dentro del grupo.
  std::string pair_id; // Empareja ejecuciones equivalentes entre variantes.
  std::size_t repetition_index{0}; // Orden usado para separar warm-ups.
  TimeUnit time_unit{TimeUnit::nanoseconds}; // Unidad de los tres tiempos.
  ElapsedMeasurements elapsed; // Tiempo, CPU y cambios de contexto.
  MemoryMeasurements memory; // RSS, page faults y DTLB.
  HardwareMeasurements hardware; // PMU: IPC, cachés y branch misses.
};

// Datos ambientales tomados en un instante. Las ausencias conservan su estado
// explícito en CollectedValue.
struct EnvironmentSnapshot {
  SystemInfo system; // Afinidad, gobernador/frecuencia y datos del equipo.
  CollectedValue<std::uint64_t> runnable_processes;
  CollectedValue<double> cpu_temperature_celsius;
  CollectedValue<bool> thermal_throttling;
};

// Contexto del entorno al principio y al final de la corrida; se informa pero no
// se usa para corregir las mediciones.
struct MeasurementContext {
  EnvironmentSnapshot before; // Estado observado al comenzar la corrida.
  EnvironmentSnapshot after; // Estado observado al terminarla.
};

// Metadatos comunes a la corrida. Los parámetros se proporcionan como pares
// nombre/valor; las flags exactas de compilación quedan fuera de esta API.
struct RunMetadata {
  std::string run_id;
  std::size_t configured_iterations{0}; // Repeticiones solicitadas por quien llama.
  std::chrono::nanoseconds total_duration{}; // Duración global de la corrida.
  std::string clock_name{"CLOCK_MONOTONIC"};
  std::chrono::nanoseconds clock_resolution{};
  std::string library_version;
  std::vector<std::pair<std::string, std::string>> algorithm_parameters;
  MeasurementContext environment;
};

// Solicitud explícita para comparar dos variantes dentro de un mismo grupo.
struct ComparisonRequest {
  std::string group_key;
  std::string baseline_variant;
  std::string candidate_variant;
  PairingPolicy pairing{PairingPolicy::automatic};
};

// Políticas configurables de selección, fiabilidad y comparación.
struct StatisticsOptions {
  std::size_t warmup_runs_per_group{0}; // Primeras repeticiones de cada grupo.
  bool apply_iqr_filter{true}; // Excluye el extremo superior Q3 + 1.5 * IQR.
  bool include_suspicious_samples{true}; // Quien llama decide si excluirlas.
  double confidence_level{0.95}; // Nivel de los intervalos bootstrap.
  std::size_t bootstrap_resamples{10000}; // Réplicas usadas para cada intervalo.
  std::uint64_t bootstrap_seed{42}; // Semilla fija para reproducir los CIs.
  std::size_t minimum_reliable_ci_samples{30}; // Umbral de baja confiabilidad.
  long double minimum_expected_tail_observations{5.0L}; // Revisión de colas.
  std::vector<ComparisonRequest> comparisons;
};

// Intervalo bootstrap-percentile junto con los parámetros para reproducirlo.
struct ConfidenceInterval {
  long double lower{0.0L}; // Extremos del intervalo bootstrap-percentile.
  long double upper{0.0L};
  double confidence_level{0.95};
  std::size_t resamples{0};
  std::uint64_t seed{0};
  ConfidenceIntervalMethod method{
      ConfidenceIntervalMethod::percentile_bootstrap};
  ConfidenceIntervalStatus status{
      ConfidenceIntervalStatus::insufficient_samples};
};

// Medidas descriptivas de una métrica sobre un conjunto concreto de muestras.
struct DescriptiveStatistics {
  std::size_t sample_count{0}; // Valores disponibles en esta vista.
  StatisticsStatus status{StatisticsStatus::unavailable};
  long double minimum{0.0L};
  long double maximum{0.0L};
  long double arithmetic_mean{0.0L};
  long double median{0.0L};
  long double median_absolute_deviation{0.0L}; // MAD sin escalar.
  long double sample_standard_deviation{0.0L}; // Denominador n-1.
  StatisticsStatus sample_standard_deviation_status{
      StatisticsStatus::insufficient_samples};
  long double p90{0.0L};
  long double p95{0.0L};
  long double p99{0.0L};
  long double p999{0.0L};
  TailReliabilityStatus p90_reliability{TailReliabilityStatus::unavailable};
  TailReliabilityStatus p95_reliability{TailReliabilityStatus::unavailable};
  TailReliabilityStatus p99_reliability{TailReliabilityStatus::unavailable};
  TailReliabilityStatus p999_reliability{TailReliabilityStatus::unavailable};
  ConfidenceInterval mean_confidence_interval;
  ConfidenceInterval median_confidence_interval;
};

// Cobertura de estados para una métrica antes de resumir solo valores válidos.
struct MetricAvailabilityCounts {
  // Conteo por estado tras warm-up; incluye las sospechosas aunque se excluyan.
  std::size_t available{0};
  std::size_t unsupported{0};
  std::size_t permission_denied{0};
  std::size_t read_error{0};
  std::size_t not_measured{0};
  std::size_t invalid_denominator{0};
  std::size_t invalid_value{0};
};

// Estadísticas de una métrica antes/después de IQR y cobertura de disponibilidad.
struct MetricSummary {
  MetricId metric{MetricId::wall_clock};
  // Cubre las muestras posteriores al warm-up, incluso las sospechosas que luego
  // se excluyan por política.
  MetricAvailabilityCounts availability;
  // before_iqr respeta la política de sospechosas; after_iqr también aplica IQR.
  DescriptiveStatistics before_iqr;
  DescriptiveStatistics after_iqr;
};

// Auditoría por muestra: conserva flags y cada regla que la excluyó del resumen.
struct SampleAudit {
  std::string sample_id;
  std::string group_key;
  std::string variant_key;
  bool suspicious{false};
  long involuntary_context_switches{0};
  long involuntary_context_switch_threshold{0};
  SuspicionReason suspicious_reason{SuspicionReason::none};
  std::vector<SampleExclusionReason> exclusion_reasons;
  bool included_after_filters{true};
};

// Resumen de una clave opaca de agrupación y una variante; no presupone que la
// clave represente el tamaño de entrada.
struct GroupStatistics {
  std::string group_key;
  std::string variant_key;
  TimeUnit time_unit{TimeUnit::nanoseconds};
  std::size_t input_sample_count{0};
  std::size_t warmup_sample_count{0};
  std::size_t suspicious_sample_count{0};
  std::size_t iqr_outlier_count{0};
  std::vector<MetricSummary> metrics;
  std::vector<SampleAudit> sample_audit;
};

// Diferencia absoluta y relativa de un estimador entre dos variantes.
struct EffectEstimate {
  long double absolute_difference{0.0L}; // Candidato menos base.
  long double relative_difference_percent{0.0L};
  std::size_t relative_sample_count{0};
  StatisticsStatus relative_status{StatisticsStatus::unavailable};
  ConfidenceInterval absolute_confidence_interval;
  ConfidenceInterval relative_confidence_interval;
  StatisticsStatus status{StatisticsStatus::unavailable};
};

// Tamaño de efecto y cobertura de una métrica comparada.
struct MetricComparison {
  MetricId metric{MetricId::wall_clock};
  EffectEstimate mean_effect;
  EffectEstimate median_effect;
  std::size_t baseline_samples{0};
  std::size_t candidate_samples{0};
  std::size_t matched_pairs{0};
  PairingUsed pairing_used{PairingUsed::unavailable};
};

// Resultado de comparar una variante base con una candidata.
struct GroupComparison {
  std::string group_key;
  std::string baseline_variant;
  std::string candidate_variant;
  std::vector<MetricComparison> metrics;
};

// Cambios observables del entorno entre los dos snapshots de la corrida.
struct EnvironmentDiagnostics {
  // Los deltas/flags son indicadores informativos; no alteran las métricas.
  CollectedValue<std::int64_t> runnable_process_delta;
  CollectedValue<double> cpu_temperature_delta_celsius;
  CollectedValue<bool> cpu_affinity_changed;
  CollectedValue<bool> cpu_governor_or_frequency_changed;
  CollectedValue<bool> thermal_throttling_observed;
};

// Reporte completo: contexto, resúmenes, comparaciones y auditoría.
struct StatisticsReport {
  RunMetadata metadata;
  StatisticsOptions options;
  EnvironmentDiagnostics environment_diagnostics;
  std::vector<GroupStatistics> groups;
  std::vector<GroupComparison> comparisons;
  std::vector<SampleAudit> sample_audit;
};

// Agrega mediciones existentes; no ejecuta el algoritmo ni modifica las muestras.
// Lanza std::invalid_argument ante opciones inválidas, IDs repetidos o unidades
// de tiempo mezcladas dentro de un mismo grupo.
StatisticsReport
calculate_statistics(const std::vector<BenchmarkSample> &samples,
                     const RunMetadata &metadata,
                     const StatisticsOptions &options);

} // namespace statistics
} // namespace ntq
