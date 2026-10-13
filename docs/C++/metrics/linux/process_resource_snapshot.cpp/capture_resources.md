# `TimeMeasurement::capture_resources()`

**Ubicación:** `src/C++/metrics/linux/process_resource_snapshot.cpp`.

## Contrato

```cpp
TimeMeasurement::ResourceSnapshot TimeMeasurement::capture_resources();
```

No recibe parámetros. Usa `getrusage(RUSAGE_SELF)` para capturar CPU user/system, switches voluntarios/involuntarios, peak RSS y page faults. Devuelve snapshot interno; falla de syscall lanza `std::system_error`.
