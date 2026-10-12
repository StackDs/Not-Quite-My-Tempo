#pragma once

#include "data/random_data_core.hpp"

namespace ntq {
namespace data {
namespace detail {

inline void validate_probability(double probability, bool allow_zero,
                                 const char *name) {
  if (!std::isfinite(probability) || probability > 1.0 ||
      (allow_zero ? probability < 0.0 : probability <= 0.0)) {
    throw std::invalid_argument(std::string(name) +
                                " must be within its valid probability range");
  }
}

template <typename Distribution>
std::vector<std::size_t> sample_discrete_vector(
    std::size_t count, Distribution distribution, std::uint64_t seed,
    const GenerationOptions &options) {
  std::mt19937_64 engine(seed);
  return generate_vector_with_distribution<std::size_t>(count, distribution,
                                                         engine, options);
}

template <typename Distribution>
std::vector<std::size_t> sample_discrete_vector(
    std::size_t count, Distribution distribution, std::mt19937_64 &engine,
    const GenerationOptions &options) {
  return generate_vector_with_distribution<std::size_t>(count, distribution,
                                                         engine, options);
}

template <std::size_t N, typename Distribution>
std::array<std::size_t, N> sample_discrete_array(
    Distribution distribution, std::uint64_t seed,
    const GenerationOptions &options) {
  std::mt19937_64 engine(seed);
  return to_array<std::size_t, N>(
      generate_vector_with_distribution<std::size_t>(N, distribution, engine,
                                                      options));
}

template <std::size_t N, typename Distribution>
std::array<std::size_t, N> sample_discrete_array(
    Distribution distribution, std::mt19937_64 &engine,
    const GenerationOptions &options) {
  return to_array<std::size_t, N>(
      generate_vector_with_distribution<std::size_t>(N, distribution, engine,
                                                      options));
}

struct GeometricAttemptsDistribution {
  std::geometric_distribution<long long> failures;

  std::size_t operator()(std::mt19937_64 &engine) {
    const long long count = failures(engine);
    if (count < 0 || static_cast<unsigned long long>(count) >=
                         static_cast<unsigned long long>(
                             (std::numeric_limits<std::size_t>::max)())) {
      throw std::overflow_error(
          "geometric attempt count exceeds size_t range");
    }
    return static_cast<std::size_t>(count) + 1;
  }
};

struct HypergeometricDistribution {
  using result_type = std::size_t;
  std::size_t population_size;
  std::size_t success_count;
  std::size_t draws;

