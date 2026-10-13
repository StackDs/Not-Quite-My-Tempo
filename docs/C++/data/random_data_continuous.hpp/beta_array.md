# `beta_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> beta_array(
    double alpha, double beta, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Puede recibir engine en lugar de seed. `N` es longitud; alpha/beta positivos finitos; options define duplicados/patrón. `T` debe ser flotante; salida en `(0,1)`; errores de dominio lanzan `std::invalid_argument`.
