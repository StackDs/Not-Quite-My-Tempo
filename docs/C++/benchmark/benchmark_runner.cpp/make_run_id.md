# `make_run_id()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` (función interna).

## Propósito

Selecciona el identificador de corrida que se asignará a los metadatos y a los IDs de muestra.

## Contrato

```cpp
std::string make_run_id(const BenchmarkConfig &config);
```

| Parámetro | Significado |
| --- | --- |
| `config` | Usa `metadata.run_id` si no está vacío; si no, combina el seed con un contador temporal monotónico. |

## Retorno

Devuelve el ID como texto. La función no altera la configuración.
