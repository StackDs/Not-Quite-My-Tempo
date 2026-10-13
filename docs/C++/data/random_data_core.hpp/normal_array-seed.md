# `normal_array()` — overload con seed

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Contrato

```cpp
template <typename T, std::size_t N>
std::array<T, N> normal_array(T mean, T standard_deviation,
                              std::uint64_t seed,
                              const GenerationOptions &options = {});
```

Genera un arreglo fijo de `N` muestras normales. `mean` debe ser finita; `standard_deviation` positiva y finita; `seed` determina la secuencia; `options` controla duplicados/patrón. Solo admite tipos flotantes y devuelve `std::array<T,N>`; los parámetros inválidos lanzan `std::invalid_argument`.
