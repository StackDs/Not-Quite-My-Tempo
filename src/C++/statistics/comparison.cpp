#include "statistics_internal.hpp"

#include "statistics_math.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

const PreparedGroup *find_group(const std::vector<PreparedGroup> &groups,
                                const std::string &group_key,
                                const std::string &variant_key) {
  for (const PreparedGroup &group : groups) {
    if (group.summary.group_key == group_key &&
        group.summary.variant_key == variant_key) {
      return &group;
    }
  }
  return nullptr;
}

long double arithmetic_mean(const std::vector<long double> &values) {
  if (values.empty()) {
    return 0.0L;
  }
  long double total = 0.0L;
  for (long double value : values) {
    total += value;
  }
  return total / static_cast<long double>(values.size());
}

long double estimate(const std::vector<long double> &values,
                     BootstrapEstimator estimator) {
  return estimator == BootstrapEstimator::mean ? arithmetic_mean(values)
                                                : median(values);
}

ConfidenceInterval bootstrap_difference_interval(
    const std::vector<long double> &baseline,
    const std::vector<long double> &candidate, BootstrapEstimator estimator,
    bool relative, const StatisticsOptions &options, std::uint64_t seed) {
  ConfidenceInterval interval;
  interval.confidence_level = options.confidence_level;
  interval.resamples = options.bootstrap_resamples;
  interval.seed = seed;
  if (baseline.size() < 2 || candidate.size() < 2 ||
      options.bootstrap_resamples == 0) {
    interval.status = ConfidenceIntervalStatus::insufficient_samples;
    return interval;
  }

  std::mt19937_64 generator(seed);
  std::uniform_int_distribution<std::size_t> pick_baseline(0,
                                                           baseline.size() - 1);
  std::uniform_int_distribution<std::size_t> pick_candidate(
      0, candidate.size() - 1);
  std::vector<long double> baseline_resample(baseline.size());
  std::vector<long double> candidate_resample(candidate.size());
  std::vector<long double> effects;
  effects.reserve(options.bootstrap_resamples);
  for (std::size_t iteration = 0; iteration < options.bootstrap_resamples;
       ++iteration) {
    // En comparación independiente, cada variante se remuestrea por separado.
    for (long double &value : baseline_resample) {
      value = baseline[pick_baseline(generator)];
    }
    for (long double &value : candidate_resample) {
      value = candidate[pick_candidate(generator)];
    }
    const long double base = estimate(baseline_resample, estimator);
    const long double difference =
        estimate(candidate_resample, estimator) - base;
    if (relative) {
      if (base == 0.0L) {
        continue;
      }
      effects.push_back(100.0L * difference / base);
    } else {
      effects.push_back(difference);
    }
  }
  if (effects.empty()) {
    interval.status = ConfidenceIntervalStatus::insufficient_samples;
    return interval;
  }

  const long double tail =
      (1.0L - static_cast<long double>(options.confidence_level)) / 2.0L;
  interval.lower = quantile(effects, tail);
  interval.upper = quantile(effects, 1.0L - tail);
  interval.resamples = effects.size();
  const std::size_t smaller_group =
      std::min(baseline.size(), candidate.size());
  interval.status = smaller_group < options.minimum_reliable_ci_samples
                        ? ConfidenceIntervalStatus::low_sample_count
                        : ConfidenceIntervalStatus::available;
  return interval;
}

