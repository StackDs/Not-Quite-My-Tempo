# `TimeMeasurement::start()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
void TimeMeasurement::start();
```

Inicia o reinicia la medición: prepara contadores TLB/PMU y captura snapshots iniciales de reloj y recursos. No recibe parámetros ni devuelve valor. Los errores de captura de recursos pueden propagarse como `std::system_error`; la disponibilidad de contadores hardware se informa mediante estados, no como ceros. Debe llamarse antes de `stop()`.
