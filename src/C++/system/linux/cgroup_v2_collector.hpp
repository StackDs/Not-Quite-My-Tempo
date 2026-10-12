#pragma once

#include "cgroup_parsers.hpp"

namespace ntq {
namespace linux_system {

SandboxInfo collect_cgroup_v2(const CgroupMount &mount,
                              const std::string &member_path);

} // namespace linux_system
} // namespace ntq
