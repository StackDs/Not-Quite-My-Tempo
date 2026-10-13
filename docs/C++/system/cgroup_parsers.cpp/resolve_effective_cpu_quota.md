# `resolve_effective_cpu_quota()`

**Contrato**

```cpp
CollectedValue<CpuQuota> resolve_effective_cpu_quota(
    const std::vector<CollectedValue<CpuQuota>> &ancestors);
```

Combina cuotas de ancestros y elige la cuota efectiva más restrictiva (menor `quota/period`). Si algún estado es desconocido, conserva ese estado; si todos son ilimitados devuelve `unlimited`. `ancestors` es solo lectura.
