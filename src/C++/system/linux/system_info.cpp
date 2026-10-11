#include "system/system_info.hpp"

#include "compiler_info.hpp"
#include "parsers.hpp"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <sched.h>
#include <sstream>
#include <string>
#include <sys/utsname.h>
#include <unistd.h>

namespace ntq {
namespace linux_system {
namespace {

CollectedValue<std::string> read_text(const std::string &path) {
  const int descriptor = open(path.c_str(), O_RDONLY | O_CLOEXEC);
  if (descriptor == -1) {
    return CollectedValue<std::string>(std::string(), status_from_errno(errno),
                                       path);
  }

  std::string contents;
  char buffer[4096];
  for (;;) {
    const ssize_t bytes = read(descriptor, buffer, sizeof(buffer));
    if (bytes == 0) {
      break;
    }
    if (bytes == -1) {
      if (errno == EINTR) {
        continue;
      }
      const int error = errno;
      close(descriptor);
      return CollectedValue<std::string>(std::string(),
                                         status_from_errno(error), path);
    }
    contents.append(buffer, static_cast<std::size_t>(bytes));
  }
  close(descriptor);
  return CollectedValue<std::string>(contents, CollectionStatus::available,
                                     path);
}

std::string trim(const std::string &value) {
  std::string::size_type first = 0;
  while (first < value.size() &&
         std::isspace(static_cast<unsigned char>(value[first]))) {
    ++first;
  }
  std::string::size_type last = value.size();
  while (last > first &&
         std::isspace(static_cast<unsigned char>(value[last - 1]))) {
    --last;
  }
  return value.substr(first, last - first);
}

CollectedValue<std::uint64_t> parse_number(const std::string &text,
                                           const std::string &source) {
  const std::string value = trim(text);
  if (value.empty()) {
    return CollectedValue<std::uint64_t>(0, CollectionStatus::malformed,
                                         source);
  }
  for (char character : value) {
    if (character < '0' || character > '9') {
      return CollectedValue<std::uint64_t>(0, CollectionStatus::malformed,
                                           source);
    }
  }
  errno = 0;
  char *end = nullptr;
  const unsigned long long number = std::strtoull(value.c_str(), &end, 10);
  if (errno == ERANGE || end == value.c_str() || *end != '\0') {
    return CollectedValue<std::uint64_t>(0, CollectionStatus::malformed,
                                         source);
  }
  return CollectedValue<std::uint64_t>(
      static_cast<std::uint64_t>(number), CollectionStatus::available, source);
}

CollectedValue<std::uint64_t> read_frequency_hz(const std::string &path) {
  const auto khz = read_text(path);
  if (khz.status != CollectionStatus::available) {
    return CollectedValue<std::uint64_t>(0, khz.status, khz.source);
  }
  const auto value = parse_number(khz.value, path);
  if (value.status != CollectionStatus::available) {
    return value;
  }
  if (value.value > std::numeric_limits<std::uint64_t>::max() / 1000) {
    return CollectedValue<std::uint64_t>(0, CollectionStatus::malformed, path);
  }
  return CollectedValue<std::uint64_t>(value.value * 1000,
                                       CollectionStatus::available, path);
}

std::string cpu_path(unsigned cpu) {
  return "/sys/devices/system/cpu/cpu" + std::to_string(cpu);
}

CollectedValue<std::vector<unsigned>> online_cpus() {
  const std::string path = "/sys/devices/system/cpu/online";
  const auto text = read_text(path);
  if (text.status == CollectionStatus::available) {
    return parse_cpu_list(text.value, path);
  }
  return CollectedValue<std::vector<unsigned>>(std::vector<unsigned>(),
                                                text.status, text.source);
}

CollectedValue<std::vector<unsigned>> process_affinity() {
  cpu_set_t set;
  CPU_ZERO(&set);
  if (sched_getaffinity(0, sizeof(set), &set) == -1) {
    return CollectedValue<std::vector<unsigned>>(
        std::vector<unsigned>(), status_from_errno(errno), "sched_getaffinity");
  }
  std::vector<unsigned> cpus;
  for (unsigned cpu = 0; cpu < CPU_SETSIZE; ++cpu) {
    if (CPU_ISSET(cpu, &set)) {
      cpus.push_back(cpu);
    }
  }
  return CollectedValue<std::vector<unsigned>>(cpus,
                                                CollectionStatus::available,
                                                "sched_getaffinity");
}

CollectedValue<bool> boost_state() {
  CollectionStatus unavailable_status = CollectionStatus::unavailable;
  std::string unavailable_source = "kernel boost/Turbo sysfs interfaces";
  const std::string intel_path =
      "/sys/devices/system/cpu/intel_pstate/no_turbo";
  const auto intel = read_text(intel_path);
  if (intel.status == CollectionStatus::available) {
    const std::string value = trim(intel.value);
    if (value == "0" || value == "1") {
      return CollectedValue<bool>(value == "0", CollectionStatus::available,
                                  intel_path);
    }
    return CollectedValue<bool>(false, CollectionStatus::malformed,
                                intel_path);
  }
  if (intel.status == CollectionStatus::permission_denied ||
      intel.status == CollectionStatus::read_error) {
    unavailable_status = intel.status;
    unavailable_source = intel_path;
  }

  const std::string generic_path = "/sys/devices/system/cpu/cpufreq/boost";
  const auto generic = read_text(generic_path);
  if (generic.status == CollectionStatus::available) {
    const std::string value = trim(generic.value);
    if (value == "0" || value == "1") {
      return CollectedValue<bool>(value == "1", CollectionStatus::available,
                                  generic_path);
    }
    return CollectedValue<bool>(false, CollectionStatus::malformed,
                                generic_path);
  }
  if (generic.status == CollectionStatus::permission_denied ||
      generic.status == CollectionStatus::read_error) {
    unavailable_status = generic.status;
    unavailable_source = generic_path;
  }

  const std::string amd_path =
      "/sys/devices/system/cpu/amd_pstate/cpb";
  const auto amd = read_text(amd_path);
  if (amd.status == CollectionStatus::available) {
    const std::string value = trim(amd.value);
    if (value == "0" || value == "1") {
      return CollectedValue<bool>(value == "1", CollectionStatus::available,
                                  amd_path);
    }
    return CollectedValue<bool>(false, CollectionStatus::malformed, amd_path);
  }
  if (amd.status == CollectionStatus::permission_denied ||
      amd.status == CollectionStatus::read_error) {
    unavailable_status = amd.status;
    unavailable_source = amd_path;
  }

  return CollectedValue<bool>(false, unavailable_status, unavailable_source);
}

CpuRuntimeInfo collect_cpu_runtime(const std::vector<unsigned> &cpus) {
  CpuRuntimeInfo runtime;
  runtime.process_affinity = process_affinity();
  runtime.turbo_or_boost_enabled = boost_state();

  std::vector<CpuFrequencyInfo> frequencies;
  for (unsigned cpu : cpus) {
    CpuFrequencyInfo info;
    info.cpu_id = cpu;
    const std::string cpufreq = cpu_path(cpu) + "/cpufreq/";

    const std::string governor_path = cpufreq + "scaling_governor";
    const auto governor = read_text(governor_path);
    if (governor.status == CollectionStatus::available) {
      info.governor = CollectedValue<std::string>(
          trim(governor.value), CollectionStatus::available, governor_path);
    } else {
      info.governor = CollectedValue<std::string>(
          std::string(), governor.status, governor_path);
    }

    info.current_frequency_hz =
        read_frequency_hz(cpufreq + "scaling_cur_freq");
    info.minimum_frequency_hz =
        read_frequency_hz(cpufreq + "scaling_min_freq");
    info.maximum_frequency_hz =
        read_frequency_hz(cpufreq + "scaling_max_freq");
    frequencies.push_back(info);
  }
  runtime.per_cpu = CollectedValue<std::vector<CpuFrequencyInfo>>(
      frequencies,
      cpus.empty() ? CollectionStatus::unavailable
                   : CollectionStatus::available,
      "/sys/devices/system/cpu/cpu*/cpufreq");
  return runtime;
}

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
  return normalized_path(directory) +
         (normalized_path(directory) == "/" ? "" : "/") + name;
}

void set_sandbox_limits_status(SandboxInfo &sandbox, CollectionStatus status,
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

std::vector<CollectedValue<CpuQuota>> collect_cpu_quota_v2(
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
    const auto contents = read_text(file);
    if (contents.status != CollectionStatus::available) {
      values.push_back(CollectedValue<CpuQuota>(CpuQuota{}, contents.status,
                                                file));
    } else {
      values.push_back(parse_cpu_quota_v2(contents.value, file));
    }
  }
  return values;
}

std::vector<CollectedValue<std::uint64_t>> collect_memory_v2(
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
    const auto contents = read_text(file);
    if (contents.status != CollectionStatus::available) {
      values.push_back(CollectedValue<std::uint64_t>(0, contents.status, file));
    } else {
      values.push_back(parse_memory_limit(contents.value, file));
    }
  }
  return values;
}

CollectedValue<std::vector<unsigned>> collect_cpuset_v2(
    const std::vector<std::string> &paths) {
  for (const std::string &path : paths) {
    const std::string effective = join_path(path, "cpuset.cpus.effective");
    const auto contents = read_text(effective);
    if (contents.status == CollectionStatus::available &&
        !trim(contents.value).empty()) {
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
    const auto contents = read_text(cpus);
    if (contents.status != CollectionStatus::available) {
      configured_sets.push_back(CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>(), contents.status, cpus));
      continue;
    }
    if (trim(contents.value).empty()) {
      continue;
    }
    configured_sets.push_back(parse_cpu_set(contents.value, cpus));
  }
  return resolve_effective_cpu_set(configured_sets);
}

SandboxInfo collect_cgroup_v2(const CgroupMount &mount,
                              const std::string &member_path) {
  SandboxInfo sandbox;
  std::string leaf;
  if (!map_cgroup_path(mount, member_path, leaf)) {
    sandbox.cgroup_version = CollectedValue<unsigned>(
        2, CollectionStatus::available, "cgroup2 mountinfo");
    sandbox.cgroup_path = CollectedValue<std::string>(
        member_path, CollectionStatus::malformed, "/proc/self/cgroup");
    set_sandbox_limits_status(sandbox, CollectionStatus::malformed,
                              "/proc/self/cgroup + mountinfo");
    return sandbox;
  }

  const std::vector<std::string> paths = ancestor_paths(leaf, mount.mount_point);
  sandbox.cgroup_version = CollectedValue<unsigned>(
      2, CollectionStatus::available, "cgroup2 mountinfo");
  sandbox.cgroup_path = CollectedValue<std::string>(
      leaf, CollectionStatus::available, "/proc/self/cgroup + mountinfo");

  const auto quota = resolve_effective_cpu_quota(
      collect_cpu_quota_v2(paths, mount.mount_point));
  sandbox.effective_cpu_quota_us = CollectedValue<std::uint64_t>(
      quota.value.quota_us, quota.status, quota.source);
  sandbox.effective_cpu_period_us = CollectedValue<std::uint64_t>(
      quota.value.period_us, quota.status, quota.source);
  sandbox.effective_cpu_quota_cores = CollectedValue<double>(
      quota.status == CollectionStatus::available &&
              quota.value.period_us != 0
          ? static_cast<double>(quota.value.quota_us) / quota.value.period_us
          : 0.0,
      quota.status, quota.source);
  sandbox.effective_memory_limit_bytes =
      resolve_effective_memory_limit(
          collect_memory_v2(paths, mount.mount_point));
  sandbox.effective_allowed_cpus = collect_cpuset_v2(paths);
  return sandbox;
}

CollectedValue<std::vector<unsigned>> collect_cpuset_v1(
    const std::vector<std::string> &paths) {
  std::vector<CollectedValue<std::vector<unsigned>>> configured_sets;
  for (const std::string &path : paths) {
    const std::string file = join_path(path, "cpuset.cpus");
    const auto contents = read_text(file);
    if (contents.status != CollectionStatus::available) {
      configured_sets.push_back(CollectedValue<std::vector<unsigned>>(
          std::vector<unsigned>(), contents.status, file));
      continue;
    }
    if (trim(contents.value).empty()) {
      continue;
    }
    configured_sets.push_back(parse_cpu_set(contents.value, file));
  }
  return resolve_effective_cpu_set(configured_sets);
}

std::vector<CollectedValue<CpuQuota>> collect_cpu_quota_v1(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<CpuQuota>> values;
  for (const std::string &path : paths) {
    const std::string quota_path = join_path(path, "cpu.cfs_quota_us");
    const std::string period_path = join_path(path, "cpu.cfs_period_us");
    const auto quota = read_text(quota_path);
    const auto period = read_text(period_path);
    if (normalized_path(path) == normalized_path(mount_point) &&
        quota.status == CollectionStatus::unavailable &&
        period.status == CollectionStatus::unavailable) {
      values.push_back(CollectedValue<CpuQuota>(
          CpuQuota{}, CollectionStatus::unlimited,
          "cgroup v1 mount root (unbounded)"));
      continue;
    }
    if (quota.status != CollectionStatus::available ||
        period.status != CollectionStatus::available) {
      const CollectionStatus status =
          quota.status != CollectionStatus::available ? quota.status
                                                       : period.status;
      values.push_back(CollectedValue<CpuQuota>(
          CpuQuota{}, status, quota.status != CollectionStatus::available
                                  ? quota_path
                                  : period_path));
    } else {
      values.push_back(parse_cpu_quota_v1(quota.value, period.value,
                                          quota_path + "," + period_path));
    }
  }
  return values;
}

std::vector<CollectedValue<std::uint64_t>> collect_memory_v1(
    const std::vector<std::string> &paths, const std::string &mount_point) {
  std::vector<CollectedValue<std::uint64_t>> values;
  for (const std::string &path : paths) {
    const std::string file = join_path(path, "memory.limit_in_bytes");
    const auto contents = read_text(file);
    if (normalized_path(path) == normalized_path(mount_point) &&
        contents.status == CollectionStatus::unavailable) {
      values.push_back(CollectedValue<std::uint64_t>(
          0, CollectionStatus::unlimited,
          "cgroup v1 mount root (unbounded)"));
      continue;
    }
    if (contents.status != CollectionStatus::available) {
      values.push_back(CollectedValue<std::uint64_t>(0, contents.status, file));
    } else {
      values.push_back(parse_memory_limit(contents.value, file));
    }
  }
  return values;
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

SandboxInfo collect_cgroup_v1(const CgroupMembership &membership,
                              const std::vector<CgroupMount> &mounts) {
  SandboxInfo sandbox;
  sandbox.cgroup_version =
      CollectedValue<unsigned>(1, CollectionStatus::available,
                               "/proc/self/mountinfo");
  set_sandbox_limits_status(sandbox, CollectionStatus::unavailable,
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
      const auto paths = ancestor_paths(leaf, cpu_mount->mount_point);
      const auto quota = resolve_effective_cpu_quota(
          collect_cpu_quota_v1(paths, cpu_mount->mount_point));
      sandbox.effective_cpu_quota_us = CollectedValue<std::uint64_t>(
          quota.value.quota_us, quota.status, quota.source);
      sandbox.effective_cpu_period_us = CollectedValue<std::uint64_t>(
          quota.value.period_us, quota.status, quota.source);
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
          collect_memory_v1(ancestor_paths(leaf, memory_mount->mount_point),
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
      sandbox.effective_allowed_cpus = collect_cpuset_v1(
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

SandboxInfo collect_sandbox_info() {
  const auto membership_text = read_text("/proc/self/cgroup");
  const auto mount_text = read_text("/proc/self/mountinfo");
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
    sandbox.cgroup_version = CollectedValue<unsigned>(
        0, status, source);
    sandbox.cgroup_path = CollectedValue<std::string>(
        std::string(), status, source);
    set_sandbox_limits_status(sandbox, status, source);
    return sandbox;
  }

  const CgroupMembership membership =
      parse_cgroup_membership(membership_text.value);
  const std::vector<CgroupMount> mounts =
      parse_cgroup_mounts(mount_text.value);
  if (membership.unified_path.size() > 0) {
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
    set_sandbox_limits_status(sandbox, CollectionStatus::unavailable,
                              "/proc/self/cgroup");
    return sandbox;
  }
  if (find_mount(mounts, 2) != nullptr) {
    SandboxInfo sandbox;
    sandbox.cgroup_version = CollectedValue<unsigned>(
        2, CollectionStatus::available, "/proc/self/mountinfo");
    sandbox.cgroup_path = CollectedValue<std::string>(
        std::string(), CollectionStatus::unavailable, "/proc/self/cgroup");
    set_sandbox_limits_status(sandbox, CollectionStatus::unavailable,
                              "/proc/self/cgroup");
    return sandbox;
  }
  if (find_mount(mounts, 1, "cpu") != nullptr ||
      find_mount(mounts, 1, "memory") != nullptr ||
      find_mount(mounts, 1, "cpuset") != nullptr) {
    return collect_cgroup_v1(membership, mounts);
  }
  SandboxInfo sandbox;
  sandbox.cgroup_version = CollectedValue<unsigned>(
      0, CollectionStatus::unavailable, "/proc/self/mountinfo");
  sandbox.cgroup_path = CollectedValue<std::string>(
      std::string(), CollectionStatus::unavailable, "/proc/self/cgroup");
  set_sandbox_limits_status(sandbox, CollectionStatus::unavailable,
                            "/proc/self/mountinfo");
  return sandbox;
}

} // namespace

SystemInfo collect_system_info() {
  SystemInfo info;
  info.compiler = collect_compiler_info();

  struct utsname system_name {};
  if (uname(&system_name) == 0) {
    info.cpu.architecture = CollectedValue<std::string>(
        system_name.machine, CollectionStatus::available, "uname.machine");
  } else {
    info.cpu.architecture = CollectedValue<std::string>(
        std::string(), status_from_errno(errno), "uname.machine");
  }

  const auto cpuinfo = read_text("/proc/cpuinfo");
  if (cpuinfo.status == CollectionStatus::available) {
    const CpuIdentity identity = parse_cpuinfo(cpuinfo.value);
    info.cpu.vendor = identity.vendor;
    info.cpu.model = identity.model;
  } else {
    info.cpu.vendor = CollectedValue<std::string>(
        std::string(), cpuinfo.status, cpuinfo.source);
    info.cpu.model = CollectedValue<std::string>(
        std::string(), cpuinfo.status, cpuinfo.source);
  }

  info.cpu.online_logical_cpus = online_cpus();
  std::vector<CacheInfo> caches;
  CollectionStatus cache_status = info.cpu.online_logical_cpus.status;
  std::string cache_source = "/sys/devices/system/cpu/cpu*/cache";
  if (info.cpu.online_logical_cpus.status == CollectionStatus::available) {
    cache_status = CollectionStatus::unavailable;
    for (unsigned cpu : info.cpu.online_logical_cpus.value) {
      for (unsigned index = 0; index < 32; ++index) {
        const std::string path = cpu_path(cpu) + "/cache/index" +
                                 std::to_string(index) + "/";
        const auto level_text = read_text(path + "level");
        const auto type_text = read_text(path + "type");
        const auto size_text = read_text(path + "size");
        const auto shared_text = read_text(path + "shared_cpu_list");
        if (level_text.status != CollectionStatus::available ||
            type_text.status != CollectionStatus::available ||
            size_text.status != CollectionStatus::available ||
            shared_text.status != CollectionStatus::available) {
          const CollectedValue<std::string> *failed = nullptr;
          const CollectedValue<std::string> *fields[] = {
              &level_text, &type_text, &size_text, &shared_text};
          for (const auto *field : fields) {
            if (field->status == CollectionStatus::permission_denied ||
                field->status == CollectionStatus::read_error ||
                field->status == CollectionStatus::malformed) {
              failed = field;
              break;
            }
          }
          if (failed != nullptr && cache_status != CollectionStatus::available) {
            cache_status = failed->status;
            cache_source = failed->source;
          }
          if (level_text.status == CollectionStatus::unavailable &&
              type_text.status == CollectionStatus::unavailable &&
              size_text.status == CollectionStatus::unavailable &&
              shared_text.status == CollectionStatus::unavailable) {
            break;
          }
          continue;
        }
        const auto level = parse_number(level_text.value, path + "level");
        const auto size = parse_cache_size(size_text.value, path + "size");
        const auto shared =
            parse_cpu_list(shared_text.value, path + "shared_cpu_list");
        if (level.status != CollectionStatus::available ||
            size.status != CollectionStatus::available ||
            shared.status != CollectionStatus::available) {
          if (cache_status != CollectionStatus::available) {
            cache_status = CollectionStatus::malformed;
            cache_source = path;
          }
          continue;
        }
        if (level.value == 0 || level.value > 3) {
          continue;
        }
        CacheInfo cache;
        cache.level = static_cast<unsigned>(level.value);
        cache.type = trim(type_text.value);
        cache.size_bytes = size.value;
        cache.shared_cpus = shared.value;
        caches.push_back(cache);
        cache_status = CollectionStatus::available;
        cache_source = path;
      }
    }
  }
  caches = deduplicate_caches(caches);
  info.cpu.caches = CollectedValue<std::vector<CacheInfo>>(
      caches, cache_status, cache_source);

  info.cpu_runtime = collect_cpu_runtime(
      info.cpu.online_logical_cpus.status == CollectionStatus::available
          ? info.cpu.online_logical_cpus.value
          : std::vector<unsigned>());
  info.sandbox = collect_sandbox_info();
  return info;
}

} // namespace linux_system

SystemInfo collect_system_info() { return linux_system::collect_system_info(); }

} // namespace ntq
