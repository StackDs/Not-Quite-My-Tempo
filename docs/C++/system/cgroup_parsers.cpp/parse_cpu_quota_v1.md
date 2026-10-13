# `parse_cpu_quota_v1()`

**Contrato**

```cpp
CollectedValue<CpuQuota> parse_cpu_quota_v1(
    const std::string &quota_text, const std::string &period_text,
    const std::string &source);
```

`quota_text` y `period_text` son los valores cgroup v1; `source` identifica su origen. Devuelve cuota/periodo disponible, `unlimited` para quota `-1`, o `malformed` si no puede interpretarlos/periodo es cero.
