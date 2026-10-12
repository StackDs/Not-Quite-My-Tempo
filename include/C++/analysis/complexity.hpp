#pragma once

#include <cstddef>
#include <vector>

namespace ntq {
namespace analysis {

enum class ComplexityModel {
  constant,
  logarithmic,
  square_root,
  linear,
  linearithmic,
  n_log_log_n,
  quadratic,
  quadratic_logarithmic,
  cubic,
  exponential_2,
  factorial
};

enum class AnalysisStatus {
  available,
  insufficient_samples,
  invalid_input,
  undefined_r_squared
};

struct ComplexityObservation {
  std::size_t input_size{0};
  long double elapsed_time{0.0L};
};

struct ComplexityFit {
  ComplexityModel model{ComplexityModel::constant};
  AnalysisStatus status{AnalysisStatus::insufficient_samples};
  long double empirical_constant{0.0L};
  long double r_squared{0.0L};
  std::size_t observation_count{0};
};

// Fits elapsed_time ~= c * f(input_size) without an additive intercept.
// Invalid input is reported per model; no observation or model is modified.
std::vector<ComplexityFit> analyze_complexity(
    const std::vector<ComplexityObservation> &observations,
    const std::vector<ComplexityModel> &models);

} // namespace analysis
} // namespace ntq
