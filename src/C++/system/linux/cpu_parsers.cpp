#include "cpu_parsers.hpp"

#include "text_parsers.hpp"

#include <algorithm>
#include <cctype>
#include <climits>
#include <limits>
#include <sstream>

namespace ntq {
namespace linux_system {
namespace {

bool parse_cpu_id(const std::string &text_value, unsigned &value) {
  std::uint64_t parsed = 0;
  if (!text::parse_unsigned(text_value, parsed) || parsed > UINT_MAX) {
    return false;
  }
  value = static_cast<unsigned>(parsed);
  return true;
}

bool same_cache(const CacheInfo &left, const CacheInfo &right) {
  return left.level == right.level && left.type == right.type &&
         left.size_bytes == right.size_bytes &&
         left.shared_cpus == right.shared_cpus;
}

} // namespace

CollectedValue<std::vector<unsigned>> parse_cpu_list(const std::string &text_value,
                                                     const std::string &source) {
  const std::string input = text::trim(text_value);
  if (input.empty()) {
    return text::status_value<std::vector<unsigned>>(
        CollectionStatus::malformed, source);
  }

  std::vector<unsigned> cpus;
  const std::vector<std::string> fields = text::split(input, ',');
  for (const std::string &raw_field : fields) {
    const std::string field = text::trim(raw_field);
    if (field.empty()) {
      return text::status_value<std::vector<unsigned>>(
          CollectionStatus::malformed, source);
    }
    const std::vector<std::string> range = text::split(field, '-');
    unsigned first = 0;
    unsigned last = 0;
    if (range.size() > 2 || !parse_cpu_id(range[0], first) ||
        (range.size() == 2 && !parse_cpu_id(range[1], last))) {
      return text::status_value<std::vector<unsigned>>(
          CollectionStatus::malformed, source);
    }
    if (range.size() == 1) {
      last = first;
    }
    if (last < first ||
        static_cast<std::uint64_t>(last) - first > 1'000'000ULL) {
      return text::status_value<std::vector<unsigned>>(
          CollectionStatus::malformed, source);
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

CollectedValue<std::uint64_t> parse_cache_size(const std::string &text_value,
                                               const std::string &source) {
  const std::string input = text::trim(text_value);
  std::string::size_type digits_end = 0;
  while (digits_end < input.size() &&
         std::isdigit(static_cast<unsigned char>(input[digits_end]))) {
    ++digits_end;
  }
  if (digits_end == 0) {
    return text::status_value<std::uint64_t>(CollectionStatus::malformed,
                                             source);
  }

  std::uint64_t value = 0;
  if (!text::parse_unsigned(input.substr(0, digits_end), value)) {
    return text::status_value<std::uint64_t>(CollectionStatus::malformed,
                                             source);
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
    return text::status_value<std::uint64_t>(CollectionStatus::malformed,
                                             source);
  }

  if (value > std::numeric_limits<std::uint64_t>::max() / multiplier) {
    return text::status_value<std::uint64_t>(CollectionStatus::malformed,
                                             source);
  }
  return CollectedValue<std::uint64_t>(value * multiplier,
                                       CollectionStatus::available, source);
}

CpuIdentity parse_cpuinfo(const std::string &text_value) {
  CpuIdentity identity;
  identity.vendor = text::status_value<std::string>(
      CollectionStatus::unavailable, "/proc/cpuinfo");
  identity.model = text::status_value<std::string>(
      CollectionStatus::unavailable, "/proc/cpuinfo");

  std::istringstream stream(text_value);
  std::string line;
  while (std::getline(stream, line)) {
    const std::string::size_type separator = line.find(':');
    if (separator == std::string::npos) {
      continue;
    }
    const std::string key = text::trim(line.substr(0, separator));
    const std::string value = text::trim(line.substr(separator + 1));
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

} // namespace linux_system
} // namespace ntq
