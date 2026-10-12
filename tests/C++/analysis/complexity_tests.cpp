#include "analysis/complexity.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <vector>

namespace {

long double basis(ntq::analysis::ComplexityModel model, std::size_t n) {
  const long double size = static_cast<long double>(n);
  switch (model) {
  case ntq::analysis::ComplexityModel::constant:
    return 1.0L;
  case ntq::analysis::ComplexityModel::logarithmic:
    return std::log(size);
  case ntq::analysis::ComplexityModel::square_root:
    return std::sqrt(size);
  case ntq::analysis::ComplexityModel::linear:
    return size;
  case ntq::analysis::ComplexityModel::linearithmic:
    return size * std::log(size);
  case ntq::analysis::ComplexityModel::n_log_log_n:
    return size * std::log(std::log(size));
  case ntq::analysis::ComplexityModel::quadratic:
    return size * size;
  case ntq::analysis::ComplexityModel::quadratic_logarithmic:
    return size * size * std::log(size);
  case ntq::analysis::ComplexityModel::cubic:
    return size * size * size;
  case ntq::analysis::ComplexityModel::exponential_2:
    return std::exp2(size);
  case ntq::analysis::ComplexityModel::factorial: {
    long double result = 1.0L;
    for (std::size_t factor = 2; factor <= n; ++factor) {
      result *= static_cast<long double>(factor);
    }
    return result;
  }
  }
  return 0.0L;
}

void test_insufficient_samples_and_order() {
  using namespace ntq::analysis;
  const std::vector<ComplexityModel> models = {
      ComplexityModel::cubic, ComplexityModel::linear,
      ComplexityModel::constant};
  const auto empty = analyze_complexity({}, models);
  assert(empty.size() == models.size());
  for (std::size_t index = 0; index < empty.size(); ++index) {
    assert(empty[index].status == AnalysisStatus::insufficient_samples);
    assert(empty[index].model == models[index]);
    assert(empty[index].observation_count == 0);
  }

  const auto one = analyze_complexity({{10, 1.0L}}, models);
  for (const ComplexityFit &fit : one) {
    assert(fit.status == AnalysisStatus::insufficient_samples);
    assert(fit.observation_count == 1);
  }
  assert(analyze_complexity({}, {}).empty());
}

void test_exact_model_fits() {
  using namespace ntq::analysis;
  const std::vector<ComplexityModel> models = {
      ComplexityModel::logarithmic, ComplexityModel::square_root,
      ComplexityModel::linear, ComplexityModel::linearithmic,
      ComplexityModel::n_log_log_n, ComplexityModel::quadratic,
      ComplexityModel::quadratic_logarithmic, ComplexityModel::cubic,
      ComplexityModel::exponential_2};
  const std::size_t sizes[] = {3, 4, 8, 16};
  for (ComplexityModel model : models) {
    std::vector<ComplexityObservation> observations;
    for (std::size_t size : sizes) {
      observations.push_back({size, 2.5L * basis(model, size)});
    }
    const auto fit = analyze_complexity(observations, {model}).front();
    assert(fit.status == AnalysisStatus::available);
    assert(std::fabs(fit.empirical_constant - 2.5L) < 1e-12L);
    assert(std::fabs(fit.r_squared - 1.0L) < 1e-12L);
  }

  const std::size_t factorial_sizes[] = {1, 2, 3, 4, 5};
  std::vector<ComplexityObservation> factorial_observations;
  for (std::size_t size : factorial_sizes) {
    factorial_observations.push_back(
        {size, 2.5L * basis(ComplexityModel::factorial, size)});
  }
  const auto factorial_fit = analyze_complexity(
      factorial_observations, {ComplexityModel::factorial}).front();
  assert(factorial_fit.status == AnalysisStatus::available);
  assert(std::fabs(factorial_fit.empirical_constant - 2.5L) < 1e-12L);
  assert(std::fabs(factorial_fit.r_squared - 1.0L) < 1e-12L);

  const auto constant_fit = analyze_complexity(
      {{2, 1.0L}, {4, 3.0L}, {8, 5.0L}},
      {ComplexityModel::constant}).front();
  assert(constant_fit.status == AnalysisStatus::available);
  assert(std::fabs(constant_fit.empirical_constant - 3.0L) < 1e-12L);
  assert(std::fabs(constant_fit.r_squared) < 1e-12L);
}

void test_invalid_and_singular_inputs() {
  using namespace ntq::analysis;
  const auto zero_size = analyze_complexity(
      {{0, 1.0L}, {2, 2.0L}}, {ComplexityModel::linear});
  assert(zero_size.front().status == AnalysisStatus::invalid_input);

  for (long double elapsed : {0.0L, -1.0L,
                              std::numeric_limits<long double>::quiet_NaN(),
                              std::numeric_limits<long double>::infinity()}) {
    const auto fit = analyze_complexity(
        {{1, elapsed}, {2, 2.0L}}, {ComplexityModel::linear});
    assert(fit.front().status == AnalysisStatus::invalid_input);
  }

  const auto log_singular = analyze_complexity(
      {{1, 1.0L}, {1, 2.0L}}, {ComplexityModel::logarithmic});
  assert(log_singular.front().status == AnalysisStatus::invalid_input);

  const auto n_log_log_small = analyze_complexity(
      {{2, 1.0L}, {3, 2.0L}}, {ComplexityModel::n_log_log_n});
  assert(n_log_log_small.front().status == AnalysisStatus::invalid_input);

  const auto constant_times = analyze_complexity(
      {{1, 4.0L}, {2, 4.0L}, {3, 4.0L}}, {ComplexityModel::linear});
  assert(constant_times.front().status == AnalysisStatus::undefined_r_squared);
  assert(std::fabs(constant_times.front().empirical_constant -
                   (12.0L / 7.0L)) < 1e-12L);

  const auto overflow = analyze_complexity(
      {{100000000, std::numeric_limits<long double>::max()},
       {100000001, std::numeric_limits<long double>::max() / 2.0L}},
      {ComplexityModel::cubic});
  assert(overflow.front().status == AnalysisStatus::invalid_input);

  const auto factorial_overflow = analyze_complexity(
      {{10000, 1.0L}, {10001, 2.0L}}, {ComplexityModel::factorial});
  assert(factorial_overflow.front().status == AnalysisStatus::invalid_input);

  const auto exponential_overflow = analyze_complexity(
      {{100000, 1.0L}, {100001, 2.0L}}, {ComplexityModel::exponential_2});
  assert(exponential_overflow.front().status == AnalysisStatus::invalid_input);
}

} // namespace

int main() {
  test_insufficient_samples_and_order();
  test_exact_model_fits();
  test_invalid_and_singular_inputs();
  return 0;
}
