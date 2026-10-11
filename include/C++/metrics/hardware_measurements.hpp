#pragma once

#include <cstdint>

namespace ntq {

// Estado de soporte/lectura de un contador o de una razón derivada.
enum class HardwareMetricStatus {
  available,
  unsupported,
  permission_denied,
  read_error,
  not_measured,
  invalid_denominator
};

// Valores crudos de PMU y razones derivadas. Los contadores conservan su estado
// para distinguir cero observado de evento no disponible.
struct HardwareMeasurements {
  std::uint64_t cpu_cycles{0}; // Ciclos de CPU en el intervalo medido.
  std::uint64_t instructions_retired{0}; // Instrucciones retiradas.
  double ipc{0.0}; // Instructions retired / CPU cycles.
  HardwareMetricStatus cpu_cycles_status{HardwareMetricStatus::not_measured};
  HardwareMetricStatus instructions_retired_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus ipc_status{HardwareMetricStatus::not_measured};

  // Los porcentajes de caché se calculan sobre accesos de lectura.
  std::uint64_t l1d_read_accesses{0};
  std::uint64_t l1d_read_misses{0}; // Fallos de lectura L1D.
  double l1d_read_miss_percent{0.0}; // Fallos / accesos * 100.
  HardwareMetricStatus l1d_read_accesses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus l1d_read_misses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus l1d_read_miss_percent_status{
      HardwareMetricStatus::not_measured};

  std::uint64_t llc_read_accesses{0};
  std::uint64_t llc_read_misses{0}; // Fallos de lectura LLC.
  double llc_read_miss_percent{0.0}; // Fallos / accesos * 100.
  HardwareMetricStatus llc_read_accesses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus llc_read_misses_status{
      HardwareMetricStatus::not_measured};
  HardwareMetricStatus llc_read_miss_percent_status{
      HardwareMetricStatus::not_measured};

  std::uint64_t branch_mispredictions{0}; // Predicciones de salto fallidas.
  HardwareMetricStatus branch_mispredictions_status{
      HardwareMetricStatus::not_measured};
};

} // namespace ntq
