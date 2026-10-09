#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace ntq {

struct RunMetadata {
    std::map<std::string, std::string> fields{};
    bool has_seed{false};
    std::uint64_t seed{0};
    std::vector<std::uint64_t> sample_seeds{};
};

}  // namespace ntq
