# `TimeMeasurement::stop()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
void TimeMeasurement::stop();
```

Captura snapshots finales, detiene los contadores y deja la medición lista para calcularse con `elapsed()`. No recibe parámetros ni devuelve valor. Requiere una medición activa; de lo contrario lanza `std::logic_error`. Un fallo de lectura de recursos puede lanzar `std::system_error`.
