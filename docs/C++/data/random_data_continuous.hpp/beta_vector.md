# `beta_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> beta_vector(
    std::size_t count, double alpha, double beta, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Overload alternativo usa engine. `count` es tamaño; `alpha` y `beta` son positivos finitos; seed/engine define secuencia; options aplica reglas comunes. `T` flotante; devuelve valores en `(0,1)`. Parámetros inválidos lanzan `std::invalid_argument`.
