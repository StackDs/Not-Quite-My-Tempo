// Public C++ vector-generation API.
#pragma once

#include "random_generator.hpp"

#include <cstddef>
#include <vector>

namespace ntq::data {

std::vector<int> uniform_integer_vector(
    RandomGenerator& generator,
    std::size_t size,
    int minimum,
    int maximum
);

std::vector<double> uniform_real_vector(
    RandomGenerator& generator,
    std::size_t size,
    double minimum,
    double maximum
);

std::vector<double> normal_vector(
    RandomGenerator& generator,
    std::size_t size,
    double mean,
    double standard_deviation
);

std::vector<bool> bernoulli_vector(
    RandomGenerator& generator,
    std::size_t size,
    double probability
);

}  // namespace ntq::data
