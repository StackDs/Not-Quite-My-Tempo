#include "statistics_internal.hpp"

#include <cmath>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

// Normaliza estados específicos de los colectores al vocabulario estadístico.
MetricValueStatus map_memory_status(MemoryMetricStatus status) {
  switch (status) {
  case MemoryMetricStatus::available:
    return MetricValueStatus::available;
  case MemoryMetricStatus::unsupported:
    return MetricValueStatus::unsupported;
  case MemoryMetricStatus::permission_denied:
    return MetricValueStatus::permission_denied;
  case MemoryMetricStatus::read_error:
    return MetricValueStatus::read_error;
  case MemoryMetricStatus::not_measured:
    return MetricValueStatus::not_measured;
  }
  return MetricValueStatus::invalid_value;
}

// Mantiene independiente el módulo estadístico del tipo de status de hardware.
MetricValueStatus map_hardware_status(HardwareMetricStatus status) {
  switch (status) {
  case HardwareMetricStatus::available:
    return MetricValueStatus::available;
  case HardwareMetricStatus::unsupported:
    return MetricValueStatus::unsupported;
  case HardwareMetricStatus::permission_denied:
    return MetricValueStatus::permission_denied;
  case HardwareMetricStatus::read_error:
    return MetricValueStatus::read_error;
  case HardwareMetricStatus::not_measured:
    return MetricValueStatus::not_measured;
  case HardwareMetricStatus::invalid_denominator:
    return MetricValueStatus::invalid_denominator;
  }
  return MetricValueStatus::invalid_value;
}

MetricObservation number(long double value, MetricValueStatus status) {
  // Un valor no finito o negativo disponible es dato inválido, no ausencia.
  if (status == MetricValueStatus::available &&
      (!std::isfinite(value) || value < 0.0L)) {
    return {0.0L, MetricValueStatus::invalid_value};
  }
  return {value, status};
}

} // namespace

const std::vector<MetricId> &metric_ids() {
  // Define el orden estable de las métricas en cada reporte.
  static const std::vector<MetricId> ids = {
      MetricId::wall_clock,
      MetricId::cpu_user,
      MetricId::cpu_system,
      MetricId::voluntary_context_switches,
      MetricId::involuntary_context_switches,
      MetricId::peak_rss_bytes,
      MetricId::minor_page_faults,
      MetricId::major_page_faults,
      MetricId::data_tlb_read_misses,
      MetricId::data_tlb_write_misses,
      MetricId::cpu_cycles,
      MetricId::instructions_retired,
      MetricId::ipc,
      MetricId::l1d_read_accesses,
      MetricId::l1d_read_misses,
      MetricId::l1d_read_miss_percent,
      MetricId::llc_read_accesses,
      MetricId::llc_read_misses,
      MetricId::llc_read_miss_percent,
      MetricId::branch_mispredictions};
  return ids;
}

MetricObservation observe_metric(const BenchmarkSample &sample,
                                 MetricId metric) {
  // Extrae una métrica escalar de los resultados sin interpretar el grupo.
  switch (metric) {
  case MetricId::wall_clock:
    return number(sample.elapsed.wall_clock, MetricValueStatus::available);
  case MetricId::cpu_user:
    return number(sample.elapsed.cpu_user, MetricValueStatus::available);
  case MetricId::cpu_system:
    return number(sample.elapsed.cpu_system, MetricValueStatus::available);
  case MetricId::voluntary_context_switches:
    return number(sample.elapsed.voluntary_context_switches,
                  MetricValueStatus::available);
  case MetricId::involuntary_context_switches:
    return number(sample.elapsed.involuntary_context_switches,
                  MetricValueStatus::available);
  case MetricId::peak_rss_bytes:
    return number(sample.memory.peak_rss_bytes,
                  map_memory_status(sample.memory.peak_rss_status));
  case MetricId::minor_page_faults:
    return number(sample.memory.minor_page_faults,
                  MetricValueStatus::available);
  case MetricId::major_page_faults:
    return number(sample.memory.major_page_faults,
                  MetricValueStatus::available);
  case MetricId::data_tlb_read_misses:
    return number(sample.memory.data_tlb_read_misses,
                  map_memory_status(sample.memory.data_tlb_read_status));
  case MetricId::data_tlb_write_misses:
    return number(sample.memory.data_tlb_write_misses,
                  map_memory_status(sample.memory.data_tlb_write_status));
  case MetricId::cpu_cycles:
    return number(sample.hardware.cpu_cycles,
                  map_hardware_status(sample.hardware.cpu_cycles_status));
  case MetricId::instructions_retired:
    return number(sample.hardware.instructions_retired,
                  map_hardware_status(
                      sample.hardware.instructions_retired_status));
  case MetricId::ipc:
    return number(sample.hardware.ipc,
                  map_hardware_status(sample.hardware.ipc_status));
  case MetricId::l1d_read_accesses:
    return number(sample.hardware.l1d_read_accesses,
                  map_hardware_status(
                      sample.hardware.l1d_read_accesses_status));
  case MetricId::l1d_read_misses:
    return number(sample.hardware.l1d_read_misses,
                  map_hardware_status(sample.hardware.l1d_read_misses_status));
  case MetricId::l1d_read_miss_percent:
    return number(sample.hardware.l1d_read_miss_percent,
                  map_hardware_status(
                      sample.hardware.l1d_read_miss_percent_status));
  case MetricId::llc_read_accesses:
    return number(sample.hardware.llc_read_accesses,
                  map_hardware_status(
                      sample.hardware.llc_read_accesses_status));
  case MetricId::llc_read_misses:
    return number(sample.hardware.llc_read_misses,
                  map_hardware_status(sample.hardware.llc_read_misses_status));
  case MetricId::llc_read_miss_percent:
    return number(sample.hardware.llc_read_miss_percent,
                  map_hardware_status(
                      sample.hardware.llc_read_miss_percent_status));
  case MetricId::branch_mispredictions:
    return number(sample.hardware.branch_mispredictions,
                  map_hardware_status(
                      sample.hardware.branch_mispredictions_status));
  }
  return {0.0L, MetricValueStatus::invalid_value};
}

} // namespace detail
} // namespace statistics
} // namespace ntq
