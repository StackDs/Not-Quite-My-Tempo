#pragma once

#include "system/system_info.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace ntq {
namespace linux_system {

namespace text {

std::string trim(const std::string &value);
bool parse_unsigned(const std::string &text, std::uint64_t &value);
std::vector<std::string> split(const std::string &text, char delimiter);
std::vector<std::string> whitespace_fields(const std::string &text);
std::string decode_mount_path(std::string path);

template <class T>
CollectedValue<T> status_value(CollectionStatus status,
                               const std::string &source) {
  return CollectedValue<T>(T(), status, source);
}

} // namespace text
} // namespace linux_system
} // namespace ntq
