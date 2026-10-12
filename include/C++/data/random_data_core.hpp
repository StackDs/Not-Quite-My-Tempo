#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ntq {
namespace data {

enum class DuplicatePolicy { allow, forbid };
enum class DataPattern { random, sorted, reversed, nearly_sorted, repeated };

struct GenerationOptions {
  DuplicatePolicy duplicates{DuplicatePolicy::allow};
  DataPattern pattern{DataPattern::random};
  std::size_t max_attempts{10000};
};

inline std::uint64_t generate_seed() {
  std::random_device source;
  const std::uint64_t high = static_cast<std::uint64_t>(source());
  const std::uint64_t low = static_cast<std::uint64_t>(source());
  return (high << 32U) ^ low;
}

namespace detail {

template <typename T>
struct is_supported_number
    : std::integral_constant<bool, std::is_arithmetic<T>::value &&
                                       !std::is_same<T, bool>::value> {};

template <typename T>
bool is_finite_value(T value) {
  return std::isfinite(static_cast<long double>(value));
}

template <typename T>
bool domain_can_hold(std::size_t count, T minimum, T maximum,
                     std::true_type) {
  if (count == 0) {
    return true;
  }
  const std::uintmax_t distance =
      static_cast<std::uintmax_t>(maximum) -
      static_cast<std::uintmax_t>(minimum);
  return distance >= static_cast<std::uintmax_t>(count - 1);
}

template <typename T>
bool domain_can_hold(std::size_t, T, T, std::false_type) {
  return true;
}

inline void validate_pattern_options(const GenerationOptions &options) {
  if (options.duplicates == DuplicatePolicy::forbid &&
      options.pattern == DataPattern::repeated) {
    throw std::invalid_argument(
        "repeated pattern is incompatible with DuplicatePolicy::forbid");
  }
}

template <typename T>
void apply_pattern(std::vector<T> &values, DataPattern pattern,
                   std::mt19937_64 &engine) {
  if (pattern == DataPattern::sorted) {
    std::sort(values.begin(), values.end());
  } else if (pattern == DataPattern::reversed) {
    std::sort(values.begin(), values.end(), std::greater<T>());
  } else if (pattern == DataPattern::nearly_sorted) {
    std::sort(values.begin(), values.end());
    if (values.size() <= 1) {
      return;
    }
    std::uniform_int_distribution<std::size_t> index(0, values.size() - 1);
    const std::size_t swaps = (std::max)(std::size_t{1}, values.size() / 10);
    for (std::size_t swap = 0; swap < swaps; ++swap) {
      const std::size_t first = index(engine);
      const std::size_t second = index(engine);
      std::swap(values[first], values[second]);
    }
  }
}

template <typename T, typename Distribution>
std::vector<T> draw_values(std::size_t count, Distribution distribution,
                           std::mt19937_64 &engine,
                           const GenerationOptions &options) {
  static_assert(is_supported_number<T>::value,
                "generated values must use a non-bool numeric type");
  static_assert(std::is_convertible<decltype(distribution(engine)), T>::value,
                "distribution result must be convertible to T");
  validate_pattern_options(options);

  if (options.duplicates == DuplicatePolicy::forbid && count > 0 &&
      options.max_attempts < count) {
    throw std::runtime_error(
        "max_attempts is smaller than the number of requested values");
  }

  std::vector<T> values;
  values.reserve(count);
  if (options.pattern == DataPattern::repeated) {
    if (count != 0) {
      const T value = static_cast<T>(distribution(engine));
      if (!is_finite_value(value)) {
        throw std::invalid_argument("distribution generated a non-finite value");
      }
      values.assign(count, value);
    }
    return values;
  }

  std::set<T> unique_values;
  std::size_t attempts = 0;
  while (values.size() < count) {
    if (options.duplicates == DuplicatePolicy::forbid &&
        attempts >= options.max_attempts) {
      throw std::runtime_error("could not generate enough unique values");
    }
    ++attempts;
    const T value = static_cast<T>(distribution(engine));
    if (!is_finite_value(value)) {
      throw std::invalid_argument("distribution generated a non-finite value");
    }
    if (options.duplicates == DuplicatePolicy::forbid) {
      if (unique_values.insert(value).second) {
        values.push_back(value);
      }
    } else {
      values.push_back(value);
    }
  }

  apply_pattern(values, options.pattern, engine);
  return values;
}

template <typename T, typename Distribution>
std::vector<T> generate_vector_with_distribution(
    std::size_t count, Distribution distribution, std::mt19937_64 &engine,
    const GenerationOptions &options) {
  return draw_values<T>(count, distribution, engine, options);
}

template <typename T>
std::uniform_int_distribution<T> make_uniform_distribution(
    T minimum, T maximum, std::true_type) {
  return std::uniform_int_distribution<T>(minimum, maximum);
}

template <typename T>
std::uniform_real_distribution<T> make_uniform_distribution(
    T minimum, T maximum, std::false_type) {
  return std::uniform_real_distribution<T>(minimum, maximum);
}

template <typename T>
void validate_uniform_bounds(T minimum, T maximum, std::true_type) {
  if (minimum > maximum) {
    throw std::invalid_argument("uniform distribution minimum exceeds maximum");
  }
}

template <typename T>
void validate_uniform_bounds(T minimum, T maximum, std::false_type) {
  if (!is_finite_value(minimum) || !is_finite_value(maximum) ||
      minimum > maximum) {
    throw std::invalid_argument("uniform real bounds must be finite and ordered");
  }
}

template <typename T, std::size_t N>
std::array<T, N> to_array(std::vector<T> values) {
  std::array<T, N> result{};
  std::copy(values.begin(), values.end(), result.begin());
  return result;
}

} // namespace detail

template <typename T, typename Distribution>
std::vector<T> generate_vector(std::size_t count, Distribution distribution,
                               std::uint64_t seed,
                               const GenerationOptions &options = {}) {
  std::mt19937_64 engine(seed);
  return detail::generate_vector_with_distribution<T>(count, distribution,
                                                       engine, options);
}

template <typename T, typename Distribution>
std::vector<T> generate_vector(std::size_t count, Distribution distribution,
                               std::mt19937_64 &engine,
                               const GenerationOptions &options = {}) {
  return detail::generate_vector_with_distribution<T>(count, distribution,
                                                       engine, options);
}

template <typename T, std::size_t N, typename Distribution>
std::array<T, N> generate_array(Distribution distribution, std::uint64_t seed,
                                const GenerationOptions &options = {}) {
  std::mt19937_64 engine(seed);
  return detail::to_array<T, N>(
      detail::generate_vector_with_distribution<T>(N, distribution, engine,
                                                    options));
}

template <typename T, std::size_t N, typename Distribution>
std::array<T, N> generate_array(Distribution distribution,
                                std::mt19937_64 &engine,
                                const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      detail::generate_vector_with_distribution<T>(N, distribution, engine,
                                                    options));
}

