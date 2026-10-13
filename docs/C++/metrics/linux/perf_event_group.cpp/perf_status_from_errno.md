# `perf_status_from_errno()`

**Contrato**

```cpp
PerfEventStatus perf_status_from_errno(int error);
```

Mapea un código errno de `perf_event_open`/lecturas a permiso denegado, unsupported o read error. `error` es errno capturado; devuelve estado sin lanzar.
