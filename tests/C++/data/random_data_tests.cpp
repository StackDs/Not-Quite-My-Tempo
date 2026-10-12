#include "data/random_data.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

struct ConstantDistribution {
  using result_type = double;
  double value;

  double operator()(std::mt19937_64 &) { return value; }
};

template <typename Callable>
bool throws_invalid_argument(Callable callable) {
  try {
    callable();
  } catch (const std::invalid_argument &) {
    return true;
  }
  return false;
}

template <typename Callable>
bool throws_runtime_error(Callable callable) {
  try {
    callable();
  } catch (const std::runtime_error &) {
    return true;
  }
  return false;
}

void test_uniform_generation_and_reproducibility() {
  const auto first = ntq::data::uniform_vector<int>(64, -5, 5, 12345);
  const auto second = ntq::data::uniform_vector<int>(64, -5, 5, 12345);
  assert(first == second);
  assert(first.size() == 64);
  for (int value : first) {
    assert(value >= -5 && value <= 5);
  }

  const auto real_values =
      ntq::data::uniform_vector<double>(32, -1.0, 1.0, 9876);
  for (double value : real_values) {
    assert(value >= -1.0 && value < 1.0);
  }

  const auto fixed = ntq::data::uniform_array<int, 4>(2, 8, 44);
  assert(fixed.size() == 4);
  for (int value : fixed) {
    assert(value >= 2 && value <= 8);
  }

  assert(throws_invalid_argument([] {
    (void)ntq::data::uniform_vector<int>(3, 10, 1, 1);
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::uniform_vector<double>(3, 0.0,
                                             std::numeric_limits<double>::infinity(),
                                             1);
  }));
}

void test_seed_generation_and_shared_engine() {
  const std::uint64_t seed = ntq::data::generate_seed();
  const auto values = ntq::data::uniform_vector<int>(20, 1, 20, seed);
  assert(ntq::data::uniform_vector<int>(20, 1, 20, seed) == values);

  std::mt19937_64 engine(55);
  const auto first = ntq::data::uniform_vector<int>(10, 0, 9, engine);
  const auto second = ntq::data::uniform_vector<int>(10, 0, 9, engine);
  assert(first != second);

  std::mt19937_64 array_engine(21);
  const auto array = ntq::data::uniform_array<double, 3>(0.0, 1.0,
                                                        array_engine);
  assert(array.size() == 3);
}

