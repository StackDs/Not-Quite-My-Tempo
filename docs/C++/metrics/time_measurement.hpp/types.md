# Tipos de medición

**Declaración:** `include/C++/metrics/time_measurement.hpp` y `hardware_measurements.hpp`.

- `TimeUnit`: unidades para wall-clock y tiempos CPU.
- `ElapsedMeasurements`: tiempos wall/user/system y context switches; incorpora marca, razón y umbral de sospecha.
- `MemoryMeasurements`: Peak RSS, deltas de page faults, misses DTLB, estados y alcance de RSS.
- `MemoryMetricStatus`: distingue disponible, no soportado, sin permisos, error de lectura y no medido.
- `PeakRssScope`: RSS de vida del proceso o del proceso aislado de medición.
- `SuspicionReason`: razón por la que una ejecución se marca sospechosa.
- `HardwareMeasurements` y `HardwareMetricStatus`: métricas PMU y disponibilidad; ver [`obtain_hardware()`](../time_measurement.cpp/TimeMeasurement-obtain_hardware.md).

Un estado no disponible no se interpreta como una medición numérica igual a cero.
