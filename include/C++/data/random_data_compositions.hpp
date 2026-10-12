#pragma once

#include "data/random_data_discrete.hpp"

#include <utility>

namespace ntq {
namespace data {

template <typename T>
struct MixtureComponent {
  double weight{0.0};
  std::function<T(std::mt19937_64 &)> sample;
};

namespace detail {

inline std::vector<double>
normalized_weights(const std::vector<double> &weights, const char *name) {
  if (weights.empty()) {
    throw std::invalid_argument(std::string(name) + " must not be empty");
  }
  double maximum_weight = 0.0;
  for (double weight : weights) {
    if (!std::isfinite(weight) || weight < 0.0) {
      throw std::invalid_argument(std::string(name) +
                                  " must contain finite nonnegative values");
    }
    maximum_weight = (std::max)(maximum_weight, weight);
  }
  if (maximum_weight == 0.0) {
    throw std::invalid_argument(std::string(name) + " must have positive total");
  }
  std::vector<double> scaled;
  scaled.reserve(weights.size());
  for (double weight : weights) {
    scaled.push_back(weight / maximum_weight);
  }
  return scaled;
}

struct ZipfDistribution {
  std::discrete_distribution<long long> ranks;

  std::size_t operator()(std::mt19937_64 &engine) {
    return static_cast<std::size_t>(ranks(engine)) + 1;
  }
};

inline ZipfDistribution make_zipf_distribution(std::size_t max_rank,
                                               double exponent) {
  if (max_rank == 0 ||
      static_cast<std::uintmax_t>(max_rank) >
          static_cast<std::uintmax_t>(
              (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("Zipf max_rank is outside the supported range");
  }
  if (!std::isfinite(exponent) || exponent <= 0.0) {
    throw std::invalid_argument("Zipf exponent must be finite and positive");
  }
  std::vector<double> weights;
  weights.reserve(max_rank);
  for (std::size_t index = 0; index < max_rank; ++index) {
    const long double rank = static_cast<long double>(index) + 1.0L;
    const long double log_weight =
        -static_cast<long double>(exponent) * std::log(rank);
    weights.push_back(static_cast<double>(std::exp(log_weight)));
  }
  return {std::discrete_distribution<long long>(weights.begin(), weights.end())};
}

template <typename T>
struct PowerLawDistribution {
  T xmin;
  T xmax;
  double exponent;

  T operator()(std::mt19937_64 &engine) {
    std::uniform_real_distribution<long double> unit(0.0L, 1.0L);
    const long double u = unit(engine);
    const long double log_min = std::log(static_cast<long double>(xmin));
    const long double log_max = std::log(static_cast<long double>(xmax));
    long double log_value = log_min;
    if (exponent == 1.0) {
      log_value += u * (log_max - log_min);
    } else if (u > 0.0L) {
      const long double power = 1.0L - static_cast<long double>(exponent);
      const long double span = log_max - log_min;
      const long double scaled_span = power * span;
      long double log_integral = 0.0L;
      if (scaled_span > std::log((std::numeric_limits<long double>::max)())) {
        log_integral = std::log(u) + scaled_span;
      } else {
        log_integral = std::log1p(u * std::expm1(scaled_span));
      }
      log_value += log_integral / power;
    }
    log_value = (std::max)(log_min, (std::min)(log_value, log_max));
    const long double sampled = std::exp(log_value);
    return static_cast<T>((std::max)(static_cast<long double>(xmin),
                                    (std::min)(sampled,
                                               static_cast<long double>(xmax))));
  }
};

template <typename T>
struct CategoricalDistribution {
  std::vector<T> categories;
  std::discrete_distribution<long long> selection;

  T operator()(std::mt19937_64 &engine) {
    const std::size_t index = static_cast<std::size_t>(selection(engine));
    return categories[index];
  }
};

template <typename T>
CategoricalDistribution<T>
make_categorical_distribution(const std::vector<T> &categories,
                              const std::vector<double> &weights) {
  static_assert(is_supported_number<T>::value,
                "categorical values must use a non-bool numeric type");
  if (categories.empty() || categories.size() != weights.size()) {
    throw std::invalid_argument(
        "categorical categories and weights must have equal nonzero length");
  }
  if (static_cast<std::uintmax_t>(categories.size()) >
      static_cast<std::uintmax_t>(
          (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("too many categorical values");
  }
  for (T category : categories) {
    if (!is_finite_value(category)) {
      throw std::invalid_argument("categorical values must be finite");
    }
  }
  const std::vector<double> scaled =
      normalized_weights(weights, "categorical weights");
  return {categories,
          std::discrete_distribution<long long>(scaled.begin(), scaled.end())};
}

template <typename T>
struct MixtureDistribution {
  std::vector<MixtureComponent<T>> components;
  std::discrete_distribution<long long> selection;

  T operator()(std::mt19937_64 &engine) {
    const std::size_t index = static_cast<std::size_t>(selection(engine));
    return components[index].sample(engine);
  }
};

template <typename T>
MixtureDistribution<T> make_mixture_distribution(
    const std::vector<MixtureComponent<T>> &components) {
  if (components.empty()) {
    throw std::invalid_argument("mixture must contain at least one component");
  }
  if (static_cast<std::uintmax_t>(components.size()) >
      static_cast<std::uintmax_t>(
          (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("too many mixture components");
  }
  std::vector<double> weights;
  weights.reserve(components.size());
  for (const MixtureComponent<T> &component : components) {
    if (!component.sample) {
      throw std::invalid_argument("mixture sampler must not be empty");
    }
    weights.push_back(component.weight);
  }
  const std::vector<double> scaled =
      normalized_weights(weights, "mixture weights");
  return {components,
          std::discrete_distribution<long long>(scaled.begin(), scaled.end())};
}

} // namespace detail

inline std::vector<std::size_t> zipf_vector(
    std::size_t count, double exponent, std::size_t max_rank,
    std::uint64_t seed, const GenerationOptions &options = {}) {
  return detail::sample_discrete_vector(
      count, detail::make_zipf_distribution(max_rank, exponent), seed, options);
}

inline std::vector<std::size_t> zipf_vector(
    std::size_t count, double exponent, std::size_t max_rank,
    std::mt19937_64 &engine, const GenerationOptions &options = {}) {
  return detail::sample_discrete_vector(
      count, detail::make_zipf_distribution(max_rank, exponent), engine,
      options);
}

template <std::size_t N>
std::array<std::size_t, N> zipf_array(
    double exponent, std::size_t max_rank, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  return detail::sample_discrete_array<N>(
      detail::make_zipf_distribution(max_rank, exponent), seed, options);
}

template <std::size_t N>
std::array<std::size_t, N> zipf_array(
    double exponent, std::size_t max_rank, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  return detail::sample_discrete_array<N>(
      detail::make_zipf_distribution(max_rank, exponent), engine, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> power_law_vector(std::size_t count, double exponent, T xmin,
                                T xmax, std::uint64_t seed,
                                const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(exponent) || exponent <= 0.0 ||
      !detail::is_finite_value(xmin) || !detail::is_finite_value(xmax) ||
      xmin <= T{0} || xmin >= xmax) {
    throw std::invalid_argument(
        "power-law requires exponent > 0 and finite 0 < xmin < xmax");
  }
  return generate_vector<T>(
      count, detail::PowerLawDistribution<T>{xmin, xmax, exponent}, seed,
      options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> power_law_vector(std::size_t count, double exponent, T xmin,
                                T xmax, std::mt19937_64 &engine,
                                const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(exponent) || exponent <= 0.0 ||
      !detail::is_finite_value(xmin) || !detail::is_finite_value(xmax) ||
      xmin <= T{0} || xmin >= xmax) {
    throw std::invalid_argument(
        "power-law requires exponent > 0 and finite 0 < xmin < xmax");
  }
  return generate_vector<T>(
      count, detail::PowerLawDistribution<T>{xmin, xmax, exponent}, engine,
      options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> power_law_array(double exponent, T xmin, T xmax,
                                 std::uint64_t seed,
                                 const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      power_law_vector<T>(N, exponent, xmin, xmax, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> power_law_array(double exponent, T xmin, T xmax,
                                 std::mt19937_64 &engine,
                                 const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      power_law_vector<T>(N, exponent, xmin, xmax, engine, options));
}

template <typename T>
std::vector<T> categorical_vector(
    std::size_t count, const std::vector<T> &categories,
    const std::vector<double> &weights, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  return generate_vector<T>(
      count, detail::make_categorical_distribution(categories, weights), seed,
      options);
}

template <typename T>
std::vector<T> categorical_vector(
    std::size_t count, const std::vector<T> &categories,
    const std::vector<double> &weights, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  return generate_vector<T>(
      count, detail::make_categorical_distribution(categories, weights),
      engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> categorical_array(
    const std::vector<T> &categories, const std::vector<double> &weights,
    std::uint64_t seed, const GenerationOptions &options = {}) {
  return generate_array<T, N>(
      detail::make_categorical_distribution(categories, weights), seed,
      options);
}

template <typename T, std::size_t N>
std::array<T, N> categorical_array(
    const std::vector<T> &categories, const std::vector<double> &weights,
    std::mt19937_64 &engine, const GenerationOptions &options = {}) {
  return generate_array<T, N>(
      detail::make_categorical_distribution(categories, weights), engine,
      options);
}

template <typename T, typename Distribution>
MixtureComponent<T> make_mixture_component(double weight,
                                           Distribution distribution) {
  static_assert(detail::is_supported_number<T>::value,
                "mixture values must use a non-bool numeric type");
  static_assert(
      std::is_convertible<decltype(distribution(std::declval<std::mt19937_64 &>())),
                          T>::value,
      "mixture distribution result must be convertible to T");
  if (!std::isfinite(weight) || weight < 0.0) {
    throw std::invalid_argument("mixture component weight must be finite and nonnegative");
  }
  MixtureComponent<T> component;
  component.weight = weight;
  component.sample = [distribution](std::mt19937_64 &engine) mutable {
    return static_cast<T>(distribution(engine));
  };
  return component;
}

template <typename T>
std::vector<T> mixture_vector(
    std::size_t count, const std::vector<MixtureComponent<T>> &components,
    std::uint64_t seed, const GenerationOptions &options = {}) {
  return generate_vector<T>(count,
                            detail::make_mixture_distribution(components), seed,
                            options);
}

template <typename T>
std::vector<T> mixture_vector(
    std::size_t count, const std::vector<MixtureComponent<T>> &components,
    std::mt19937_64 &engine, const GenerationOptions &options = {}) {
  return generate_vector<T>(count,
                            detail::make_mixture_distribution(components),
                            engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> mixture_array(
    const std::vector<MixtureComponent<T>> &components, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  return generate_array<T, N>(detail::make_mixture_distribution(components),
                              seed, options);
}

template <typename T, std::size_t N>
std::array<T, N> mixture_array(
    const std::vector<MixtureComponent<T>> &components,
    std::mt19937_64 &engine, const GenerationOptions &options = {}) {
  return generate_array<T, N>(detail::make_mixture_distribution(components),
                              engine, options);
}

} // namespace data
} // namespace ntq
