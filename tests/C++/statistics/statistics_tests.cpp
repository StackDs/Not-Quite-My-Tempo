#include "statistics/statistics.hpp"

#include <cassert>
#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

ntq::statistics::BenchmarkSample make_sample(
    const std::string &sample_id, const std::string &group,
    const std::string &variant, std::size_t repetition, double wall_clock,
    bool suspicious = false, const std::string &pair_id = std::string()) {
  ntq::statistics::BenchmarkSample sample;
  sample.sample_id = sample_id;
  sample.group_key = group;
  sample.variant_key = variant;
  sample.pair_id = pair_id;
  sample.repetition_index = repetition;
  sample.time_unit = ntq::TimeUnit::seconds;
  sample.elapsed.wall_clock = wall_clock;
  sample.elapsed.cpu_user = wall_clock * 0.8;
  sample.elapsed.cpu_system = wall_clock * 0.1;
  sample.elapsed.voluntary_context_switches = 1;
  sample.elapsed.involuntary_context_switches = suspicious ? 1 : 0;
  sample.elapsed.suspicious = suspicious;
  sample.elapsed.involuntary_context_switch_threshold = 0;
  sample.elapsed.suspicious_reason =
      suspicious ? ntq::SuspicionReason::involuntary_context_switches_exceeded
                 : ntq::SuspicionReason::none;

  sample.memory.peak_rss_bytes = 1024;
  sample.memory.peak_rss_status = ntq::MemoryMetricStatus::available;
  sample.memory.minor_page_faults = 2;
  sample.memory.major_page_faults = 0;
  sample.memory.data_tlb_read_misses = 4;
  sample.memory.data_tlb_read_status = ntq::MemoryMetricStatus::available;
  sample.memory.data_tlb_write_misses = 1;
  sample.memory.data_tlb_write_status = ntq::MemoryMetricStatus::available;

  sample.hardware.cpu_cycles = 100;
  sample.hardware.cpu_cycles_status = ntq::HardwareMetricStatus::available;
  sample.hardware.instructions_retired = 200;
  sample.hardware.instructions_retired_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.ipc = 2.0;
  sample.hardware.ipc_status = ntq::HardwareMetricStatus::available;
  sample.hardware.l1d_read_accesses = 100;
  sample.hardware.l1d_read_accesses_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.l1d_read_misses = 5;
  sample.hardware.l1d_read_misses_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.l1d_read_miss_percent = 5.0;
  sample.hardware.l1d_read_miss_percent_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.llc_read_accesses = 50;
  sample.hardware.llc_read_accesses_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.llc_read_misses = 2;
  sample.hardware.llc_read_misses_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.llc_read_miss_percent = 4.0;
  sample.hardware.llc_read_miss_percent_status =
      ntq::HardwareMetricStatus::available;
  sample.hardware.branch_mispredictions = 3;
  sample.hardware.branch_mispredictions_status =
      ntq::HardwareMetricStatus::available;
  return sample;
}

const ntq::statistics::GroupStatistics &find_group(
    const ntq::statistics::StatisticsReport &report, const std::string &group,
    const std::string &variant) {
  for (const auto &item : report.groups) {
    if (item.group_key == group && item.variant_key == variant) {
      return item;
    }
  }
  throw std::runtime_error("expected group not found");
}

const ntq::statistics::MetricSummary &find_metric(
    const ntq::statistics::GroupStatistics &group,
    ntq::statistics::MetricId metric) {
  for (const auto &item : group.metrics) {
    if (item.metric == metric) {
      return item;
    }
  }
  throw std::runtime_error("expected metric not found");
}

