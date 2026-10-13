# `monotonic_now()`

**Ubicación:** `src/C++/metrics/linux/monotonic_clock.cpp` (helper interno Linux).

## Contrato

```cpp
std::chrono::nanoseconds monotonic_now();
```

Lee `CLOCK_MONOTONIC` y devuelve el instante convertido a nanosegundos. No recibe parámetros. Si `clock_gettime` falla, lanza `std::system_error` con el error del sistema.
