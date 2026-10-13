# `categorical_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> categorical_array(
    const std::vector<T> &categories, const std::vector<double> &weights,
    std::uint64_t seed, const GenerationOptions &options = {});
```

Overload alternativo recibe engine. `N` es longitud fija; categories/weights igual longitud no vacía, valores y pesos finitos, pesos no negativos con total positivo; seed/engine controla aleatoriedad; options configura patrón. Devuelve array; inválidos lanzan `std::invalid_argument`.
