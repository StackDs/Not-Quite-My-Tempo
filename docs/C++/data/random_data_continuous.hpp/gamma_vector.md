# `gamma_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> gamma_vector(
    std::size_t count, double shape, double scale, std::uint64_t seed,
    const GenerationOptions &options = {});
```

También acepta engine. `count` es longitud; `shape` y `scale` son positivos finitos; seed/engine controla secuencia; options aplica reglas comunes. `T` debe ser flotante. Devuelve muestras gamma; parámetros inválidos lanzan `std::invalid_argument`.
