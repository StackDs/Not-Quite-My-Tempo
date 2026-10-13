# `TimeMeasurement::elapsed()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
void TimeMeasurement::elapsed();
```

Calcula deltas de wall-clock, CPU, cambios de contexto, page faults y memoria, y deriva los valores hardware. No toma parámetros ni devuelve valor. Requiere `stop()` previo y que la medición no siga activa; si el ciclo de vida no es válido lanza `std::logic_error`.
