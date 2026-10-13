# `pareto_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> pareto_array(
    double xmin, double alpha, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Overload equivalente recibe engine. `N` define cantidad; `xmin` y `alpha` positivos finitos; seed/engine gobierna secuencia; options define salida. Retorna array flotante y rechaza parámetros inválidos con `std::invalid_argument`.
