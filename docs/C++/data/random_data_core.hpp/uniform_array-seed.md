# `uniform_array()` — overload con seed

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Contrato

```cpp
template <typename T, std::size_t N>
std::array<T, N> uniform_array(T minimum, T maximum, std::uint64_t seed,
                               const GenerationOptions &options = {});
```

Genera `N` valores uniformes reproducibles. `minimum` y `maximum` definen límites ordenados (enteros inclusivos, flotantes semiabiertos); `seed` inicializa el motor; `options` define duplicados y patrón. `N` se fija en compilación. Retorna `std::array<T,N>`; límites o rango insuficiente con unicidad lanzan `std::invalid_argument`.
