# `detail::draw_values()`

**Contrato**

```cpp
template <typename T, typename Distribution>
std::vector<T> draw_values(std::size_t count, Distribution distribution,
                           std::mt19937_64 &engine,
                           const GenerationOptions &options);
```

Muestrea, valida finitud, aplica política de duplicados y patrón. `distribution` debe poder invocarse con engine y convertir a `T`; `count` determina tamaño; `options` aplica los límites de generación; engine mutable se consume. Error de dominio lanza `std::invalid_argument`, agotamiento de unicidad `std::runtime_error`.
