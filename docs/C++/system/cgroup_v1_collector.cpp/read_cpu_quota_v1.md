# `read_cpu_quota_v1()`

**Contrato**

```cpp
std::vector<CollectedValue<CpuQuota>> read_cpu_quota_v1(
    const std::vector<std::string> &paths, const std::string &mount_point);
```

Lee cuota/periodo por cada ancestor path v1, interpreta raíz ausente como ilimitada y parsea valores disponibles. `mount_point` identifica raíz; devuelve una observación por ruta en orden.