void test_generic_grouping_and_summary_views() {
  const std::string key = "parameter=block_size:16|index=7";
  std::vector<ntq::statistics::BenchmarkSample> samples{
      make_sample("warmup", key, "candidate", 0, 0.1),
      make_sample("s1", key, "candidate", 1, 1.0),
      make_sample("s2", key, "candidate", 2, 2.0),
      make_sample("s3", key, "candidate", 3, 3.0),
      make_sample("outlier", key, "candidate", 4, 100.0, true),
  };

  ntq::statistics::StatisticsOptions options;
  options.warmup_runs_per_group = 1;
  options.bootstrap_resamples = 200;
  options.minimum_reliable_ci_samples = 2;
  options.minimum_expected_tail_observations = 0.5L;
  ntq::statistics::RunMetadata metadata;
  metadata.run_id = "run-1";
  metadata.library_version = "test-version";
  metadata.algorithm_parameters.push_back({"block_size", "16"});
  metadata.environment.before.system.compiler.family = ntq::CollectedValue<std::string>(
      "GCC", ntq::CollectionStatus::available, "fixture");
  metadata.environment.before.runnable_processes =
      ntq::CollectedValue<std::uint64_t>(2, ntq::CollectionStatus::available,
                                         "/proc/loadavg");
  metadata.environment.after.runnable_processes =
      ntq::CollectedValue<std::uint64_t>(3, ntq::CollectionStatus::available,
                                         "/proc/loadavg");
  metadata.environment.before.cpu_temperature_celsius =
      ntq::CollectedValue<double>(58.0, ntq::CollectionStatus::available,
                                  "thermal-zone");
  metadata.environment.after.cpu_temperature_celsius =
      ntq::CollectedValue<double>(62.5, ntq::CollectionStatus::available,
                                  "/sys/class/thermal/thermal_zone0/temp");
  metadata.environment.before.thermal_throttling =
      ntq::CollectedValue<bool>(false, ntq::CollectionStatus::available,
                                "thermal-zone");
  metadata.environment.after.thermal_throttling =
      ntq::CollectedValue<bool>(true, ntq::CollectionStatus::available,
                                "thermal-zone");
  metadata.environment.before.system.cpu_runtime.process_affinity =
      ntq::CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>{0}, ntq::CollectionStatus::available,
          "fixture");
  metadata.environment.after.system.cpu_runtime.process_affinity =
      ntq::CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>{1}, ntq::CollectionStatus::available,
          "fixture");
  ntq::CpuFrequencyInfo cpu_before;
  cpu_before.cpu_id = 0;
  cpu_before.governor = ntq::CollectedValue<std::string>(
      "performance", ntq::CollectionStatus::available, "fixture");
  cpu_before.current_frequency_hz = ntq::CollectedValue<std::uint64_t>(
      2000000000, ntq::CollectionStatus::available, "fixture");
  ntq::CpuFrequencyInfo cpu_after = cpu_before;
  cpu_after.governor = ntq::CollectedValue<std::string>(
      "powersave", ntq::CollectionStatus::available, "fixture");
  cpu_after.current_frequency_hz = ntq::CollectedValue<std::uint64_t>(
      1000000000, ntq::CollectionStatus::available, "fixture");
  metadata.environment.before.system.cpu_runtime.per_cpu =
      ntq::CollectedValue<std::vector<ntq::CpuFrequencyInfo>>(
          std::vector<ntq::CpuFrequencyInfo>{cpu_before},
          ntq::CollectionStatus::available, "fixture");
  metadata.environment.after.system.cpu_runtime.per_cpu =
      ntq::CollectedValue<std::vector<ntq::CpuFrequencyInfo>>(
          std::vector<ntq::CpuFrequencyInfo>{cpu_after},
          ntq::CollectionStatus::available, "fixture");

  const auto report =
      ntq::statistics::calculate_statistics(samples, metadata, options);
  assert(report.groups.size() == 1);
  const std::string candidate_variant = "candidate";
  const auto &group = find_group(report, key, candidate_variant);
  assert(group.warmup_sample_count == 1);
  assert(group.suspicious_sample_count == 1);
  assert(group.iqr_outlier_count == 1);
  assert(report.metadata.run_id == "run-1");
  assert(report.metadata.algorithm_parameters[0].first == "block_size");
  assert(report.metadata.environment.before.system.compiler.family.value ==
         "GCC");
  assert(report.metadata.environment.before.runnable_processes.value == 2);
  assert(report.metadata.environment.after.cpu_temperature_celsius.value ==
         62.5);
  assert(report.environment_diagnostics.runnable_process_delta.value == 1);
  assert(report.environment_diagnostics.cpu_temperature_delta_celsius.value ==
         4.5);
  assert(report.environment_diagnostics.cpu_affinity_changed.value);
  assert(report.environment_diagnostics.cpu_governor_or_frequency_changed.value);
  assert(report.environment_diagnostics.thermal_throttling_observed.value);

  const auto &wall = find_metric(group, ntq::statistics::MetricId::wall_clock);
  assert(wall.before_iqr.sample_count == 4);
  assert(wall.before_iqr.minimum == 1.0L);
  assert(wall.before_iqr.maximum == 100.0L);
  assert(wall.before_iqr.arithmetic_mean == 26.5L);
  assert(wall.before_iqr.median == 2.5L);
  assert(wall.after_iqr.sample_count == 3);
  assert(wall.after_iqr.minimum == 1.0L);
  assert(wall.after_iqr.maximum == 3.0L);
  assert(wall.after_iqr.arithmetic_mean == 2.0L);
  assert(wall.after_iqr.median == 2.0L);
  assert(wall.after_iqr.median_absolute_deviation == 1.0L);
  assert(wall.after_iqr.sample_standard_deviation == 1.0L);
  assert(wall.after_iqr.p90 == 2.8L);
  assert(wall.after_iqr.mean_confidence_interval.resamples == 200);
  assert(wall.after_iqr.mean_confidence_interval.method ==
         ntq::statistics::ConfidenceIntervalMethod::percentile_bootstrap);
  const auto repeated =
      ntq::statistics::calculate_statistics(samples, metadata, options);
  const auto &repeated_wall = find_metric(
      find_group(repeated, key, candidate_variant),
      ntq::statistics::MetricId::wall_clock);
  assert(repeated_wall.after_iqr.mean_confidence_interval.lower ==
         wall.after_iqr.mean_confidence_interval.lower);
  assert(repeated_wall.after_iqr.mean_confidence_interval.upper ==
         wall.after_iqr.mean_confidence_interval.upper);

  bool found_warmup = false;
  bool found_outlier = false;
  for (const auto &audit : group.sample_audit) {
    if (audit.sample_id == "warmup") {
      found_warmup =
          audit.exclusion_reasons.size() == 1 &&
          audit.exclusion_reasons[0] ==
              ntq::statistics::SampleExclusionReason::warmup;
    }
    if (audit.sample_id == "outlier") {
      found_outlier =
          audit.exclusion_reasons.size() == 1 &&
          audit.exclusion_reasons[0] ==
              ntq::statistics::SampleExclusionReason::iqr_outlier;
    }
  }
  assert(found_warmup && found_outlier);
}

