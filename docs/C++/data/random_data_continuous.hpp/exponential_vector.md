# `exponential_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> exponential_vector(
    std::size_t count, double rate, std::uint64_t seed,
    const GenerationOptions &options = {});
```

También acepta engine en lugar de seed. `count` es la cantidad; `rate` debe ser positivo y finito; seed/engine controla aleatoriedad; `options` define duplicados/patrón. `T` debe ser flotante. Devuelve valores exponenciales positivos; parámetros inválidos lanzan `std::invalid_argument`.
