# `gamma_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> gamma_array(
    double shape, double scale, std::uint64_t seed,
    const GenerationOptions &options = {});
```

También existe overload con engine. `N` fija longitud; `shape`/`scale` son positivos finitos; seed o engine controlan aleatoriedad; options configura salida. Devuelve array flotante y rechaza parámetros inválidos con `std::invalid_argument`.
