# `write_benchmark_csv()`

**Implementación:** `src/C++/output/benchmark_csv.cpp` —
**Declaración:** `include/C++/output/benchmark_csv.hpp`
**Namespace:** `ntq::output`

## Propósito

Escribe el CSV detallado de iteraciones y el CSV de resúmenes estadísticos del resultado de un benchmark.

## Contrato

```cpp
void write_benchmark_csv(const benchmark::BenchmarkResult &result,
                         const benchmark::BenchmarkConfig &config);
```

| Parámetro | Significado |
| --- | --- |
| `result` | Resultado con muestras, estadísticas y rutas asociadas. Las colecciones deben estar alineadas y ser consistentes. |
| `config` | Provee rutas no vacías y distintas para iteraciones y estadísticas. Los directorios deben existir. |

## Efectos, errores y formato

Crea o trunca ambos archivos. El CSV de iteraciones conserva también warm-ups; el de estadísticas guarda filas `before_iqr` y `after_iqr`. Los textos se escapan para CSV y los saltos de línea se representan de forma que una muestra ocupe una línea física. Lanza `std::invalid_argument` para rutas inválidas o resultados inconsistentes y `std::runtime_error` si falla la apertura/escritura.
