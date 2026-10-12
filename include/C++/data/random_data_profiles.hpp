#pragma once

#include "data/random_data_compositions.hpp"

namespace ntq {
namespace data {
namespace detail {

template <typename T, typename Distribution>
struct SparseDistribution {
  double zero_probability;
  Distribution nonzero_distribution;

  T operator()(std::mt19937_64 &engine) {
    std::bernoulli_distribution choose_zero(zero_probability);
    if (choose_zero(engine)) {
      return T{0};
    }
    return static_cast<T>(nonzero_distribution(engine));
  }
};

} // namespace detail

template <typename T>
std::vector<T> constant_vector(std::size_t count, T value) {
  static_assert(detail::is_supported_number<T>::value,
                "constant values must use a non-bool numeric type");
  if (!detail::is_finite_value(value)) {
    throw std::invalid_argument("constant value must be finite");
  }
  return std::vector<T>(count, value);
}

template <typename T, std::size_t N>
std::array<T, N> constant_array(T value) {
  static_assert(detail::is_supported_number<T>::value,
                "constant values must use a non-bool numeric type");
  if (!detail::is_finite_value(value)) {
    throw std::invalid_argument("constant value must be finite");
  }
  std::array<T, N> values{};
  values.fill(value);
  return values;
}

template <typename T>
std::vector<T> low_cardinality_vector(
    std::size_t count, const std::vector<T> &palette, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  return categorical_vector<T>(count, palette,
                               std::vector<double>(palette.size(), 1.0), seed,
                               options);
}

template <typename T>
std::vector<T> low_cardinality_vector(
    std::size_t count, const std::vector<T> &palette,
    const std::vector<double> &weights, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  return categorical_vector<T>(count, palette, weights, seed, options);
}

template <typename T>
std::vector<T> low_cardinality_vector(
    std::size_t count, const std::vector<T> &palette, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  return categorical_vector<T>(count, palette,
                               std::vector<double>(palette.size(), 1.0), engine,
                               options);
}

template <typename T>
std::vector<T> low_cardinality_vector(
    std::size_t count, const std::vector<T> &palette,
    const std::vector<double> &weights, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  return categorical_vector<T>(count, palette, weights, engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> low_cardinality_array(
    const std::vector<T> &palette, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  return categorical_array<T, N>(palette,
                                 std::vector<double>(palette.size(), 1.0), seed,
                                 options);
}

template <typename T, std::size_t N>
std::array<T, N> low_cardinality_array(
    const std::vector<T> &palette, const std::vector<double> &weights,
    std::uint64_t seed, const GenerationOptions &options = {}) {
  return categorical_array<T, N>(palette, weights, seed, options);
}

template <typename T, std::size_t N>
std::array<T, N> low_cardinality_array(
    const std::vector<T> &palette, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  return categorical_array<T, N>(palette,
                                 std::vector<double>(palette.size(), 1.0),
                                 engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> low_cardinality_array(
    const std::vector<T> &palette, const std::vector<double> &weights,
    std::mt19937_64 &engine, const GenerationOptions &options = {}) {
  return categorical_array<T, N>(palette, weights, engine, options);
}

template <typename T>
std::vector<T> clustered_vector(
    std::size_t count, const std::vector<T> &centers,
    const std::vector<T> &standard_deviations,
    const std::vector<double> &weights, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  std::mt19937_64 engine(seed);
  return clustered_vector<T>(count, centers, standard_deviations, weights,
                             engine, options);
}

template <typename T>
std::vector<T> clustered_vector(
    std::size_t count, const std::vector<T> &centers,
    const std::vector<T> &standard_deviations,
    const std::vector<double> &weights, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  static_assert(std::is_floating_point<T>::value,
                "clustered normal data requires a floating-point type");
  if (centers.empty() || centers.size() != standard_deviations.size() ||
      centers.size() != weights.size()) {
    throw std::invalid_argument(
        "cluster centers, deviations and weights must have equal nonzero length");
  }
  std::vector<MixtureComponent<T>> components;
  components.reserve(centers.size());
  for (std::size_t index = 0; index < centers.size(); ++index) {
    if (!detail::is_finite_value(centers[index])) {
      throw std::invalid_argument("cluster centers must be finite");
    }
    const T deviation = standard_deviations[index];
    if (!detail::is_finite_value(deviation) || deviation <= T{0}) {
      throw std::invalid_argument("cluster deviations must be positive and finite");
    }
    components.push_back(make_mixture_component<T>(
        weights[index],
        std::normal_distribution<T>(centers[index], deviation)));
  }
  return mixture_vector<T>(count, components, engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> clustered_array(
    const std::vector<T> &centers,
    const std::vector<T> &standard_deviations,
    const std::vector<double> &weights, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  std::mt19937_64 engine(seed);
  return clustered_array<T, N>(centers, standard_deviations, weights, engine,
                               options);
}

template <typename T, std::size_t N>
std::array<T, N> clustered_array(
    const std::vector<T> &centers,
    const std::vector<T> &standard_deviations,
    const std::vector<double> &weights, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(clustered_vector<T>(
      N, centers, standard_deviations, weights, engine, options));
}

template <typename T, typename Distribution>
std::vector<T> sparse_vector(std::size_t count, double zero_probability,
                             Distribution nonzero_distribution,
                             std::uint64_t seed,
                             const GenerationOptions &options = {}) {
  static_assert(detail::is_supported_number<T>::value,
                "sparse values must use a non-bool numeric type");
  detail::validate_probability(zero_probability, true, "Zero probability");
  return generate_vector<T>(
      count,
      detail::SparseDistribution<T, Distribution>{zero_probability,
                                                  nonzero_distribution},
      seed, options);
}

template <typename T, typename Distribution>
std::vector<T> sparse_vector(std::size_t count, double zero_probability,
                             Distribution nonzero_distribution,
                             std::mt19937_64 &engine,
                             const GenerationOptions &options = {}) {
  static_assert(detail::is_supported_number<T>::value,
                "sparse values must use a non-bool numeric type");
  detail::validate_probability(zero_probability, true, "Zero probability");
  return generate_vector<T>(
      count,
      detail::SparseDistribution<T, Distribution>{zero_probability,
                                                  nonzero_distribution},
      engine, options);
}

template <typename T, std::size_t N, typename Distribution>
std::array<T, N> sparse_array(double zero_probability,
                              Distribution nonzero_distribution,
                              std::uint64_t seed,
                              const GenerationOptions &options = {}) {
  detail::validate_probability(zero_probability, true, "Zero probability");
  return generate_array<T, N>(
      detail::SparseDistribution<T, Distribution>{zero_probability,
                                                  nonzero_distribution},
      seed, options);
}

template <typename T, std::size_t N, typename Distribution>
std::array<T, N> sparse_array(double zero_probability,
                              Distribution nonzero_distribution,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {}) {
  detail::validate_probability(zero_probability, true, "Zero probability");
  return generate_array<T, N>(
      detail::SparseDistribution<T, Distribution>{zero_probability,
                                                  nonzero_distribution},
      engine, options);
}

} // namespace data
} // namespace ntq
