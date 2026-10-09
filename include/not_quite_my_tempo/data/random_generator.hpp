#pragma once

#include "not_quite_my_tempo/data/seed.hpp"

#include <random>

namespace ntq::data {

class RandomGenerator {
public:
    explicit RandomGenerator(Seed seed);

    Seed seed() const noexcept;

private:
    Seed seed_;
    std::mt19937_64 engine_;
};

}  // namespace ntq::data
