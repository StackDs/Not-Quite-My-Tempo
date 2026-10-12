#include "statistics/statistics.hpp"

#include "statistics_internal.hpp"
#include "statistics_math.hpp"

#include <vector>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

void count_status(MetricAvailabilityCounts &counts,
                  MetricValueStatus status) {
  switch (status) {
  case MetricValueStatus::available:
    ++counts.available;
    break;
  case MetricValueStatus::unsupported:
    ++counts.unsupported;
    break;
  case MetricValueStatus::permission_denied:
    ++counts.permission_denied;
    break;
  case MetricValueStatus::read_error:
    ++counts.read_error;
    break;
  case MetricValueStatus::not_measured:
    ++counts.not_measured;
    break;
  case MetricValueStatus::invalid_denominator:
    ++counts.invalid_denominator;
    break;
  case MetricValueStatus::invalid_value:
    ++counts.invalid_value;
    break;
  }
}

std::vector<long double> available_values(
    const std::vector<const BenchmarkSample *> &samples, MetricId metric,
    MetricAvailabilityCounts *availability) {
  std::vector<long double> values;
  for (const BenchmarkSample *sample : samples) {
    const MetricObservation observation = observe_metric(*sample, metric);
    if (availability != nullptr) {
      count_status(*availability, observation.status);
    }
    if (observation.status == MetricValueStatus::available) {
      values.push_back(observation.value);
    }
  }
  return values;
}

} // namespace

StatisticsReport calculate_statistics_report(
    const std::vector<BenchmarkSample> &samples, const RunMetadata &metadata,
    const StatisticsOptions &options) {
  ValidatedSamples validated = validate_and_group_samples(samples, options);
  StatisticsReport report;
  report.metadata = metadata;
  report.options = options;
  report.environment_diagnostics = diagnose_environment(metadata.environment);
  report.sample_audit = validated.sample_audit;
  if (samples.empty()) {
    return report;
  }

  std::vector<PreparedGroup> prepared_groups =
      prepare_groups(samples, options, validated);
  report.sample_audit = validated.sample_audit;
  for (PreparedGroup &prepared : prepared_groups) {
    for (MetricId metric : metric_ids()) {
      MetricSummary summary;
      summary.metric = metric;
      available_values(prepared.coverage_samples, metric,
                       &summary.availability);
      const auto after =
          available_values(prepared.after_iqr, metric, nullptr);
      summary.before_iqr = describe(
          available_values(prepared.before_iqr, metric, nullptr), options,
          prepared.summary.group_key + prepared.summary.variant_key, metric);
      summary.after_iqr = describe(
          after, options,
          prepared.summary.group_key + prepared.summary.variant_key, metric);
      prepared.summary.metrics.push_back(summary);
    }
    report.groups.push_back(prepared.summary);
  }
  report.comparisons =
      calculate_comparisons(prepared_groups, options.comparisons, options);
  return report;
}

} // namespace detail

StatisticsReport calculate_statistics(
    const std::vector<BenchmarkSample> &samples, const RunMetadata &metadata,
    const StatisticsOptions &options) {
  return detail::calculate_statistics_report(samples, metadata, options);
}

} // namespace statistics
} // namespace ntq
