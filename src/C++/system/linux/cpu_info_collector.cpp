#include "cpu_info_collector.hpp"

#include "cpu_parsers.hpp"
#include "file_reader.hpp"
#include "text_parsers.hpp"

#include <cerrno>
#include <limits>
#include <sched.h>
#include <string>
#include <sys/utsname.h>
#include <vector>

namespace ntq {
namespace linux_system {
namespace {

CollectedValue<std::uint64_t> parse_number(const std::string &value,
                                           const std::string &source) {
  std::uint64_t parsed = 0;
  if (!text::parse_unsigned(value, parsed)) {
    return CollectedValue<std::uint64_t>(0, CollectionStatus::malformed,
                                         source);
  }
  return CollectedValue<std::uint64_t>(parsed, CollectionStatus::available,
                                       source);
}

CollectedValue<std::uint64_t> read_frequency_hz(const std::string &path) {
  const auto khz = read_text_file(path);
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
  const auto contents = read_text_file(path);
  if (contents.status == CollectionStatus::available) {
    return parse_cpu_list(contents.value, path);
  }
  return CollectedValue<std::vector<unsigned>>(
      std::vector<unsigned>(), contents.status, contents.source);
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
  return CollectedValue<std::vector<unsigned>>(
      cpus, CollectionStatus::available, "sched_getaffinity");
}

CollectedValue<bool> boost_state() {
  CollectionStatus unavailable_status = CollectionStatus::unavailable;
  std::string unavailable_source = "kernel boost/Turbo sysfs interfaces";
  const std::string intel_path =
      "/sys/devices/system/cpu/intel_pstate/no_turbo";
  const auto intel = read_text_file(intel_path);
  if (intel.status == CollectionStatus::available) {
    const std::string value = text::trim(intel.value);
    if (value == "0" || value == "1") {
      return CollectedValue<bool>(value == "0", CollectionStatus::available,
                                  intel_path);
    }
    return CollectedValue<bool>(false, CollectionStatus::malformed, intel_path);
  }
  if (intel.status == CollectionStatus::permission_denied ||
      intel.status == CollectionStatus::read_error) {
    unavailable_status = intel.status;
    unavailable_source = intel_path;
  }

  const std::string generic_path = "/sys/devices/system/cpu/cpufreq/boost";
  const auto generic = read_text_file(generic_path);
  if (generic.status == CollectionStatus::available) {
    const std::string value = text::trim(generic.value);
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

  const std::string amd_path = "/sys/devices/system/cpu/amd_pstate/cpb";
  const auto amd = read_text_file(amd_path);
  if (amd.status == CollectionStatus::available) {
    const std::string value = text::trim(amd.value);
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
    const auto governor = read_text_file(governor_path);
    if (governor.status == CollectionStatus::available) {
      info.governor = CollectedValue<std::string>(
          text::trim(governor.value), CollectionStatus::available,
          governor_path);
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

CollectedValue<std::vector<CacheInfo>> collect_caches(
    const std::vector<unsigned> &cpus,
    CollectionStatus cpu_status) {
  std::vector<CacheInfo> caches;
  CollectionStatus cache_status = cpu_status;
  std::string cache_source = "/sys/devices/system/cpu/cpu*/cache";
  if (cpu_status == CollectionStatus::available) {
    cache_status = CollectionStatus::unavailable;
    for (unsigned cpu : cpus) {
      for (unsigned index = 0; index < 32; ++index) {
        const std::string path = cpu_path(cpu) + "/cache/index" +
                                 std::to_string(index) + "/";
        const auto level_text = read_text_file(path + "level");
        const auto type_text = read_text_file(path + "type");
        const auto size_text = read_text_file(path + "size");
        const auto shared_text = read_text_file(path + "shared_cpu_list");
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
        cache.type = text::trim(type_text.value);
        cache.size_bytes = size.value;
        cache.shared_cpus = shared.value;
        caches.push_back(cache);
        cache_status = CollectionStatus::available;
        cache_source = path;
      }
    }
  }
  return CollectedValue<std::vector<CacheInfo>>(
      deduplicate_caches(caches), cache_status, cache_source);
}

} // namespace

CpuCollection collect_cpu_info() {
  CpuCollection collection;
  struct utsname system_name {};
  if (uname(&system_name) == 0) {
    collection.info.architecture = CollectedValue<std::string>(
        system_name.machine, CollectionStatus::available, "uname.machine");
  } else {
    collection.info.architecture = CollectedValue<std::string>(
        std::string(), status_from_errno(errno), "uname.machine");
  }

  const auto cpuinfo = read_text_file("/proc/cpuinfo");
  if (cpuinfo.status == CollectionStatus::available) {
    const CpuIdentity identity = parse_cpuinfo(cpuinfo.value);
    collection.info.vendor = identity.vendor;
    collection.info.model = identity.model;
  } else {
    collection.info.vendor = CollectedValue<std::string>(
        std::string(), cpuinfo.status, cpuinfo.source);
    collection.info.model = CollectedValue<std::string>(
        std::string(), cpuinfo.status, cpuinfo.source);
  }

  collection.info.online_logical_cpus = online_cpus();
  collection.info.caches = collect_caches(
      collection.info.online_logical_cpus.value,
      collection.info.online_logical_cpus.status);
  collection.runtime = collect_cpu_runtime(
      collection.info.online_logical_cpus.status == CollectionStatus::available
          ? collection.info.online_logical_cpus.value
          : std::vector<unsigned>());
  return collection;
}

} // namespace linux_system
} // namespace ntq
