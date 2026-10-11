#include "statistics_math.hpp"

#include <algorithm>
#include <cmath>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

long double mean(const std::vector<long double> &values) {
  long double sum = 0.0L;
  for (long double value : values) {
    sum += value;
  }
  return sum / static_cast<long double>(values.size());
}

TailReliabilityStatus reliability(std::size_t count, long double percentile,
                                  long double minimum_tail_observations) {
  // Estima cuántas observaciones se esperarían en la cola superior.
  const long double expected_tail =
      static_cast<long double>(count) * (1.0L - percentile);
  return expected_tail >= minimum_tail_observations
             ? TailReliabilityStatus::reliable
             : TailReliabilityStatus::low_sample_count;
}

} // namespace

long double quantile(std::vector<long double> values,
                     long double probability) {
  if (values.empty()) {
    return 0.0L;
  }
  if (probability <= 0.0L) {
    return *std::min_element(values.begin(), values.end());
  }
  if (probability >= 1.0L) {
    return *std::max_element(values.begin(), values.end());
  }

  // Hyndman-Fan tipo 7: h=(n-1)*p, con interpolación lineal.
  const long double position =
      static_cast<long double>(values.size() - 1) * probability;
  const std::size_t lower = static_cast<std::size_t>(position);
  const std::size_t upper = std::min(lower + 1, values.size() - 1);
  const long double fraction = position - static_cast<long double>(lower);
  // Solo particiona alrededor del orden requerido; no ordena toda la muestra.
  std::nth_element(values.begin(), values.begin() + upper, values.end());
  const long double upper_value = values[upper];
  if (lower == upper) {
    return upper_value;
  }
  const long double lower_value =
      *std::max_element(values.begin(), values.begin() + lower + 1);
  return lower_value + (upper_value - lower_value) * fraction;
}

long double median(const std::vector<long double> &values) {
  // p50 coincide con la mediana reportada en los resúmenes.
  return quantile(values, 0.5L);
}

long double median_absolute_deviation(const std::vector<long double> &values) {
  if (values.empty()) {
    return 0.0L;
  }
  const long double center = median(values);
  // MAD es la mediana de las distancias absolutas al centro, sin escalar.
  std::vector<long double> deviations;
  deviations.reserve(values.size());
  for (long double value : values) {
    deviations.push_back(std::fabs(value - center));
  }
  return median(deviations);
}

long double sample_standard_deviation(
    const std::vector<long double> &values) {
  if (values.size() < 2) {
    return 0.0L;
  }
  const long double center = mean(values);
  long double squared_deviations = 0.0L;
  for (long double value : values) {
    const long double difference = value - center;
    squared_deviations += difference * difference;
  }
  // Usa n-1: la dispersión se estima a partir de una muestra.
  return std::sqrt(squared_deviations /
                   static_cast<long double>(values.size() - 1));
}

std::uint64_t stable_seed(std::uint64_t seed, const std::string &key,
                          MetricId metric) {
  // FNV-1a mantiene la secuencia bootstrap estable entre bibliotecas estándar.
  std::uint64_t hash = 14695981039346656037ULL ^ seed;
  for (unsigned char character : key) {
    hash ^= character;
    hash *= 1099511628211ULL;
  }
  hash ^= static_cast<std::uint64_t>(metric);
  hash *= 1099511628211ULL;
  return hash;
}

DescriptiveStatistics describe(const std::vector<long double> &values,
                               const StatisticsOptions &options,
                               const std::string &group_key, MetricId metric) {
  DescriptiveStatistics summary;
  summary.sample_count = values.size();
  if (values.empty()) {
    summary.status = StatisticsStatus::unavailable;
    return summary;
  }

  const auto bounds = std::minmax_element(values.begin(), values.end());
  summary.minimum = *bounds.first;
  summary.maximum = *bounds.second;
  summary.arithmetic_mean = mean(values);
  summary.median = median(values);
  summary.median_absolute_deviation = median_absolute_deviation(values);
  if (values.size() >= 2) {
    summary.sample_standard_deviation = sample_standard_deviation(values);
    summary.sample_standard_deviation_status = StatisticsStatus::available;
  } else {
    summary.sample_standard_deviation_status =
        StatisticsStatus::insufficient_samples;
  }

  // Se conservan los valores de cola y, por separado, su fiabilidad.
  summary.p90 = quantile(values, 0.90L);
  summary.p95 = quantile(values, 0.95L);
  summary.p99 = quantile(values, 0.99L);
  summary.p999 = quantile(values, 0.999L);
  summary.p90_reliability = reliability(
      values.size(), 0.90L, options.minimum_expected_tail_observations);
  summary.p95_reliability = reliability(
      values.size(), 0.95L, options.minimum_expected_tail_observations);
  summary.p99_reliability = reliability(
      values.size(), 0.99L, options.minimum_expected_tail_observations);
  summary.p999_reliability = reliability(
      values.size(), 0.999L, options.minimum_expected_tail_observations);

  // Se deriva una semilla estable por grupo/métrica/estimador para que repetir
  // el análisis con las mismas entradas reproduzca los intervalos.
  const std::string metric_key = group_key + "\x1f" +
                                 std::to_string(static_cast<unsigned>(metric));
  summary.mean_confidence_interval = bootstrap_interval(
      values, BootstrapEstimator::mean, options,
      stable_seed(options.bootstrap_seed, metric_key + ":mean", metric));
  summary.median_confidence_interval = bootstrap_interval(
      values, BootstrapEstimator::median, options,
      stable_seed(options.bootstrap_seed, metric_key + ":median", metric));
  summary.status = StatisticsStatus::available;
  if (values.size() < options.minimum_reliable_ci_samples) {
    summary.status = StatisticsStatus::low_reliability;
  }
  return summary;
}

} // namespace detail
} // namespace statistics
} // namespace ntq
