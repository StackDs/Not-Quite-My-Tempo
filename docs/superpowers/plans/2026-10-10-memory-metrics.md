# Memory Metrics Implementation Plan

> **For agentic workers:** implement natively, task by task, using the checks below before moving to the next task.

**Goal:** Add Peak RSS, minor/major page-fault deltas, and explicitly scoped DTLB miss counters to each isolated C++/Linux benchmark measurement.

**Architecture:** Extend `TimeMeasurement`'s existing `getrusage` snapshots to carry process peak RSS and fault counters, and expose a separate `MemoryMeasurements` result through `obtain_memory()`. A private Linux `perf_event_open` helper manages DTLB read/write miss events; failure to open or read hardware events marks only those counters unavailable and does not fail time or RSS measurement.

**Tech Stack:** C++14, Linux `getrusage`, `perf_event_open`, `ioctl`, and the existing fork/pipe isolated measurement API; standalone assertion-based C++ tests.

**Spec:** `requisitos_de_evaluaci_n.md`, section 3, plus the agreed interpretation that TLB telemetry reports data-TLB read and write misses separately.

## Global Constraints

- Linux and C++ only for this implementation.
- Preserve the existing time-unit API; memory values are independent of `TimeUnit`.
- `ru_maxrss` is a process-lifetime high-water mark, not a subtractable interval counter. In isolated runs, report its Linux value in bytes and label its scope as the isolated measurement process.
- Minor and major page faults are deltas of `ru_minflt` and `ru_majflt` across the measurement interval.
- TLB event failures must not fail the algorithm measurement and must never be reported as a valid zero count.
- The class must release perf file descriptors on normal stop, error, move, and destruction paths.
- Keep C++14 compatibility and add no external dependency.

## Review Focus

- Peak RSS scope: verify isolated results label the fresh child-process high-water mark, while manual measurements explicitly identify their process-lifetime scope.
- `ru_maxrss` units/overflow: convert Linux KiB to bytes with checked arithmetic; do not subtract start and end peaks.
- Counter availability: `EPERM`/`EACCES`, unsupported event encodings, and read failures must produce distinct non-available statuses, never a valid zero.
- TLB event scope: report DTLB read and write misses separately and document that instruction-TLB misses are not included.
- Isolation and lifetime: serialize the expanded result through the existing pipe and ensure perf descriptors close if the callback throws or the measurement object moves/dies.

---

### Task 1: Memory result type and `getrusage` metrics

**Files:**
- Modify: `include/C++/metrics/time_measurement.hpp`
- Modify: `src/C++/metrics/time_measurement.cpp`
- Create: `src/C++/metrics/linux/memory_helpers.hpp`
- Test: `tests/C++/metrics/time_measurement_tests.cpp`

**Interfaces:**
- Add `enum class MemoryMetricStatus { available, unsupported, permission_denied, read_error, not_measured }`.
- Add `enum class PeakRssScope { process_lifetime, isolated_measurement_process }`.
- Add `MemoryMeasurements` with `std::uint64_t peak_rss_bytes`, `minor_page_faults`, `major_page_faults`, `data_tlb_read_misses`, `data_tlb_write_misses`; `MemoryMetricStatus peak_rss_status`, statuses for the two TLB counters; and `peak_rss_scope`.
- Add internal inline signature `bool peak_rss_kib_to_bytes(std::uint64_t kib, std::uint64_t &bytes)` in `memory_helpers.hpp`; return `false` if multiplying by 1024 would overflow.
- Add `MemoryMeasurements TimeMeasurement::obtain_memory() const`; it is available after `elapsed()` and does not take a time unit.
- Extend private `ResourceSnapshot` with Linux `ru_maxrss`, `ru_minflt`, and `ru_majflt`; store the computed memory result with the time result.
- Extend `measure_in_isolated_process()`'s pipe payload to transfer both elapsed and memory results. The payload remains fixed-size and trivially copyable.
- Make `TimeMeasurement` move-only if required by the RAII perf-counter member; preserve its default and threshold constructors and return-by-value isolated API.

- [ ] **Step 1: Add failing tests for memory result state and conversions**

