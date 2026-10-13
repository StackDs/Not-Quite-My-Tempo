# `TimeMeasurement::operator=(TimeMeasurement&&)`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
TimeMeasurement &operator=(TimeMeasurement &&other) noexcept;
```

Reemplaza el estado del objeto destino con el de `other`, transfiriendo contadores y resultados; comprueba autoasignación. Devuelve `*this`. La operación es `noexcept`.
