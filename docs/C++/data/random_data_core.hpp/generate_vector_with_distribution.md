# `detail::generate_vector_with_distribution()`

**Contrato**

```cpp
template <typename T, typename Distribution>
std::vector<T> generate_vector_with_distribution(
    std::size_t count, Distribution distribution, std::mt19937_64 &engine,
    const GenerationOptions &options);
```

Adaptador interno que delega muestreo a `draw_values<T>()`. `count`, sampler, motor y opciones conservan las mismas restricciones. Retorna vector generado; propaga sus excepciones.
