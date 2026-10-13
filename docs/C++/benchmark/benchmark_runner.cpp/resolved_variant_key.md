# `resolved_variant_key()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` (función interna).

## Propósito

Obtiene la clave efectiva de la variante para las muestras.

## Contrato

```cpp
std::string resolved_variant_key(const BenchmarkConfig &config);
```

| Parámetro | Significado |
| --- | --- |
| `config` | Si `variant_key` no está vacía, se devuelve; de lo contrario se usa `function`. |

## Retorno

Devuelve la clave por valor. No modifica la configuración.
