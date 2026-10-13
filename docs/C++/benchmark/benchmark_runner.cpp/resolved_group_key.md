# `resolved_group_key()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` (función interna).

## Propósito

Obtiene la clave de grupo efectiva usada para agrupar las muestras.

## Contrato

```cpp
std::string resolved_group_key(const BenchmarkConfig &config);
```

| Parámetro | Significado |
| --- | --- |
| `config` | Si `group_key` está definido, se devuelve tal cual; si está vacío, se genera `input_size=<N>` desde `input_size`. |

## Retorno

Devuelve una copia de la clave. No modifica la configuración.
