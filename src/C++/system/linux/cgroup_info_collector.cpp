#include "cgroup_info_collector.hpp"

#include "cgroup_collector_common.hpp"
#include "cgroup_parsers.hpp"
#include "cgroup_v1_collector.hpp"
#include "cgroup_v2_collector.hpp"
#include "file_reader.hpp"

#include <string>
#include <vector>

namespace ntq {
namespace linux_system {
namespace {

using cgroup_detail::find_mount;
using cgroup_detail::find_mount_for_path;
using cgroup_detail::set_limits_status;

SandboxInfo unavailable_sandbox(CollectionStatus status,
                                const std::string &source) {
  SandboxInfo sandbox;
  sandbox.cgroup_version = CollectedValue<unsigned>(0, status, source);
  sandbox.cgroup_path =
      CollectedValue<std::string>(std::string(), status, source);
  set_limits_status(sandbox, status, source);
  return sandbox;
}

} // namespace

SandboxInfo collect_sandbox_info() {
  const auto membership_text = read_text_file("/proc/self/cgroup");
  const auto mount_text = read_text_file("/proc/self/mountinfo");
  if (membership_text.status != CollectionStatus::available ||
      mount_text.status != CollectionStatus::available) {
    const CollectionStatus status =
        membership_text.status != CollectionStatus::available
            ? membership_text.status
            : mount_text.status;
    const std::string source =
        membership_text.status != CollectionStatus::available
            ? membership_text.source
            : mount_text.source;
    return unavailable_sandbox(status, source);
  }

  const CgroupMembership membership =
      parse_cgroup_membership(membership_text.value);
  const std::vector<CgroupMount> mounts = parse_cgroup_mounts(mount_text.value);
  if (!membership.unified_path.empty()) {
    const CgroupMount *unified =
        find_mount_for_path(mounts, 2, std::string(), membership.unified_path);
    if (unified != nullptr) {
      return collect_cgroup_v2(*unified, membership.unified_path);
    }
    const bool has_unified_mount = find_mount(mounts, 2) != nullptr;
    SandboxInfo sandbox;
    sandbox.cgroup_version = CollectedValue<unsigned>(
        2, has_unified_mount ? CollectionStatus::available
                             : CollectionStatus::unavailable,
        "/proc/self/mountinfo");
    sandbox.cgroup_path = CollectedValue<std::string>(
        std::string(), CollectionStatus::unavailable, "/proc/self/cgroup");
    set_limits_status(sandbox, CollectionStatus::unavailable,
                      "/proc/self/cgroup");
    return sandbox;
  }

  if (find_mount(mounts, 2) != nullptr) {
    SandboxInfo sandbox;
    sandbox.cgroup_version = CollectedValue<unsigned>(
        2, CollectionStatus::available, "/proc/self/mountinfo");
    sandbox.cgroup_path = CollectedValue<std::string>(
        std::string(), CollectionStatus::unavailable, "/proc/self/cgroup");
    set_limits_status(sandbox, CollectionStatus::unavailable,
                      "/proc/self/cgroup");
    return sandbox;
  }

  if (find_mount(mounts, 1, "cpu") != nullptr ||
      find_mount(mounts, 1, "memory") != nullptr ||
      find_mount(mounts, 1, "cpuset") != nullptr) {
    return collect_cgroup_v1(membership, mounts);
  }

  return unavailable_sandbox(CollectionStatus::unavailable,
                             "/proc/self/mountinfo");
}

} // namespace linux_system
} // namespace ntq
