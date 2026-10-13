# `hypergeometric_array()`

**Contrato**

```cpp
template <std::size_t N>
std::array<std::size_t, N> hypergeometric_array(
    std::size_t population_size, std::size_t success_count,
    std::size_t draws, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload engine consume motor compartido. `N` es número de resultados; `population_size`, `success_count` y `draws` describen población/muestra y `success_count, draws <= population_size`; seed/engine controla aleatoriedad; `options` define salida. Devuelve éxitos; dominio inválido lanza `std::invalid_argument`.