void test_suspicious_policy_and_unavailable_metrics() {
  auto suspicious = make_sample("suspect", "group", "base", 1, 2.0, true);
  suspicious.hardware.cpu_cycles_status =
      ntq::HardwareMetricStatus::permission_denied;
  auto ordinary = make_sample("ordinary", "group", "base", 2, 4.0);
  ordinary.memory.data_tlb_read_status = ntq::MemoryMetricStatus::unsupported;

  ntq::statistics::StatisticsOptions options;
  options.apply_iqr_filter = false;
  options.include_suspicious_samples = false;
  options.bootstrap_resamples = 100;
  ntq::statistics::RunMetadata metadata;
  const auto report = ntq::statistics::calculate_statistics(
      {suspicious, ordinary}, metadata, options);
  const std::string group_key = "group";
  const std::string variant_key = "base";
  const auto &group = find_group(report, group_key, variant_key);
  const auto &wall = find_metric(group, ntq::statistics::MetricId::wall_clock);
  assert(group.suspicious_sample_count == 1);
  assert(wall.after_iqr.sample_count == 1);
  bool suspicion_audited = false;
  for (const auto &audit : group.sample_audit) {
    if (audit.sample_id == "suspect") {
      assert(audit.suspicious);
      assert(audit.involuntary_context_switch_threshold == 0);
      assert(audit.suspicious_reason ==
             ntq::SuspicionReason::involuntary_context_switches_exceeded);
      assert(audit.exclusion_reasons.size() == 1);
      assert(audit.exclusion_reasons[0] ==
             ntq::statistics::SampleExclusionReason::suspicious_context_switches);
      suspicion_audited = true;
    }
  }
  assert(suspicion_audited);
  const auto &cycles =
      find_metric(group, ntq::statistics::MetricId::cpu_cycles);
  assert(cycles.availability.permission_denied == 1);
  const auto &dtlb =
      find_metric(group, ntq::statistics::MetricId::data_tlb_read_misses);
  assert(dtlb.availability.unsupported == 1);

  options.include_suspicious_samples = true;
  const auto included_report = ntq::statistics::calculate_statistics(
      {suspicious, ordinary}, metadata, options);
  const auto &included_group =
      find_group(included_report, group_key, variant_key);
  assert(find_metric(included_group, ntq::statistics::MetricId::wall_clock)
             .after_iqr.sample_count == 2);
}

void test_confidence_and_tail_reliability() {
  std::vector<ntq::statistics::BenchmarkSample> samples;
  for (std::size_t index = 0; index < 100; ++index) {
    samples.push_back(make_sample("sample-" + std::to_string(index), "g", "v",
                                  index, 1.0 + index * 0.01));
  }
  ntq::statistics::StatisticsOptions options;
  options.apply_iqr_filter = false;
  options.bootstrap_resamples = 200;
  options.minimum_reliable_ci_samples = 30;
  options.minimum_expected_tail_observations = 5.0L;
  const auto report = ntq::statistics::calculate_statistics(
      samples, ntq::statistics::RunMetadata{}, options);
  const auto &wall = find_metric(report.groups.front(),
                                ntq::statistics::MetricId::wall_clock);
  assert(wall.after_iqr.mean_confidence_interval.status ==
         ntq::statistics::ConfidenceIntervalStatus::available);
  assert(wall.after_iqr.p99_reliability ==
         ntq::statistics::TailReliabilityStatus::low_sample_count);
  assert(wall.after_iqr.p999_reliability ==
         ntq::statistics::TailReliabilityStatus::low_sample_count);
}

