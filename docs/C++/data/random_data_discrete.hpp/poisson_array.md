# `poisson_array()`

**Contrato**

```cpp
template <std::size_t N>
std::array<std::size_t, N> poisson_array(
    double lambda, std::uint64_t seed,
    const GenerationOptions &options = {});
```

El overload con engine usa un motor compartido. `N` fija longitud; `lambda` debe ser finita y no negativa; seed/engine define secuencia; `options` configura salida. Retorna conteos; `lambda == 0` produce ceros y valor inválido lanza `std::invalid_argument`.
