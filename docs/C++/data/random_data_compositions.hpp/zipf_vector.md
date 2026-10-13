# `zipf_vector()`

**Contrato**

```cpp
std::vector<std::size_t> zipf_vector(std::size_t count, double exponent,
                                     std::size_t max_rank,
                                     std::uint64_t seed,
                                     const GenerationOptions &options = {});
```

También acepta engine. `count` es longitud; `exponent` positivo finito; `max_rank` positivo y representable; seed/engine controla aleatoriedad; options define salida. Devuelve rangos `[1,max_rank]`; argumentos inválidos lanzan `std::invalid_argument`.
