# `cache_summary()`

**Contrato**

```cpp
std::string cache_summary(
    const CollectedValue<std::vector<CacheInfo>> &caches);
```

`caches` contiene cachés y disponibilidad. Devuelve estado, fuente y, si está disponible, nivel/tipo/tamaño/CPUs compartidas para el snapshot CSV.
