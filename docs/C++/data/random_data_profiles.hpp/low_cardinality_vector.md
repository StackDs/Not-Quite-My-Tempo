# `low_cardinality_vector()`

**Contrato**

```cpp
template <typename T>
std::vector<T> low_cardinality_vector(
    std::size_t count, const std::vector<T> &palette, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Genera `count` elementos de una paleta pequeña. `palette` debe ser no vacía; `seed` hace reproducible el muestreo; `options` controla patrón/duplicados. Sin pesos usa selección uniforme. Existe overload con `weights` y overloads equivalentes que consumen `std::mt19937_64&`. Devuelve `std::vector<T>` y hereda validaciones de `categorical_vector()`.