EffectEstimate make_effect(const std::vector<long double> &baseline,
                           const std::vector<long double> &candidate,
                           BootstrapEstimator estimator, bool paired,
                           const std::vector<long double> &paired_differences,
                           const std::vector<long double> &paired_percentages,
                           const StatisticsOptions &options,
                           std::uint64_t seed) {
  EffectEstimate effect;
  if (paired) {
    // El efecto pareado se estima sobre las diferencias de cada pair_id común.
    if (paired_differences.empty()) {
      effect.status = StatisticsStatus::unavailable;
      return effect;
    }
    effect.absolute_difference = estimate(paired_differences, estimator);
    effect.status = StatisticsStatus::available;
    effect.absolute_confidence_interval = bootstrap_interval(
        paired_differences, estimator, options, seed);
    // En el modo pareado, el porcentaje se calcula muestra a muestra antes de
    // resumirse para no perder la correspondencia entre ejecuciones.
    effect.relative_sample_count = paired_percentages.size();
    if (!paired_percentages.empty()) {
      effect.relative_difference_percent =
          estimate(paired_percentages, estimator);
      effect.relative_status = StatisticsStatus::available;
      effect.relative_confidence_interval = bootstrap_interval(
          paired_percentages, estimator, options, seed ^ 0x9e3779b97f4a7c15ULL);
    } else {
      effect.relative_status = StatisticsStatus::invalid_denominator;
    }
    return effect;
  }

  if (baseline.empty() || candidate.empty()) {
    effect.status = StatisticsStatus::unavailable;
    return effect;
  }
  const long double baseline_estimate = estimate(baseline, estimator);
  const long double candidate_estimate = estimate(candidate, estimator);
  // Convención de signo: candidato menos base; porcentaje relativo a la base.
  effect.absolute_difference = candidate_estimate - baseline_estimate;
  effect.status = StatisticsStatus::available;
  effect.absolute_confidence_interval = bootstrap_difference_interval(
      baseline, candidate, estimator, false, options, seed);
  if (baseline_estimate == 0.0L) {
    effect.relative_status = StatisticsStatus::invalid_denominator;
  } else {
    effect.relative_difference_percent =
        100.0L * effect.absolute_difference / baseline_estimate;
    effect.relative_status = StatisticsStatus::available;
    effect.relative_sample_count =
        std::min(baseline.size(), candidate.size());
    effect.relative_confidence_interval = bootstrap_difference_interval(
        baseline, candidate, estimator, true, options,
        seed ^ 0x9e3779b97f4a7c15ULL);
  }
  return effect;
}

std::map<std::string, const BenchmarkSample *> samples_by_pair(
    const std::vector<const BenchmarkSample *> &samples) {
  std::map<std::string, const BenchmarkSample *> by_pair;
  // Los IDs vacíos indican que la muestra no puede participar en un par.
  for (const BenchmarkSample *sample : samples) {
    if (!sample->pair_id.empty()) {
      by_pair[sample->pair_id] = sample;
    }
  }
  return by_pair;
}

std::vector<long double> metric_values(
    const std::vector<const BenchmarkSample *> &samples, MetricId metric) {
  std::vector<long double> values;
  for (const BenchmarkSample *sample : samples) {
    const MetricObservation value = observe_metric(*sample, metric);
    if (value.status == MetricValueStatus::available) {
      values.push_back(value.value);
    }
  }
  return values;
}

std::uint64_t comparison_seed(const StatisticsOptions &options,
                              const ComparisonRequest &request,
                              MetricId metric, const char *estimator) {
  const std::string key = request.group_key + ":" + request.baseline_variant +
                          ":" + request.candidate_variant + ":" + estimator;
  return stable_seed(options.bootstrap_seed, key, metric);
}

