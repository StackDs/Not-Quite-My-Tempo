# `clustered_vector()`

**Contrato**

```cpp
template <typename T>
std::vector<T> clustered_vector(
    std::size_t count, const std::vector<T> &centers,
    const std::vector<T> &standard_deviations,
    const std::vector<double> &weights, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Genera `count` observaciones desde una mezcla de distribuciones normales. `centers`, `standard_deviations` y `weights` deben tener igual longitud no vacía; centros finitos, desviaciones positivas finitas y pesos válidos. `seed` controla la secuencia y `options` controla duplicados/patrón. Solo acepta `T` flotante; devuelve vector. Existe overload equivalente con `std::mt19937_64&` en lugar de `seed`.
