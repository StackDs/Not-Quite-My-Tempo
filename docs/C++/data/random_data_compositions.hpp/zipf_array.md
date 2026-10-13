# `zipf_array()`

**Contrato**

```cpp
template <std::size_t N> std::array<std::size_t, N> zipf_array(
    double exponent, std::size_t max_rank, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Existe overload con engine. `N` es cantidad; `exponent` debe ser positivo finito y `max_rank` positivo/representable; seed/engine controla secuencia; options configura salida. Devuelve rangos Zipf; dominio inválido lanza `std::invalid_argument`.
