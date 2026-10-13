# `read_cpu_quota_v2()`

**Contrato**

```cpp
std::vector<CollectedValue<CpuQuota>> read_cpu_quota_v2(
    const std::vector<std::string> &paths, const std::string &mount_point);
```

Lee `cpu.max` para cada ancestor; el mount root se considera ilimitado. Retorna cuota/periodo con estado por cada ruta.
