# `binomial_array()`

**Contrato**

```cpp
template <std::size_t N>
std::array<std::size_t, N> binomial_array(
    std::size_t trials, double probability, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload engine consume `std::mt19937_64&`. `N` fija la cantidad de muestras; `trials` debe caber en `long long`; `probability` en `[0,1]`; seed/engine elige secuencia; `options` afecta duplicados/patrón. Devuelve conteos; parámetros inválidos lanzan `std::invalid_argument`.
