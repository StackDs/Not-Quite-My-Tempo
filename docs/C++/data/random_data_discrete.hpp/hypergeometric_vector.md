# `hypergeometric_vector()`

**Contrato**

```cpp
std::vector<std::size_t> hypergeometric_vector(
    std::size_t count, std::size_t population_size,
    std::size_t success_count, std::size_t draws, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Hay overload con engine. `count` es cantidad; `population_size` total de elementos; `success_count` éxitos en población; `draws` extracciones sin reemplazo; ambos conteos no exceden población. Seed/engine controla secuencia y options el patrón/salida. Devuelve éxitos; límites inválidos lanzan `std::invalid_argument`.
