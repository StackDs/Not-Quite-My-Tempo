#include "linux/hardware_counters.hpp"

#include <linux/perf_event.h>

namespace ntq {
namespace linux_metrics {
namespace {

HardwareMetricStatus hardware_status(PerfEventStatus status) {
  switch (status) {
  case PerfEventStatus::available:
    return HardwareMetricStatus::available;
  case PerfEventStatus::unsupported:
    return HardwareMetricStatus::unsupported;
  case PerfEventStatus::permission_denied:
    return HardwareMetricStatus::permission_denied;
  case PerfEventStatus::read_error:
    return HardwareMetricStatus::read_error;
  case PerfEventStatus::not_measured:
    return HardwareMetricStatus::not_measured;
  }
  return HardwareMetricStatus::read_error;
}

void apply_event(const PerfEventGroup &group, std::size_t index,
                 std::uint64_t &value, HardwareMetricStatus &status) {
  const auto &results = group.results();
  if (index >= results.size()) {
    status = HardwareMetricStatus::not_measured;
    return;
  }
  value = results[index].value;
  status = hardware_status(results[index].status);
}

} // namespace

// Agrupa ciclos/instrucciones y accesos/fallos de cada caché para que cada razón
// se calcule con contadores observados durante el mismo intervalo.
HardwareCounters::HardwareCounters()
    : cycles_and_instructions_({
          make_hardware_event(PERF_COUNT_HW_CPU_CYCLES),
          make_hardware_event(PERF_COUNT_HW_INSTRUCTIONS),
      }),
      l1d_reads_({
          make_cache_event(PERF_COUNT_HW_CACHE_L1D,
                           PERF_COUNT_HW_CACHE_OP_READ,
                           PERF_COUNT_HW_CACHE_RESULT_ACCESS),
          make_cache_event(PERF_COUNT_HW_CACHE_L1D,
                           PERF_COUNT_HW_CACHE_OP_READ,
                           PERF_COUNT_HW_CACHE_RESULT_MISS),
      }),
      llc_reads_({
          make_cache_event(PERF_COUNT_HW_CACHE_LL,
                           PERF_COUNT_HW_CACHE_OP_READ,
                           PERF_COUNT_HW_CACHE_RESULT_ACCESS),
          make_cache_event(PERF_COUNT_HW_CACHE_LL,
                           PERF_COUNT_HW_CACHE_OP_READ,
                           PERF_COUNT_HW_CACHE_RESULT_MISS),
      }),
      branch_misses_({make_hardware_event(PERF_COUNT_HW_BRANCH_MISSES)}) {}

void HardwareCounters::prepare() noexcept {
  cycles_and_instructions_.prepare();
  l1d_reads_.prepare();
  llc_reads_.prepare();
  branch_misses_.prepare();
}

void HardwareCounters::start() noexcept {
  cycles_and_instructions_.start();
  l1d_reads_.start();
  llc_reads_.start();
  branch_misses_.start();
}

void HardwareCounters::stop() noexcept {
  cycles_and_instructions_.stop();
  l1d_reads_.stop();
  llc_reads_.stop();
  branch_misses_.stop();
}

void HardwareCounters::apply(HardwareMeasurements &measurements) const noexcept {
  // Conserva valores y estados de cada evento por separado; el cálculo de IPC y
  // porcentajes se realiza después, en hardware_measurement_calculations.cpp.
  apply_event(cycles_and_instructions_, 0, measurements.cpu_cycles,
              measurements.cpu_cycles_status);
  apply_event(cycles_and_instructions_, 1, measurements.instructions_retired,
              measurements.instructions_retired_status);
  apply_event(l1d_reads_, 0, measurements.l1d_read_accesses,
              measurements.l1d_read_accesses_status);
  apply_event(l1d_reads_, 1, measurements.l1d_read_misses,
              measurements.l1d_read_misses_status);
  apply_event(llc_reads_, 0, measurements.llc_read_accesses,
              measurements.llc_read_accesses_status);
  apply_event(llc_reads_, 1, measurements.llc_read_misses,
              measurements.llc_read_misses_status);
  apply_event(branch_misses_, 0, measurements.branch_mispredictions,
              measurements.branch_mispredictions_status);
}

} // namespace linux_metrics
} // namespace ntq