MetricComparison compare_metric(
    const ComparisonRequest &request, MetricId metric,
    const PreparedGroup *baseline, const PreparedGroup *candidate,
    bool paired, const StatisticsOptions &options) {
  MetricComparison result;
  result.metric = metric;
  result.pairing_used = paired ? PairingUsed::paired : PairingUsed::unpaired;
  if (baseline == nullptr || candidate == nullptr) {
    result.pairing_used = PairingUsed::unavailable;
    return result;
  }

  const auto &baseline_samples = baseline->after_iqr;
  const auto &candidate_samples = candidate->after_iqr;
  if (paired) {
    // Solo se comparan muestras supervivientes a los mismos filtros y con ID
    // emparejado; valores no disponibles se omiten para esta métrica.
    const auto baseline_pairs = samples_by_pair(baseline_samples);
    const auto candidate_pairs = samples_by_pair(candidate_samples);
    std::vector<long double> base_values;
    std::vector<long double> candidate_values;
    std::vector<long double> differences;
    std::vector<long double> relative_differences;
    for (const auto &pair : baseline_pairs) {
      const auto candidate_match = candidate_pairs.find(pair.first);
      if (candidate_match == candidate_pairs.end()) {
        continue;
      }
      const MetricObservation base = observe_metric(*pair.second, metric);
      const MetricObservation next =
          observe_metric(*candidate_match->second, metric);
      if (base.status != MetricValueStatus::available ||
          next.status != MetricValueStatus::available) {
        continue;
      }
      base_values.push_back(base.value);
      candidate_values.push_back(next.value);
      const long double difference = next.value - base.value;
      differences.push_back(difference);
      if (base.value != 0.0L) {
        relative_differences.push_back(100.0L * difference / base.value);
      }
    }
    result.baseline_samples = base_values.size();
    result.candidate_samples = candidate_values.size();
    result.matched_pairs = differences.size();
    const auto seed = comparison_seed(options, request, metric, "paired");
    result.mean_effect = make_effect(
        base_values, candidate_values, BootstrapEstimator::mean, true,
        differences, relative_differences, options, seed);
    result.median_effect = make_effect(
        base_values, candidate_values, BootstrapEstimator::median, true,
        differences, relative_differences, options, seed ^ 0x517cc1b727220a95ULL);
    return result;
  }

  const std::vector<long double> base_values =
      metric_values(baseline_samples, metric);
  const std::vector<long double> candidate_values =
      metric_values(candidate_samples, metric);
  result.baseline_samples = base_values.size();
  result.candidate_samples = candidate_values.size();
  const auto seed = comparison_seed(options, request, metric, "unpaired");
  result.mean_effect = make_effect(
      base_values, candidate_values, BootstrapEstimator::mean, false,
      std::vector<long double>(), std::vector<long double>(), options, seed);
  result.median_effect = make_effect(
      base_values, candidate_values, BootstrapEstimator::median, false,
      std::vector<long double>(), std::vector<long double>(), options,
      seed ^ 0x517cc1b727220a95ULL);
  return result;
}

} // namespace

std::vector<GroupComparison> calculate_comparisons(
    const std::vector<PreparedGroup> &groups,
    const std::vector<ComparisonRequest> &requests,
    const StatisticsOptions &options) {
  // Solo compara pares de grupos pedidos explícitamente por el caller.
  std::vector<GroupComparison> comparisons;
  for (const ComparisonRequest &request : requests) {
    const PreparedGroup *baseline =
        find_group(groups, request.group_key, request.baseline_variant);
    const PreparedGroup *candidate =
        find_group(groups, request.group_key, request.candidate_variant);
    GroupComparison comparison;
    comparison.group_key = request.group_key;
    comparison.baseline_variant = request.baseline_variant;
    comparison.candidate_variant = request.candidate_variant;

    // En modo automático, basta una intersección de pair_id para preferir la
    // comparación pareada; require_paired fuerza esa política explícitamente.
    bool use_pairs = false;
    if (baseline != nullptr && candidate != nullptr &&
        request.pairing != PairingPolicy::unpaired) {
      const auto baseline_pairs = samples_by_pair(baseline->after_iqr);
      const auto candidate_pairs = samples_by_pair(candidate->after_iqr);
      for (const auto &pair : baseline_pairs) {
        if (candidate_pairs.find(pair.first) != candidate_pairs.end()) {
          use_pairs = true;
          break;
        }
      }
    }
    if (request.pairing == PairingPolicy::require_paired) {
      use_pairs = true;
    }

    for (MetricId metric : metric_ids()) {
      comparison.metrics.push_back(compare_metric(
          request, metric, baseline, candidate, use_pairs, options));
    }
    comparisons.push_back(comparison);
  }
  return comparisons;
}

} // namespace detail
} // namespace statistics
} // namespace ntq
