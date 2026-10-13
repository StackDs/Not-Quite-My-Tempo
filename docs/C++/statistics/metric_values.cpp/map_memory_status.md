# `map_memory_status()`

**Contrato**

```cpp
MetricValueStatus map_memory_status(MemoryMetricStatus status);
```

Traduce el estado específico de memoria al vocabulario estadístico. Estado desconocido se representa como `invalid_value`; no modifica la medición.
