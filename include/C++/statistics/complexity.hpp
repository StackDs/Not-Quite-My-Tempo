// Public C++ complexity analysis interface.
#pragma once

#include <cstddef>
#include <vector>

namespace ntq::statistics {

enum class ComplexityModel {
    linear,
    linearithmic,
    quadratic,
    cubic,
    logarithmic,
    constant
};

struct ComplexityFit {
    ComplexityModel model{ComplexityModel::linear};
    double r_squared{0.0};
    double empirical_constant{0.0};
};

ComplexityFit fit_complexity(
    const std::vector<std::size_t>& input_sizes,
    const std::vector<double>& median_times,
    ComplexityModel model
);

}  // namespace ntq::statistics
