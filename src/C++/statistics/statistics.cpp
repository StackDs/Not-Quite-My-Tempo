#include "statistics/statistics.hpp"

#include "statistics_internal.hpp"
#include "statistics_math.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <utility>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

using GroupKey = std::pair<std::string, std::string>;

// Comprueba que la unidad tiene un divisor conocido antes de agregar tiempos.
bool valid_time_unit(TimeUnit unit) {
  switch (unit) {
  case TimeUnit::nanoseconds:
  case TimeUnit::microseconds:
  case TimeUnit::milliseconds:
  case TimeUnit::seconds:
  case TimeUnit::minutes:
  case TimeUnit::hours:
    return true;
  }
  return false;
}

void validate_options(const StatisticsOptions &options) {
  // Rechaza parámetros que volverían indefinidos los cuantiles o el bootstrap.
  if (!std::isfinite(options.confidence_level) ||
      options.confidence_level <= 0.0 || options.confidence_level >= 1.0) {
    throw std::invalid_argument("El nivel de confianza debe estar entre 0 y 1");
  }
  if (options.bootstrap_resamples == 0) {
    throw std::invalid_argument(
        "El número de remuestreos bootstrap debe ser mayor que cero");
  }
  if (!std::isfinite(options.minimum_expected_tail_observations) ||
      options.minimum_expected_tail_observations < 0.0L) {
    throw std::invalid_argument(
        "El mínimo de observaciones de cola no puede ser negativo");
  }
}

void count_status(MetricAvailabilityCounts &counts,
                  MetricValueStatus status) {
  // Cuenta los estados por separado para que la ausencia nunca parezca un cero.
  switch (status) {
  case MetricValueStatus::available:
    ++counts.available;
    break;
  case MetricValueStatus::unsupported:
    ++counts.unsupported;
    break;
  case MetricValueStatus::permission_denied:
    ++counts.permission_denied;
    break;
  case MetricValueStatus::read_error:
    ++counts.read_error;
    break;
  case MetricValueStatus::not_measured:
    ++counts.not_measured;
    break;
  case MetricValueStatus::invalid_denominator:
    ++counts.invalid_denominator;
    break;
  case MetricValueStatus::invalid_value:
    ++counts.invalid_value;
    break;
  }
}

std::vector<long double> available_values(
    const std::vector<const BenchmarkSample *> &samples, MetricId metric,
    MetricAvailabilityCounts *availability) {
  // Devuelve valores agregables y, si se pide, actualiza su cobertura por estado.
  std::vector<long double> values;
  for (const BenchmarkSample *sample : samples) {
    const MetricObservation observation = observe_metric(*sample, metric);
    if (availability != nullptr) {
      count_status(*availability, observation.status);
    }
    if (observation.status == MetricValueStatus::available) {
      values.push_back(observation.value);
    }
  }
  return values;
}

void add_reason(SampleAudit &audit, SampleExclusionReason reason) {
  // Registra la exclusión sin modificar la muestra original.
  audit.exclusion_reasons.push_back(reason);
  audit.included_after_filters = false;
}

std::string sample_id_or_index(const BenchmarkSample &sample,
                               std::size_t index) {
  // Usa la posición de entrada como ID de auditoría si quien llama no lo indicó.
  return sample.sample_id.empty()
             ? std::string("sample-") + std::to_string(index)
             : sample.sample_id;
}

} // namespace