template <typename T>
std::vector<T> uniform_vector(std::size_t count, T minimum, T maximum,
                              std::uint64_t seed,
                              const GenerationOptions &options = {}) {
  static_assert(detail::is_supported_number<T>::value,
                "uniform values must use a non-bool numeric type");
  using is_integral = std::is_integral<T>;
  detail::validate_uniform_bounds(minimum, maximum, is_integral{});
  if (options.duplicates == DuplicatePolicy::forbid &&
      !detail::domain_can_hold(count, minimum, maximum, is_integral{})) {
    throw std::invalid_argument("uniform integer range is too small for unique values");
  }
  auto distribution =
      detail::make_uniform_distribution(minimum, maximum, is_integral{});
  return generate_vector<T>(count, distribution, seed, options);
}

template <typename T>
std::vector<T> uniform_vector(std::size_t count, T minimum, T maximum,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {}) {
  static_assert(detail::is_supported_number<T>::value,
                "uniform values must use a non-bool numeric type");
  using is_integral = std::is_integral<T>;
  detail::validate_uniform_bounds(minimum, maximum, is_integral{});
  if (options.duplicates == DuplicatePolicy::forbid &&
      !detail::domain_can_hold(count, minimum, maximum, is_integral{})) {
    throw std::invalid_argument("uniform integer range is too small for unique values");
  }
  auto distribution =
      detail::make_uniform_distribution(minimum, maximum, is_integral{});
  return generate_vector<T>(count, distribution, engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> uniform_array(T minimum, T maximum, std::uint64_t seed,
                               const GenerationOptions &options = {}) {
  std::mt19937_64 engine(seed);
  return uniform_array<T, N>(minimum, maximum, engine, options);
}

template <typename T, std::size_t N>
std::array<T, N> uniform_array(T minimum, T maximum,
                               std::mt19937_64 &engine,
                               const GenerationOptions &options = {}) {
  static_assert(detail::is_supported_number<T>::value,
                "uniform values must use a non-bool numeric type");
  using is_integral = std::is_integral<T>;
  detail::validate_uniform_bounds(minimum, maximum, is_integral{});
  if (options.duplicates == DuplicatePolicy::forbid &&
      !detail::domain_can_hold(N, minimum, maximum, is_integral{})) {
    throw std::invalid_argument("uniform integer range is too small for unique values");
  }
  auto distribution =
      detail::make_uniform_distribution(minimum, maximum, is_integral{});
  return detail::to_array<T, N>(
      generate_vector<T>(N, distribution, engine, options));
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> normal_vector(std::size_t count, T mean,
                             T standard_deviation, std::uint64_t seed,
                             const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(mean) ||
      !detail::is_finite_value(standard_deviation) ||
      standard_deviation <= T{0}) {
    throw std::invalid_argument(
        "normal distribution requires a finite mean and positive finite deviation");
  }
  return generate_vector<T>(count,
                            std::normal_distribution<T>(mean, standard_deviation),
                            seed, options);
}

template <typename T, typename = typename std::enable_if<
                          std::is_floating_point<T>::value>::type>
std::vector<T> normal_vector(std::size_t count, T mean,
                             T standard_deviation, std::mt19937_64 &engine,
                             const GenerationOptions &options = {}) {
  if (!detail::is_finite_value(mean) ||
      !detail::is_finite_value(standard_deviation) ||
      standard_deviation <= T{0}) {
    throw std::invalid_argument(
        "normal distribution requires a finite mean and positive finite deviation");
  }
  return generate_vector<T>(count,
                            std::normal_distribution<T>(mean, standard_deviation),
                            engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> normal_array(T mean, T standard_deviation,
                              std::uint64_t seed,
                              const GenerationOptions &options = {}) {
  std::mt19937_64 engine(seed);
  return normal_array<T, N>(mean, standard_deviation, engine, options);
}

template <typename T, std::size_t N,
          typename = typename std::enable_if<
              std::is_floating_point<T>::value>::type>
std::array<T, N> normal_array(T mean, T standard_deviation,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {}) {
  return detail::to_array<T, N>(
      normal_vector<T>(N, mean, standard_deviation, engine, options));
}

} // namespace data
} // namespace ntq
