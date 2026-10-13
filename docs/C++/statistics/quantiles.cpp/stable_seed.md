# `stable_seed()`

**Contrato**

```cpp
std::uint64_t stable_seed(std::uint64_t seed, const std::string &key,
                          MetricId metric);
```

Deriva una semilla reproducible de seed maestro, clave de grupo y métrica mediante FNV-1a. Devuelve un seed de 64 bits para que los bootstrap por grupo/métrica sean estables.
