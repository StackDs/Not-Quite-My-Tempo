# `memory_status_name()`

**Contrato**

```cpp
std::string memory_status_name(MemoryMetricStatus status);
```

`status` representa la disponibilidad de una métrica de memoria. Devuelve una etiqueta estable para CSV, preservando estados como no soportado, denegado o no medido.
