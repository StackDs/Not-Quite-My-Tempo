#pragma once

#include "not_quite_my_tempo/types.hpp"
#include "not_quite_my_tempo/metadata.hpp"

#include <cstddef>
#include <map>
#include <string>

namespace ntq {

struct BenchmarkConfig {
    std::size_t warmup_runs{0};
    std::size_t measurement_runs{1};
    TimeUnit output_time_unit{TimeUnit::nanoseconds};
    bool collect_process_metrics{true};
    bool collect_hardware_metrics{false};
    std::string csv_path{};
    std::map<std::string, std::string> metadata{};
    RunMetadata run_metadata{};
};

}  // namespace ntq
