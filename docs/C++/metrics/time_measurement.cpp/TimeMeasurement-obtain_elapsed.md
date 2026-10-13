# `TimeMeasurement::obtain_elapsed()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
ElapsedMeasurements obtain_elapsed(TimeUnit unit) const;
```

`unit` elige la unidad de wall-clock, CPU user y CPU system. Devuelve esos tiempos, switches y estado de sospecha/umbral. Requiere que `elapsed()` haya calculado la medición; en otro caso lanza `std::logic_error`. Una unidad inválida lanza `std::invalid_argument`. Las llamadas repetidas no consumen el resultado.