void test_duplicate_policy_and_patterns() {
  using ntq::data::DataPattern;
  using ntq::data::DuplicatePolicy;
  using ntq::data::GenerationOptions;

  GenerationOptions allow_duplicates;
  const auto repeated_values =
      ntq::data::uniform_vector<int>(8, 3, 3, 20, allow_duplicates);
  for (int value : repeated_values) {
    assert(value == 3);
  }

  GenerationOptions unique;
  unique.duplicates = DuplicatePolicy::forbid;
  const auto unique_values = ntq::data::uniform_vector<int>(10, 1, 10, 10, unique);
  std::vector<int> sorted_unique = unique_values;
  std::sort(sorted_unique.begin(), sorted_unique.end());
  assert(std::adjacent_find(sorted_unique.begin(), sorted_unique.end()) ==
         sorted_unique.end());

  assert(throws_invalid_argument([] {
    GenerationOptions options;
    options.duplicates = DuplicatePolicy::forbid;
    (void)ntq::data::uniform_vector<int>(4, 1, 3, 2, options);
  }));

  GenerationOptions exhausted;
  exhausted.duplicates = DuplicatePolicy::forbid;
  exhausted.max_attempts = 12;
  assert(throws_runtime_error([&exhausted] {
    (void)ntq::data::generate_vector<double>(
        2, ConstantDistribution{4.0}, 1, exhausted);
  }));

  GenerationOptions sorted_options;
  sorted_options.pattern = DataPattern::sorted;
  const auto sorted = ntq::data::uniform_vector<int>(30, -20, 20, 4,
                                                     sorted_options);
  assert(std::is_sorted(sorted.begin(), sorted.end()));

  GenerationOptions reversed_options;
  reversed_options.pattern = DataPattern::reversed;
  const auto reversed = ntq::data::uniform_vector<int>(30, -20, 20, 4,
                                                       reversed_options);
  assert(std::is_sorted(reversed.begin(), reversed.end(), std::greater<int>()));

  GenerationOptions nearly_sorted_options;
  nearly_sorted_options.pattern = DataPattern::nearly_sorted;
  const auto nearly_sorted = ntq::data::uniform_vector<int>(30, 0, 10000, 4,
                                                             nearly_sorted_options);
  assert(nearly_sorted.size() == 30);
  GenerationOptions same_values_sorted_options;
  same_values_sorted_options.pattern = DataPattern::sorted;
  const auto same_values_sorted = ntq::data::uniform_vector<int>(
      30, 0, 10000, 4, same_values_sorted_options);
  assert(nearly_sorted != same_values_sorted);

  GenerationOptions repeated_options;
  repeated_options.pattern = DataPattern::repeated;
  const auto repeated = ntq::data::uniform_vector<int>(5, 1, 100, 9,
                                                       repeated_options);
  assert(std::all_of(repeated.begin(), repeated.end(), [&repeated](int value) {
    return value == repeated.front();
  }));

  repeated_options.duplicates = DuplicatePolicy::forbid;
  assert(throws_invalid_argument([&repeated_options] {
    (void)ntq::data::uniform_vector<int>(5, 1, 100, 9, repeated_options);
  }));
}

void test_normal_and_custom_distributions() {
  const auto normal_first = ntq::data::normal_vector<double>(40, 2.0, 0.5, 91);
  const auto normal_second = ntq::data::normal_vector<double>(40, 2.0, 0.5, 91);
  assert(normal_first == normal_second);
  assert(throws_invalid_argument([] {
    (void)ntq::data::normal_vector<double>(5, 0.0, 0.0, 1);
  }));

  const auto custom = ntq::data::generate_vector<float>(
      4, std::uniform_int_distribution<int>(2, 5), 3);
  for (float value : custom) {
    assert(value >= 2.0f && value <= 5.0f);
  }
  const auto custom_array =
      ntq::data::generate_array<float, 5>(std::uniform_int_distribution<int>(1, 8), 7);
  for (float value : custom_array) {
    assert(value >= 1.0f && value <= 8.0f);
  }

  std::mt19937_64 engine(100);
  const auto custom_engine = ntq::data::generate_vector<double>(
      6, std::uniform_real_distribution<double>(-2.0, 2.0), engine);
  assert(custom_engine.size() == 6);
  const auto normal_fixed = ntq::data::normal_array<double, 3>(1.0, 0.25, 5);
  assert(normal_fixed.size() == 3);

  assert(throws_invalid_argument([] {
    (void)ntq::data::generate_vector<double>(
        2, ConstantDistribution{std::numeric_limits<double>::quiet_NaN()}, 1);
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::generate_vector<double>(
        2, ConstantDistribution{std::numeric_limits<double>::infinity()}, 1);
  }));
}

