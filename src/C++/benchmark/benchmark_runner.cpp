#include "benchmark/benchmark_runner.hpp"

#include "metrics/time_measurement.hpp"
#include "output/benchmark_csv.hpp"
#include "system/system_info.hpp"

#include <chrono>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace ntq {
namespace benchmark {
namespace {

bool valid_time_unit(TimeUnit unit) {
  switch (unit) {
  case TimeUnit::nanoseconds:
  case TimeUnit::microseconds:
  case TimeUnit::milliseconds:
  case TimeUnit::seconds:
  case TimeUnit::minutes:
  case TimeUnit::hours:
    return true;
  }
  return false;
}

void validate_config(const BenchmarkCallbacks &callbacks,
                     const BenchmarkConfig &config) {
  if (!callbacks.function) {
    throw std::invalid_argument(
        "benchmark function callback must not be empty");
  }
  if (config.total_iterations == 0 ||
      config.statistics_options.warmup_runs_per_group >=
          config.total_iterations) {
    throw std::invalid_argument(
        "total_iterations must exceed the configured warm-up count");
  }
  if (config.max_involuntary_context_switches < 0) {
    throw std::invalid_argument(
        "max_involuntary_context_switches must not be negative");
  }
  if (!valid_time_unit(config.time_unit)) {
    throw std::invalid_argument("invalid benchmark time unit");
  }
  if (config.iterations_csv_path.empty() ||
      config.statistics_csv_path.empty() ||
      config.iterations_csv_path == config.statistics_csv_path) {
    throw std::invalid_argument(
        "benchmark requires distinct iteration and statistics CSV paths");
  }
  if (!config.statistics_options.comparisons.empty()) {
    throw std::invalid_argument(
        "comparisons require samples from multiple benchmark calls; combine "
        "samples and call calculate_statistics directly");
  }
  if (!std::isfinite(config.statistics_options.confidence_level) ||
      config.statistics_options.confidence_level <= 0.0 ||
      config.statistics_options.confidence_level >= 1.0 ||
      config.statistics_options.bootstrap_resamples == 0 ||
      !std::isfinite(
          config.statistics_options.minimum_expected_tail_observations) ||
      config.statistics_options.minimum_expected_tail_observations < 0.0L) {
    throw std::invalid_argument(
        "invalid statistics options in benchmark config");
  }
  switch (config.execution_mode) {
  case ExecutionMode::in_process:
  case ExecutionMode::isolated_process:
    break;
  default:
    throw std::invalid_argument("invalid benchmark execution mode");
  }
}

std::string resolved_group_key(const BenchmarkConfig &config) {
  return config.group_key.empty()
             ? std::string("input_size=") + std::to_string(config.input_size)
             : config.group_key;
}

std::string resolved_variant_key(const BenchmarkConfig &config) {
  return config.variant_key.empty() ? "function" : config.variant_key;
}

std::string make_run_id(const BenchmarkConfig &config) {
  if (!config.metadata.run_id.empty()) {
    return config.metadata.run_id;
  }
  const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
  return std::string("run-") + std::to_string(config.seed) + "-" +
         std::to_string(now);
}

TimeMeasurement measure_callback(const BenchmarkCallbacks &callbacks,
                                 const BenchmarkConfig &config) {
  if (config.execution_mode == ExecutionMode::isolated_process) {
    return TimeMeasurement::measure_in_isolated_process(
        callbacks.function, config.max_involuntary_context_switches);
  }

  TimeMeasurement measurement(config.max_involuntary_context_switches);
  measurement.start();
  callbacks.function();
  measurement.stop();
  measurement.elapsed();
  return measurement;
}

} // namespace

BenchmarkResult run_benchmark(const BenchmarkCallbacks &callbacks,
                              const BenchmarkConfig &config) {
  validate_config(callbacks, config);

  BenchmarkResult result;
  result.input_size = config.input_size;
  result.iterations_csv_path = config.iterations_csv_path;
  result.statistics_csv_path = config.statistics_csv_path;
  result.samples.reserve(config.total_iterations);
  result.iteration_seeds.reserve(config.total_iterations);

  statistics::RunMetadata metadata = config.metadata;
  metadata.run_id = make_run_id(config);
  metadata.configured_iterations = config.total_iterations;
  metadata.clock_name = "CLOCK_MONOTONIC";
  metadata.clock_resolution = TimeMeasurement::reported_clock_resolution();
  metadata.environment.before.system = collect_system_info();
  metadata.algorithm_parameters.push_back(
      {"benchmark_seed", std::to_string(config.seed)});
  metadata.algorithm_parameters.push_back(
      {"input_size", std::to_string(config.input_size)});
  metadata.algorithm_parameters.push_back(
      {"execution_mode", config.execution_mode == ExecutionMode::in_process
                             ? "in_process"
                             : "isolated_process"});

  const std::string group_key = resolved_group_key(config);
  const std::string variant_key = resolved_variant_key(config);
  std::mt19937_64 iteration_seed_generator(config.seed);
  const auto run_start = std::chrono::steady_clock::now();

  for (std::size_t iteration = 0; iteration < config.total_iterations;
       ++iteration) {
    BenchmarkIterationContext context;
    context.iteration_index = iteration;
    context.input_size = config.input_size;
    context.seed = iteration_seed_generator();
    context.is_warmup =
        iteration < config.statistics_options.warmup_runs_per_group;
    result.iteration_seeds.push_back(context.seed);

    // The caller-owned preparation step is deliberately outside the timer.
    if (callbacks.prepare) {
      callbacks.prepare(context);
    }
    const TimeMeasurement measurement = measure_callback(callbacks, config);

    statistics::BenchmarkSample sample;
    sample.sample_id = metadata.run_id + ":" + group_key + ":" + variant_key +
                       ":" + std::to_string(iteration);
    sample.group_key = group_key;
    sample.variant_key = variant_key;
    if (!config.pair_id_prefix.empty()) {
      sample.pair_id = config.pair_id_prefix + ":" + group_key + ":" +
                       std::to_string(iteration);
    }
    sample.repetition_index = iteration;
    sample.time_unit = config.time_unit;
    sample.elapsed = measurement.obtain_elapsed(config.time_unit);
    sample.memory = measurement.obtain_memory();
    sample.hardware = measurement.obtain_hardware();
    result.samples.push_back(sample);
  }

  metadata.total_duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now() - run_start);
  metadata.environment.after.system = collect_system_info();
  result.statistics = statistics::calculate_statistics(
      result.samples, metadata, config.statistics_options);
  output::write_benchmark_csv(result, config);
  return result;
}

} // namespace benchmark
} // namespace ntq
