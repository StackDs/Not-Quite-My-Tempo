# `mixture_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> mixture_array(
    const std::vector<MixtureComponent<T>> &components,
    std::uint64_t seed, const GenerationOptions &options = {});
```

Overload alternativo recibe engine. `N` es cantidad fija; `components` requiere samplers válidos y pesos no negativos finitos con total positivo; seed/engine gobierna muestreo; options configura salida. Devuelve array; componentes inválidos lanzan `std::invalid_argument`.
