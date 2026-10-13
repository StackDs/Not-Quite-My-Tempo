# `cpu_time_delta_nanoseconds()`

**Ubicación:** `src/C++/metrics/time_conversion.cpp` (helper interno).

## Contrato

```cpp
long double cpu_time_delta_nanoseconds(long seconds, long microseconds);
```

Convierte un delta `timeval` a nanosegundos. `seconds` aporta segundos enteros y `microseconds` microsegundos; el retorno es `(seconds * 1'000'000 + microseconds) * 1'000`. Devuelve `long double`; no valida ni normaliza las partes.
