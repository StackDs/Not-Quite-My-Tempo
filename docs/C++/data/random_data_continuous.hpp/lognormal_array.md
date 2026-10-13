# `lognormal_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> lognormal_array(
    double log_mean, double log_stddev, std::uint64_t seed,
    const GenerationOptions &options = {});
```

También acepta engine. `N` es longitud fija; media logarítmica finita y desviación positiva finita; seed/engine determina la secuencia; options gobierna patrón/duplicados. Retorna array flotante positivo; argumentos inválidos lanzan `std::invalid_argument`.
