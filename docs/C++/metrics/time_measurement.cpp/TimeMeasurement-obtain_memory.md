# `TimeMeasurement::obtain_memory()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
MemoryMeasurements obtain_memory() const;
```

No recibe parámetros. Devuelve Peak RSS, deltas minor/major page faults, fallos DTLB y estados/scope de medición. Requiere `elapsed()` previo; de otro modo lanza `std::logic_error`. Un estado distinto de `available` significa que el contador no se midió correctamente, no que sea cero.
