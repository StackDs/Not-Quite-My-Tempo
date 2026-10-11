#pragma once

#include <cstdint>
#include <limits>

namespace ntq {
namespace linux_metrics {

inline bool peak_rss_kib_to_bytes(std::uint64_t kib, std::uint64_t &bytes) {
  const std::uint64_t multiplier = 1024;
  if (kib > std::numeric_limits<std::uint64_t>::max() / multiplier) {
    return false;
  }
  bytes = kib * multiplier;
  return true;
}

} // namespace linux_metrics
} // namespace ntq
