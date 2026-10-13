# `TimeMeasurement::obtain_hardware()`

**Definición:** `src/C++/metrics/time_measurement.cpp`.

## Contrato

```cpp
HardwareMeasurements obtain_hardware() const;
```

No recibe parámetros. Devuelve ciclos, instrucciones retiradas, IPC, fallos/accesos de caché y branch mispredictions con estados de disponibilidad. Requiere `elapsed()` previo y lanza `std::logic_error` si todavía no se calculó. La medición puede depender de permisos, kernel y PMU.
