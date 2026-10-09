// Public C++ value types.
#pragma once

namespace ntq {

enum class TimeUnit {
    nanoseconds,
    microseconds,
    milliseconds,
    seconds,
    minutes,
    hours
};

enum class MetricAvailability {
    available,
    unavailable,
    error
};

enum class MetricKind {
    wall_clock,
    cpu_user,
    cpu_system,
    voluntary_context_switches,
    involuntary_context_switches,
    peak_rss,
    minor_page_faults,
    major_page_faults,
    tlb_misses,
    ipc,
    retired_instructions,
    l1d_cache_miss_rate,
    llc_cache_miss_rate,
    branch_mispredictions
};

}  // namespace ntq
