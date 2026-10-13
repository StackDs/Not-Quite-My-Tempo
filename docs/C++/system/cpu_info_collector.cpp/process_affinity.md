# `process_affinity()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> process_affinity();
```

Consulta `sched_getaffinity` del proceso actual y devuelve CPUs permitidas. Error POSIX se transforma en estado vía errno; no cambia afinidad.