StatisticsReport calculate_statistics_report(
    const std::vector<BenchmarkSample> &samples, const RunMetadata &metadata,
    const StatisticsOptions &options) {
  validate_options(options);
  StatisticsReport report;
  report.metadata = metadata;
  report.options = options;
  report.environment_diagnostics = diagnose_environment(metadata.environment);
  report.sample_audit.resize(samples.size());
  if (samples.empty()) {
    return report;
  }

  // Cada pareja (group_key, variant_key) es una serie independiente. Se validan
  // claves, unidades e IDs antes de agregar para evitar resultados ambiguos.
  std::map<GroupKey, std::vector<std::size_t>> grouped_indices;
  std::set<std::string> sample_ids;
  std::map<GroupKey, TimeUnit> units_by_group;
  std::map<GroupKey, std::set<std::size_t>> repetition_indices;
  std::map<GroupKey, std::set<std::string>> pair_ids;
  for (std::size_t index = 0; index < samples.size(); ++index) {
    const BenchmarkSample &sample = samples[index];
    const std::string sample_id = sample_id_or_index(sample, index);
    if (!sample_ids.insert(sample_id).second) {
      throw std::invalid_argument("Los sample_id deben ser únicos");
    }
    const GroupKey key(sample.group_key, sample.variant_key);
    if (!valid_time_unit(sample.time_unit)) {
      throw std::invalid_argument("La muestra contiene una TimeUnit inválida");
    }
    const auto unit = units_by_group.find(key);
    if (unit == units_by_group.end()) {
      units_by_group[key] = sample.time_unit;
    } else if (unit->second != sample.time_unit) {
      throw std::invalid_argument(
          "Las muestras de un mismo grupo deben usar la misma TimeUnit");
    }
    if (!repetition_indices[key].insert(sample.repetition_index).second) {
      throw std::invalid_argument(
          "El índice de repetición debe ser único dentro del grupo/variante");
    }
    if (!sample.pair_id.empty() &&
        !pair_ids[key].insert(sample.pair_id).second) {
      throw std::invalid_argument(
          "El pair_id debe ser único dentro del grupo/variante");
    }
    grouped_indices[key].push_back(index);

    SampleAudit &audit = report.sample_audit[index];
    audit.sample_id = sample_id;
    audit.group_key = sample.group_key;
    audit.variant_key = sample.variant_key;
    audit.suspicious = sample.elapsed.suspicious;
    audit.involuntary_context_switches =
        sample.elapsed.involuntary_context_switches;
    audit.involuntary_context_switch_threshold =
        sample.elapsed.involuntary_context_switch_threshold;
    audit.suspicious_reason = sample.elapsed.suspicious_reason;
  }

  for (const ComparisonRequest &request : options.comparisons) {
    const auto baseline =
        units_by_group.find({request.group_key, request.baseline_variant});
    const auto candidate =
        units_by_group.find({request.group_key, request.candidate_variant});
    if (baseline != units_by_group.end() && candidate != units_by_group.end() &&
        baseline->second != candidate->second) {
      throw std::invalid_argument(
          "Los grupos comparados deben usar la misma TimeUnit");
    }
  }

  std::vector<PreparedGroup> prepared_groups;
  prepared_groups.reserve(grouped_indices.size());
  for (auto &entry : grouped_indices) {
    const GroupKey &key = entry.first;
    std::vector<std::size_t> &indices = entry.second;
    std::stable_sort(indices.begin(), indices.end(), [&](std::size_t left,
                                                         std::size_t right) {
      return samples[left].repetition_index < samples[right].repetition_index;
    });

    PreparedGroup prepared;
    prepared.summary.group_key = key.first;
    prepared.summary.variant_key = key.second;
    prepared.summary.time_unit = units_by_group.at(key);
    prepared.summary.input_sample_count = indices.size();

    // Aparta warm-ups por orden y aplica la política configurable de sospecha.
    for (std::size_t position = 0; position < indices.size(); ++position) {
      const std::size_t sample_index = indices[position];
      const BenchmarkSample &sample = samples[sample_index];
      SampleAudit &audit = report.sample_audit[sample_index];
      if (sample.elapsed.suspicious) {
        ++prepared.summary.suspicious_sample_count;
      }

      if (position < options.warmup_runs_per_group) {
        ++prepared.summary.warmup_sample_count;
        add_reason(audit, SampleExclusionReason::warmup);
        continue;
      }
      prepared.coverage_samples.push_back(&sample);
      if (sample.elapsed.suspicious) {
        if (!options.include_suspicious_samples) {
          add_reason(audit,
                     SampleExclusionReason::suspicious_context_switches);
          continue;
        }
      }

      const MetricObservation wall_clock =
          observe_metric(sample, MetricId::wall_clock);
      if (wall_clock.status != MetricValueStatus::available) {
        add_reason(audit, SampleExclusionReason::invalid_wall_clock);
        continue;
      }
      prepared.before_iqr.push_back(&sample);
    }

    // El fence usa wall-clock y, si se supera, se excluye toda la muestra para
    // mantener juntas sus métricas.
    if (options.apply_iqr_filter && !prepared.before_iqr.empty()) {
      const std::vector<long double> wall_clocks = available_values(
          prepared.before_iqr, MetricId::wall_clock, nullptr);
      const long double q1 = quantile(wall_clocks, 0.25L);
      const long double q3 = quantile(wall_clocks, 0.75L);
      const long double upper_fence = q3 + 1.5L * (q3 - q1);
      for (const BenchmarkSample *sample : prepared.before_iqr) {
        const std::size_t sample_index = static_cast<std::size_t>(
            sample - samples.data());
        if (static_cast<long double>(sample->elapsed.wall_clock) >
            upper_fence) {
          ++prepared.summary.iqr_outlier_count;
          add_reason(report.sample_audit[sample_index],
                     SampleExclusionReason::iqr_outlier);
        } else {
          prepared.after_iqr.push_back(sample);
        }
      }
    } else {
      prepared.after_iqr = prepared.before_iqr;
    }

    for (std::size_t index : indices) {
      const SampleAudit &audit = report.sample_audit[index];
      if (audit.exclusion_reasons.empty()) {
        report.sample_audit[index].included_after_filters = true;
      }
    }

    // La cobertura incluye todas las muestras post-warm-up; los estadísticos se
    // calculan antes y después de la selección IQR.
    for (MetricId metric : metric_ids()) {
      MetricSummary summary;
      summary.metric = metric;
      available_values(prepared.coverage_samples, metric,
                       &summary.availability);
      const auto after = available_values(prepared.after_iqr, metric, nullptr);
      summary.before_iqr = describe(
          available_values(prepared.before_iqr, metric, nullptr), options,
          key.first + key.second, metric);
      summary.after_iqr = describe(after, options, key.first + key.second,
                                   metric);
      prepared.summary.metrics.push_back(summary);
    }

    for (std::size_t index : indices) {
      prepared.summary.sample_audit.push_back(report.sample_audit[index]);
    }
    prepared_groups.push_back(std::move(prepared));
  }

  for (const PreparedGroup &group : prepared_groups) {
    report.groups.push_back(group.summary);
  }
  report.comparisons =
      calculate_comparisons(prepared_groups, options.comparisons, options);
  return report;
}

} // namespace detail

StatisticsReport calculate_statistics(
    const std::vector<BenchmarkSample> &samples, const RunMetadata &metadata,
    const StatisticsOptions &options) {
  return detail::calculate_statistics_report(samples, metadata, options);
}

} // namespace statistics
} // namespace ntq
