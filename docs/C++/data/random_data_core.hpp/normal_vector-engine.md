# `normal_vector()` — overload con motor

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Contrato

```cpp
template <typename T>
std::vector<T> normal_vector(std::size_t count, T mean,
                             T standard_deviation, std::mt19937_64 &engine,
                             const GenerationOptions &options = {});
```

Genera `count` valores de una distribución normal y avanza `engine`. `mean` debe ser finita y `standard_deviation` positiva y finita; `options` controla duplicados y patrón. `T` debe ser de punto flotante. Devuelve un vector y lanza `std::invalid_argument` ante parámetros inválidos.
