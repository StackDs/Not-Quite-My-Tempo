# `bernoulli_vector()`

**Header:** `include/C++/data/random_data_discrete.hpp`.

## Contrato

```cpp
std::vector<std::size_t> bernoulli_vector(
    std::size_t count, double probability, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Hay overload equivalente que recibe `std::mt19937_64&` en lugar de seed. `count` es la cantidad de resultados; `probability` es la probabilidad finita `[0,1]`; `seed` reproduce la secuencia o `engine` se consume; `options` configura duplicados/patrón. Retorna valores 0/1 como `size_t`; probabilidad inválida lanza `std::invalid_argument`.
