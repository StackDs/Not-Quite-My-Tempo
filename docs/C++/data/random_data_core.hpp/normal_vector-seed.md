# `normal_vector()` — overload con seed

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Contrato

```cpp
template <typename T>
std::vector<T> normal_vector(std::size_t count, T mean,
                             T standard_deviation, std::uint64_t seed,
                             const GenerationOptions &options = {});
```

Genera una muestra normal reproducible de tipo flotante. `count` es el tamaño; `mean` es la media finita; `standard_deviation` debe ser positiva y finita; `seed` inicializa el motor; `options` controla duplicados y orden/patrón. Retorna `std::vector<T>`. Tipo no flotante o parámetros inválidos se rechazan con `std::invalid_argument`.
