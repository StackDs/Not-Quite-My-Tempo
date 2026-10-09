#pragma once

#include "not_quite_my_tempo/config.hpp"
#include "not_quite_my_tempo/result.hpp"

#include <functional>
#include <string>

namespace ntq {

class Benchmark {
public:
    explicit Benchmark(BenchmarkConfig config);

    MeasurementResult run(
        const std::string& name,
        const std::function<void()>& operation,
        RunMetadata metadata = {}
    );

private:
    BenchmarkConfig config_{};
};

}  // namespace ntq
