#include "cgroup_v2_collector.hpp"

#include "cgroup_collector_common.hpp"
#include "file_reader.hpp"
#include "text_parsers.hpp"

#include <string>
#include <vector>

namespace ntq {
namespace linux_system {
namespace {

using cgroup_detail::ancestor_paths;
using cgroup_detail::join_path;
using cgroup_detail::map_cgroup_path;
using cgroup_detail::normalized_path;
using cgroup_detail::set_limits_status;

std::vector<CollectedValue<CpuQuota>> read_cpu_quota_v2(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<CpuQuota>> values;
  for (const std::string &path : paths) {
    if (normalized_path(path) == normalized_path(mount_point)) {
      values.push_back(CollectedValue<CpuQuota>(
          CpuQuota{}, CollectionStatus::unlimited,
          "cgroup v2 mount root (unbounded)"));
      continue;
    }
    const std::string file = join_path(path, "cpu.max");
    const auto contents = read_text_file(file);
    values.push_back(contents.status == CollectionStatus::available
                         ? parse_cpu_quota_v2(contents.value, file)
                         : CollectedValue<CpuQuota>(CpuQuota{}, contents.status,
                                                    file));
  }
  return values;
}

std::vector<CollectedValue<std::uint64_t>> read_memory_v2(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<std::uint64_t>> values;
  for (const std::string &path : paths) {
    if (normalized_path(path) == normalized_path(mount_point)) {
      values.push_back(CollectedValue<std::uint64_t>(
          0, CollectionStatus::unlimited,
          "cgroup v2 mount root (unbounded)"));
      continue;
    }
    const std::string file = join_path(path, "memory.max");
    const auto contents = read_text_file(file);
    values.push_back(contents.status == CollectionStatus::available
                         ? parse_memory_limit(contents.value, file)
                         : CollectedValue<std::uint64_t>(0, contents.status,
                                                         file));
  }
  return values;
}

CollectedValue<std::vector<unsigned>> read_cpuset_v2(
    const std::vector<std::string> &paths) {
  for (const std::string &path : paths) {
    const std::string effective = join_path(path, "cpuset.cpus.effective");
    const auto contents = read_text_file(effective);
    if (contents.status == CollectionStatus::available &&
        !text::trim(contents.value).empty()) {
      return parse_cpu_set(contents.value, effective);
    }
    if (contents.status == CollectionStatus::permission_denied ||
        contents.status == CollectionStatus::read_error ||
        contents.status == CollectionStatus::malformed) {
      return CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>(), contents.status, effective);
    }
  }

  std::vector<CollectedValue<std::vector<unsigned>>> configured_sets;
  for (const std::string &path : paths) {
    const std::string cpus = join_path(path, "cpuset.cpus");
    const auto contents = read_text_file(cpus);
    if (contents.status != CollectionStatus::available) {
      configured_sets.push_back(CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>(), contents.status, cpus));
    } else if (!text::trim(contents.value).empty()) {
      configured_sets.push_back(parse_cpu_set(contents.value, cpus));
    }
  }
  return resolve_effective_cpu_set(configured_sets);
}

} // namespace

SandboxInfo collect_cgroup_v2(const CgroupMount &mount,
                              const std::string &member_path) {
  SandboxInfo sandbox;
  std::string leaf;
  if (!map_cgroup_path(mount, member_path, leaf)) {
    sandbox.cgroup_version = CollectedValue<unsigned>(
        2, CollectionStatus::available, "cgroup2 mountinfo");
    sandbox.cgroup_path = CollectedValue<std::string>(
        member_path, CollectionStatus::malformed, "/proc/self/cgroup");
    set_limits_status(sandbox, CollectionStatus::malformed,
                      "/proc/self/cgroup + mountinfo");
    return sandbox;
  }

  const auto paths = ancestor_paths(leaf, mount.mount_point);
  sandbox.cgroup_version = CollectedValue<unsigned>(
      2, CollectionStatus::available, "cgroup2 mountinfo");
  sandbox.cgroup_path = CollectedValue<std::string>(
      leaf, CollectionStatus::available, "/proc/self/cgroup + mountinfo");

  const auto quota = resolve_effective_cpu_quota(
      read_cpu_quota_v2(paths, mount.mount_point));
  sandbox.effective_cpu_quota_us =
      CollectedValue<std::uint64_t>(quota.value.quota_us, quota.status,
                                    quota.source);
  sandbox.effective_cpu_period_us =
      CollectedValue<std::uint64_t>(quota.value.period_us, quota.status,
                                    quota.source);
  sandbox.effective_cpu_quota_cores = CollectedValue<double>(
      quota.status == CollectionStatus::available && quota.value.period_us != 0
          ? static_cast<double>(quota.value.quota_us) / quota.value.period_us
          : 0.0,
      quota.status, quota.source);
  sandbox.effective_memory_limit_bytes =
      resolve_effective_memory_limit(read_memory_v2(paths, mount.mount_point));
  sandbox.effective_allowed_cpus = read_cpuset_v2(paths);
  return sandbox;
}

} // namespace linux_system
} // namespace ntq
