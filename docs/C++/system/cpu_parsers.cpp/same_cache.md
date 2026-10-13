# `same_cache()`

**Contrato**

```cpp
bool same_cache(const CacheInfo &left, const CacheInfo &right);
```

Compara nivel, tipo, tamaño en bytes y CPUs compartidas. Devuelve igualdad estructural de esos campos; no compara identidad externa.
