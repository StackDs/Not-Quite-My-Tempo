#pragma once

#include "system/system_info.hpp"

#include <map>
#include <string>
#include <vector>

namespace ntq {
namespace linux_system {

CollectionStatus status_from_errno(int error);

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

struct CpuQuota {
  std::uint64_t quota_us{0};
  std::uint64_t period_us{0};
};

struct CgroupMembership {
  std::string unified_path;
  std::map<std::string, std::string> controller_paths;
};

struct CgroupMount {
  unsigned version{0};
  std::string root;
  std::string mount_point;
  std::vector<std::string> controllers;
};

CgroupMembership parse_cgroup_membership(const std::string &text);
std::vector<CgroupMount> parse_cgroup_mounts(const std::string &text);
CollectedValue<CpuQuota> parse_cpu_quota_v1(const std::string &quota,
                                            const std::string &period,
                                            const std::string &source);
CollectedValue<CpuQuota> parse_cpu_quota_v2(const std::string &text,
                                            const std::string &source);
CollectedValue<std::uint64_t> parse_memory_limit(const std::string &text,
                                                 const std::string &source);
CollectedValue<std::vector<unsigned>> parse_cpu_set(const std::string &text,
                                                    const std::string &source);
CollectedValue<CpuQuota> resolve_effective_cpu_quota(
    const std::vector<CollectedValue<CpuQuota>> &ancestors);
CollectedValue<std::vector<unsigned>> resolve_effective_cpu_set(
    const std::vector<CollectedValue<std::vector<unsigned>>> &ancestors);
CollectedValue<std::uint64_t> resolve_effective_memory_limit(
    const std::vector<CollectedValue<std::uint64_t>> &ancestors);

} // namespace linux_system
} // namespace ntq
