# `weibull_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> weibull_vector(
    std::size_t count, double shape, double scale, std::uint64_t seed,
    const GenerationOptions &options = {});
```

También ofrece engine. `count` es número de muestras; shape y scale positivos finitos; seed/engine controla muestreo; options controla salida. `T` flotante. Retorna vector Weibull no negativo; inválidos lanzan `std::invalid_argument`.
