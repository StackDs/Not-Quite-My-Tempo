#include "system/system_info.hpp"
#include "cgroup_parsers.hpp"
#include "cpu_parsers.hpp"
#include "file_reader.hpp"

#include <cassert>
#include <cerrno>
#include <cstdint>
#include <sched.h>
#include <string>
#include <vector>

namespace {

void test_cpu_list_parser() {
  const auto cpus = ntq::linux_system::parse_cpu_list("0-2,5", "fixture");
  assert(cpus.status == ntq::CollectionStatus::available);
  assert((cpus.value == std::vector<unsigned>{0, 1, 2, 5}));
  assert(ntq::linux_system::parse_cpu_list("1-x", "fixture").status ==
         ntq::CollectionStatus::malformed);

  const auto size = ntq::linux_system::parse_cache_size("32K", "fixture");
  assert(size.status == ntq::CollectionStatus::available);
  assert(size.value == 32 * 1024);
  assert(ntq::linux_system::parse_cache_size("18446744073709551615G", "fixture")
             .status == ntq::CollectionStatus::malformed);

  const auto identity = ntq::linux_system::parse_cpuinfo(
      "vendor_id : ExampleVendor\nmodel name : Example Processor\n");
  assert(identity.vendor.value == "ExampleVendor");
  assert(identity.model.value == "Example Processor");

  ntq::CacheInfo cache{};
  cache.level = 1;
  cache.type = "Data";
  cache.size_bytes = 32768;
  cache.shared_cpus = {0, 1};
  assert(ntq::linux_system::deduplicate_caches({cache, cache}).size() == 1);

  assert(ntq::linux_system::status_from_errno(EACCES) ==
         ntq::CollectionStatus::permission_denied);
  assert(ntq::linux_system::status_from_errno(ENOENT) ==
         ntq::CollectionStatus::unavailable);
}

void test_cgroup_parser() {
  const auto membership = ntq::linux_system::parse_cgroup_membership(
      "0::/group/job\n2:cpu,cpuacct:/group/job\n3:memory:/group/job\n");
  assert(membership.unified_path == "/group/job");
  assert(membership.controller_paths.at("cpu") == "/group/job");

  const auto mounts = ntq::linux_system::parse_cgroup_mounts(
      "29 23 0:26 / /sys/fs/cgroup rw - cgroup2 cgroup2 rw\n"
      "30 23 0:27 / /sys/fs/cgroup/cpu rw - cgroup cgroup rw,cpu,cpuacct\n");
  assert(mounts.size() == 2);
  assert(mounts[0].version == 2);
  assert(mounts[1].version == 1);

  const auto quota =
      ntq::linux_system::parse_cpu_quota_v2("50000 100000", "fixture");
  assert(quota.status == ntq::CollectionStatus::available);
  assert(quota.value.quota_us == 50000);
  assert(quota.value.period_us == 100000);
  assert(ntq::linux_system::parse_cpu_quota_v2("max 100000", "fixture")
             .status == ntq::CollectionStatus::unlimited);
  assert(ntq::linux_system::parse_cpu_quota_v1("-1", "100000", "fixture")
             .status == ntq::CollectionStatus::unlimited);

  const auto memory =
      ntq::linux_system::parse_memory_limit("1048576", "fixture");
  assert(memory.value == 1048576);
  assert(ntq::linux_system::parse_memory_limit("max", "fixture").status ==
         ntq::CollectionStatus::unlimited);

  const auto cpuset =
      ntq::linux_system::parse_cpu_set("0-3,6", "fixture");
  assert((cpuset.value == std::vector<unsigned>{0, 1, 2, 3, 6}));
  const auto effective_set =
      ntq::linux_system::resolve_effective_cpu_set(
          {ntq::CollectedValue<std::vector<unsigned>>(
               std::vector<unsigned>{0, 1, 2, 3},
               ntq::CollectionStatus::available, "child"),
           ntq::CollectedValue<std::vector<unsigned>>(
               std::vector<unsigned>{2, 3, 4, 5},
               ntq::CollectionStatus::available, "parent")});
  assert((effective_set.value == std::vector<unsigned>{2, 3}));

  const auto effective_quota =
      ntq::linux_system::resolve_effective_cpu_quota(
          {ntq::CollectedValue<ntq::linux_system::CpuQuota>(
               {200000, 100000}, ntq::CollectionStatus::available, "parent"),
           ntq::CollectedValue<ntq::linux_system::CpuQuota>(
               {50000, 100000}, ntq::CollectionStatus::available, "child")});
  assert(effective_quota.value.quota_us == 50000);
  const auto effective_memory =
      ntq::linux_system::resolve_effective_memory_limit(
          {ntq::CollectedValue<std::uint64_t>(2000000,
                                               ntq::CollectionStatus::available,
                                               "parent"),
           ntq::CollectedValue<std::uint64_t>(1000000,
                                               ntq::CollectionStatus::available,
                                               "child")});
  assert(effective_memory.value == 1000000);
}

void test_live_collection_is_partial_and_read_only() {
  cpu_set_t before;
  cpu_set_t after;
  CPU_ZERO(&before);
  CPU_ZERO(&after);
  assert(sched_getaffinity(0, sizeof(before), &before) == 0);

  const ntq::SystemInfo info = ntq::collect_system_info();

  assert(sched_getaffinity(0, sizeof(after), &after) == 0);
  for (unsigned cpu = 0; cpu < CPU_SETSIZE; ++cpu) {
    assert(CPU_ISSET(cpu, &before) == CPU_ISSET(cpu, &after));
  }
  assert(info.compiler.family.status == ntq::CollectionStatus::available);
  assert(info.compiler.version.status == ntq::CollectionStatus::available);
  assert(info.compiler.language_standard.status ==
         ntq::CollectionStatus::available);
  if (info.cpu.online_logical_cpus.status ==
      ntq::CollectionStatus::available) {
    assert(!info.cpu.online_logical_cpus.value.empty());
  }
  if (info.cpu.caches.status == ntq::CollectionStatus::available) {
    for (const auto &cache : info.cpu.caches.value) {
      assert(cache.level >= 1 && cache.level <= 3);
      assert(cache.size_bytes > 0);
    }
  }
}

} // namespace

int main() {
  test_cpu_list_parser();
  test_cgroup_parser();
  test_live_collection_is_partial_and_read_only();
}
