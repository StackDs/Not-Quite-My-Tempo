# `uniform_vector()` — overload con seed

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Genera `count` valores uniformes dentro de límites dados, usando un motor local reproducible.

## Contrato

```cpp
template <typename T>
std::vector<T> uniform_vector(std::size_t count, T minimum, T maximum,
                              std::uint64_t seed,
                              const GenerationOptions &options = {});
```

| Parámetro | Significado |
| --- | --- |
| `count` | Número de valores. |
| `minimum`, `maximum` | Extremos ordenados; enteros usan intervalo inclusivo y flotantes intervalo semiabierto. |
| `seed` | Semilla del motor local. |
| `options` | Controla duplicados y patrón. `forbid` requiere que el rango entero tenga suficientes valores. |

Retorna `std::vector<T>`. `T` debe ser numérico distinto de `bool`; límites inválidos lanzan `std::invalid_argument`.
