# `lognormal_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> lognormal_vector(
    std::size_t count, double log_mean, double log_stddev,
    std::uint64_t seed, const GenerationOptions &options = {});
```

Hay overload con engine. `count` es tamaño; `log_mean` finita; `log_stddev` positiva finita; seed/engine controla aleatoriedad; options configura salida. Solo tipo flotante. Devuelve valores positivos; entradas fuera de dominio lanzan `std::invalid_argument`.
