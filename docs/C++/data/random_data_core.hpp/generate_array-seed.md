# `generate_array()` — overload con seed

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Genera un arreglo fijo de `N` elementos mediante una distribución, con motor local inicializado por seed.

## Contrato

```cpp
template <typename T, std::size_t N, typename Distribution>
std::array<T, N> generate_array(Distribution distribution, std::uint64_t seed,
                                const GenerationOptions &options = {});
```

| Parámetro | Significado |
| --- | --- |
| `distribution` | Sampler invocable con el motor `std::mt19937_64&`; sus valores deben convertirse a `T`. |
| `seed` | Semilla reproducible del motor local. |
| `options` | Opciones de duplicados y patrón, aplicadas antes de convertir el vector a arreglo. |
| `N` | Cantidad fija de valores, definida en compilación. |

Devuelve `std::array<T,N>`. Aplica las mismas restricciones de tipo y errores de generación que `generate_vector()`.
