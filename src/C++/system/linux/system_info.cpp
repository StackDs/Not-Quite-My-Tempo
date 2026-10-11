#include "system/system_info.hpp"

#include "cgroup_info_collector.hpp"
#include "compiler_info.hpp"
#include "cpu_info_collector.hpp"

namespace ntq {
namespace linux_system {

SystemInfo collect_system_info() {
  SystemInfo info;
  const CpuCollection cpu = collect_cpu_info();
  info.cpu = cpu.info;
  info.cpu_runtime = cpu.runtime;
  info.compiler = collect_compiler_info();
  info.sandbox = collect_sandbox_info();
  return info;
}

} // namespace linux_system

SystemInfo collect_system_info() { return linux_system::collect_system_info(); }

} // namespace ntq
