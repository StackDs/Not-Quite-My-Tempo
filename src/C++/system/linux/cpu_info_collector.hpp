#pragma once

#include "system/system_info.hpp"

namespace ntq {
namespace linux_system {

struct CpuCollection {
  CpuInfo info;
  CpuRuntimeInfo runtime;
};

CpuCollection collect_cpu_info();

} // namespace linux_system
} // namespace ntq
