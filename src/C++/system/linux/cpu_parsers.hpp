#pragma once

#include "system/system_info.hpp"

#include <string>
#include <vector>

namespace ntq {
namespace linux_system {

struct CpuIdentity {
  CollectedValue<std::string> vendor;
  CollectedValue<std::string> model;
};

CollectedValue<std::vector<unsigned>> parse_cpu_list(const std::string &text,
                                                     const std::string &source);
CollectedValue<std::uint64_t> parse_cache_size(const std::string &text,
                                               const std::string &source);
CpuIdentity parse_cpuinfo(const std::string &text);
std::vector<CacheInfo>
deduplicate_caches(const std::vector<CacheInfo> &caches);

} // namespace linux_system
} // namespace ntq
