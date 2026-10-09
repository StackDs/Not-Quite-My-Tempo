// Public C++ summary statistics interface.
#pragma once

#include <cstddef>
#include <vector>

namespace ntq::statistics {

struct SummaryStatistics {
    std::size_t sample_count{0};
    double median{0.0};
    double median_absolute_deviation{0.0};
    double p90{0.0};
    double p95{0.0};
    double p99{0.0};
    double p999{0.0};
    std::vector<std::size_t> outlier_indices{};
};

SummaryStatistics summarize(const std::vector<double>& samples);
std::vector<std::size_t> detect_iqr_outliers(const std::vector<double>& samples);

}  // namespace ntq::statistics
