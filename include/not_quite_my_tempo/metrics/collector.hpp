#pragma once

#include "not_quite_my_tempo/result.hpp"

namespace ntq::metrics {

class MetricCollector {
public:
    virtual ~MetricCollector() = default;

    virtual MetricAvailability start() = 0;
    virtual MetricAvailability stop(MeasurementSample& sample) = 0;
};

}  // namespace ntq::metrics
