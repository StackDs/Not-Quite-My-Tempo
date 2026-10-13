# `nanoseconds_per_unit()`

**Ubicación:** `src/C++/metrics/time_conversion.cpp` (helper interno).

## Contrato

```cpp
double nanoseconds_per_unit(TimeUnit unit);
```

`unit` identifica nanosegundos, microsegundos, milisegundos, segundos, minutos u horas. Devuelve el factor de nanosegundos por unidad para convertir métricas temporales. Una unidad no reconocida lanza `std::invalid_argument`.
