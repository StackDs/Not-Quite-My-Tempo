# `read_memory_v1()`

**Contrato**

```cpp
std::vector<CollectedValue<std::uint64_t>> read_memory_v1(
    const std::vector<std::string> &paths, const std::string &mount_point);
```

Lee `memory.limit_in_bytes` por ancestor path; mount root ausente se clasifica unlimited. Devuelve estados/bytes, una entrada por `paths`.
