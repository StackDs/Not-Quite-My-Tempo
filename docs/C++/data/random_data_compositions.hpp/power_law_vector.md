# `power_law_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> power_law_vector(
    std::size_t count, double exponent, T xmin, T xmax,
    std::uint64_t seed, const GenerationOptions &options = {});
```

Overload con engine disponible. `count` define longitud; `exponent` positivo finito; `xmin`, `xmax` finitos con `0 < xmin < xmax`; seed/engine fuente aleatoria; options reglas comunes. Solo `T` flotante. Devuelve muestras acotadas; argumentos inválidos lanzan `std::invalid_argument`.
