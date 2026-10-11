#include "linux/tlb_miss_counters.hpp"
#include "linux/perf_event_group.hpp"
#include "linux/hardware_measurement_calculations.hpp"
#include "metrics/time_measurement.hpp"
#include "linux/memory_helpers.hpp"

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

bool valid_status(ntq::MemoryMetricStatus status) {
  switch (status) {
  case ntq::MemoryMetricStatus::available:
  case ntq::MemoryMetricStatus::unsupported:
  case ntq::MemoryMetricStatus::permission_denied:
  case ntq::MemoryMetricStatus::read_error:
  case ntq::MemoryMetricStatus::not_measured:
    return true;
  }
  return false;
}

bool valid_hardware_status(ntq::HardwareMetricStatus status) {
  switch (status) {
  case ntq::HardwareMetricStatus::available:
  case ntq::HardwareMetricStatus::unsupported:
  case ntq::HardwareMetricStatus::permission_denied:
  case ntq::HardwareMetricStatus::read_error:
  case ntq::HardwareMetricStatus::not_measured:
  case ntq::HardwareMetricStatus::invalid_denominator:
    return true;
  }
  return false;
}

void test_peak_rss_conversion() {
  std::uint64_t bytes = 0;
  assert(ntq::linux_metrics::peak_rss_kib_to_bytes(7, bytes));
  assert(bytes == 7168);
  assert(!ntq::linux_metrics::peak_rss_kib_to_bytes(
      std::numeric_limits<std::uint64_t>::max(), bytes));
}

void test_memory_result_state_and_manual_capture() {
  ntq::TimeMeasurement unmeasured;
  bool memory_requires_elapsed = false;
  try {
    (void)unmeasured.obtain_memory();
  } catch (const std::logic_error &) {
    memory_requires_elapsed = true;
  }
  assert(memory_requires_elapsed);
  bool hardware_requires_elapsed = false;
  try {
    (void)unmeasured.obtain_hardware();
  } catch (const std::logic_error &) {
    hardware_requires_elapsed = true;
  }
  assert(hardware_requires_elapsed);

  ntq::TimeMeasurement measurement;
  measurement.start();
  std::vector<char> allocation(4 * 1024 * 1024);
  for (std::size_t offset = 0; offset < allocation.size(); offset += 4096) {
    allocation[offset] = static_cast<char>(offset / 4096);
  }
  measurement.stop();
  measurement.elapsed();

  const auto memory = measurement.obtain_memory();
  assert(memory.peak_rss_status == ntq::MemoryMetricStatus::available);
  assert(memory.peak_rss_bytes > 0);
  assert(memory.peak_rss_scope == ntq::PeakRssScope::process_lifetime);
  assert(memory.minor_page_faults > 0);
  assert(valid_status(memory.data_tlb_read_status));
  assert(valid_status(memory.data_tlb_write_status));
  assert(measurement.obtain_elapsed(ntq::TimeUnit::milliseconds).wall_clock >=
         0.0);
  const auto elapsed = measurement.obtain_elapsed(ntq::TimeUnit::nanoseconds);
  assert(elapsed.involuntary_context_switch_threshold == 0);
  assert(elapsed.suspicious_reason ==
         (elapsed.suspicious
              ? ntq::SuspicionReason::involuntary_context_switches_exceeded
              : ntq::SuspicionReason::none));
  assert(measurement.obtain_elapsed(ntq::TimeUnit::seconds).wall_clock >= 0.0);
}

void test_move_preserves_active_measurement() {
  ntq::TimeMeasurement original;
  original.start();
  ntq::TimeMeasurement moved(std::move(original));
  moved.stop();
  moved.elapsed();
  assert(moved.obtain_memory().peak_rss_bytes > 0);
}

void test_perf_error_mapping() {
  using ntq::MemoryMetricStatus;
  using ntq::linux_metrics::status_from_perf_errno;
  assert(status_from_perf_errno(EPERM) == MemoryMetricStatus::permission_denied);
  assert(status_from_perf_errno(EACCES) ==
         MemoryMetricStatus::permission_denied);
  assert(status_from_perf_errno(EINVAL) == MemoryMetricStatus::unsupported);
  assert(status_from_perf_errno(EOPNOTSUPP) ==
         MemoryMetricStatus::unsupported);
  assert(status_from_perf_errno(EIO) == MemoryMetricStatus::read_error);

  using ntq::linux_metrics::PerfEventStatus;
  using ntq::linux_metrics::perf_status_from_errno;
  assert(perf_status_from_errno(EPERM) ==
         PerfEventStatus::permission_denied);
  assert(perf_status_from_errno(EINVAL) == PerfEventStatus::unsupported);
  assert(perf_status_from_errno(EIO) == PerfEventStatus::read_error);
}

