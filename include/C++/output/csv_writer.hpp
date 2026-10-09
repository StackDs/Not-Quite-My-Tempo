// Public C++ CSV writer interface.
#pragma once

#include "../result.hpp"

#include <string>

namespace ntq::output {

class CsvWriter {
public:
    explicit CsvWriter(std::string path);

    void write_header(const MeasurementResult& result);
    void append(const MeasurementResult& result);

private:
    std::string path_;
};

}  // namespace ntq::output
