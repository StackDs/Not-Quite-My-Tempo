#pragma once

#include "benchmark/benchmark_config.hpp"
#include "benchmark/benchmark_result.hpp"

namespace ntq {
namespace benchmark {

// Runs one caller-configured function repeatedly, collects metrics, calculates
// one group's statistics, and writes iteration and statistics CSV files.
BenchmarkResult run_benchmark(const BenchmarkCallbacks &callbacks,
                              const BenchmarkConfig &config);

} // namespace benchmark
} // namespace ntq
