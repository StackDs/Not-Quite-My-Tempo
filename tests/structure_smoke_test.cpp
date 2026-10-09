#include "not_quite_my_tempo/benchmark.hpp"
#include "not_quite_my_tempo/data/random_generator.hpp"
#include "not_quite_my_tempo/data/vector_generators.hpp"
#include "not_quite_my_tempo/metrics/hardware_metrics_collector.hpp"
#include "not_quite_my_tempo/metrics/process_metrics_collector.hpp"
#include "not_quite_my_tempo/metrics/time_collector.hpp"
#include "not_quite_my_tempo/output/csv_writer.hpp"
#include "not_quite_my_tempo/statistics/summary.hpp"
#include "not_quite_my_tempo/statistics/complexity.hpp"
#include "not_quite_my_tempo/system/system_info.hpp"

#include <type_traits>

static_assert(std::is_class_v<ntq::Benchmark>);
static_assert(std::is_class_v<ntq::BenchmarkConfig>);
static_assert(std::is_class_v<ntq::MeasurementResult>);
static_assert(ntq::TimeUnit::nanoseconds != ntq::TimeUnit::seconds);
static_assert(ntq::MetricKind::wall_clock != ntq::MetricKind::cpu_user);
static_assert(std::is_class_v<ntq::data::RandomGenerator>);
static_assert(std::is_class_v<ntq::statistics::SummaryStatistics>);

int main() {
    const ntq::BenchmarkConfig config{};
    const ntq::MeasurementResult result{};
    (void)config;
    (void)result;
    return 0;
}