Test that `obtain_memory()` before `elapsed()` throws `std::logic_error`; test `peak_rss_kib_to_bytes(7, bytes) == true` with `bytes == 7168` and that `peak_rss_kib_to_bytes(UINT64_MAX, bytes) == false`; test time results remain convertible to multiple units independently of memory and a direct measurement labels its RSS scope `process_lifetime`.

- [ ] **Step 2: Run the standalone metrics test and confirm failure**

Run: `g++ -std=c++14 -Wall -Wextra -Wpedantic -Iinclude/C++ -Isrc/C++/metrics -Isrc/C++/metrics/linux tests/C++/metrics/time_measurement_tests.cpp src/C++/metrics/time_measurement.cpp src/C++/metrics/time_conversion.cpp src/C++/metrics/linux/monotonic_clock.cpp src/C++/metrics/linux/process_resource_snapshot.cpp src/C++/metrics/linux/isolated_process.cpp src/C++/metrics/linux/perf_event_group.cpp src/C++/metrics/linux/tlb_miss_counters.cpp src/C++/metrics/linux/hardware_counters.cpp src/C++/metrics/linux/hardware_measurement_calculations.cpp -o /tmp/time_measurement_tests`

Expected: missing memory API/helper or failed assertions.

- [ ] **Step 3: Capture `ru_maxrss`, `ru_minflt`, and `ru_majflt`**

Copy the fields in `capture_resources()`. Compute minor/major values as end-minus-start. Use only the final `ru_maxrss` value, convert Linux KiB to bytes with overflow checking, and set the scope to `process_lifetime` for manual measurements.

- [ ] **Step 4: Add `obtain_memory()` and extend isolated-result transfer**

Require `has_elapsed_`, then return the stored `MemoryMeasurements`. In the child, mark the peak scope `isolated_measurement_process`; send elapsed and memory structs together; restore both in the parent.

- [ ] **Step 5: Run the standalone metrics test**

Run the command from Step 2, then `/tmp/time_measurement_tests`.

Expected: memory API state checks, KiB-to-byte conversion, page-fault deltas, and existing time-unit checks pass.

### Task 2: Linux DTLB perf counters

**Files:**
- Create: `src/C++/metrics/linux/tlb_miss_counters.hpp`
- Create: `src/C++/metrics/linux/tlb_miss_counters.cpp`
- Modify: `include/C++/metrics/time_measurement.hpp`
- Modify: `src/C++/metrics/time_measurement.cpp`
- Test: `tests/C++/metrics/time_measurement_tests.cpp`

**Interfaces:**
- Add a private move-only `TlbMissCounters` RAII helper with `start()`, `stop()`, and a fixed-size result containing read/write miss counts and individual `MemoryMetricStatus` values.
- Declare `MemoryMetricStatus status_from_perf_errno(int error)` in `tlb_miss_counters.hpp` for deterministic status tests.
- Use `perf_event_open` with `PERF_TYPE_HW_CACHE`, `PERF_COUNT_HW_CACHE_DTLB`, `PERF_COUNT_HW_CACHE_OP_READ/WRITE`, and `PERF_COUNT_HW_CACHE_RESULT_MISS`.
- Map `EPERM`/`EACCES` to `permission_denied`, `EINVAL`/`ENOENT`/`EOPNOTSUPP` to `unsupported`, and other ioctl/read failures to `read_error`.
- Store the helper behind an out-of-line-managed pointer in `TimeMeasurement` so Linux perf internals do not enter the public header.
- Declare `TimeMeasurement` move-only with out-of-line destructor and move constructor/assignment; delete copy construction/assignment to avoid duplicating active RAII counter state.

- [ ] **Step 1: Add failing tests for perf error mapping and unavailable-counter behavior**

Test `status_from_perf_errno(int error)` for `EPERM`, `EACCES`, `EINVAL`, `EOPNOTSUPP`, and `EIO`; verify an unavailable read/write event remains marked unavailable instead of appearing as a valid zero. Add a host integration assertion that each status is one of the defined values; do not require perf permissions or hardware support in the test environment.

- [ ] **Step 2: Run the standalone metrics test and confirm failure**

