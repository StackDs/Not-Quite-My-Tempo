#pragma once

#include "system/system_info.hpp"

#include <string>

namespace ntq {
namespace linux_system {

CollectionStatus status_from_errno(int error);
CollectedValue<std::string> read_text_file(const std::string &path);

} // namespace linux_system
} // namespace ntq
