# `geometric_vector()`

**Contrato**

```cpp
std::vector<std::size_t> geometric_vector(
    std::size_t count, double probability, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Existe overload con engine. `count` es cantidad; `probability` es finita y pertenece a `(0,1]`; seed/engine controla secuencia; `options` aplica políticas comunes. Devuelve intentos desde 1. Conteos fuera de `size_t` lanzan `std::overflow_error`; parámetros inválidos, `std::invalid_argument`.
