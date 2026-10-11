#include "parsers.hpp"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <set>
#include <sstream>

namespace ntq {
namespace linux_system {
namespace {

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

bool parse_unsigned(const std::string &text, std::uint64_t &value) {
  const std::string input = trim(text);
  if (input.empty() || input[0] == '-') {
    return false;
  }
  for (char character : input) {
    if (!std::isdigit(static_cast<unsigned char>(character))) {
      return false;
    }
  }
  errno = 0;
  char *end = nullptr;
  const unsigned long long parsed = std::strtoull(input.c_str(), &end, 10);
  if (errno == ERANGE || end == input.c_str() || *end != '\0') {
    return false;
  }
  value = static_cast<std::uint64_t>(parsed);
  return true;
}

bool parse_cpu_id(const std::string &text, unsigned &value) {
  std::uint64_t parsed = 0;
  if (!parse_unsigned(text, parsed) || parsed > UINT_MAX) {
    return false;
  }
  value = static_cast<unsigned>(parsed);
  return true;
}

std::vector<std::string> split(const std::string &text, char delimiter) {
  std::vector<std::string> fields;
  std::stringstream stream(text);
  std::string field;
  while (std::getline(stream, field, delimiter)) {
    fields.push_back(field);
  }
  if (!text.empty() && text[text.size() - 1] == delimiter) {
    fields.push_back(std::string());
  }
  return fields;
}

std::vector<std::string> whitespace_fields(const std::string &text) {
  std::istringstream stream(text);
  std::vector<std::string> fields;
  std::string field;
  while (stream >> field) {
    fields.push_back(field);
  }
  return fields;
}

std::string decode_mount_path(std::string path) {
  const std::pair<const char *, const char *> escapes[] = {
      {"\\040", " "}, {"\\011", "\t"}, {"\\012", "\n"},
      {"\\134", "\\"}};
  for (const auto &escape : escapes) {
    std::string::size_type position = 0;
    while ((position = path.find(escape.first, position)) !=
           std::string::npos) {
      path.replace(position, 4, escape.second);
      position += std::char_traits<char>::length(escape.second);
    }
  }
  return path;
}

bool same_cache(const CacheInfo &left, const CacheInfo &right) {
  return left.level == right.level && left.type == right.type &&
         left.size_bytes == right.size_bytes &&
         left.shared_cpus == right.shared_cpus;
}

template <class T>
CollectedValue<T> status_value(CollectionStatus status,
                               const std::string &source) {
  return CollectedValue<T>(T(), status, source);
}

} // namespace

CollectionStatus status_from_errno(int error) {
  if (error == EACCES || error == EPERM) {
    return CollectionStatus::permission_denied;
  }
  if (error == ENOENT || error == ENOTDIR || error == ENOTSUP ||
      error == EOPNOTSUPP) {
    return CollectionStatus::unavailable;
  }
  return CollectionStatus::read_error;
}

CollectedValue<std::vector<unsigned>> parse_cpu_list(const std::string &text,
                                                     const std::string &source) {
  const std::string input = trim(text);
  if (input.empty()) {
    return status_value<std::vector<unsigned>>(CollectionStatus::malformed,
                                              source);
  }

  std::vector<unsigned> cpus;
  const std::vector<std::string> fields = split(input, ',');
  for (const std::string &raw_field : fields) {
    const std::string field = trim(raw_field);
    if (field.empty()) {
      return status_value<std::vector<unsigned>>(CollectionStatus::malformed,
                                                source);
    }
    const std::vector<std::string> range = split(field, '-');
    unsigned first = 0;
    unsigned last = 0;
    if (range.size() > 2 || !parse_cpu_id(range[0], first) ||
        (range.size() == 2 && !parse_cpu_id(range[1], last))) {
      return status_value<std::vector<unsigned>>(CollectionStatus::malformed,
                                                source);
    }
    if (range.size() == 1) {
      last = first;
    }
    if (last < first ||
        static_cast<std::uint64_t>(last) - first > 1'000'000ULL) {
      return status_value<std::vector<unsigned>>(CollectionStatus::malformed,
                                                source);
    }
    for (unsigned cpu = first;; ++cpu) {
      cpus.push_back(cpu);
      if (cpu == last) {
        break;
      }
    }
  }
  std::sort(cpus.begin(), cpus.end());
  cpus.erase(std::unique(cpus.begin(), cpus.end()), cpus.end());
  return CollectedValue<std::vector<unsigned>>(
      cpus, CollectionStatus::available, source);
}

CollectedValue<std::uint64_t> parse_cache_size(const std::string &text,
                                               const std::string &source) {
  const std::string input = trim(text);
  std::string::size_type digits_end = 0;
  while (digits_end < input.size() &&
         std::isdigit(static_cast<unsigned char>(input[digits_end]))) {
    ++digits_end;
  }
  if (digits_end == 0) {
    return status_value<std::uint64_t>(CollectionStatus::malformed, source);
  }

  std::uint64_t value = 0;
  if (!parse_unsigned(input.substr(0, digits_end), value)) {
    return status_value<std::uint64_t>(CollectionStatus::malformed, source);
  }
  const std::string suffix = input.substr(digits_end);
  std::uint64_t multiplier = 1;
  if (suffix.empty() || suffix == "B" || suffix == "b") {
    multiplier = 1;
  } else if (suffix == "K" || suffix == "k" || suffix == "KB" ||
             suffix == "kB" || suffix == "KiB") {
    multiplier = 1024;
  } else if (suffix == "M" || suffix == "m" || suffix == "MB" ||
             suffix == "mB" || suffix == "MiB") {
    multiplier = 1024ULL * 1024ULL;
  } else if (suffix == "G" || suffix == "g" || suffix == "GB" ||
             suffix == "gB" || suffix == "GiB") {
    multiplier = 1024ULL * 1024ULL * 1024ULL;
  } else {
    return status_value<std::uint64_t>(CollectionStatus::malformed, source);
  }

  if (value > std::numeric_limits<std::uint64_t>::max() / multiplier) {
    return status_value<std::uint64_t>(CollectionStatus::malformed, source);
  }
  return CollectedValue<std::uint64_t>(value * multiplier,
                                       CollectionStatus::available, source);
}

CpuIdentity parse_cpuinfo(const std::string &text) {
  CpuIdentity identity;
  identity.vendor = status_value<std::string>(CollectionStatus::unavailable,
                                               "/proc/cpuinfo");
  identity.model = status_value<std::string>(CollectionStatus::unavailable,
                                              "/proc/cpuinfo");

  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    const std::string::size_type separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }
    const std::string key = trim(line.substr(0, separator));
    const std::string value = trim(line.substr(separator + 1));
    if (value.empty()) {
      continue;
    }
    if (key == "vendor_id" || key == "CPU implementer") {
      identity.vendor = CollectedValue<std::string>(
          value, CollectionStatus::available, "/proc/cpuinfo");
    } else if (key == "model name" || key == "Hardware" ||
               key == "Processor") {
      identity.model = CollectedValue<std::string>(
          value, CollectionStatus::available, "/proc/cpuinfo");
    }
  }
  return identity;
}

