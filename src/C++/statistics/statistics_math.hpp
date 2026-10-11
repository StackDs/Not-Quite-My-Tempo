#pragma once

#include "statistics/statistics.hpp"

namespace ntq {
namespace statistics {
namespace detail {

// Estimador central que se recalcula en cada réplica bootstrap.
enum class BootstrapEstimator { mean, median };

// Cuantil lineal tipo 7 (Hyndman-Fan) para una probabilidad entre 0 y 1.
long double quantile(std::vector<long double> values, long double probability);
long double median(const std::vector<long double> &values);
long double median_absolute_deviation(const std::vector<long double> &values);
long double sample_standard_deviation(const std::vector<long double> &values);
// Deriva una semilla estable de la corrida, grupo y métrica.
std::uint64_t stable_seed(std::uint64_t seed, const std::string &key,
                          MetricId metric);
// Calcula los estadísticos descriptivos y sus intervalos para un vector válido.
ConfidenceInterval bootstrap_interval(const std::vector<long double> &values,
                                      BootstrapEstimator estimator,
                                      const StatisticsOptions &options,
                                      std::uint64_t seed);
DescriptiveStatistics describe(const std::vector<long double> &values,
                               const StatisticsOptions &options,
                               const std::string &group_key, MetricId metric);

} // namespace detail
} // namespace statistics
} // namespace ntq