void test_paired_effect_sizes() {
  std::vector<ntq::statistics::BenchmarkSample> samples{
      make_sample("a1", "data-A", "baseline", 1, 10.0, false, "pair-1"),
      make_sample("a2", "data-A", "baseline", 2, 20.0, false, "pair-2"),
      make_sample("a3", "data-A", "baseline", 3, 30.0, false, "pair-3"),
      make_sample("b1", "data-A", "candidate", 1, 8.0, false, "pair-1"),
      make_sample("b2", "data-A", "candidate", 2, 18.0, false, "pair-2"),
      make_sample("b3", "data-A", "candidate", 3, 27.0, false, "pair-3"),
  };
  ntq::statistics::StatisticsOptions options;
  options.apply_iqr_filter = false;
  options.bootstrap_resamples = 200;
  options.minimum_reliable_ci_samples = 2;
  options.minimum_expected_tail_observations = 0.1L;
  options.comparisons.push_back(
      {"data-A", "baseline", "candidate",
       ntq::statistics::PairingPolicy::automatic});

  const auto report = ntq::statistics::calculate_statistics(
      samples, ntq::statistics::RunMetadata{}, options);
  assert(report.comparisons.size() == 1);
  const auto &comparison = report.comparisons.front();
  bool checked_pairing = false;

  bool checked_wall = false;
  for (const auto &metric : comparison.metrics) {
    if (metric.metric == ntq::statistics::MetricId::wall_clock) {
      assert(metric.pairing_used == ntq::statistics::PairingUsed::paired);
      assert(metric.matched_pairs == 3);
      assert(std::fabs(static_cast<double>(metric.mean_effect.absolute_difference) +
                       7.0 / 3.0) < 1e-12);
      assert(metric.median_effect.absolute_difference == -2.0L);
      assert(metric.mean_effect.relative_difference_percent < 0.0L);
      checked_pairing = metric.pairing_used ==
                        ntq::statistics::PairingUsed::paired;
      checked_wall = true;
    }
  }
  assert(checked_wall && checked_pairing);
}

void test_mixed_units_rejected() {
  auto first = make_sample("s1", "g", "v", 1, 1.0);
  auto second = make_sample("s2", "g", "v", 2, 1000.0);
  second.time_unit = ntq::TimeUnit::milliseconds;
  bool rejected = false;
  try {
    (void)ntq::statistics::calculate_statistics(
        {first, second}, ntq::statistics::RunMetadata{},
        ntq::statistics::StatisticsOptions{});
  } catch (const std::invalid_argument &) {
    rejected = true;
  }
  assert(rejected);
}

void test_unpaired_effect_with_zero_baseline() {
  const std::string group = "mode=unpaired";
  const std::string baseline = "old";
  const std::string candidate = "new";
  auto a1 = make_sample("a1", group, baseline, 1, 0.0);
  auto a2 = make_sample("a2", group, baseline, 2, 0.0);
  auto b1 = make_sample("b1", group, candidate, 1, 1.0);
  auto b2 = make_sample("b2", group, candidate, 2, 2.0);

  ntq::statistics::StatisticsOptions options;
  options.apply_iqr_filter = false;
  options.bootstrap_resamples = 100;
  options.minimum_reliable_ci_samples = 2;
  options.comparisons.push_back(
      {group, baseline, candidate, ntq::statistics::PairingPolicy::unpaired});
  const auto report = ntq::statistics::calculate_statistics(
      {a1, a2, b1, b2}, ntq::statistics::RunMetadata{}, options);
  assert(report.comparisons.size() == 1);
  bool found_wall = false;
  for (const auto &metric : report.comparisons[0].metrics) {
    if (metric.metric == ntq::statistics::MetricId::wall_clock) {
      assert(metric.pairing_used == ntq::statistics::PairingUsed::unpaired);
      assert(metric.mean_effect.relative_status ==
             ntq::statistics::StatisticsStatus::invalid_denominator);
      found_wall = true;
    }
  }
  assert(found_wall);
}

} // namespace

int main() {
  test_generic_grouping_and_summary_views();
  test_suspicious_policy_and_unavailable_metrics();
  test_confidence_and_tail_reliability();
  test_paired_effect_sizes();
  test_unpaired_effect_with_zero_baseline();
  test_mixed_units_rejected();
}
