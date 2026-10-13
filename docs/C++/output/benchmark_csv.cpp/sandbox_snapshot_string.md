# `sandbox_snapshot_string()`

**Contrato**

```cpp
std::string sandbox_snapshot_string(const SystemInfo &system);
```

`system` contiene el snapshot de sandbox. Devuelve versión/ruta cgroup, cuota/periodo de CPU, CPUs permitidas y límite de memoria en representación compacta CSV.
