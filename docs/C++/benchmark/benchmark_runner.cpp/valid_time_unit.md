# `valid_time_unit()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` (función interna).

## Propósito

Comprueba que una unidad de tiempo sea uno de los valores soportados antes de iniciar un benchmark.

## Contrato

```cpp
bool valid_time_unit(TimeUnit unit);
```

| Parámetro | Significado |
| --- | --- |
| `unit` | Unidad por validar: nanosegundos, microsegundos, milisegundos, segundos, minutos u horas. |

## Retorno

Devuelve `true` para una unidad enumerada válida y `false` para cualquier valor fuera del enum. No modifica estado ni lanza excepciones.
