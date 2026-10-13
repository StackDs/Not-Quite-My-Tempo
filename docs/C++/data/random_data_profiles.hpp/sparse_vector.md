# `sparse_vector()`

**Contrato**

```cpp
template <typename T, typename Distribution>
std::vector<T> sparse_vector(std::size_t count, double zero_probability,
                             Distribution nonzero_distribution,
                             std::uint64_t seed,
                             const GenerationOptions &options = {});
```

Genera vector disperso con masa en cero. `count` fija longitud; `zero_probability` debe estar en `[0,1]`; `nonzero_distribution` debe ser invocable con el motor y producir valores convertibles a `T`; `seed` controla secuencia; `options` define duplicados/patrón. Existe overload con engine. `T` numérico no-bool; devuelve `std::vector<T>`.
