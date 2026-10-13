# `resolve_effective_cpu_set()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> resolve_effective_cpu_set(
    const std::vector<CollectedValue<std::vector<unsigned>>> &ancestors);
```

Calcula la intersección de cpusets disponibles en los ancestros. Vacíos no limitan; estados desconocidos se propagan; sin valores disponibles retorna `unavailable`. Las listas deben estar ordenadas para `set_intersection`.
