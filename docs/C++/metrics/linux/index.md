# Helpers Linux de métricas

Los contadores `perf_event_open` dependen de PMU, kernel y permisos; sus estados no disponibles no equivalen a cero.

- [Grupo de eventos PMU](perf_event_group.cpp/PerfEventGroup-prepare.md) y [catálogo de funciones](perf_event_group.cpp/index.md).
- [Contadores hardware](hardware_counters.cpp/index.md).
- [Contadores DTLB](tlb_miss_counters.cpp/index.md).
- [Métricas derivadas](hardware_measurement_calculations.cpp/calculate_hardware_derived_metrics.md).
- [Snapshots de recursos](process_resource_snapshot.cpp/capture_resources.md).
- [Contadores DTLB y ejecución aislada](isolated_process.cpp/measure_in_isolated_process.md).
