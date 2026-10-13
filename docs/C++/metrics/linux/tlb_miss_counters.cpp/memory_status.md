# `memory_status()`

**Contrato**

```cpp
MemoryMetricStatus memory_status(PerfEventStatus status);
```

Traduce estado de evento PMU a disponibilidad de métrica de memoria. Devuelve mapeo equivalente y usa `read_error` para estado no reconocido.
