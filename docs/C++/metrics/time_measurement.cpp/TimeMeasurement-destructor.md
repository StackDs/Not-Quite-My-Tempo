# `TimeMeasurement::~TimeMeasurement()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
TimeMeasurement::~TimeMeasurement();
```

No recibe parámetros ni devuelve valor. Destruye los recursos administrados por la instancia, incluidos contadores RAII; no realiza ni exporta una medición.
