#include "statistics_internal.hpp"

#include <limits>

namespace ntq {
namespace statistics {
namespace detail {
namespace {

template <class T>
CollectedValue<bool> compare_values(const CollectedValue<T> &before,
                                    const CollectedValue<T> &after,
                                    const std::string &source) {
  // Si falta cualquiera de los snapshots, no se infiere que el valor cambió.
  if (before.status == CollectionStatus::available &&
      after.status == CollectionStatus::available) {
    return CollectedValue<bool>(before.value != after.value,
                                CollectionStatus::available, source);
  }
  const CollectionStatus status =
      before.status != CollectionStatus::available ? before.status
                                                    : after.status;
  return CollectedValue<bool>(false, status, source);
}

void compare_cpu_value(const CollectedValue<std::string> &before,
                       const CollectedValue<std::string> &after,
                       bool &changed, CollectionStatus &unknown_status) {
  if (before.status == CollectionStatus::available &&
      after.status == CollectionStatus::available) {
    changed = changed || before.value != after.value;
  } else if (unknown_status == CollectionStatus::available) {
    unknown_status = before.status != CollectionStatus::available
                         ? before.status
                         : after.status;
  }
}

void compare_cpu_value(const CollectedValue<std::uint64_t> &before,
                       const CollectedValue<std::uint64_t> &after,
                       bool &changed, CollectionStatus &unknown_status) {
  if (before.status == CollectionStatus::available &&
      after.status == CollectionStatus::available) {
    changed = changed || before.value != after.value;
  } else if (unknown_status == CollectionStatus::available) {
    unknown_status = before.status != CollectionStatus::available
                         ? before.status
                         : after.status;
  }
}

CollectedValue<bool> cpu_policy_changed(const CpuRuntimeInfo &before,
                                        const CpuRuntimeInfo &after) {
  const std::string source = "SystemInfo.cpu_runtime.per_cpu";
  if (before.per_cpu.status != CollectionStatus::available ||
      after.per_cpu.status != CollectionStatus::available) {
    const CollectionStatus status =
        before.per_cpu.status != CollectionStatus::available
            ? before.per_cpu.status
            : after.per_cpu.status;
    return CollectedValue<bool>(false, status, source);
  }

  if (before.per_cpu.value.size() != after.per_cpu.value.size()) {
    return CollectedValue<bool>(true, CollectionStatus::available, source);
  }

  bool changed = false;
  CollectionStatus unknown_status = CollectionStatus::available;
  // Compara gobernador y límites/frecuencia por CPU; un campo no disponible
  // vuelve indeterminado el resultado si no se detecta otro cambio conocido.
  for (std::size_t index = 0; index < before.per_cpu.value.size(); ++index) {
    const CpuFrequencyInfo &first = before.per_cpu.value[index];
    const CpuFrequencyInfo &last = after.per_cpu.value[index];
    changed = changed || first.cpu_id != last.cpu_id;
    compare_cpu_value(first.governor, last.governor, changed, unknown_status);
    compare_cpu_value(first.current_frequency_hz, last.current_frequency_hz,
                      changed, unknown_status);
    compare_cpu_value(first.minimum_frequency_hz, last.minimum_frequency_hz,
                      changed, unknown_status);
    compare_cpu_value(first.maximum_frequency_hz, last.maximum_frequency_hz,
                      changed, unknown_status);
  }
  if (changed) {
    return CollectedValue<bool>(true, CollectionStatus::available, source);
  }
  return CollectedValue<bool>(false, unknown_status, source);
}

CollectedValue<bool> throttling_observed(
    const CollectedValue<bool> &before, const CollectedValue<bool> &after) {
  const std::string source = "environment thermal throttling indicators";
  if ((before.status == CollectionStatus::available && before.value) ||
      (after.status == CollectionStatus::available && after.value)) {
    return CollectedValue<bool>(true, CollectionStatus::available, source);
  }
  if (before.status == CollectionStatus::available &&
      after.status == CollectionStatus::available) {
    return CollectedValue<bool>(false, CollectionStatus::available, source);
  }
  const CollectionStatus status =
      before.status != CollectionStatus::available ? before.status
                                                   : after.status;
  return CollectedValue<bool>(false, status, source);
}

} // namespace

EnvironmentDiagnostics diagnose_environment(const MeasurementContext &context) {
  // Solo deriva indicadores de los snapshots; nunca ajusta ni invalida métricas.
  EnvironmentDiagnostics diagnostics;
  diagnostics.runnable_process_delta =
      CollectedValue<std::int64_t>(0, CollectionStatus::unavailable,
                                   "environment runnable_processes");
  const auto &runnable_before = context.before.runnable_processes;
  const auto &runnable_after = context.after.runnable_processes;
  if (runnable_before.status == CollectionStatus::available &&
      runnable_after.status == CollectionStatus::available) {
    if (runnable_after.value >= runnable_before.value) {
      const std::uint64_t delta = runnable_after.value - runnable_before.value;
      if (delta <= static_cast<std::uint64_t>(
                       std::numeric_limits<std::int64_t>::max())) {
        diagnostics.runnable_process_delta = CollectedValue<std::int64_t>(
            static_cast<std::int64_t>(delta), CollectionStatus::available,
            "environment runnable_processes");
      } else {
        diagnostics.runnable_process_delta.status = CollectionStatus::malformed;
      }
    } else {
      const std::uint64_t delta = runnable_before.value - runnable_after.value;
      if (delta <= static_cast<std::uint64_t>(
                       std::numeric_limits<std::int64_t>::max())) {
        diagnostics.runnable_process_delta = CollectedValue<std::int64_t>(
            -static_cast<std::int64_t>(delta), CollectionStatus::available,
            "environment runnable_processes");
      } else {
        diagnostics.runnable_process_delta.status = CollectionStatus::malformed;
      }
    }
  } else {
    diagnostics.runnable_process_delta.status =
        runnable_before.status != CollectionStatus::available
            ? runnable_before.status
            : runnable_after.status;
  }

  const auto &temperature_before = context.before.cpu_temperature_celsius;
  const auto &temperature_after = context.after.cpu_temperature_celsius;
  if (temperature_before.status == CollectionStatus::available &&
      temperature_after.status == CollectionStatus::available) {
    diagnostics.cpu_temperature_delta_celsius = CollectedValue<double>(
        temperature_after.value - temperature_before.value,
        CollectionStatus::available, "environment cpu_temperature_celsius");
  } else {
    diagnostics.cpu_temperature_delta_celsius.status =
        temperature_before.status != CollectionStatus::available
            ? temperature_before.status
            : temperature_after.status;
  }

  diagnostics.cpu_affinity_changed = compare_values(
      context.before.system.cpu_runtime.process_affinity,
      context.after.system.cpu_runtime.process_affinity,
      "SystemInfo.cpu_runtime.process_affinity");
  diagnostics.cpu_governor_or_frequency_changed = cpu_policy_changed(
      context.before.system.cpu_runtime, context.after.system.cpu_runtime);
  diagnostics.thermal_throttling_observed = throttling_observed(
      context.before.thermal_throttling,
      context.after.thermal_throttling);
  return diagnostics;
}

} // namespace detail
} // namespace statistics
} // namespace ntq
