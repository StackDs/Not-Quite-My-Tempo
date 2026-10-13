# `geometric_array()`

**Contrato**

```cpp
template <std::size_t N>
std::array<std::size_t, N> geometric_array(
    double probability, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload alternativo consume un engine. `N` define longitud; `probability` debe ser finita en `(0,1]`; seed/engine fija la aleatoriedad; `options` aplica política de salida. Devuelve intentos desde 1. Puede lanzar `std::overflow_error` o `std::invalid_argument` por los motivos documentados en el vectorial.
