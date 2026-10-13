# `uniform_array()` — overload con motor

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Contrato

```cpp
template <typename T, std::size_t N>
std::array<T, N> uniform_array(T minimum, T maximum,
                               std::mt19937_64 &engine,
                               const GenerationOptions &options = {});
```

Genera `N` valores uniformes y avanza `engine`. `minimum` y `maximum` son límites ordenados; `options` configura duplicados/patrón. Devuelve un arreglo fijo. `T` debe ser numérico no-bool; los extremos no finitos (para flotantes), invertidos o rango entero insuficiente con unicidad producen `std::invalid_argument`.
