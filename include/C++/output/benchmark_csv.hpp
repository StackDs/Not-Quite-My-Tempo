#pragma once

#include "benchmark/benchmark_config.hpp"
#include "benchmark/benchmark_result.hpp"

namespace ntq {
namespace output {

// Writes one row per sample and a separate normalized statistics CSV.
void write_benchmark_csv(const benchmark::BenchmarkResult &result,
                         const benchmark::BenchmarkConfig &config);

} // namespace output
} // namespace ntq
