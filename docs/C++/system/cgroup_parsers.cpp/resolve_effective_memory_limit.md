# `resolve_effective_memory_limit()`

**Contrato**

```cpp
CollectedValue<std::uint64_t> resolve_effective_memory_limit(
    const std::vector<CollectedValue<std::uint64_t>> &ancestors);
```

Elige el menor límite disponible de memoria entre ancestros. Si no hay límite finito pero se observó información ilimitada, devuelve `unlimited`; estados desconocidos se propagan y ausencia total produce `unavailable`.
