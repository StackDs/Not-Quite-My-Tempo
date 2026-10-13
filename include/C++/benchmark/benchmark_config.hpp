#pragma once

#include "statistics/statistics.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace ntq {
namespace benchmark {

enum class ExecutionMode { in_process, isolated_process };

struct BenchmarkIterationContext {
  std::size_t iteration_index{0};
  std::size_t input_size{1};
  std::uint64_t seed{0};
  bool is_warmup{false};
};

// `prepare` runs before timing so callers can rebuild mutable input per run.
// `function` is the single algorithm callback that the runner measures.
struct BenchmarkCallbacks {
  std::function<void(const BenchmarkIterationContext &)> prepare;
  std::function<void()> function;
};

struct BenchmarkConfig {
  // Total callback invocations, including the configured warm-up prefix.
  std::size_t total_iterations{1};
  std::size_t input_size{1};
  std::uint64_t seed{42};
  TimeUnit time_unit{TimeUnit::nanoseconds};
  long max_involuntary_context_switches{0};
  ExecutionMode execution_mode{ExecutionMode::in_process};

  std::string group_key;
  std::string variant_key;
  // Use the same prefix and seed in separate variant calls to create matching
  // pair IDs for equivalent iterations.
  std::string pair_id_prefix;

  std::string iterations_csv_path;
  std::string statistics_csv_path;
  statistics::StatisticsOptions statistics_options;
  statistics::RunMetadata metadata;
};

} // namespace benchmark
} // namespace ntq
