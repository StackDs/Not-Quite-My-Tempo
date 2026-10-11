#pragma once

#include <cstdint>
#include <vector>

namespace ntq {
namespace linux_metrics {

enum class PerfEventStatus {
  available,
  unsupported,
  permission_denied,
  read_error,
  not_measured
};

struct PerfEventSpec {
  std::uint32_t type{0};
  std::uint64_t config{0};
};

struct PerfEventResult {
  std::uint64_t value{0};
  PerfEventStatus status{PerfEventStatus::not_measured};
};

PerfEventStatus perf_status_from_errno(int error);
PerfEventSpec make_hardware_event(std::uint64_t config);
PerfEventSpec make_cache_event(unsigned cache, unsigned operation,
                               unsigned result);

class PerfEventGroup {
public:
  explicit PerfEventGroup(std::vector<PerfEventSpec> events);
  ~PerfEventGroup();

  PerfEventGroup(const PerfEventGroup &) = delete;
  PerfEventGroup &operator=(const PerfEventGroup &) = delete;
  PerfEventGroup(PerfEventGroup &&other) noexcept;
  PerfEventGroup &operator=(PerfEventGroup &&other) noexcept;

  void prepare() noexcept;
  void start() noexcept;
  void stop() noexcept;
  const std::vector<PerfEventResult> &results() const noexcept;

private:
  void close_descriptors() noexcept;
  void mark_group_unavailable(PerfEventStatus status) noexcept;

  std::vector<PerfEventSpec> specs_;
  std::vector<int> descriptors_;
  std::vector<std::uint64_t> ids_;
  std::vector<PerfEventResult> results_;
  std::vector<std::uint64_t> read_buffer_;
  int leader_descriptor_{-1};
  bool is_prepared_{false};
  bool is_running_{false};
};

} // namespace linux_metrics
} // namespace ntq
