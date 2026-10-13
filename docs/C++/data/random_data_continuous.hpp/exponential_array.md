# `exponential_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> exponential_array(
    double rate, std::uint64_t seed, const GenerationOptions &options = {});
```

Overload alternativo recibe engine. `N` es longitud fija; `rate` es positivo finito; seed/engine controla muestreo; `options` configura duplicados/patrón. Retorna array de tipo flotante; `rate` inválido lanza `std::invalid_argument`.
