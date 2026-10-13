# Métricas

La sección `ntq::TimeMeasurement` recoge tiempos, cambios de contexto, memoria y contadores hardware. Los colectores concretos dependen de Linux; disponibilidad y permisos deben revisarse mediante los estados devueltos.

## `time_measurement.cpp`

- [`TimeMeasurement::TimeMeasurement`](time_measurement.cpp/TimeMeasurement-constructor.md)
- [`TimeMeasurement::~TimeMeasurement`](time_measurement.cpp/TimeMeasurement-destructor.md)
- [Constructor de movimiento](time_measurement.cpp/TimeMeasurement-move-constructor.md)
- [Asignación por movimiento](time_measurement.cpp/TimeMeasurement-move-assignment.md)
- [`TimeMeasurement::start`](time_measurement.cpp/TimeMeasurement-start.md)
- [`TimeMeasurement::stop`](time_measurement.cpp/TimeMeasurement-stop.md)
- [`TimeMeasurement::elapsed`](time_measurement.cpp/TimeMeasurement-elapsed.md)
- [`TimeMeasurement::obtain_elapsed`](time_measurement.cpp/TimeMeasurement-obtain_elapsed.md)
- [`TimeMeasurement::obtain_memory`](time_measurement.cpp/TimeMeasurement-obtain_memory.md)
- [`TimeMeasurement::obtain_hardware`](time_measurement.cpp/TimeMeasurement-obtain_hardware.md)

## `monotonic_clock.cpp`

- [`monotonic_now`](linux/monotonic_clock.cpp/monotonic_now.md)
- [`TimeMeasurement::reported_clock_resolution`](linux/monotonic_clock.cpp/reported_clock_resolution.md)

## `time_conversion.cpp`

- [`cpu_time_delta_nanoseconds`](time_conversion.cpp/cpu_time_delta_nanoseconds.md)
- [`nanoseconds_per_unit`](time_conversion.cpp/nanoseconds_per_unit.md)

## `isolated_process.cpp`

- [`TimeMeasurement::measure_in_isolated_process`](linux/isolated_process.cpp/measure_in_isolated_process.md)

- [Implementación Linux de PMU y recursos](linux/index.md)

Los colectores PMU y helpers internos Linux son implementación específica; no son una API portable.
