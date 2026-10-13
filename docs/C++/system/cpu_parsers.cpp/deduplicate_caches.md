# `deduplicate_caches()`

**Contrato**

```cpp
std::vector<CacheInfo> deduplicate_caches(
    const std::vector<CacheInfo> &caches);
```

Devuelve la primera aparición de cada caché estructuralmente igual según `same_cache()`. Entrada sin modificar.
