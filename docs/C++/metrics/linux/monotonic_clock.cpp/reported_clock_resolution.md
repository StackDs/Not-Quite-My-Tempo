# `TimeMeasurement::reported_clock_resolution()`

**Ubicación:** `src/C++/metrics/linux/monotonic_clock.cpp`.

## Contrato

```cpp
static std::chrono::nanoseconds reported_clock_resolution();
```

No recibe parámetros. Consulta `clock_getres(CLOCK_MONOTONIC)` y devuelve la resolución declarada por el kernel como nanosegundos. No garantiza la precisión efectiva de una medición. Ante fallo POSIX lanza `std::system_error`.
