# `collect_caches()`

**Contrato**

```cpp
CollectedValue<std::vector<CacheInfo>> collect_caches(
    const std::vector<unsigned> &cpus, CollectionStatus cpu_status);
```

Busca índices de caché sysfs por CPU y recolecta nivel, tipo, bytes y CPUs compartidas. `cpus` son CPUs online; `cpu_status` representa disponibilidad de esa lista. Devuelve cachés deduplicadas con estado/fuente parcial.
