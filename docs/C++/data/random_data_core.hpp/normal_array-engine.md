# `normal_array()` — overload con motor

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Contrato

```cpp
template <typename T, std::size_t N>
std::array<T, N> normal_array(T mean, T standard_deviation,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {});
```

Genera `N` muestras normales usando y avanzando `engine`. `mean` debe ser finita; `standard_deviation`, positiva y finita; `options` configura duplicados/patrón. `T` debe ser flotante. Devuelve `std::array<T,N>` y rechaza parámetros inválidos con `std::invalid_argument`.