Run: `g++ -std=c++14 -Wall -Wextra -Wpedantic -Iinclude/C++ -Isrc/C++/metrics -Isrc/C++/metrics/linux tests/C++/metrics/time_measurement_tests.cpp src/C++/metrics/time_measurement.cpp src/C++/metrics/time_conversion.cpp src/C++/metrics/linux/monotonic_clock.cpp src/C++/metrics/linux/process_resource_snapshot.cpp src/C++/metrics/linux/isolated_process.cpp src/C++/metrics/linux/perf_event_group.cpp src/C++/metrics/linux/tlb_miss_counters.cpp src/C++/metrics/linux/hardware_counters.cpp src/C++/metrics/linux/hardware_measurement_calculations.cpp -o /tmp/time_measurement_tests`

Expected: perf status mapping or counter lifecycle cases fail until implemented.

- [ ] **Step 3: Implement DTLB event setup and RAII lifecycle**

Open separate read/write events with the kernel perf syscall, configure user-space counting (`exclude_kernel` and `exclude_hv`), reset/enable at `start()`, disable/read/close at `stop()`. If setup or access is unavailable, retain the time and `getrusage` metrics and store the corresponding TLB status.

- [ ] **Step 4: Integrate counter results with `TimeMeasurement` and the isolated child**

Start counters around the measured interval, stop them before the final resource snapshot, copy their values/statuses into `MemoryMeasurements`, and serialize them with the same isolated result payload. Ensure the helper closes file descriptors during callback failure and object destruction.

- [ ] **Step 5: Run the standalone metrics test**

Run the command from Step 2, then `/tmp/time_measurement_tests`.

Expected: if perf is permitted, counts have `available` status; otherwise tests pass with the appropriate non-available status and the rest of the measurement remains valid.

### Task 3: Documentation and end-to-end verification

**Files:**
- Modify: `docs/C++/metrics/time_measurement.hpp.md`
- Modify: `docs/C++/metrics/time_measurement.cpp.md`
- Modify: `tests/C++/metrics/time_measurement_tests.cpp`

**Interfaces:**
- Document `MemoryMeasurements TimeMeasurement::obtain_memory() const`, every field/status, units, and peak RSS scope.

- [ ] **Step 1: Add isolated memory integration assertions**

Run an isolated callback that touches a bounded anonymous allocation. Assert peak RSS is nonzero, has `available` status, and is labeled `isolated_measurement_process`; page-fault counters are nonnegative; memory values survive the child-to-parent pipe; and TLB statuses are valid whether or not perf is available. Also move a running `TimeMeasurement`, stop/read it after the move, and verify a throwing isolated callback leaves the parent usable and propagates the existing runtime error.

- [ ] **Step 2: Run the integration test**

Run the full command from Task 2.

Expected: the callback's memory measurements arrive with the time result and no perf restriction aborts the measurement.

- [ ] **Step 3: Document metric meanings and limits**

State that Peak RSS is the child process's lifetime high-water RSS (Linux KiB converted to bytes), page-fault counts are interval deltas, and TLB values are separate DTLB read/write misses. Explain that perf permissions/hardware may make those counters unavailable and that ITLB misses are not included.

- [ ] **Step 4: Run final compile, tests, and whitespace validation**

Run: `g++ -std=c++14 -Wall -Wextra -Wpedantic -Werror -Iinclude/C++ -Isrc/C++/metrics -Isrc/C++/metrics/linux tests/C++/metrics/time_measurement_tests.cpp src/C++/metrics/time_measurement.cpp src/C++/metrics/time_conversion.cpp src/C++/metrics/linux/monotonic_clock.cpp src/C++/metrics/linux/process_resource_snapshot.cpp src/C++/metrics/linux/isolated_process.cpp src/C++/metrics/linux/perf_event_group.cpp src/C++/metrics/linux/tlb_miss_counters.cpp src/C++/metrics/linux/hardware_counters.cpp src/C++/metrics/linux/hardware_measurement_calculations.cpp -o /tmp/time_measurement_tests && /tmp/time_measurement_tests`

Run: `git diff --check`

Expected: successful compile/test exit and no whitespace errors.
