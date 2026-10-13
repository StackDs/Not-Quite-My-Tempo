# `TimeMeasurement::TimeMeasurement()`

**Definición:** `src/C++/metrics/time_measurement.cpp` · **Declaración:** `include/C++/metrics/time_measurement.hpp`.

## Propósito y contrato

```cpp
explicit TimeMeasurement(long max_involuntary_context_switches = 0);
```

Construye el recolector y configura el umbral de cambios involuntarios de contexto. `max_involuntary_context_switches` debe ser no negativo; su valor predeterminado `0` marca como sospechosa cualquier muestra con cambio involuntario. Un valor negativo lanza `std::invalid_argument`. La instancia administra los recursos de contadores hardware que adquiere durante la medición.
