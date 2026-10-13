# `generate_array()` — overload con motor

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Genera un arreglo fijo usando un motor mutable compartido.

## Contrato

```cpp
template <typename T, std::size_t N, typename Distribution>
std::array<T, N> generate_array(Distribution distribution,
                                std::mt19937_64 &engine,
                                const GenerationOptions &options = {});
```

| Parámetro | Significado |
| --- | --- |
| `distribution` | Sampler invocable con el motor y convertible a `T`. |
| `engine` | Motor mutable que avanza durante la generación. |
| `options` | Opciones de generación; por defecto `GenerationOptions{}`. |
| `N` | Longitud del arreglo en tiempo de compilación. |

Retorna `std::array<T,N>` y propaga los errores de validación o muestreo de `generate_vector()`.
