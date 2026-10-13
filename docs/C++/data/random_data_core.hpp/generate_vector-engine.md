# `generate_vector()` — overload con motor

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Genera un vector con una distribución y consume el motor aleatorio proporcionado, permitiendo continuar una secuencia compartida.

## Contrato

```cpp
template <typename T, typename Distribution>
std::vector<T> generate_vector(std::size_t count, Distribution distribution,
                               std::mt19937_64 &engine,
                               const GenerationOptions &options = {});
```

| Parámetro | Significado |
| --- | --- |
| `count` | Número de valores solicitados. |
| `distribution` | Sampler invocable con `std::mt19937_64&`, convertible a `T`. |
| `engine` | Motor mutable que se avanza durante el muestreo. |
| `options` | Reglas de duplicados y patrón; por defecto `GenerationOptions{}`. |

Retorna `std::vector<T>`. `T` debe ser numérico no-bool. Resultados no finitos u opciones incompatibles lanzan `std::invalid_argument`; no poder producir suficientes valores únicos lanza `std::runtime_error`.
