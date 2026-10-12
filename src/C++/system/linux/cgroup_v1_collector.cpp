#include "cgroup_v1_collector.hpp"

#include "cgroup_collector_common.hpp"
#include "file_reader.hpp"
#include "text_parsers.hpp"

#include <string>
#include <vector>

namespace ntq {
namespace linux_system {
namespace {

using cgroup_detail::ancestor_paths;
using cgroup_detail::find_mount_for_path;
using cgroup_detail::join_path;
using cgroup_detail::map_cgroup_path;
using cgroup_detail::set_limits_status;

std::vector<CollectedValue<CpuQuota>> read_cpu_quota_v1(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<CpuQuota>> values;
  for (const std::string &path : paths) {
    const std::string quota_path = join_path(path, "cpu.cfs_quota_us");
    const std::string period_path = join_path(path, "cpu.cfs_period_us");
    const auto quota = read_text_file(quota_path);
    const auto period = read_text_file(period_path);
    if (cgroup_detail::normalized_path(path) ==
            cgroup_detail::normalized_path(mount_point) &&
        quota.status == CollectionStatus::unavailable &&
        period.status == CollectionStatus::unavailable) {
      values.push_back(CollectedValue<CpuQuota>(
          CpuQuota{}, CollectionStatus::unlimited,
          "cgroup v1 mount root (unbounded)"));
    } else if (quota.status != CollectionStatus::available ||
               period.status != CollectionStatus::available) {
      const bool quota_unavailable =
          quota.status != CollectionStatus::available;
      values.push_back(CollectedValue<CpuQuota>(
          CpuQuota(), quota_unavailable ? quota.status : period.status,
          quota_unavailable ? quota_path : period_path));
    } else {
      values.push_back(parse_cpu_quota_v1(quota.value, period.value,
                                          quota_path + "," + period_path));
    }
  }
  return values;
}

std::vector<CollectedValue<std::uint64_t>> read_memory_v1(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<std::uint64_t>> values;
  for (const std::string &path : paths) {
    const std::string file = join_path(path, "memory.limit_in_bytes");
    const auto contents = read_text_file(file);
    if (cgroup_detail::normalized_path(path) ==
            cgroup_detail::normalized_path(mount_point) &&
        contents.status == CollectionStatus::unavailable) {
      values.push_back(CollectedValue<std::uint64_t>(
          0, CollectionStatus::unlimited,
          "cgroup v1 mount root (unbounded)"));
    } else {
      values.push_back(contents.status == CollectionStatus::available
                           ? parse_memory_limit(contents.value, file)
                           : CollectedValue<std::uint64_t>(0, contents.status,
                                                           file));
    }
  }
  return values;
}

CollectedValue<std::vector<unsigned>> read_cpuset_v1(
    const std::vector<std::string> &paths) {
  std::vector<CollectedValue<std::vector<unsigned>>> configured_sets;
  for (const std::string &path : paths) {
    const std::string file = join_path(path, "cpuset.cpus");
    const auto contents = read_text_file(file);
    if (contents.status != CollectionStatus::available) {
      configured_sets.push_back(CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>(), contents.status, file));
    } else if (!text::trim(contents.value).empty()) {
      configured_sets.push_back(parse_cpu_set(contents.value, file));
    }
  }
  return resolve_effective_cpu_set(configured_sets);
}

} // namespace

SandboxInfo collect_cgroup_v1(const CgroupMembership &membership,
                              const std::vector<CgroupMount> &mounts) {
  SandboxInfo sandbox;
  sandbox.cgroup_version =
      CollectedValue<unsigned>(1, CollectionStatus::available,
                               "/proc/self/mountinfo");
  set_limits_status(sandbox, CollectionStatus::unavailable,
                    "cgroup v1 controller files");
  sandbox.cgroup_path = CollectedValue<std::string>(
      std::string(), CollectionStatus::unavailable, "/proc/self/cgroup");

  for (const auto &controller_path : membership.controller_paths) {
    const CgroupMount *mount = find_mount_for_path(
        mounts, 1, controller_path.first, controller_path.second);
    std::string leaf;
    if (mount != nullptr &&
        map_cgroup_path(*mount, controller_path.second, leaf)) {
      sandbox.cgroup_path = CollectedValue<std::string>(
          leaf, CollectionStatus::available, "/proc/self/cgroup + mountinfo");
      break;
    }
  }

  const auto cpu_member = membership.controller_paths.find("cpu");
  const CgroupMount *cpu_mount =
      cpu_member == membership.controller_paths.end()
          ? nullptr
          : find_mount_for_path(mounts, 1, "cpu", cpu_member->second);
  if (cpu_member != membership.controller_paths.end() && cpu_mount != nullptr) {
    std::string leaf;
    if (map_cgroup_path(*cpu_mount, cpu_member->second, leaf)) {
      const auto quota = resolve_effective_cpu_quota(read_cpu_quota_v1(
          ancestor_paths(leaf, cpu_mount->mount_point), cpu_mount->mount_point));
      sandbox.effective_cpu_quota_us =
          CollectedValue<std::uint64_t>(quota.value.quota_us, quota.status,
                                        quota.source);
      sandbox.effective_cpu_period_us =
          CollectedValue<std::uint64_t>(quota.value.period_us, quota.status,
                                        quota.source);
      sandbox.effective_cpu_quota_cores = CollectedValue<double>(
          quota.status == CollectionStatus::available &&
                  quota.value.period_us != 0
              ? static_cast<double>(quota.value.quota_us) /
                    quota.value.period_us
              : 0.0,
          quota.status, quota.source);
      sandbox.cgroup_path = CollectedValue<std::string>(
          leaf, CollectionStatus::available, "/proc/self/cgroup + mountinfo");
    } else {
      sandbox.effective_cpu_quota_us.status = CollectionStatus::malformed;
      sandbox.effective_cpu_period_us.status = CollectionStatus::malformed;
      sandbox.effective_cpu_quota_cores.status = CollectionStatus::malformed;
      sandbox.effective_cpu_quota_us.source = cpu_member->second;
      sandbox.effective_cpu_period_us.source = cpu_member->second;
      sandbox.effective_cpu_quota_cores.source = cpu_member->second;
      sandbox.cgroup_path = CollectedValue<std::string>(
          cpu_member->second, CollectionStatus::malformed,
          "/proc/self/cgroup + mountinfo");
    }
  }

  const auto memory_member = membership.controller_paths.find("memory");
  const CgroupMount *memory_mount =
      memory_member == membership.controller_paths.end()
          ? nullptr
          : find_mount_for_path(mounts, 1, "memory", memory_member->second);
  if (memory_member != membership.controller_paths.end() &&
      memory_mount != nullptr) {
    std::string leaf;
    if (map_cgroup_path(*memory_mount, memory_member->second, leaf)) {
      sandbox.effective_memory_limit_bytes = resolve_effective_memory_limit(
          read_memory_v1(ancestor_paths(leaf, memory_mount->mount_point),
                         memory_mount->mount_point));
      if (sandbox.cgroup_path.status != CollectionStatus::available) {
        sandbox.cgroup_path = CollectedValue<std::string>(
            leaf, CollectionStatus::available,
            "/proc/self/cgroup + mountinfo");
      }
    } else {
      sandbox.effective_memory_limit_bytes.status =
          CollectionStatus::malformed;
      sandbox.effective_memory_limit_bytes.source = memory_member->second;
    }
  }

  const auto cpuset_member = membership.controller_paths.find("cpuset");
  const CgroupMount *cpuset_mount =
      cpuset_member == membership.controller_paths.end()
          ? nullptr
          : find_mount_for_path(mounts, 1, "cpuset", cpuset_member->second);
  if (cpuset_member != membership.controller_paths.end() &&
      cpuset_mount != nullptr) {
    std::string leaf;
    if (map_cgroup_path(*cpuset_mount, cpuset_member->second, leaf)) {
      sandbox.effective_allowed_cpus = read_cpuset_v1(
          ancestor_paths(leaf, cpuset_mount->mount_point));
    } else {
      sandbox.effective_allowed_cpus.status = CollectionStatus::malformed;
      sandbox.effective_allowed_cpus.source = cpuset_member->second;
    }
  }

  if (sandbox.effective_cpu_quota_cores.status ==
      CollectionStatus::unavailable) {
    sandbox.effective_cpu_quota_cores.source = "cgroup v1 cpu controller";
  }
  if (sandbox.effective_memory_limit_bytes.status ==
      CollectionStatus::unavailable) {
    sandbox.effective_memory_limit_bytes.source =
        "cgroup v1 memory controller";
  }
  if (sandbox.effective_allowed_cpus.status == CollectionStatus::unavailable) {
    sandbox.effective_allowed_cpus.source = "cgroup v1 cpuset controller";
  }
  return sandbox;
}

} // namespace linux_system
} // namespace ntq
