#include "statistics_math.hpp"

#include <algorithm>
#include <random>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

long double arithmetic_mean(const std::vector<long double> &values) {
  long double sum = 0.0L;
  for (long double value : values) {
    sum += value;
  }
  return sum / static_cast<long double>(values.size());
}

// Obtiene el estimador solicitado para una réplica.
long double estimate(const std::vector<long double> &values,
                     BootstrapEstimator estimator) {
  return estimator == BootstrapEstimator::mean ? arithmetic_mean(values)
                                                : median(values);
}

} // namespace

ConfidenceInterval bootstrap_interval(const std::vector<long double> &values,
                                      BootstrapEstimator estimator,
                                      const StatisticsOptions &options,
                                      std::uint64_t seed) {
  ConfidenceInterval interval;
  interval.confidence_level = options.confidence_level;
  interval.resamples = options.bootstrap_resamples;
  interval.seed = seed;
  if (values.size() < 2 || options.bootstrap_resamples == 0) {
    interval.status = ConfidenceIntervalStatus::insufficient_samples;
    return interval;
  }

  std::mt19937_64 generator(seed);
  std::uniform_int_distribution<std::size_t> index(0, values.size() - 1);
  std::vector<long double> estimates;
  estimates.reserve(options.bootstrap_resamples);
  std::vector<long double> resample(values.size());
  // Cada réplica conserva n y selecciona observaciones con reemplazo.
  for (std::size_t iteration = 0; iteration < options.bootstrap_resamples;
       ++iteration) {
    for (long double &value : resample) {
      value = values[index(generator)];
    }
    estimates.push_back(estimate(resample, estimator));
  }

  // Usa las colas simétricas del nivel de confianza solicitado.
  const long double tail =
      (1.0L - static_cast<long double>(options.confidence_level)) / 2.0L;
  interval.lower = quantile(estimates, tail);
  interval.upper = quantile(estimates, 1.0L - tail);
  interval.status = values.size() < options.minimum_reliable_ci_samples
                        ? ConfidenceIntervalStatus::low_sample_count
                        : ConfidenceIntervalStatus::available;
  return interval;
}

} // namespace detail
} // namespace statistics
} // namespace ntq
