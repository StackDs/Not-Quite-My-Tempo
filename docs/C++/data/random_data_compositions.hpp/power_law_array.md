# `power_law_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> power_law_array(
    double exponent, T xmin, T xmax, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload engine comparte motor. `N` fija tamaño; exponent positivo finito; límites `0 < xmin < xmax` finitos; options controla salida. `T` flotante. Retorna array y lanza `std::invalid_argument` ante dominio inválido.
