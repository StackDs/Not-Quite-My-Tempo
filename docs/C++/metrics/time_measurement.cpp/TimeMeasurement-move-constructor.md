# `TimeMeasurement::TimeMeasurement(TimeMeasurement&&)`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
TimeMeasurement(TimeMeasurement &&other) noexcept;
```

Transfiere de `other` los snapshots, medidas, contadores y estado del ciclo de vida. `other` es la instancia origen y queda sin poseer los contadores transferidos. El movimiento no lanza excepción.
