#include "analysis/complexity.hpp"

#include <cmath>
#include <limits>
#include <vector>

namespace ntq {
namespace analysis {
namespace {

bool model_basis(ComplexityModel model, std::size_t input_size,
                 long double &basis) {
  const long double n = static_cast<long double>(input_size);
  switch (model) {
  case ComplexityModel::constant:
    basis = 1.0L;
    break;
  case ComplexityModel::logarithmic:
    basis = std::log(n);
    break;
  case ComplexityModel::square_root:
    basis = std::sqrt(n);
    break;
  case ComplexityModel::linear:
    basis = n;
    break;
  case ComplexityModel::linearithmic:
    basis = n * std::log(n);
    break;
  case ComplexityModel::n_log_log_n:
    // ln(ln(N)) is positive for integer input sizes N >= 3.
    if (input_size < 3) {
      return false;
    }
    basis = n * std::log(std::log(n));
    break;
  case ComplexityModel::quadratic:
    basis = n * n;
    break;
  case ComplexityModel::quadratic_logarithmic:
    basis = n * n * std::log(n);
    break;
  case ComplexityModel::cubic:
    basis = n * n * n;
    break;
  case ComplexityModel::exponential_2:
    basis = std::exp2(n);
    break;
  case ComplexityModel::factorial:
    basis = 1.0L;
    for (std::size_t factor = 2; factor <= input_size; ++factor) {
      const long double multiplier = static_cast<long double>(factor);
      if (basis > std::numeric_limits<long double>::max() / multiplier) {
        return false;
      }
      basis *= multiplier;
    }
    break;
  default:
    return false;
  }
  return std::isfinite(basis) && basis >= 0.0L;
}

ComplexityFit invalid_fit(ComplexityModel model, std::size_t count) {
  ComplexityFit fit;
  fit.model = model;
  fit.status = AnalysisStatus::invalid_input;
  fit.observation_count = count;
  return fit;
}

ComplexityFit fit_model(const std::vector<ComplexityObservation> &observations,
                        ComplexityModel model) {
  ComplexityFit fit;
  fit.model = model;
  fit.observation_count = observations.size();
  if (observations.size() < 2) {
    fit.status = AnalysisStatus::insufficient_samples;
    return fit;
  }

  std::vector<long double> bases;
  bases.reserve(observations.size());
  long double basis_time_sum = 0.0L;
  long double basis_square_sum = 0.0L;
  long double time_sum = 0.0L;
  for (const ComplexityObservation &observation : observations) {
    if (observation.input_size == 0 ||
        !std::isfinite(observation.elapsed_time) ||
        observation.elapsed_time <= 0.0L) {
      return invalid_fit(model, observations.size());
    }
    long double basis = 0.0L;
    if (!model_basis(model, observation.input_size, basis)) {
      return invalid_fit(model, observations.size());
    }
    const long double basis_time = basis * observation.elapsed_time;
    const long double basis_square = basis * basis;
    const long double next_basis_time_sum = basis_time_sum + basis_time;
    const long double next_basis_square_sum = basis_square_sum + basis_square;
    const long double next_time_sum = time_sum + observation.elapsed_time;
    if (!std::isfinite(basis_time) || !std::isfinite(basis_square) ||
        !std::isfinite(next_basis_time_sum) ||
        !std::isfinite(next_basis_square_sum) || !std::isfinite(next_time_sum)) {
      return invalid_fit(model, observations.size());
    }
    bases.push_back(basis);
    basis_time_sum = next_basis_time_sum;
    basis_square_sum = next_basis_square_sum;
    time_sum = next_time_sum;
  }

  if (basis_square_sum == 0.0L) {
    return invalid_fit(model, observations.size());
  }
  const long double coefficient = basis_time_sum / basis_square_sum;
  if (!std::isfinite(coefficient)) {
    return invalid_fit(model, observations.size());
  }
  fit.empirical_constant = coefficient;

  const long double mean = time_sum / static_cast<long double>(observations.size());
  long double total_sum_squares = 0.0L;
  long double residual_sum_squares = 0.0L;
  for (std::size_t index = 0; index < observations.size(); ++index) {
    const long double centered = observations[index].elapsed_time - mean;
    const long double residual = observations[index].elapsed_time -
                                 coefficient * bases[index];
    const long double next_total = total_sum_squares + centered * centered;
    const long double next_residual =
        residual_sum_squares + residual * residual;
    if (!std::isfinite(residual) || !std::isfinite(next_total) ||
        !std::isfinite(next_residual)) {
      return invalid_fit(model, observations.size());
    }
    total_sum_squares = next_total;
    residual_sum_squares = next_residual;
  }

  if (total_sum_squares == 0.0L) {
    fit.status = AnalysisStatus::undefined_r_squared;
    return fit;
  }
  fit.r_squared = 1.0L - residual_sum_squares / total_sum_squares;
  if (!std::isfinite(fit.r_squared)) {
    return invalid_fit(model, observations.size());
  }
  fit.status = AnalysisStatus::available;
  return fit;
}

} // namespace

std::vector<ComplexityFit> analyze_complexity(
    const std::vector<ComplexityObservation> &observations,
    const std::vector<ComplexityModel> &models) {
  std::vector<ComplexityFit> fits;
  fits.reserve(models.size());
  for (ComplexityModel model : models) {
    fits.push_back(fit_model(observations, model));
  }
  return fits;
}

} // namespace analysis
} // namespace ntq
