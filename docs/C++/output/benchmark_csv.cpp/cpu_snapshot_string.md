# `cpu_snapshot_string()`

**Contrato**

```cpp
std::string cpu_snapshot_string(const SystemInfo &system);
```

`system` es el snapshot del entorno. Devuelve arquitectura, fabricante/modelo, CPUs online, cachés, afinidad, boost y frecuencias en texto compacto para metadatos CSV.
