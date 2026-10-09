#pragma once

#include "not_quite_my_tempo/metadata.hpp"
#include "not_quite_my_tempo/types.hpp"

#include <string>
#include <vector>

namespace ntq {

struct MetricValue {
    MetricKind kind{MetricKind::wall_clock};
    std::string name{};
    double value{0.0};
    std::string unit{};
    MetricAvailability availability{MetricAvailability::unavailable};
    std::string message{};
};

struct MeasurementSample {
    std::vector<MetricValue> metrics{};
};

struct MeasurementResult {
    std::string name{};
    std::vector<MeasurementSample> samples{};
    RunMetadata metadata{};
};

}  // namespace ntq
