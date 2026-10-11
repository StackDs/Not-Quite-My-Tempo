#include "linux/tlb_miss_counters.hpp"
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
  test_isolated_memory_transfer_and_optional_tlb();
  test_isolated_callback_exception();
}
