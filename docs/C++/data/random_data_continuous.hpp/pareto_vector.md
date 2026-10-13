# `pareto_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> pareto_vector(
    std::size_t count, double xmin, double alpha, std::uint64_t seed,
    const GenerationOptions &options = {});
```

También acepta engine. `count` es longitud; `xmin` y `alpha` deben ser positivos finitos; seed/engine controla generación; options configura salida. `T` flotante. Retorna cola Pareto desde xmin; parámetros inválidos lanzan `std::invalid_argument`.
