#pragma once

#include "statistics/statistics.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ntq {
namespace benchmark {

struct BenchmarkResult {
  std::size_t input_size{0};
  std::string iterations_csv_path;
  std::string statistics_csv_path;
  std::vector<std::uint64_t> iteration_seeds;
  std::vector<statistics::BenchmarkSample> samples;
  statistics::StatisticsReport statistics;
};

} // namespace benchmark
} // namespace ntq