std::vector<CacheInfo>
deduplicate_caches(const std::vector<CacheInfo> &caches) {
  std::vector<CacheInfo> unique;
  for (const CacheInfo &cache : caches) {
    if (std::find_if(unique.begin(), unique.end(), [&](const CacheInfo &other) {
          return same_cache(cache, other);
        }) == unique.end()) {
      unique.push_back(cache);
    }
  }
  return unique;
}

CgroupMembership parse_cgroup_membership(const std::string &text) {
  CgroupMembership membership;
  std::istringstream stream(text);
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
    const std::string path = decode_mount_path(line.substr(second_separator + 1));
    if (controller_field.empty() && hierarchy == "0") {
      membership.unified_path = path;
      continue;
    }
    for (const std::string &controller : split(controller_field, ',')) {
      if (!controller.empty()) {
        membership.controller_paths[controller] = path;
      }
    }
  }
  return membership;
}

std::vector<CgroupMount> parse_cgroup_mounts(const std::string &text) {
  std::vector<CgroupMount> mounts;
  std::istringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    const std::string::size_type separator = line.find(" - ");
    if (separator == std::string::npos) {
      continue;
    }
    const std::vector<std::string> before =
        whitespace_fields(line.substr(0, separator));
    const std::vector<std::string> after =
        whitespace_fields(line.substr(separator + 3));
    if (before.size() < 6 || after.size() < 3) {
      continue;
    }
    if (after[0] != "cgroup" && after[0] != "cgroup2") {
      continue;
    }

    CgroupMount mount;
    mount.version = after[0] == "cgroup2" ? 2U : 1U;
    mount.root = decode_mount_path(before[3]);
    mount.mount_point = decode_mount_path(before[4]);
    if (mount.version == 1) {
      const std::vector<std::string> options = split(after[2], ',');
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
  const std::string quota = trim(quota_text);
  const std::string period = trim(period_text);
  std::uint64_t period_value = 0;
  if (!parse_unsigned(period, period_value) || period_value == 0) {
    return status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  if (quota == "-1") {
    return status_value<CpuQuota>(CollectionStatus::unlimited, source);
  }
  std::uint64_t quota_value = 0;
  if (!parse_unsigned(quota, quota_value) || quota_value == 0) {
    return status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  return CollectedValue<CpuQuota>(CpuQuota{quota_value, period_value},
                                  CollectionStatus::available, source);
}

CollectedValue<CpuQuota> parse_cpu_quota_v2(const std::string &text,
                                            const std::string &source) {
  const std::vector<std::string> fields = whitespace_fields(text);
  if (fields.size() != 2) {
    return status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  std::uint64_t period = 0;
  if (!parse_unsigned(fields[1], period) || period == 0) {
    return status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  if (fields[0] == "max") {
    return status_value<CpuQuota>(CollectionStatus::unlimited, source);
  }
  std::uint64_t quota = 0;
  if (!parse_unsigned(fields[0], quota) || quota == 0) {
    return status_value<CpuQuota>(CollectionStatus::malformed, source);
  }
  return CollectedValue<CpuQuota>(CpuQuota{quota, period},
                                  CollectionStatus::available, source);
}

CollectedValue<std::uint64_t> parse_memory_limit(const std::string &text,
                                                 const std::string &source) {
  const std::string value_text = trim(text);
  if (value_text == "max") {
    return status_value<std::uint64_t>(CollectionStatus::unlimited, source);
  }
  std::uint64_t value = 0;
  if (!parse_unsigned(value_text, value)) {
    return status_value<std::uint64_t>(CollectionStatus::malformed, source);
  }
  // cgroup v1 represents unlimited memory with a very large page-aligned value.
  if (value >= (1ULL << 60)) {
    return status_value<std::uint64_t>(CollectionStatus::unlimited, source);
  }
  return CollectedValue<std::uint64_t>(value, CollectionStatus::available,
                                       source);
}

CollectedValue<std::vector<unsigned>> parse_cpu_set(const std::string &text,
                                                    const std::string &source) {
  return parse_cpu_list(text, source);
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
    return status_value<CpuQuota>(unknown->status, unknown->source);
  }
  if (tightest != nullptr) {
    return *tightest;
  }
  if (saw_value) {
    return status_value<CpuQuota>(CollectionStatus::unlimited,
                                  "cgroup ancestors");
  }
  return status_value<CpuQuota>(CollectionStatus::unavailable,
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
    return status_value<std::vector<unsigned>>(unknown->status,
                                              unknown->source);
  }
  if (!saw_value) {
    return status_value<std::vector<unsigned>>(CollectionStatus::unavailable,
                                              "cgroup cpuset ancestors");
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
    return status_value<std::uint64_t>(unknown->status, unknown->source);
  }
  if (tightest != nullptr) {
    return *tightest;
  }
  if (saw_value) {
    return status_value<std::uint64_t>(CollectionStatus::unlimited,
                                       "cgroup ancestors");
  }
  return status_value<std::uint64_t>(CollectionStatus::unavailable,
                                     "cgroup ancestors");
}

} // namespace linux_system
} // namespace ntq
