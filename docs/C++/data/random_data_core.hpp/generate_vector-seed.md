# `generate_vector()` — overload con seed

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Genera un vector aplicando una distribución invocable con `std::mt19937_64&`; crea un motor local inicializado con el seed indicado.

## Contrato

```cpp
template <typename T, typename Distribution>
std::vector<T> generate_vector(std::size_t count, Distribution distribution,
                               std::uint64_t seed,
                               const GenerationOptions &options = {});
```

| Parámetro | Significado |
| --- | --- |
| `count` | Número de valores solicitados; cero devuelve un vector vacío. |
| `distribution` | Sampler invocable con `std::mt19937_64&`, cuyo resultado se puede convertir a `T`. |
| `seed` | Semilla reproducible para inicializar el motor local. |
| `options` | Política de duplicados, patrón, intentos máximos y parámetros de patrón; por defecto usa `GenerationOptions{}`. |

Devuelve `std::vector<T>`. `T` debe ser aritmético y distinto de `bool`; resultados no finitos generan `std::invalid_argument`. Errores de patrón y agotamiento de intentos pueden lanzar `std::invalid_argument` o `std::runtime_error`.
