#include "statistics_internal.hpp"

#include "statistics_math.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

void add_reason(SampleAudit &audit, SampleExclusionReason reason) {
  audit.exclusion_reasons.push_back(reason);
  audit.included_after_filters = false;
}

} // namespace

std::vector<PreparedGroup> prepare_groups(
    const std::vector<BenchmarkSample> &samples,
    const StatisticsOptions &options, ValidatedSamples &validated) {
  std::vector<PreparedGroup> prepared_groups;
  prepared_groups.reserve(validated.grouped_indices.size());

  for (auto &entry : validated.grouped_indices) {
    const GroupKey &key = entry.first;
    std::vector<std::size_t> &indices = entry.second;
    std::stable_sort(indices.begin(), indices.end(), [&](std::size_t left,
                                                         std::size_t right) {
      return samples[left].repetition_index < samples[right].repetition_index;
    });

    PreparedGroup prepared;
    prepared.summary.group_key = key.first;
    prepared.summary.variant_key = key.second;
    prepared.summary.time_unit = validated.units_by_group.at(key);
    prepared.summary.input_sample_count = indices.size();

    for (std::size_t position = 0; position < indices.size(); ++position) {
      const std::size_t sample_index = indices[position];
      const BenchmarkSample &sample = samples[sample_index];
      SampleAudit &audit = validated.sample_audit[sample_index];
      if (sample.elapsed.suspicious) {
        ++prepared.summary.suspicious_sample_count;
      }

      if (position < options.warmup_runs_per_group) {
        ++prepared.summary.warmup_sample_count;
        add_reason(audit, SampleExclusionReason::warmup);
        continue;
      }
      prepared.coverage_samples.push_back(&sample);
      if (sample.elapsed.suspicious && !options.include_suspicious_samples) {
        add_reason(audit,
                   SampleExclusionReason::suspicious_context_switches);
        continue;
      }

      const MetricObservation wall_clock =
          observe_metric(sample, MetricId::wall_clock);
      if (wall_clock.status != MetricValueStatus::available) {
        add_reason(audit, SampleExclusionReason::invalid_wall_clock);
        continue;
      }
      prepared.before_iqr.push_back(&sample);
    }

    if (options.apply_iqr_filter && !prepared.before_iqr.empty()) {
      std::vector<long double> wall_clocks;
      wall_clocks.reserve(prepared.before_iqr.size());
      for (const BenchmarkSample *sample : prepared.before_iqr) {
        wall_clocks.push_back(
            observe_metric(*sample, MetricId::wall_clock).value);
      }
      const long double q1 = quantile(wall_clocks, 0.25L);
      const long double q3 = quantile(wall_clocks, 0.75L);
      const long double upper_fence = q3 + 1.5L * (q3 - q1);
      for (const BenchmarkSample *sample : prepared.before_iqr) {
        const std::size_t sample_index = static_cast<std::size_t>(
            sample - samples.data());
        if (static_cast<long double>(sample->elapsed.wall_clock) >
            upper_fence) {
          ++prepared.summary.iqr_outlier_count;
          add_reason(validated.sample_audit[sample_index],
                     SampleExclusionReason::iqr_outlier);
        } else {
          prepared.after_iqr.push_back(sample);
        }
      }
    } else {
      prepared.after_iqr = prepared.before_iqr;
    }

    for (std::size_t index : indices) {
      SampleAudit &audit = validated.sample_audit[index];
      if (audit.exclusion_reasons.empty()) {
        audit.included_after_filters = true;
      }
      prepared.summary.sample_audit.push_back(audit);
    }
    prepared_groups.push_back(std::move(prepared));
  }
  return prepared_groups;
}

} // namespace detail
} // namespace statistics
} // namespace ntq
