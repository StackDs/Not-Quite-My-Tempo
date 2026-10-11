#include "cgroup_parsers.hpp"

#include "cpu_parsers.hpp"
#include "text_parsers.hpp"

#include <algorithm>
#include <iterator>
#include <sstream>

namespace ntq {
namespace linux_system {

CgroupMembership parse_cgroup_membership(const std::string &input) {
  CgroupMembership membership;
  std::istringstream stream(input);
  std::string line;
  while (std::getline(stream, line)) {
    const std::string::size_type first_separator = line.find(':');
    const std::string::size_type second_separator =
        first_separator == std::string::npos
            ? std::string::npos
            : line.find(':', first_separator + 1);
    if (first_separator == std::string::npos ||
        second_separator == std::string::npos) {
      continue;
    }
    const std::string hierarchy = line.substr(0, first_separator);
    const std::string controller_field =
        line.substr(first_separator + 1,
                    second_separator - first_separator - 1);
    const std::string path = text::decode_mount_path(line.substr(second_separator + 1));
    if (controller_field.empty() && hierarchy == "0") {
      membership.unified_path = path;
      continue;
    }
    for (const std::string &controller : text::split(controller_field, ',')) {
      if (!controller.empty()) {
        membership.controller_paths[controller] = path;
      }
    }
  }
  return membership;
}

std::vector<CgroupMount> parse_cgroup_mounts(const std::string &input) {
  std::vector<CgroupMount> mounts;
  std::istringstream stream(input);
  std::string line;
  while (std::getline(stream, line)) {
    const std::string::size_type separator = line.find(" - ");
    if (separator == std::string::npos) {
      continue;
    }
    const std::vector<std::string> before =
        text::whitespace_fields(line.substr(0, separator));
    const std::vector<std::string> after =
        text::whitespace_fields(line.substr(separator + 3));
    if (before.size() < 6 || after.size() < 3) {
      continue;
    }
    if (after[0] != "cgroup" && after[0] != "cgroup2") {
      continue;
    }

    CgroupMount mount;
    mount.version = after[0] == "cgroup2" ? 2U : 1U;
    mount.root = text::decode_mount_path(before[3]);
    mount.mount_point = text::decode_mount_path(before[4]);
    if (mount.version == 1) {
      const std::vector<std::string> options = text::split(after[2], ',');
      for (const std::string &option : options) {
        if (option == "cpu" || option == "cpuacct" || option == "cpuset" ||
            option == "memory") {
          mount.controllers.push_back(option);
        }
      }
    }
    mounts.push_back(mount);
  }
  return mounts;
}

CollectedValue<CpuQuota> parse_cpu_quota_v1(const std::string &quota_text,
                                            const std::string &period_text,
                                            const std::string &source) {
  const std::string quota = text::trim(quota_text);
  const std::string period = text::trim(period_text);
  std::uint64_t period_value = 0;
  if (!text::parse_unsigned(period, period_value) || period_value == 0) {
    return text::status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  if (quota == "-1") {
    return text::status_value<CpuQuota>(CollectionStatus::unlimited, source);
  }
  std::uint64_t quota_value = 0;
  if (!text::parse_unsigned(quota, quota_value) || quota_value == 0) {
    return text::status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  return CollectedValue<CpuQuota>(CpuQuota{quota_value, period_value},
                                  CollectionStatus::available, source);
}

CollectedValue<CpuQuota> parse_cpu_quota_v2(const std::string &input,
                                            const std::string &source) {
  const std::vector<std::string> fields = text::whitespace_fields(input);
  if (fields.size() != 2) {
    return text::status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  std::uint64_t period = 0;
  if (!text::parse_unsigned(fields[1], period) || period == 0) {
    return text::status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  if (fields[0] == "max") {
    return text::status_value<CpuQuota>(CollectionStatus::unlimited, source);
  }
  std::uint64_t quota = 0;
  if (!text::parse_unsigned(fields[0], quota) || quota == 0) {
    return text::status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  return CollectedValue<CpuQuota>(CpuQuota{quota, period},
                                  CollectionStatus::available, source);
}

CollectedValue<std::uint64_t> parse_memory_limit(const std::string &input,
                                                 const std::string &source) {
  const std::string value_text = text::trim(input);
  if (value_text == "max") {
    return text::status_value<std::uint64_t>(CollectionStatus::unlimited,
                                             source);
  }
  std::uint64_t value = 0;
  if (!text::parse_unsigned(value_text, value)) {
    return text::status_value<std::uint64_t>(CollectionStatus::malformed,
                                             source);
  }
  // cgroup v1 represents unlimited memory with a very large page-aligned value.
  if (value >= (1ULL << 60)) {
    return text::status_value<std::uint64_t>(CollectionStatus::unlimited,
                                             source);
  }
  return CollectedValue<std::uint64_t>(value, CollectionStatus::available,
                                       source);
}

CollectedValue<std::vector<unsigned>> parse_cpu_set(const std::string &input,
                                                    const std::string &source) {
  return parse_cpu_list(input, source);
}

CollectedValue<CpuQuota> resolve_effective_cpu_quota(
    const std::vector<CollectedValue<CpuQuota>> &ancestors) {
  bool saw_value = false;
  const CollectedValue<CpuQuota> *tightest = nullptr;
  const CollectedValue<CpuQuota> *unknown = nullptr;
  for (const auto &ancestor : ancestors) {
    if (ancestor.status == CollectionStatus::available) {
      saw_value = true;
      if (tightest == nullptr ||
          static_cast<long double>(ancestor.value.quota_us) /
                  ancestor.value.period_us <
              static_cast<long double>(tightest->value.quota_us) /
                  tightest->value.period_us) {
        tightest = &ancestor;
      }
    } else if (ancestor.status == CollectionStatus::unlimited) {
      saw_value = true;
    } else if (unknown == nullptr) {
      unknown = &ancestor;
    }
  }
  if (unknown != nullptr) {
    return text::status_value<CpuQuota>(unknown->status, unknown->source);
  }
  if (tightest != nullptr) {
    return *tightest;
  }
  if (saw_value) {
    return text::status_value<CpuQuota>(CollectionStatus::unlimited,
                                        "cgroup ancestors");
  }
  return text::status_value<CpuQuota>(CollectionStatus::unavailable,
                                      "cgroup ancestors");
}

CollectedValue<std::vector<unsigned>> resolve_effective_cpu_set(
    const std::vector<CollectedValue<std::vector<unsigned>>> &ancestors) {
  bool saw_value = false;
  std::vector<unsigned> effective;
  const CollectedValue<std::vector<unsigned>> *unknown = nullptr;
  for (const auto &ancestor : ancestors) {
    if (ancestor.status == CollectionStatus::available) {
      if (ancestor.value.empty()) {
        continue;
      }
      if (!saw_value) {
        effective = ancestor.value;
        saw_value = true;
      } else {
        std::vector<unsigned> intersection;
        std::set_intersection(effective.begin(), effective.end(),
                              ancestor.value.begin(), ancestor.value.end(),
                              std::back_inserter(intersection));
        effective.swap(intersection);
      }
    } else if (unknown == nullptr) {
      unknown = &ancestor;
    }
  }
  if (unknown != nullptr) {
    return text::status_value<std::vector<unsigned>>(unknown->status,
                                                     unknown->source);
  }
  if (!saw_value) {
    return text::status_value<std::vector<unsigned>>(
        CollectionStatus::unavailable, "cgroup cpuset ancestors");
  }
  return CollectedValue<std::vector<unsigned>>(
      effective, CollectionStatus::available, "cgroup cpuset ancestors");
}

CollectedValue<std::uint64_t> resolve_effective_memory_limit(
    const std::vector<CollectedValue<std::uint64_t>> &ancestors) {
  bool saw_value = false;
  const CollectedValue<std::uint64_t> *tightest = nullptr;
  const CollectedValue<std::uint64_t> *unknown = nullptr;
  for (const auto &ancestor : ancestors) {
    if (ancestor.status == CollectionStatus::available) {
      saw_value = true;
      if (tightest == nullptr || ancestor.value < tightest->value) {
        tightest = &ancestor;
      }
    } else if (ancestor.status == CollectionStatus::unlimited) {
      saw_value = true;
    } else if (unknown == nullptr) {
      unknown = &ancestor;
    }
  }
  if (unknown != nullptr) {
    return text::status_value<std::uint64_t>(unknown->status,
                                             unknown->source);
  }
  if (tightest != nullptr) {
    return *tightest;
  }
  if (saw_value) {
    return text::status_value<std::uint64_t>(CollectionStatus::unlimited,
                                             "cgroup ancestors");
  }
  return text::status_value<std::uint64_t>(CollectionStatus::unavailable,
                                           "cgroup ancestors");
}

} // namespace linux_system
} // namespace ntq
