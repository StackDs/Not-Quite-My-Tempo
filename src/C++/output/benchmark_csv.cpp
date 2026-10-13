#include "output/benchmark_csv.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ntq {
namespace output {
namespace {

std::string csv_escape(const std::string &value) {
  if (value.find_first_of(",\"\\\r\n") == std::string::npos) {
    return value;
  }
  std::string escaped;
  escaped.reserve(value.size() + 2);
  for (char character : value) {
    if (character == '\\') {
      escaped.append("\\\\");
    } else if (character == '\n') {
      escaped.append("\\n");
    } else if (character == '\r') {
      escaped.append("\\r");
    } else if (character == '"') {
      escaped.push_back('"');
      escaped.push_back('"');
    } else {
      escaped.push_back(character);
    }
  }
  return '"' + escaped + '"';
}

template <typename T> std::string number(T value) {
  std::ostringstream output;
  output.imbue(std::locale::classic());
  output << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
  return output.str();
}

std::string boolean(bool value) { return value ? "true" : "false"; }

void write_row(std::ofstream &file, const std::vector<std::string> &fields) {
  for (std::size_t index = 0; index < fields.size(); ++index) {
    if (index != 0) {
      file << ',';
    }
    file << csv_escape(fields[index]);
  }
  file << '\n';
  if (!file) {
    throw std::runtime_error("failed while writing benchmark CSV");
  }
}

std::string time_unit_name(TimeUnit unit) {
  switch (unit) {
  case TimeUnit::nanoseconds:
    return "nanoseconds";
  case TimeUnit::microseconds:
    return "microseconds";
  case TimeUnit::milliseconds:
    return "milliseconds";
  case TimeUnit::seconds:
    return "seconds";
  case TimeUnit::minutes:
    return "minutes";
  case TimeUnit::hours:
    return "hours";
  }
  return "invalid";
}

std::string suspicion_reason_name(SuspicionReason reason) {
  switch (reason) {
  case SuspicionReason::none:
    return "none";
  case SuspicionReason::involuntary_context_switches_exceeded:
    return "involuntary_context_switches_exceeded";
  }
  return "invalid";
}

std::string memory_status_name(MemoryMetricStatus status) {
  switch (status) {
  case MemoryMetricStatus::available:
    return "available";
  case MemoryMetricStatus::unsupported:
    return "unsupported";
  case MemoryMetricStatus::permission_denied:
    return "permission_denied";
  case MemoryMetricStatus::read_error:
    return "read_error";
  case MemoryMetricStatus::not_measured:
    return "not_measured";
  }
  return "invalid";
}

std::string peak_rss_scope_name(PeakRssScope scope) {
  switch (scope) {
  case PeakRssScope::process_lifetime:
    return "process_lifetime";
  case PeakRssScope::isolated_measurement_process:
    return "isolated_measurement_process";
  }
  return "invalid";
}

std::string hardware_status_name(HardwareMetricStatus status) {
  switch (status) {
  case HardwareMetricStatus::available:
    return "available";
  case HardwareMetricStatus::unsupported:
    return "unsupported";
  case HardwareMetricStatus::permission_denied:
    return "permission_denied";
  case HardwareMetricStatus::read_error:
    return "read_error";
  case HardwareMetricStatus::not_measured:
    return "not_measured";
  case HardwareMetricStatus::invalid_denominator:
    return "invalid_denominator";
  }
  return "invalid";
}

std::string exclusion_reason_name(
    statistics::SampleExclusionReason reason) {
  using statistics::SampleExclusionReason;
  switch (reason) {
  case SampleExclusionReason::warmup:
    return "warmup";
  case SampleExclusionReason::suspicious_context_switches:
    return "suspicious_context_switches";
  case SampleExclusionReason::iqr_outlier:
    return "iqr_outlier";
  case SampleExclusionReason::invalid_wall_clock:
    return "invalid_wall_clock";
  }
  return "invalid";
}

std::string metric_id_name(statistics::MetricId metric) {
  using statistics::MetricId;
  switch (metric) {
  case MetricId::wall_clock:
    return "wall_clock";
  case MetricId::cpu_user:
    return "cpu_user";
  case MetricId::cpu_system:
    return "cpu_system";
  case MetricId::voluntary_context_switches:
    return "voluntary_context_switches";
  case MetricId::involuntary_context_switches:
    return "involuntary_context_switches";
  case MetricId::peak_rss_bytes:
    return "peak_rss_bytes";
  case MetricId::minor_page_faults:
    return "minor_page_faults";
  case MetricId::major_page_faults:
    return "major_page_faults";
  case MetricId::data_tlb_read_misses:
    return "data_tlb_read_misses";
  case MetricId::data_tlb_write_misses:
    return "data_tlb_write_misses";
  case MetricId::cpu_cycles:
    return "cpu_cycles";
  case MetricId::instructions_retired:
    return "instructions_retired";
  case MetricId::ipc:
    return "ipc";
  case MetricId::l1d_read_accesses:
    return "l1d_read_accesses";
  case MetricId::l1d_read_misses:
    return "l1d_read_misses";
  case MetricId::l1d_read_miss_percent:
    return "l1d_read_miss_percent";
  case MetricId::llc_read_accesses:
    return "llc_read_accesses";
  case MetricId::llc_read_misses:
    return "llc_read_misses";
  case MetricId::llc_read_miss_percent:
    return "llc_read_miss_percent";
  case MetricId::branch_mispredictions:
    return "branch_mispredictions";
  }
  return "invalid";
}

std::string statistics_status_name(statistics::StatisticsStatus status) {
  using statistics::StatisticsStatus;
  switch (status) {
  case StatisticsStatus::available:
    return "available";
  case StatisticsStatus::insufficient_samples:
    return "insufficient_samples";
  case StatisticsStatus::low_reliability:
    return "low_reliability";
  case StatisticsStatus::invalid_input:
    return "invalid_input";
  case StatisticsStatus::invalid_denominator:
    return "invalid_denominator";
  case StatisticsStatus::unavailable:
    return "unavailable";
  }
  return "invalid";
}

std::string ci_status_name(statistics::ConfidenceIntervalStatus status) {
  using statistics::ConfidenceIntervalStatus;
  switch (status) {
  case ConfidenceIntervalStatus::available:
    return "available";
  case ConfidenceIntervalStatus::low_sample_count:
    return "low_sample_count";
  case ConfidenceIntervalStatus::insufficient_samples:
    return "insufficient_samples";
  }
  return "invalid";
}

std::string ci_method_name(statistics::ConfidenceIntervalMethod method) {
  switch (method) {
  case statistics::ConfidenceIntervalMethod::percentile_bootstrap:
    return "percentile_bootstrap";
  }
  return "invalid";
}

std::string tail_status_name(statistics::TailReliabilityStatus status) {
  using statistics::TailReliabilityStatus;
  switch (status) {
  case TailReliabilityStatus::reliable:
    return "reliable";
  case TailReliabilityStatus::low_sample_count:
    return "low_sample_count";
  case TailReliabilityStatus::unavailable:
    return "unavailable";
  }
  return "invalid";
}

std::string availability_status_name(CollectionStatus status) {
  switch (status) {
  case CollectionStatus::available:
    return "available";
  case CollectionStatus::unavailable:
    return "unavailable";
  case CollectionStatus::permission_denied:
    return "permission_denied";
  case CollectionStatus::malformed:
    return "malformed";
  case CollectionStatus::read_error:
    return "read_error";
  case CollectionStatus::unlimited:
    return "unlimited";
  }
  return "invalid";
}

template <typename T>
std::string collected(const CollectedValue<T> &value) {
  std::ostringstream output;
  output << availability_status_name(value.status) << '|';
  output << value.source;
  if (value.status == CollectionStatus::available) {
    output << '|' << value.value;
  }
  return output.str();
}

std::string collected(const CollectedValue<std::vector<unsigned>> &value) {
  std::ostringstream output;
  output << availability_status_name(value.status) << '|' << value.source;
  if (value.status == CollectionStatus::available) {
    output << '|';
    for (std::size_t index = 0; index < value.value.size(); ++index) {
      if (index != 0) {
        output << ';';
      }
      output << value.value[index];
    }
  }
  return output.str();
}

std::string parameters_string(
    const std::vector<std::pair<std::string, std::string>> &parameters) {
  std::ostringstream output;
  for (std::size_t index = 0; index < parameters.size(); ++index) {
    if (index != 0) {
      output << ';';
    }
    output << parameters[index].first << '=' << parameters[index].second;
  }
  return output.str();
}

std::string cache_summary(const CollectedValue<std::vector<CacheInfo>> &caches) {
  std::ostringstream output;
  output << availability_status_name(caches.status) << '|' << caches.source;
  if (caches.status == CollectionStatus::available) {
    for (const CacheInfo &cache : caches.value) {
      output << ';' << cache.level << ':' << cache.type << ':' << cache.size_bytes
             << ':';
      for (std::size_t index = 0; index < cache.shared_cpus.size(); ++index) {
        if (index != 0) {
          output << '|';
        }
        output << cache.shared_cpus[index];
      }
    }
  }
  return output.str();
}

std::string cpu_frequency_summary(
    const CollectedValue<std::vector<CpuFrequencyInfo>> &frequencies) {
  std::ostringstream output;
  output << availability_status_name(frequencies.status) << '|'
         << frequencies.source;
  if (frequencies.status == CollectionStatus::available) {
    for (const CpuFrequencyInfo &cpu : frequencies.value) {
      output << ';' << cpu.cpu_id << ':' << collected(cpu.governor) << ':'
             << collected(cpu.current_frequency_hz) << ':'
             << collected(cpu.minimum_frequency_hz) << ':'
             << collected(cpu.maximum_frequency_hz);
    }
  }
  return output.str();
}

std::string cpu_snapshot_string(const SystemInfo &system) {
  std::ostringstream output;
  output << "architecture=" << collected(system.cpu.architecture)
         << ";vendor=" << collected(system.cpu.vendor)
         << ";model=" << collected(system.cpu.model)
         << ";online_cpus=" << collected(system.cpu.online_logical_cpus)
         << ";caches=" << cache_summary(system.cpu.caches)
         << ";affinity=" << collected(system.cpu_runtime.process_affinity)
         << ";boost=" << collected(system.cpu_runtime.turbo_or_boost_enabled)
         << ";frequencies=" << cpu_frequency_summary(system.cpu_runtime.per_cpu);
  return output.str();
}

std::string compiler_snapshot_string(const SystemInfo &system) {
  std::ostringstream output;
  output << "family=" << collected(system.compiler.family)
         << ";version=" << collected(system.compiler.version)
         << ";language_standard=" << collected(system.compiler.language_standard)
         << ";optimization_profile="
         << collected(system.compiler.optimization_profile)
         << ";target_architecture="
         << collected(system.compiler.target_architecture);
  return output.str();
}

std::string sandbox_snapshot_string(const SystemInfo &system) {
  std::ostringstream output;
  output << "cgroup_version=" << collected(system.sandbox.cgroup_version)
         << ";cgroup_path=" << collected(system.sandbox.cgroup_path)
         << ";cpu_quota_us=" << collected(system.sandbox.effective_cpu_quota_us)
         << ";cpu_period_us="
         << collected(system.sandbox.effective_cpu_period_us)
         << ";cpu_quota_cores="
         << collected(system.sandbox.effective_cpu_quota_cores)
         << ";allowed_cpus="
         << collected(system.sandbox.effective_allowed_cpus)
         << ";memory_limit_bytes="
         << collected(system.sandbox.effective_memory_limit_bytes);
  return output.str();
}

void append_confidence_interval_header(std::vector<std::string> &header,
                                      const std::string &prefix) {
  header.push_back(prefix + "_lower");
  header.push_back(prefix + "_upper");
  header.push_back(prefix + "_confidence_level");
  header.push_back(prefix + "_resamples");
  header.push_back(prefix + "_seed");
  header.push_back(prefix + "_method");
  header.push_back(prefix + "_status");
}

void append_confidence_interval(std::vector<std::string> &row,
                                const statistics::ConfidenceInterval &ci) {
  row.push_back(number(ci.lower));
  row.push_back(number(ci.upper));
  row.push_back(number(ci.confidence_level));
  row.push_back(number(ci.resamples));
  row.push_back(number(ci.seed));
  row.push_back(ci_method_name(ci.method));
  row.push_back(ci_status_name(ci.status));
}

void append_descriptive_header(std::vector<std::string> &header) {
  header.push_back("sample_count");
  header.push_back("status");
  header.push_back("minimum");
  header.push_back("maximum");
  header.push_back("arithmetic_mean");
  header.push_back("median");
  header.push_back("median_absolute_deviation");
  header.push_back("sample_standard_deviation");
  header.push_back("sample_standard_deviation_status");
  header.push_back("p90");
  header.push_back("p95");
  header.push_back("p99");
  header.push_back("p999");
  header.push_back("p90_reliability");
  header.push_back("p95_reliability");
  header.push_back("p99_reliability");
  header.push_back("p999_reliability");
  append_confidence_interval_header(header, "mean_ci");
  append_confidence_interval_header(header, "median_ci");
}

void append_descriptive(std::vector<std::string> &row,
                        const statistics::DescriptiveStatistics &summary) {
  row.push_back(number(summary.sample_count));
  row.push_back(statistics_status_name(summary.status));
  row.push_back(number(summary.minimum));
  row.push_back(number(summary.maximum));
  row.push_back(number(summary.arithmetic_mean));
  row.push_back(number(summary.median));
  row.push_back(number(summary.median_absolute_deviation));
  row.push_back(number(summary.sample_standard_deviation));
  row.push_back(statistics_status_name(
      summary.sample_standard_deviation_status));
  row.push_back(number(summary.p90));
  row.push_back(number(summary.p95));
  row.push_back(number(summary.p99));
  row.push_back(number(summary.p999));
  row.push_back(tail_status_name(summary.p90_reliability));
  row.push_back(tail_status_name(summary.p95_reliability));
  row.push_back(tail_status_name(summary.p99_reliability));
  row.push_back(tail_status_name(summary.p999_reliability));
  append_confidence_interval(row, summary.mean_confidence_interval);
  append_confidence_interval(row, summary.median_confidence_interval);
}

std::vector<std::string> statistics_header() {
  std::vector<std::string> header{
      "run_id", "input_size", "group_key", "variant_key", "time_unit",
      "configured_iterations", "total_duration_ns", "clock_name",
      "clock_resolution_ns", "library_version", "algorithm_parameters",
      "cpu_before", "compiler_before", "sandbox_before", "cpu_after",
      "compiler_after", "sandbox_after", "runnable_process_delta",
      "cpu_temperature_delta_celsius", "cpu_affinity_changed",
      "cpu_governor_or_frequency_changed", "thermal_throttling_observed",
      "runnable_processes_before", "runnable_processes_after",
      "cpu_temperature_before_celsius", "cpu_temperature_after_celsius",
      "thermal_throttling_before", "thermal_throttling_after",
      "metric_id", "view", "availability_available", "availability_unsupported",
      "availability_permission_denied", "availability_read_error",
      "availability_not_measured", "availability_invalid_denominator",
      "availability_invalid_value"};
  append_descriptive_header(header);
  return header;
}

void append_environment_diagnostics(std::vector<std::string> &row,
                                    const statistics::StatisticsReport &report) {
  const auto &diagnostics = report.environment_diagnostics;
  row.push_back(collected(diagnostics.runnable_process_delta));
  row.push_back(collected(diagnostics.cpu_temperature_delta_celsius));
  row.push_back(collected(diagnostics.cpu_affinity_changed));
  row.push_back(collected(diagnostics.cpu_governor_or_frequency_changed));
  row.push_back(collected(diagnostics.thermal_throttling_observed));
  row.push_back(collected(report.metadata.environment.before.runnable_processes));
  row.push_back(collected(report.metadata.environment.after.runnable_processes));
  row.push_back(
      collected(report.metadata.environment.before.cpu_temperature_celsius));
  row.push_back(
      collected(report.metadata.environment.after.cpu_temperature_celsius));
  row.push_back(collected(report.metadata.environment.before.thermal_throttling));
  row.push_back(collected(report.metadata.environment.after.thermal_throttling));
}

std::vector<std::string> statistics_row(
    const statistics::StatisticsReport &report,
    const statistics::GroupStatistics &group,
    const statistics::MetricSummary &metric, const char *view,
    std::size_t input_size,
    const statistics::DescriptiveStatistics &summary) {
  const auto &metadata = report.metadata;
  std::vector<std::string> row{
      metadata.run_id,
      number(input_size),
      group.group_key,
      group.variant_key,
      time_unit_name(group.time_unit),
      number(metadata.configured_iterations),
      number(metadata.total_duration.count()),
      metadata.clock_name,
      number(metadata.clock_resolution.count()),
      metadata.library_version,
      parameters_string(metadata.algorithm_parameters),
      cpu_snapshot_string(metadata.environment.before.system),
      compiler_snapshot_string(metadata.environment.before.system),
      sandbox_snapshot_string(metadata.environment.before.system),
      cpu_snapshot_string(metadata.environment.after.system),
      compiler_snapshot_string(metadata.environment.after.system),
      sandbox_snapshot_string(metadata.environment.after.system)};
  append_environment_diagnostics(row, report);
  row.push_back(metric_id_name(metric.metric));
  row.push_back(view);
  row.push_back(number(metric.availability.available));
  row.push_back(number(metric.availability.unsupported));
  row.push_back(number(metric.availability.permission_denied));
  row.push_back(number(metric.availability.read_error));
  row.push_back(number(metric.availability.not_measured));
  row.push_back(number(metric.availability.invalid_denominator));
  row.push_back(number(metric.availability.invalid_value));
  append_descriptive(row, summary);
  return row;
}

} // namespace

void write_benchmark_csv(const benchmark::BenchmarkResult &result,
                         const benchmark::BenchmarkConfig &config) {
  if (config.iterations_csv_path.empty() ||
      config.statistics_csv_path.empty() ||
      config.iterations_csv_path == config.statistics_csv_path) {
    throw std::invalid_argument(
        "benchmark CSV paths must be nonempty and distinct");
  }
  if (result.iteration_seeds.size() != result.samples.size() ||
      result.statistics.sample_audit.size() != result.samples.size() ||
      (!result.samples.empty() && result.statistics.groups.empty())) {
    throw std::invalid_argument(
        "benchmark result samples, seeds, audit and statistics do not align");
  }
  std::ofstream iterations(config.iterations_csv_path.c_str(),
                           std::ios::out | std::ios::trunc | std::ios::binary);
  if (!iterations) {
    throw std::runtime_error("could not open iteration CSV: " +
                             config.iterations_csv_path);
  }
  const std::vector<std::string> iteration_header{
      "run_id", "input_size", "group_key", "variant_key", "iteration_seed",
      "sample_id", "pair_id", "repetition_index", "is_warmup", "time_unit",
      "wall_clock", "cpu_user", "cpu_system", "voluntary_context_switches",
      "involuntary_context_switches", "suspicious",
      "involuntary_context_switch_threshold", "suspicious_reason",
      "peak_rss_bytes", "minor_page_faults", "major_page_faults",
      "data_tlb_read_misses", "data_tlb_write_misses", "peak_rss_status",
      "data_tlb_read_status", "data_tlb_write_status", "peak_rss_scope",
      "cpu_cycles", "instructions_retired", "ipc", "cpu_cycles_status",
      "instructions_retired_status", "ipc_status", "l1d_read_accesses",
      "l1d_read_misses", "l1d_read_miss_percent", "l1d_read_accesses_status",
      "l1d_read_misses_status", "l1d_read_miss_percent_status",
      "llc_read_accesses", "llc_read_misses", "llc_read_miss_percent",
      "llc_read_accesses_status", "llc_read_misses_status",
      "llc_read_miss_percent_status", "branch_mispredictions",
      "branch_mispredictions_status", "included_after_filters",
      "exclusion_reasons"};
  write_row(iterations, iteration_header);
  const auto &metadata = result.statistics.metadata;
  for (std::size_t index = 0; index < result.samples.size(); ++index) {
    const statistics::BenchmarkSample &sample = result.samples[index];
    const statistics::SampleAudit &audit = result.statistics.sample_audit[index];
    std::ostringstream reasons;
    for (std::size_t reason = 0; reason < audit.exclusion_reasons.size();
         ++reason) {
      if (reason != 0) {
        reasons << '|';
      }
      reasons << exclusion_reason_name(audit.exclusion_reasons[reason]);
    }
    const MemoryMeasurements &memory = sample.memory;
    const HardwareMeasurements &hardware = sample.hardware;
    std::vector<std::string> row{
        metadata.run_id,
        number(result.input_size),
        sample.group_key,
        sample.variant_key,
        number(result.iteration_seeds[index]),
        sample.sample_id,
        sample.pair_id,
        number(sample.repetition_index),
        boolean(sample.repetition_index <
                result.statistics.options.warmup_runs_per_group),
        time_unit_name(sample.time_unit),
        number(sample.elapsed.wall_clock),
        number(sample.elapsed.cpu_user),
        number(sample.elapsed.cpu_system),
        number(sample.elapsed.voluntary_context_switches),
        number(sample.elapsed.involuntary_context_switches),
        boolean(sample.elapsed.suspicious),
        number(sample.elapsed.involuntary_context_switch_threshold),
        suspicion_reason_name(sample.elapsed.suspicious_reason),
        number(memory.peak_rss_bytes),
        number(memory.minor_page_faults),
        number(memory.major_page_faults),
        number(memory.data_tlb_read_misses),
        number(memory.data_tlb_write_misses),
        memory_status_name(memory.peak_rss_status),
        memory_status_name(memory.data_tlb_read_status),
        memory_status_name(memory.data_tlb_write_status),
        peak_rss_scope_name(memory.peak_rss_scope),
        number(hardware.cpu_cycles),
        number(hardware.instructions_retired),
        number(hardware.ipc),
        hardware_status_name(hardware.cpu_cycles_status),
        hardware_status_name(hardware.instructions_retired_status),
        hardware_status_name(hardware.ipc_status),
        number(hardware.l1d_read_accesses),
        number(hardware.l1d_read_misses),
        number(hardware.l1d_read_miss_percent),
        hardware_status_name(hardware.l1d_read_accesses_status),
        hardware_status_name(hardware.l1d_read_misses_status),
        hardware_status_name(hardware.l1d_read_miss_percent_status),
        number(hardware.llc_read_accesses),
        number(hardware.llc_read_misses),
        number(hardware.llc_read_miss_percent),
        hardware_status_name(hardware.llc_read_accesses_status),
        hardware_status_name(hardware.llc_read_misses_status),
        hardware_status_name(hardware.llc_read_miss_percent_status),
        number(hardware.branch_mispredictions),
        hardware_status_name(hardware.branch_mispredictions_status),
        boolean(audit.included_after_filters),
        reasons.str()};
    write_row(iterations, row);
  }
  iterations.close();
  if (!iterations) {
    throw std::runtime_error("failed while closing iteration CSV");
  }

  std::ofstream summaries(config.statistics_csv_path.c_str(),
                          std::ios::out | std::ios::trunc | std::ios::binary);
  if (!summaries) {
    throw std::runtime_error("could not open statistics CSV: " +
                             config.statistics_csv_path);
  }
  write_row(summaries, statistics_header());
  for (const statistics::GroupStatistics &group : result.statistics.groups) {
    for (const statistics::MetricSummary &metric : group.metrics) {
      write_row(summaries,
                statistics_row(result.statistics, group, metric, "before_iqr",
                               result.input_size, metric.before_iqr));
      write_row(summaries,
                statistics_row(result.statistics, group, metric, "after_iqr",
                               result.input_size, metric.after_iqr));
    }
  }
  summaries.close();
  if (!summaries) {
    throw std::runtime_error("failed while closing statistics CSV");
  }
}

} // namespace output
} // namespace ntq
