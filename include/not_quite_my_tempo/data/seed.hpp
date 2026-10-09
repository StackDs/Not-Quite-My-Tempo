#pragma once

#include <cstdint>

namespace ntq::data {

using Seed = std::uint64_t;

Seed make_seed_from_entropy();

}  // namespace ntq::data
