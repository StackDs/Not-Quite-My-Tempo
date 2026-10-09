#pragma once

#include <string>

namespace ntq::system {

struct SystemInfo {
    std::string operating_system{};
    std::string cpu_model{};
    std::string architecture{};
    std::string cache_l1{};
    std::string cache_l2{};
    std::string cache_l3{};
    std::string compiler{};
    std::string compiler_version{};
    std::string compiler_flags{};
    std::string cpu_governor{};
    std::string turbo_boost_state{};
    std::string cpu_affinity{};
    std::string cpu_quota{};
    std::string memory_limit{};
};

SystemInfo collect_system_info();

}  // namespace ntq::system
