#pragma once

#include "statistics/statistics.hpp"

#include <vector>

namespace ntq {
namespace statistics {
namespace detail {

// Valor extraído de una muestra y estado que determina si puede resumirse.
struct MetricObservation {
  long double value{0.0L};
  MetricValueStatus status{MetricValueStatus::available};
};

// Muestra los dos conjuntos usados por el resumen y por las comparaciones.
struct PreparedGroup {
  GroupStatistics summary;
  std::vector<const BenchmarkSample *> coverage_samples;
  std::vector<const BenchmarkSample *> before_iqr;
  std::vector<const BenchmarkSample *> after_iqr;
};

// Devuelve el catálogo de métricas numéricas procesadas por el módulo.
const std::vector<MetricId> &metric_ids();
// Extrae una métrica y traduce sus estados específicos a MetricValueStatus.
MetricObservation observe_metric(const BenchmarkSample &sample, MetricId metric);
// Compara indicadores de entorno sin aplicar correcciones a las mediciones.
EnvironmentDiagnostics diagnose_environment(const MeasurementContext &context);
StatisticsReport calculate_statistics_report(
    const std::vector<BenchmarkSample> &samples, const RunMetadata &metadata,
    const StatisticsOptions &options);
std::vector<GroupComparison> calculate_comparisons(
    const std::vector<PreparedGroup> &groups,
    const std::vector<ComparisonRequest> &requests,
    const StatisticsOptions &options);

} // namespace detail
} // namespace statistics
} // namespace ntq
