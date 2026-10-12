#include "cgroup_collector_common.hpp"

#include <algorithm>

namespace ntq {
namespace linux_system {
namespace cgroup_detail {

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

const CgroupMount *find_mount(const std::vector<CgroupMount> &mounts,
                              unsigned version,
                              const std::string &controller) {
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

} // namespace cgroup_detail
} // namespace linux_system
} // namespace ntq
