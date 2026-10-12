#pragma once

#include "cgroup_parsers.hpp"

namespace ntq {
namespace linux_system {

SandboxInfo collect_cgroup_v1(const CgroupMembership &membership,
                              const std::vector<CgroupMount> &mounts);

} // namespace linux_system
} // namespace ntq
