# `read_memory_v2()`

**Contrato**

```cpp
std::vector<CollectedValue<std::uint64_t>> read_memory_v2(
    const std::vector<std::string> &paths, const std::string &mount_point);
```

Lee `memory.max` de ancestros; el mount root representa unlimited. Cada resultado lleva valor/status/fuente.
