#pragma once

#include "data/random_data_core.hpp"

namespace ntq {
namespace data {
namespace detail {

template <typename T>
T checked_finite_parameter(double value, const char *name) {
  if (!std::isfinite(value) ||
      static_cast<long double>(value) >
          static_cast<long double>((std::numeric_limits<T>::max)()) ||
      static_cast<long double>(value) <
          static_cast<long double>((std::numeric_limits<T>::lowest)())) {
    throw std::invalid_argument(std::string(name) + " must be finite");
  }
  return static_cast<T>(value);
}

template <typename T>
T checked_positive_parameter(double value, const char *name) {
  const T converted = checked_finite_parameter<T>(value, name);
  if (value <= 0.0 || converted <= T{0}) {
    throw std::invalid_argument(std::string(name) + " must be positive");
  }
  return converted;
}

template <typename T, typename Distribution>
struct PositiveOutputDistribution {
  Distribution distribution;

  T operator()(std::mt19937_64 &engine) {
    const T value = static_cast<T>(distribution(engine));
    if (std::isfinite(static_cast<long double>(value)) && value <= T{0}) {
      return std::nextafter(T{0}, T{1});
    }
    return value;
  }
};

template <typename T>
struct BetaDistribution {
  std::gamma_distribution<T> alpha;
  std::gamma_distribution<T> beta;
  T zero_fallback;

  T operator()(std::mt19937_64 &engine) {
    const T first = alpha(engine);
    const T second = beta(engine);
    if (!is_finite_value(first) || !is_finite_value(second)) {
      return std::numeric_limits<T>::infinity();
    }

    T value = zero_fallback;
    if (first == T{0} && second == T{0}) {
      value = zero_fallback;
    } else if (first >= second) {
      value = T{1} / (T{1} + second / first);
    } else {
      const T ratio = first / second;
      value = ratio / (T{1} + ratio);
    }
    if (value <= T{0}) {
      return std::nextafter(T{0}, T{1});
    }
    if (value >= T{1}) {
      return std::nextafter(T{1}, T{0});
    }
    return value;
  }
};

template <typename T>
struct ParetoDistribution {
  T xmin;
  T alpha;

