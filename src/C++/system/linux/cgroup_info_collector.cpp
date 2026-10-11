#include "cgroup_info_collector.hpp"

#include "cgroup_parsers.hpp"
#include "file_reader.hpp"
#include "text_parsers.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace ntq {
namespace linux_system {
namespace {

std::string normalized_path(std::string path) {
  if (path.empty()) {
    return "/";
  }
  while (path.size() > 1 && path[path.size() - 1] == '/') {
    path.erase(path.size() - 1);
  }
  return path;
}

bool path_is_within(const std::string &path, const std::string &root) {
  if (root == "/") {
    return !path.empty() && path[0] == '/';
  }
  return path == root ||
         (path.size() > root.size() && path.compare(0, root.size(), root) == 0 &&
          path[root.size()] == '/');
}

bool map_cgroup_path(const CgroupMount &mount, const std::string &member,
                     std::string &mapped) {
  const std::string root = normalized_path(mount.root);
  const std::string mount_point = normalized_path(mount.mount_point);
  const std::string path = normalized_path(member);
  if (!path_is_within(path, root)) {
    return false;
  }
  std::string relative = root == "/" ? path.substr(1) : path.substr(root.size());
  if (!relative.empty() && relative[0] == '/') {
    relative.erase(0, 1);
  }
  mapped = mount_point;
  if (!relative.empty()) {
    if (mapped != "/") {
      mapped += "/";
    }
    mapped += relative;
  }
  return true;
}

std::vector<std::string> ancestor_paths(const std::string &leaf,
                                       const std::string &mount_point) {
  std::vector<std::string> paths;
  std::string path = normalized_path(leaf);
  const std::string root = normalized_path(mount_point);
  if (!path_is_within(path, root)) {
    return paths;
  }
  for (;;) {
    paths.push_back(path);
    if (path == root || path == "/") {
      break;
    }
    const std::string::size_type slash = path.find_last_of('/');
    path = slash == 0 ? "/" : path.substr(0, slash);
  }
  return paths;
}

std::string join_path(const std::string &directory, const std::string &name) {
  const std::string normalized = normalized_path(directory);
  return normalized + (normalized == "/" ? "" : "/") + name;
}

void set_limits_status(SandboxInfo &sandbox, CollectionStatus status,
                       const std::string &source) {
  sandbox.effective_cpu_quota_us =
      CollectedValue<std::uint64_t>(0, status, source);
  sandbox.effective_cpu_period_us =
      CollectedValue<std::uint64_t>(0, status, source);
  sandbox.effective_cpu_quota_cores = CollectedValue<double>(0.0, status, source);
  sandbox.effective_allowed_cpus =
      CollectedValue<std::vector<unsigned>>(std::vector<unsigned>(), status,
                                            source);
  sandbox.effective_memory_limit_bytes =
      CollectedValue<std::uint64_t>(0, status, source);
}

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

SandboxInfo collect_v2(const CgroupMount &mount,
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

std::vector<CollectedValue<CpuQuota>> read_cpu_quota_v1(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<CpuQuota>> values;
  for (const std::string &path : paths) {
    const std::string quota_path = join_path(path, "cpu.cfs_quota_us");
    const std::string period_path = join_path(path, "cpu.cfs_period_us");
    const auto quota = read_text_file(quota_path);
    const auto period = read_text_file(period_path);
    if (normalized_path(path) == normalized_path(mount_point) &&
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
    if (normalized_path(path) == normalized_path(mount_point) &&
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

const CgroupMount *find_mount(const std::vector<CgroupMount> &mounts,
                              unsigned version,
                              const std::string &controller = std::string()) {
  for (const CgroupMount &mount : mounts) {
    if (mount.version != version) {
      continue;
    }
    if (version == 2 ||
        std::find(mount.controllers.begin(), mount.controllers.end(),
                  controller) != mount.controllers.end()) {
      return &mount;
    }
  }
  return nullptr;
}

const CgroupMount *find_mount_for_path(const std::vector<CgroupMount> &mounts,
                                       unsigned version,
                                       const std::string &controller,
                                       const std::string &member_path) {
  const CgroupMount *best = nullptr;
  for (const CgroupMount &mount : mounts) {
    if (mount.version != version ||
        (version == 1 &&
         std::find(mount.controllers.begin(), mount.controllers.end(),
                   controller) == mount.controllers.end())) {
      continue;
    }
    std::string mapped;
    if (map_cgroup_path(mount, member_path, mapped) &&
        (best == nullptr || mount.root.size() > best->root.size())) {
      best = &mount;
    }
  }
  return best;
}

SandboxInfo collect_v1(const CgroupMembership &membership,
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

} // namespace

SandboxInfo collect_sandbox_info() {
  const auto membership_text = read_text_file("/proc/self/cgroup");
  const auto mount_text = read_text_file("/proc/self/mountinfo");
  if (membership_text.status != CollectionStatus::available ||
      mount_text.status != CollectionStatus::available) {
    SandboxInfo sandbox;
    const CollectionStatus status =
        membership_text.status != CollectionStatus::available
            ? membership_text.status
            : mount_text.status;
    const std::string source =
        membership_text.status != CollectionStatus::available
            ? membership_text.source
            : mount_text.source;
    sandbox.cgroup_version = CollectedValue<unsigned>(0, status, source);
    sandbox.cgroup_path =
        CollectedValue<std::string>(std::string(), status, source);
    set_limits_status(sandbox, status, source);
    return sandbox;
  }

  const CgroupMembership membership =
      parse_cgroup_membership(membership_text.value);
  const auto mounts = parse_cgroup_mounts(mount_text.value);
  if (!membership.unified_path.empty()) {
    const CgroupMount *unified =
        find_mount_for_path(mounts, 2, std::string(), membership.unified_path);
    if (unified != nullptr) {
      return collect_v2(*unified, membership.unified_path);
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
    return collect_v1(membership, mounts);
  }
  SandboxInfo sandbox;
  sandbox.cgroup_version = CollectedValue<unsigned>(
      0, CollectionStatus::unavailable, "/proc/self/mountinfo");
  sandbox.cgroup_path = CollectedValue<std::string>(
      std::string(), CollectionStatus::unavailable, "/proc/self/cgroup");
  set_limits_status(sandbox, CollectionStatus::unavailable,
                    "/proc/self/mountinfo");
  return sandbox;
}

} // namespace linux_system
} // namespace ntq