void test_hardware_metric_calculations() {
  ntq::HardwareMeasurements measurements;
  measurements.cpu_cycles = 2000;
  measurements.instructions_retired = 4000;
  measurements.cpu_cycles_status = ntq::HardwareMetricStatus::available;
  measurements.instructions_retired_status =
      ntq::HardwareMetricStatus::available;
  measurements.l1d_read_accesses = 200;
  measurements.l1d_read_misses = 10;
  measurements.l1d_read_accesses_status =
      ntq::HardwareMetricStatus::available;
  measurements.l1d_read_misses_status =
      ntq::HardwareMetricStatus::available;
  measurements.llc_read_accesses = 500;
  measurements.llc_read_misses = 25;
  measurements.llc_read_accesses_status =
      ntq::HardwareMetricStatus::available;
  measurements.llc_read_misses_status =
      ntq::HardwareMetricStatus::available;

  ntq::linux_metrics::calculate_hardware_derived_metrics(measurements);
  assert(measurements.ipc_status == ntq::HardwareMetricStatus::available);
  assert(measurements.ipc == 2.0);
  assert(measurements.l1d_read_miss_percent_status ==
         ntq::HardwareMetricStatus::available);
  assert(measurements.l1d_read_miss_percent == 5.0);
  assert(measurements.llc_read_miss_percent == 5.0);

  measurements.cpu_cycles = 0;
  ntq::linux_metrics::calculate_hardware_derived_metrics(measurements);
  assert(measurements.ipc_status ==
         ntq::HardwareMetricStatus::invalid_denominator);

  measurements.l1d_read_accesses = 0;
  ntq::linux_metrics::calculate_hardware_derived_metrics(measurements);
  assert(measurements.l1d_read_miss_percent_status ==
         ntq::HardwareMetricStatus::invalid_denominator);

  measurements.instructions_retired_status =
      ntq::HardwareMetricStatus::permission_denied;
  ntq::linux_metrics::calculate_hardware_derived_metrics(measurements);
  assert(measurements.ipc_status ==
         ntq::HardwareMetricStatus::permission_denied);
}

void test_isolated_memory_transfer_and_optional_tlb() {
  const ntq::TimeMeasurement measured =
      ntq::TimeMeasurement::measure_in_isolated_process([] {
        std::vector<char> allocation(16 * 1024 * 1024);
        for (std::size_t offset = 0; offset < allocation.size(); offset += 4096) {
          allocation[offset] = static_cast<char>(offset / 4096);
        }
      });

  const auto memory = measured.obtain_memory();
  assert(memory.peak_rss_status == ntq::MemoryMetricStatus::available);
  assert(memory.peak_rss_bytes >= 8 * 1024 * 1024);
  assert(memory.peak_rss_scope ==
         ntq::PeakRssScope::isolated_measurement_process);
  assert(memory.minor_page_faults > 0);
  assert(valid_status(memory.data_tlb_read_status));
  assert(valid_status(memory.data_tlb_write_status));
  const auto hardware = measured.obtain_hardware();
  assert(valid_hardware_status(hardware.cpu_cycles_status));
  assert(valid_hardware_status(hardware.instructions_retired_status));
  assert(valid_hardware_status(hardware.ipc_status));
  assert(valid_hardware_status(hardware.l1d_read_accesses_status));
  assert(valid_hardware_status(hardware.l1d_read_misses_status));
  assert(valid_hardware_status(hardware.l1d_read_miss_percent_status));
  assert(valid_hardware_status(hardware.llc_read_accesses_status));
  assert(valid_hardware_status(hardware.llc_read_misses_status));
  assert(valid_hardware_status(hardware.llc_read_miss_percent_status));
  assert(valid_hardware_status(hardware.branch_mispredictions_status));
  assert(measured.obtain_elapsed(ntq::TimeUnit::microseconds).wall_clock >=
         0.0);
}

void test_isolated_callback_exception() {
  bool failure_propagated = false;
  try {
    (void)ntq::TimeMeasurement::measure_in_isolated_process([] {
      throw std::runtime_error("expected callback failure");
    });
  } catch (const std::runtime_error &) {
    failure_propagated = true;
  }
  assert(failure_propagated);
}

} // namespace

int main() {
  test_peak_rss_conversion();
  test_memory_result_state_and_manual_capture();
  test_move_preserves_active_measurement();
  test_perf_error_mapping();
  test_hardware_metric_calculations();
  test_isolated_memory_transfer_and_optional_tlb();
  test_isolated_callback_exception();
}
