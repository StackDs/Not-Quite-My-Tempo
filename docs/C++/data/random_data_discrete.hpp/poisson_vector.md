# `poisson_vector()`

**Contrato**

```cpp
std::vector<std::size_t> poisson_vector(
    std::size_t count, double lambda, std::uint64_t seed,
    const GenerationOptions &options = {});
```

Tiene overload con motor. `count` es cantidad; `lambda` es media finita no negativa (cero genera ceros); seed o engine determina la secuencia; `options` configura duplicados/patrón. Devuelve conteos como `size_t`; `lambda` inválido lanza `std::invalid_argument`.
