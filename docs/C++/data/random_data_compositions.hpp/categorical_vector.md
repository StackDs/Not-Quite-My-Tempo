# `categorical_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> categorical_vector(
    std::size_t count, const std::vector<T> &categories,
    const std::vector<double> &weights, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Hay overload con engine. `count` es cantidad; `categories` y `weights` deben tener longitud igual no vacía; categorías finitas, pesos finitos/no negativos con suma positiva; seed/engine controla elección; options aplica patrón/duplicados. Retorna vector categórico; errores de dominio lanzan `std::invalid_argument`.
