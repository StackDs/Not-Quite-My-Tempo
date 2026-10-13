# `weibull_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> weibull_array(
    double shape, double scale, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload engine utiliza motor mutable compartido. `N` fija tamaño; shape/scale positivos finitos; options controla patrón. Solo `T` flotante; devuelve array y lanza `std::invalid_argument` por parámetros inválidos.
