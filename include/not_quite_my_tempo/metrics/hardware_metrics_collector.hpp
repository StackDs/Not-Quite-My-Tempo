#pragma once

#include "not_quite_my_tempo/metrics/collector.hpp"

namespace ntq::metrics {

class HardwareMetricsCollector : public MetricCollector {
public:
    MetricAvailability start() override;
    MetricAvailability stop(MeasurementSample& sample) override;
};

}  // namespace ntq::metrics
