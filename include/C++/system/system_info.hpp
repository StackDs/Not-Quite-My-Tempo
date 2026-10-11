#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ntq {

// Enumera el estado de la recolección de información del sistema.
enum class CollectionStatus {
  available,
  unavailable,
  permission_denied,
  malformed,
  read_error,
  unlimited
};

template <class T> struct CollectedValue {
  T value;
  CollectionStatus status;
  std::string source;

  CollectedValue() : value(), status(CollectionStatus::unavailable), source() {}
  CollectedValue(const T &collected_value, CollectionStatus collected_status,
                 const std::string &collected_source)
      : value(collected_value), status(collected_status),
        source(collected_source) {}
};

struct CacheInfo {
  unsigned level{0};
  std::string type;
  std::uint64_t size_bytes{0};
  std::vector<unsigned> shared_cpus;
};

struct CpuInfo {
  CollectedValue<std::string> architecture;
  CollectedValue<std::string> vendor;
  CollectedValue<std::string> model;
  CollectedValue<std::vector<unsigned>> online_logical_cpus;
  CollectedValue<std::vector<CacheInfo>> caches;
};

struct CpuFrequencyInfo {
  unsigned cpu_id{0};
  CollectedValue<std::string> governor;
  CollectedValue<std::uint64_t> current_frequency_hz;
  CollectedValue<std::uint64_t> minimum_frequency_hz;
  CollectedValue<std::uint64_t> maximum_frequency_hz;
};

struct CpuRuntimeInfo {
  CollectedValue<std::vector<CpuFrequencyInfo>> per_cpu;
  CollectedValue<bool> turbo_or_boost_enabled;
  CollectedValue<std::vector<unsigned>> process_affinity;
};

struct CompilerInfo {
  CollectedValue<std::string> family;
  CollectedValue<std::string> version;
  CollectedValue<std::string> language_standard;
  CollectedValue<std::string> optimization_profile;
  CollectedValue<std::string> target_architecture;
};

struct SandboxInfo {
  CollectedValue<unsigned> cgroup_version;
  CollectedValue<std::string> cgroup_path;
  CollectedValue<std::uint64_t> effective_cpu_quota_us;
  CollectedValue<std::uint64_t> effective_cpu_period_us;
  CollectedValue<double> effective_cpu_quota_cores;
  CollectedValue<std::vector<unsigned>> effective_allowed_cpus;
  CollectedValue<std::uint64_t> effective_memory_limit_bytes;
};

struct SystemInfo {
  CpuInfo cpu;
  CpuRuntimeInfo cpu_runtime;
  CompilerInfo compiler;
  SandboxInfo sandbox;
};

// Recolecta datos visibles para el proceso en Linux sin modificar el sistema.
SystemInfo collect_system_info();

} // namespace ntq
