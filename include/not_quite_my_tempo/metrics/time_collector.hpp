#pragma once

#include "not_quite_my_tempo/metrics/collector.hpp"

#include <chrono>

namespace ntq::metrics {

class TimeCollector : public MetricCollector {
public:
    MetricAvailability start() override;
    MetricAvailability stop(MeasurementSample& sample) override;

private:
    std::chrono::steady_clock::time_point wall_start_{};
};

}  // namespace ntq::metrics
