#include "statistics_internal.hpp"

#include <cmath>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

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

std::string sample_id_or_index(const BenchmarkSample &sample,
                               std::size_t index) {
  return sample.sample_id.empty()
             ? std::string("sample-") + std::to_string(index)
             : sample.sample_id;
}

} // namespace

ValidatedSamples validate_and_group_samples(
    const std::vector<BenchmarkSample> &samples,
    const StatisticsOptions &options) {
  validate_options(options);
  ValidatedSamples validated;
  validated.sample_audit.resize(samples.size());
  std::set<std::string> sample_ids;
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
    const auto unit = validated.units_by_group.find(key);
    if (unit == validated.units_by_group.end()) {
      validated.units_by_group[key] = sample.time_unit;
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
    validated.grouped_indices[key].push_back(index);

    SampleAudit &audit = validated.sample_audit[index];
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
    const auto baseline = validated.units_by_group.find(
        {request.group_key, request.baseline_variant});
    const auto candidate = validated.units_by_group.find(
        {request.group_key, request.candidate_variant});
    if (baseline != validated.units_by_group.end() &&
        candidate != validated.units_by_group.end() &&
        baseline->second != candidate->second) {
      throw std::invalid_argument(
          "Los grupos comparados deben usar la misma TimeUnit");
    }
  }

  return validated;
}

} // namespace detail
} // namespace statistics
} // namespace ntq
