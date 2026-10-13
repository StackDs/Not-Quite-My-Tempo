# `TimeMeasurement::measure_in_isolated_process()`

**Ubicación:** `src/C++/metrics/linux/isolated_process.cpp` · **Declaración:** `include/C++/metrics/time_measurement.hpp`.

## Contrato

```cpp
static TimeMeasurement measure_in_isolated_process(
    const std::function<void()> &algorithm,
    long max_involuntary_context_switches = 0);
```

`algorithm` es el callback sin argumentos que se ejecuta en el hijo; no puede estar vacío. `max_involuntary_context_switches` es el umbral no negativo de sospecha. Devuelve una medición detenida y calculada, con Peak RSS etiquetado como alcance del proceso aislado. Requiere caller monohilo. Usa `fork()` y un pipe; los cambios de memoria del hijo no se reflejan en el padre. Parámetros inválidos lanzan `std::invalid_argument`; fallos POSIX lanzan `std::system_error`; fallo de callback/medición se comunica como `std::runtime_error`.
