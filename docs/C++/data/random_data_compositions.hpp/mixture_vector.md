# `mixture_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> mixture_vector(
    std::size_t count, const std::vector<MixtureComponent<T>> &components,
    std::uint64_t seed, const GenerationOptions &options = {});
```

También acepta engine. `count` es número de extracciones; `components` no vacío, con samplers válidos y pesos finitos/no negativos de suma positiva; seed/engine controla secuencia; options define salida. Retorna vector de mezcla; invalidación lanza `std::invalid_argument`.
