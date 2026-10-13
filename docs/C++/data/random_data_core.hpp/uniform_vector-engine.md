# `uniform_vector()` — overload con motor

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Genera valores uniformes consumiendo el motor mutable entregado.

## Contrato

```cpp
template <typename T>
std::vector<T> uniform_vector(std::size_t count, T minimum, T maximum,
                              std::mt19937_64 &engine,
                              const GenerationOptions &options = {});
```

| Parámetro | Significado |
| --- | --- |
| `count` | Cantidad de valores. |
| `minimum`, `maximum` | Límites ordenados; enteros inclusivos y flotantes semiabiertos. |
| `engine` | Motor mutable compartido. |
| `options` | Política de duplicados y patrón. |

Devuelve un vector. Rechaza tipo bool/no numérico y límites inválidos; si se prohíben duplicados, el rango entero debe poder contener la solicitud.
