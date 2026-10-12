#pragma once

#include "cgroup_parsers.hpp"

#include <string>
#include <vector>

namespace ntq {
namespace linux_system {
namespace cgroup_detail {

std::string normalized_path(std::string path);
bool path_is_within(const std::string &path, const std::string &root);
bool map_cgroup_path(const CgroupMount &mount, const std::string &member,
                     std::string &mapped);
std::vector<std::string> ancestor_paths(const std::string &leaf,
                                        const std::string &mount_point);
std::string join_path(const std::string &directory, const std::string &name);
void set_limits_status(SandboxInfo &sandbox, CollectionStatus status,
                       const std::string &source);
const CgroupMount *find_mount(const std::vector<CgroupMount> &mounts,
                              unsigned version,
                              const std::string &controller = std::string());
const CgroupMount *find_mount_for_path(const std::vector<CgroupMount> &mounts,
                                       unsigned version,
                                       const std::string &controller,
                                       const std::string &member_path);

} // namespace cgroup_detail
} // namespace linux_system
} // namespace ntq