  T operator()(std::mt19937_64 &engine) {
    std::uniform_real_distribution<T> unit(T{0}, T{1});
    const T uniform = unit(engine);
    return xmin / std::pow(T{1} - uniform, T{1} / alpha);
  }
};

} // namespace detail

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> exponential_vector(std::size_t count, double rate,
                                  std::uint64_t seed,
                                  const GenerationOptions &options = {}) {
  const T converted_rate = detail::checked_positive_parameter<T>(
      rate, "Exponential rate");
  using Distribution = std::exponential_distribution<T>;
  const detail::PositiveOutputDistribution<T, Distribution> distribution{
      Distribution(converted_rate)};
  return generate_vector<T>(count, distribution, seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> exponential_vector(std::size_t count, double rate,
                                  std::mt19937_64 &engine,
                                  const GenerationOptions &options = {}) {
  const T converted_rate = detail::checked_positive_parameter<T>(
      rate, "Exponential rate");
  using Distribution = std::exponential_distribution<T>;
  const detail::PositiveOutputDistribution<T, Distribution> distribution{
      Distribution(converted_rate)};
  return generate_vector<T>(count, distribution, engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> exponential_array(double rate, std::uint64_t seed,
                                   const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      exponential_vector<T>(N, rate, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> exponential_array(double rate, std::mt19937_64 &engine,
                                   const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      exponential_vector<T>(N, rate, engine, options));
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> gamma_vector(std::size_t count, double shape, double scale,
                            std::uint64_t seed,
                            const GenerationOptions &options = {}) {
  const T converted_shape =
      detail::checked_positive_parameter<T>(shape, "Gamma shape");
  const T converted_scale =
      detail::checked_positive_parameter<T>(scale, "Gamma scale");
  using Distribution = std::gamma_distribution<T>;
  const detail::PositiveOutputDistribution<T, Distribution> distribution{
      Distribution(converted_shape, converted_scale)};
  return generate_vector<T>(count, distribution, seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> gamma_vector(std::size_t count, double shape, double scale,
                            std::mt19937_64 &engine,
                            const GenerationOptions &options = {}) {
  const T converted_shape =
      detail::checked_positive_parameter<T>(shape, "Gamma shape");
  const T converted_scale =
      detail::checked_positive_parameter<T>(scale, "Gamma scale");
  using Distribution = std::gamma_distribution<T>;
  const detail::PositiveOutputDistribution<T, Distribution> distribution{
      Distribution(converted_shape, converted_scale)};
  return generate_vector<T>(count, distribution, engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> gamma_array(double shape, double scale, std::uint64_t seed,
                             const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(gamma_vector<T>(N, shape, scale, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> gamma_array(double shape, double scale,
                             std::mt19937_64 &engine,
                             const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      gamma_vector<T>(N, shape, scale, engine, options));
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> beta_vector(std::size_t count, double alpha, double beta,
                           std::uint64_t seed,
                           const GenerationOptions &options = {}) {
  const T converted_alpha =
      detail::checked_positive_parameter<T>(alpha, "Beta alpha");
  const T converted_beta =
      detail::checked_positive_parameter<T>(beta, "Beta beta");
  const long double alpha_ld = static_cast<long double>(converted_alpha);
  const long double beta_ld = static_cast<long double>(converted_beta);
  const T fallback = static_cast<T>(alpha_ld / (alpha_ld + beta_ld));
  const detail::BetaDistribution<T> distribution{
      std::gamma_distribution<T>(converted_alpha, T{1}),
      std::gamma_distribution<T>(converted_beta, T{1}), fallback};
  return generate_vector<T>(count, distribution, seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> beta_vector(std::size_t count, double alpha, double beta,
                           std::mt19937_64 &engine,
                           const GenerationOptions &options = {}) {
  const T converted_alpha =
      detail::checked_positive_parameter<T>(alpha, "Beta alpha");
  const T converted_beta =
      detail::checked_positive_parameter<T>(beta, "Beta beta");
  const long double alpha_ld = static_cast<long double>(converted_alpha);
  const long double beta_ld = static_cast<long double>(converted_beta);
  const T fallback = static_cast<T>(alpha_ld / (alpha_ld + beta_ld));
  const detail::BetaDistribution<T> distribution{
      std::gamma_distribution<T>(converted_alpha, T{1}),
      std::gamma_distribution<T>(converted_beta, T{1}), fallback};
  return generate_vector<T>(count, distribution, engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> beta_array(double alpha, double beta, std::uint64_t seed,
                            const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(beta_vector<T>(N, alpha, beta, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> beta_array(double alpha, double beta,
                            std::mt19937_64 &engine,
                            const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      beta_vector<T>(N, alpha, beta, engine, options));
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> lognormal_vector(std::size_t count, double log_mean,
                                double log_stddev, std::uint64_t seed,
                                const GenerationOptions &options = {}) {
  const T converted_mean =
      detail::checked_finite_parameter<T>(log_mean, "Lognormal log_mean");
  const T converted_stddev =
      detail::checked_positive_parameter<T>(log_stddev,
                                             "Lognormal log_stddev");
  using Distribution = std::lognormal_distribution<T>;
  const detail::PositiveOutputDistribution<T, Distribution> distribution{
      Distribution(converted_mean, converted_stddev)};
  return generate_vector<T>(count, distribution, seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> lognormal_vector(std::size_t count, double log_mean,
                                double log_stddev, std::mt19937_64 &engine,
                                const GenerationOptions &options = {}) {
  const T converted_mean =
      detail::checked_finite_parameter<T>(log_mean, "Lognormal log_mean");
  const T converted_stddev =
      detail::checked_positive_parameter<T>(log_stddev,
                                             "Lognormal log_stddev");
  using Distribution = std::lognormal_distribution<T>;
  const detail::PositiveOutputDistribution<T, Distribution> distribution{
      Distribution(converted_mean, converted_stddev)};
  return generate_vector<T>(count, distribution, engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> lognormal_array(double log_mean, double log_stddev,
                                 std::uint64_t seed,
                                 const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      lognormal_vector<T>(N, log_mean, log_stddev, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> lognormal_array(double log_mean, double log_stddev,
                                 std::mt19937_64 &engine,
                                 const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      lognormal_vector<T>(N, log_mean, log_stddev, engine, options));
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> weibull_vector(std::size_t count, double shape, double scale,
                              std::uint64_t seed,
                              const GenerationOptions &options = {}) {
  const T converted_shape =
      detail::checked_positive_parameter<T>(shape, "Weibull shape");
  const T converted_scale =
      detail::checked_positive_parameter<T>(scale, "Weibull scale");
  return generate_vector<T>(
      count, std::weibull_distribution<T>(converted_shape, converted_scale),
      seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> weibull_vector(std::size_t count, double shape, double scale,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {}) {
  const T converted_shape =
      detail::checked_positive_parameter<T>(shape, "Weibull shape");
  const T converted_scale =
      detail::checked_positive_parameter<T>(scale, "Weibull scale");
  return generate_vector<T>(
      count, std::weibull_distribution<T>(converted_shape, converted_scale),
      engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> weibull_array(double shape, double scale,
                               std::uint64_t seed,
                               const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      weibull_vector<T>(N, shape, scale, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> weibull_array(double shape, double scale,
                               std::mt19937_64 &engine,
                               const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      weibull_vector<T>(N, shape, scale, engine, options));
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> pareto_vector(std::size_t count, double xmin, double alpha,
                             std::uint64_t seed,
                             const GenerationOptions &options = {}) {
  const T converted_xmin =
      detail::checked_positive_parameter<T>(xmin, "Pareto xmin");
  const T converted_alpha =
      detail::checked_positive_parameter<T>(alpha, "Pareto alpha");
  const detail::PositiveOutputDistribution<
      T, detail::ParetoDistribution<T>> distribution{
      detail::ParetoDistribution<T>{converted_xmin, converted_alpha}};
  return generate_vector<T>(count, distribution, seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> pareto_vector(std::size_t count, double xmin, double alpha,
                             std::mt19937_64 &engine,
                             const GenerationOptions &options = {}) {
  const T converted_xmin =
      detail::checked_positive_parameter<T>(xmin, "Pareto xmin");
  const T converted_alpha =
      detail::checked_positive_parameter<T>(alpha, "Pareto alpha");
  const detail::PositiveOutputDistribution<
      T, detail::ParetoDistribution<T>> distribution{
      detail::ParetoDistribution<T>{converted_xmin, converted_alpha}};
  return generate_vector<T>(count, distribution, engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> pareto_array(double xmin, double alpha, std::uint64_t seed,
                              const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(pareto_vector<T>(N, xmin, alpha, seed, options));
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> pareto_array(double xmin, double alpha,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      pareto_vector<T>(N, xmin, alpha, engine, options));
}

} // namespace data
} // namespace ntq