void test_named_discrete_distributions() {
  const auto bernoulli_zero =
      ntq::data::bernoulli_vector(12, 0.0, std::uint64_t{71});
  const auto bernoulli_one =
      ntq::data::bernoulli_vector(12, 1.0, std::uint64_t{71});
  assert(std::all_of(bernoulli_zero.begin(), bernoulli_zero.end(),
                     [](std::size_t value) { return value == 0; }));
  assert(std::all_of(bernoulli_one.begin(), bernoulli_one.end(),
                     [](std::size_t value) { return value == 1; }));
  assert(ntq::data::bernoulli_vector(40, 0.35, std::uint64_t{8}) ==
         ntq::data::bernoulli_vector(40, 0.35, std::uint64_t{8}));
  const auto bernoulli_array = ntq::data::bernoulli_array<6>(0.0, 42);
  assert(std::all_of(bernoulli_array.begin(), bernoulli_array.end(),
                     [](std::size_t value) { return value == 0; }));

  const auto binomial_zero =
      ntq::data::binomial_vector(8, 0, 0.5, std::uint64_t{5});
  const auto binomial_failures =
      ntq::data::binomial_vector(8, 10, 0.0, std::uint64_t{5});
  const auto binomial_successes =
      ntq::data::binomial_vector(8, 10, 1.0, std::uint64_t{5});
  const auto binomial_samples =
      ntq::data::binomial_vector(50, 10, 0.25, std::uint64_t{6});
  assert(std::all_of(binomial_zero.begin(), binomial_zero.end(),
                     [](std::size_t value) { return value == 0; }));
  assert(std::all_of(binomial_failures.begin(), binomial_failures.end(),
                     [](std::size_t value) { return value == 0; }));
  assert(std::all_of(binomial_successes.begin(), binomial_successes.end(),
                     [](std::size_t value) { return value == 10; }));
  assert(std::all_of(binomial_samples.begin(), binomial_samples.end(),
                     [](std::size_t value) { return value <= 10; }));
  const auto binomial_array =
      ntq::data::binomial_array<4>(5, 1.0, std::uint64_t{9});
  assert(std::all_of(binomial_array.begin(), binomial_array.end(),
                     [](std::size_t value) { return value == 5; }));

  const auto geometric =
      ntq::data::geometric_vector(20, 1.0, std::uint64_t{11});
  assert(std::all_of(geometric.begin(), geometric.end(),
                     [](std::size_t value) { return value == 1; }));
  assert(ntq::data::geometric_vector(20, 0.4, std::uint64_t{12}) ==
         ntq::data::geometric_vector(20, 0.4, std::uint64_t{12}));
  const auto geometric_array =
      ntq::data::geometric_array<3>(1.0, std::uint64_t{13});
  assert(std::all_of(geometric_array.begin(), geometric_array.end(),
                     [](std::size_t value) { return value == 1; }));

  const auto poisson_zero =
      ntq::data::poisson_vector(15, 0.0, std::uint64_t{14});
  assert(std::all_of(poisson_zero.begin(), poisson_zero.end(),
                     [](std::size_t value) { return value == 0; }));
  const auto poisson =
      ntq::data::poisson_array<8>(3.0, std::uint64_t{15});
  assert(std::all_of(poisson.begin(), poisson.end(),
                     [](std::size_t value) { return value <= 20; }));

  const auto empty_population = ntq::data::hypergeometric_vector(
      3, 0, 0, 0, std::uint64_t{16});
  assert(std::all_of(empty_population.begin(), empty_population.end(),
                     [](std::size_t value) { return value == 0; }));
  const auto hypergeometric = ntq::data::hypergeometric_vector(
      50, 10, 6, 4, std::uint64_t{17});
  for (std::size_t successes : hypergeometric) {
    assert(successes <= 4);
  }
  const auto hypergeometric_array =
      ntq::data::hypergeometric_array<5>(8, 6, 6, std::uint64_t{18});
  for (std::size_t successes : hypergeometric_array) {
    assert(successes >= 4 && successes <= 6);
  }

  std::mt19937_64 discrete_engine(19);
  assert(ntq::data::bernoulli_vector(3, 0.5, discrete_engine).size() == 3);
  assert(ntq::data::binomial_array<3>(4, 0.5, discrete_engine).size() == 3);
  assert(ntq::data::geometric_vector(3, 0.5, discrete_engine).size() == 3);
  assert(ntq::data::poisson_array<3>(2.0, discrete_engine).size() == 3);
  assert(ntq::data::hypergeometric_vector(3, 10, 4, 2, discrete_engine).size() ==
         3);

  assert(throws_invalid_argument([] {
    (void)ntq::data::bernoulli_vector(2, -0.1, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::binomial_vector(2, 3, 1.1, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::geometric_vector(2, 0.0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::poisson_vector(2, -1.0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::hypergeometric_vector(2, 5, 6, 1, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::hypergeometric_vector(2, 5, 2, 6, std::uint64_t{1});
  }));
}

void test_named_continuous_distributions() {
  const auto exponential =
      ntq::data::exponential_vector<double>(32, 1.5, std::uint64_t{31});
  assert(exponential == ntq::data::exponential_vector<double>(
                            32, 1.5, std::uint64_t{31}));
  for (double value : exponential) {
    assert(value > 0.0);
  }
  const auto exponential_array =
      ntq::data::exponential_array<float, 4>(2.0, std::uint64_t{32});
  for (float value : exponential_array) {
    assert(value > 0.0f);
  }

  const auto gamma = ntq::data::gamma_vector<double>(32, 2.0, 3.0, 33);
  for (double value : gamma) {
    assert(value > 0.0);
  }
  const auto beta = ntq::data::beta_vector<double>(64, 2.0, 5.0, 34);
  for (double value : beta) {
    assert(value > 0.0 && value < 1.0);
  }
  const auto beta_array = ntq::data::beta_array<float, 4>(2.0, 2.0, 35);
  for (float value : beta_array) {
    assert(value > 0.0f && value < 1.0f);
  }

  const auto lognormal =
      ntq::data::lognormal_vector<double>(32, 0.0, 0.5, 36);
  for (double value : lognormal) {
    assert(value > 0.0);
  }
  const auto weibull =
      ntq::data::weibull_vector<double>(32, 1.5, 2.0, 37);
  for (double value : weibull) {
    assert(value >= 0.0);
  }
  const auto pareto = ntq::data::pareto_vector<double>(32, 2.0, 3.0, 38);
  for (double value : pareto) {
    assert(value >= 2.0 && std::isfinite(value));
  }

  assert(throws_invalid_argument([] {
    (void)ntq::data::exponential_vector<double>(2, 0.0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::gamma_vector<double>(2, 1.0, -1.0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::beta_vector<double>(2, 0.0, 2.0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::lognormal_vector<double>(2, 0.0,
                                                std::numeric_limits<double>::infinity(),
                                                std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::weibull_vector<double>(2, 1.0, 0.0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::pareto_vector<double>(2, 0.0, 2.0, std::uint64_t{1});
  }));

  std::mt19937_64 continuous_engine(39);
  assert(ntq::data::exponential_vector<double>(3, 1.0, continuous_engine).size() ==
         3);
  assert(ntq::data::gamma_array<double, 3>(2.0, 1.0, continuous_engine).size() ==
         3);
  assert(ntq::data::beta_vector<double>(3, 2.0, 2.0, continuous_engine).size() ==
         3);
  assert(ntq::data::lognormal_array<double, 3>(0.0, 1.0, continuous_engine).size() ==
         3);
  assert(ntq::data::weibull_vector<double>(3, 1.0, 1.0, continuous_engine).size() ==
         3);
  assert(ntq::data::pareto_array<double, 3>(1.0, 2.0, continuous_engine).size() ==
         3);
}

void test_zipf_power_law_categorical_and_mixtures() {
  const auto zipf = ntq::data::zipf_vector(100, 1.2, 8, std::uint64_t{41});
  assert(zipf == ntq::data::zipf_vector(100, 1.2, 8, std::uint64_t{41}));
  for (std::size_t rank : zipf) {
    assert(rank >= 1 && rank <= 8);
  }
  const auto zipf_array = ntq::data::zipf_array<5>(1.0, 4, std::uint64_t{42});
  for (std::size_t rank : zipf_array) {
    assert(rank >= 1 && rank <= 4);
  }

  const auto power_law =
      ntq::data::power_law_vector<double>(100, 1.0, 2.0, 5.0, 43);
  for (double value : power_law) {
    assert(value >= 2.0 && value <= 5.0);
  }
  const auto power_law_array =
      ntq::data::power_law_array<float, 5>(2.5, 0.5, 4.0, 44);
  for (float value : power_law_array) {
    assert(value >= 0.5f && value <= 4.0f);
  }

  const std::vector<int> categories{10, 20, 30};
  const std::vector<double> category_weights{0.0, 0.0, 1.0};
  const auto categorical = ntq::data::categorical_vector<int>(
      20, categories, category_weights, 45);
  assert(std::all_of(categorical.begin(), categorical.end(),
                     [](int value) { return value == 30; }));
  const auto categorical_array = ntq::data::categorical_array<int, 4>(
      categories, category_weights, 46);
  assert(std::all_of(categorical_array.begin(), categorical_array.end(),
                     [](int value) { return value == 30; }));

  using ntq::data::make_mixture_component;
  using ntq::data::MixtureComponent;
  const std::vector<MixtureComponent<double>> components{
      make_mixture_component<double>(0.0, ConstantDistribution{10.0}),
      make_mixture_component<double>(1.0, ConstantDistribution{20.0})};
  const auto mixture = ntq::data::mixture_vector<double>(
      20, components, std::uint64_t{47});
  assert(std::all_of(mixture.begin(), mixture.end(),
                     [](double value) { return value == 20.0; }));
  const auto mixture_array = ntq::data::mixture_array<double, 3>(
      components, std::uint64_t{48});
  assert(std::all_of(mixture_array.begin(), mixture_array.end(),
                     [](double value) { return value == 20.0; }));

  std::mt19937_64 composition_engine(49);
  assert(ntq::data::zipf_vector(4, 1.1, 8, composition_engine).size() == 4);
  assert(ntq::data::power_law_array<double, 4>(1.0, 1.0, 3.0,
                                                composition_engine)
             .size() == 4);
  assert(ntq::data::categorical_vector<int>(4, categories, category_weights,
                                            composition_engine)
             .size() == 4);
  assert(ntq::data::mixture_vector<double>(4, components, composition_engine)
             .size() == 4);

  assert(throws_invalid_argument([] {
    (void)ntq::data::zipf_vector(2, 0.0, 4, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::zipf_vector(2, 1.0, 0, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::power_law_vector<double>(2, 2.0, 1.0, 1.0,
                                               std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::categorical_vector<int>(
        2, std::vector<int>{1}, std::vector<double>{1.0, 2.0},
        std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::categorical_vector<int>(
        2, std::vector<int>{1, 2}, std::vector<double>{-1.0, 1.0},
        std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::categorical_vector<int>(
        2, std::vector<int>{1, 2}, std::vector<double>{0.0, 0.0},
        std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::mixture_vector<double>(
        2, std::vector<MixtureComponent<double>>{}, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    MixtureComponent<double> empty_sampler;
    empty_sampler.weight = 1.0;
    (void)ntq::data::mixture_vector<double>(
        2, std::vector<MixtureComponent<double>>{empty_sampler},
        std::uint64_t{1});
  }));
}

void test_dataset_profiles() {
  const auto constant = ntq::data::constant_vector<double>(8, 2.5);
  assert(std::all_of(constant.begin(), constant.end(),
                     [](double value) { return value == 2.5; }));
  const auto constant_array = ntq::data::constant_array<int, 4>(-3);
  assert(std::all_of(constant_array.begin(), constant_array.end(),
                     [](int value) { return value == -3; }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::constant_vector<double>(
        2, std::numeric_limits<double>::infinity());
  }));

  const std::vector<int> palette{2, 4, 8};
  const auto low_cardinality =
      ntq::data::low_cardinality_vector<int>(50, palette, 51);
  for (int value : low_cardinality) {
    assert(value == 2 || value == 4 || value == 8);
  }
  const auto low_cardinality_array = ntq::data::low_cardinality_array<int, 4>(
      palette, std::vector<double>{0.0, 1.0, 0.0}, 52);
  assert(std::all_of(low_cardinality_array.begin(), low_cardinality_array.end(),
                     [](int value) { return value == 4; }));

  const auto clustered = ntq::data::clustered_vector<double>(
      30, std::vector<double>{-10.0, 10.0},
      std::vector<double>{0.1, 0.1}, std::vector<double>{0.0, 1.0}, 53);
  for (double value : clustered) {
    assert(value > 9.0 && value < 11.0);
  }
  const auto clustered_array = ntq::data::clustered_array<float, 4>(
      std::vector<float>{5.0f}, std::vector<float>{0.25f},
      std::vector<double>{1.0}, 54);
  for (float value : clustered_array) {
    assert(value > 4.0f && value < 6.0f);
  }

  const auto all_zero = ntq::data::sparse_vector<double>(
      10, 1.0, ConstantDistribution{3.0}, std::uint64_t{55});
  assert(std::all_of(all_zero.begin(), all_zero.end(),
                     [](double value) { return value == 0.0; }));
  const auto all_nonzero = ntq::data::sparse_vector<double>(
      10, 0.0, ConstantDistribution{3.0}, std::uint64_t{56});
  assert(std::all_of(all_nonzero.begin(), all_nonzero.end(),
                     [](double value) { return value == 3.0; }));
  const auto sparse_array = ntq::data::sparse_array<double, 4>(
      1.0, ConstantDistribution{3.0}, std::uint64_t{57});
  assert(std::all_of(sparse_array.begin(), sparse_array.end(),
                     [](double value) { return value == 0.0; }));
  const auto sparse_mixed = ntq::data::sparse_vector<double>(
      100, 0.5, ConstantDistribution{3.0}, std::uint64_t{58});
  const std::size_t zero_count = static_cast<std::size_t>(std::count(
      sparse_mixed.begin(), sparse_mixed.end(), 0.0));
  assert(zero_count > 30 && zero_count < 70);
  std::mt19937_64 profile_engine(59);
  assert(ntq::data::low_cardinality_vector<int>(4, palette, profile_engine)
             .size() == 4);
  assert(ntq::data::clustered_array<double, 4>(
             std::vector<double>{2.0}, std::vector<double>{0.1},
             std::vector<double>{1.0}, profile_engine)
             .size() == 4);
  assert(ntq::data::sparse_vector<double>(
             4, 0.5, ConstantDistribution{1.0}, profile_engine)
             .size() == 4);

  ntq::data::GenerationOptions unique;
  unique.duplicates = ntq::data::DuplicatePolicy::forbid;
  const auto high_cardinality =
      ntq::data::uniform_vector<int>(10, 0, 9, 58, unique);
  std::vector<int> ordered = high_cardinality;
  std::sort(ordered.begin(), ordered.end());
  assert(std::adjacent_find(ordered.begin(), ordered.end()) == ordered.end());

  assert(throws_invalid_argument([] {
    (void)ntq::data::low_cardinality_vector<int>(
        2, std::vector<int>{}, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::clustered_vector<double>(
        2, std::vector<double>{0.0}, std::vector<double>{0.1, 0.2},
        std::vector<double>{1.0}, std::uint64_t{1});
  }));
  assert(throws_invalid_argument([] {
    (void)ntq::data::sparse_vector<double>(
        2, 1.1, ConstantDistribution{3.0}, std::uint64_t{1});
  }));
}

} // namespace

int main() {
  test_uniform_generation_and_reproducibility();
  test_seed_generation_and_shared_engine();
  test_duplicate_policy_and_patterns();
  test_normal_and_custom_distributions();
  test_named_discrete_distributions();
  test_named_continuous_distributions();
  test_zipf_power_law_categorical_and_mixtures();
  test_dataset_profiles();
  return 0;
}
