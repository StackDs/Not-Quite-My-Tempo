# `status_from_perf_errno()`

**Contrato**

```cpp
MemoryMetricStatus status_from_perf_errno(int error);
```

Mapea errno primero a `PerfEventStatus` y luego al estado de memoria. `error` proviene de syscall PMU; devuelve estado sin lanzar.
