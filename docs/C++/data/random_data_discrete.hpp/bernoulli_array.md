# `bernoulli_array()`

**Header:** `include/C++/data/random_data_discrete.hpp`.

## Contrato

```cpp
template <std::size_t N>
std::array<std::size_t, N> bernoulli_array(
    double probability, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload con motor sustituye `seed` por `std::mt19937_64&`. `N` fija la longitud; `probability` debe estar en `[0,1]`; `options` controla duplicados/patrón. Devuelve un array de 0/1; probabilidad inválida lanza `std::invalid_argument`.