  result_type operator()(std::mt19937_64 &engine) {
    std::size_t remaining_population = population_size;
    std::size_t remaining_successes = success_count;
    std::size_t successes = 0;
    for (std::size_t draw = 0; draw < draws; ++draw) {
      if (remaining_successes == 0) {
        break;
      }
      if (remaining_successes == remaining_population) {
        successes += draws - draw;
        break;
      }
      std::uniform_int_distribution<std::size_t> pick(0,
                                                       remaining_population - 1);
      if (pick(engine) < remaining_successes) {
        ++successes;
        --remaining_successes;
      }
      --remaining_population;
    }
    return successes;
  }
};

} // namespace detail

inline std::vector<std::size_t>
bernoulli_vector(std::size_t count, double probability, std::uint64_t seed,
                 const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Bernoulli probability");
  return detail::sample_discrete_vector(
      count, std::bernoulli_distribution(probability), seed, options);
}

inline std::vector<std::size_t>
bernoulli_vector(std::size_t count, double probability,
                 std::mt19937_64 &engine,
                 const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Bernoulli probability");
  return detail::sample_discrete_vector(
      count, std::bernoulli_distribution(probability), engine, options);
}

template <std::size_t N>
std::array<std::size_t, N>
bernoulli_array(double probability, std::uint64_t seed,
                const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Bernoulli probability");
  return detail::sample_discrete_array<N>(
      std::bernoulli_distribution(probability), seed, options);
}

template <std::size_t N>
std::array<std::size_t, N>
bernoulli_array(double probability, std::mt19937_64 &engine,
                const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Bernoulli probability");
  return detail::sample_discrete_array<N>(
      std::bernoulli_distribution(probability), engine, options);
}

inline std::vector<std::size_t>
binomial_vector(std::size_t count, std::size_t trials, double probability,
                std::uint64_t seed,
                const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Binomial probability");
  if (static_cast<std::uintmax_t>(trials) >
      static_cast<std::uintmax_t>(
          (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("binomial trials exceed supported range");
  }
  return detail::sample_discrete_vector(
      count,
      std::binomial_distribution<long long>(
          static_cast<long long>(trials), probability),
      seed, options);
}

inline std::vector<std::size_t>
binomial_vector(std::size_t count, std::size_t trials, double probability,
                std::mt19937_64 &engine,
                const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Binomial probability");
  if (static_cast<std::uintmax_t>(trials) >
      static_cast<std::uintmax_t>(
          (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("binomial trials exceed supported range");
  }
  return detail::sample_discrete_vector(
      count,
      std::binomial_distribution<long long>(
          static_cast<long long>(trials), probability),
      engine, options);
}

template <std::size_t N>
std::array<std::size_t, N>
binomial_array(std::size_t trials, double probability, std::uint64_t seed,
               const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Binomial probability");
  if (static_cast<std::uintmax_t>(trials) >
      static_cast<std::uintmax_t>(
          (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("binomial trials exceed supported range");
  }
  return detail::sample_discrete_array<N>(
      std::binomial_distribution<long long>(
          static_cast<long long>(trials), probability),
      seed, options);
}

template <std::size_t N>
std::array<std::size_t, N>
binomial_array(std::size_t trials, double probability,
               std::mt19937_64 &engine,
               const GenerationOptions &options = {}) {
  detail::validate_probability(probability, true, "Binomial probability");
  if (static_cast<std::uintmax_t>(trials) >
      static_cast<std::uintmax_t>(
          (std::numeric_limits<long long>::max)())) {
    throw std::invalid_argument("binomial trials exceed supported range");
  }
  return detail::sample_discrete_array<N>(
      std::binomial_distribution<long long>(
          static_cast<long long>(trials), probability),
      engine, options);
}

inline std::vector<std::size_t>
geometric_vector(std::size_t count, double probability, std::uint64_t seed,
                 const GenerationOptions &options = {}) {
  detail::validate_probability(probability, false, "Geometric probability");
  if (probability == 1.0) {
    return detail::sample_discrete_vector(
        count, std::uniform_int_distribution<std::size_t>(1, 1), seed, options);
  }
  detail::GeometricAttemptsDistribution distribution{
      std::geometric_distribution<long long>(probability)};
  return detail::sample_discrete_vector(count, distribution, seed, options);
}

inline std::vector<std::size_t>
geometric_vector(std::size_t count, double probability,
                 std::mt19937_64 &engine,
                 const GenerationOptions &options = {}) {
  detail::validate_probability(probability, false, "Geometric probability");
  if (probability == 1.0) {
    return detail::sample_discrete_vector(
        count, std::uniform_int_distribution<std::size_t>(1, 1), engine,
        options);
  }
  detail::GeometricAttemptsDistribution distribution{
      std::geometric_distribution<long long>(probability)};
  return detail::sample_discrete_vector(count, distribution, engine, options);
}

template <std::size_t N>
std::array<std::size_t, N>
geometric_array(double probability, std::uint64_t seed,
                const GenerationOptions &options = {}) {
  detail::validate_probability(probability, false, "Geometric probability");
  if (probability == 1.0) {
    return detail::sample_discrete_array<N>(
        std::uniform_int_distribution<std::size_t>(1, 1), seed, options);
  }
  detail::GeometricAttemptsDistribution distribution{
      std::geometric_distribution<long long>(probability)};
  return detail::sample_discrete_array<N>(distribution, seed, options);
}

template <std::size_t N>
std::array<std::size_t, N>
geometric_array(double probability, std::mt19937_64 &engine,
                const GenerationOptions &options = {}) {
  detail::validate_probability(probability, false, "Geometric probability");
  if (probability == 1.0) {
    return detail::sample_discrete_array<N>(
        std::uniform_int_distribution<std::size_t>(1, 1), engine, options);
  }
  detail::GeometricAttemptsDistribution distribution{
      std::geometric_distribution<long long>(probability)};
  return detail::sample_discrete_array<N>(distribution, engine, options);
}

inline std::vector<std::size_t>
poisson_vector(std::size_t count, double lambda, std::uint64_t seed,
               const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(lambda) || lambda < 0.0) {
    throw std::invalid_argument("Poisson lambda must be finite and nonnegative");
  }
  if (lambda == 0.0) {
    return detail::sample_discrete_vector(
        count, std::uniform_int_distribution<std::size_t>(0, 0), seed, options);
  }
  return detail::sample_discrete_vector(
      count, std::poisson_distribution<long long>(lambda), seed, options);
}

inline std::vector<std::size_t>
poisson_vector(std::size_t count, double lambda, std::mt19937_64 &engine,
               const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(lambda) || lambda < 0.0) {
    throw std::invalid_argument("Poisson lambda must be finite and nonnegative");
  }
  if (lambda == 0.0) {
    return detail::sample_discrete_vector(
        count, std::uniform_int_distribution<std::size_t>(0, 0), engine,
        options);
  }
  return detail::sample_discrete_vector(
      count, std::poisson_distribution<long long>(lambda), engine, options);
}

template <std::size_t N>
std::array<std::size_t, N>
poisson_array(double lambda, std::uint64_t seed,
              const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(lambda) || lambda < 0.0) {
    throw std::invalid_argument("Poisson lambda must be finite and nonnegative");
  }
  if (lambda == 0.0) {
    return detail::sample_discrete_array<N>(
        std::uniform_int_distribution<std::size_t>(0, 0), seed, options);
  }
  return detail::sample_discrete_array<N>(
      std::poisson_distribution<long long>(lambda), seed, options);
}

template <std::size_t N>
std::array<std::size_t, N>
poisson_array(double lambda, std::mt19937_64 &engine,
              const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(lambda) || lambda < 0.0) {
    throw std::invalid_argument("Poisson lambda must be finite and nonnegative");
  }
  if (lambda == 0.0) {
    return detail::sample_discrete_array<N>(
        std::uniform_int_distribution<std::size_t>(0, 0), engine, options);
  }
  return detail::sample_discrete_array<N>(
      std::poisson_distribution<long long>(lambda), engine, options);
}

inline std::vector<std::size_t> hypergeometric_vector(
    std::size_t count, std::size_t population_size,
    std::size_t success_count, std::size_t draws, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  if (success_count > population_size || draws > population_size) {
    throw std::invalid_argument(
        "hypergeometric successes and draws must not exceed population size");
  }
  return detail::sample_discrete_vector(
      count,
      detail::HypergeometricDistribution{population_size, success_count, draws},
      seed, options);
}

inline std::vector<std::size_t> hypergeometric_vector(
    std::size_t count, std::size_t population_size,
    std::size_t success_count, std::size_t draws, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  if (success_count > population_size || draws > population_size) {
    throw std::invalid_argument(
        "hypergeometric successes and draws must not exceed population size");
  }
  return detail::sample_discrete_vector(
      count,
      detail::HypergeometricDistribution{population_size, success_count, draws},
      engine, options);
}

template <std::size_t N>
std::array<std::size_t, N> hypergeometric_array(
    std::size_t population_size, std::size_t success_count,
    std::size_t draws, std::uint64_t seed,
    const GenerationOptions &options = {}) {
  if (success_count > population_size || draws > population_size) {
    throw std::invalid_argument(
        "hypergeometric successes and draws must not exceed population size");
  }
  return detail::sample_discrete_array<N>(
      detail::HypergeometricDistribution{population_size, success_count, draws},
      seed, options);
}

template <std::size_t N>
std::array<std::size_t, N> hypergeometric_array(
    std::size_t population_size, std::size_t success_count,
    std::size_t draws, std::mt19937_64 &engine,
    const GenerationOptions &options = {}) {
  if (success_count > population_size || draws > population_size) {
    throw std::invalid_argument(
        "hypergeometric successes and draws must not exceed population size");
  }
  return detail::sample_discrete_array<N>(
      detail::HypergeometricDistribution{population_size, success_count, draws},
      engine, options);
}

} // namespace data
} // namespace ntq
