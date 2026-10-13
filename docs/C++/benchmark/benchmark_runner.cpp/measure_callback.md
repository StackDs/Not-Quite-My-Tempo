# `measure_callback()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` (función interna).

## Propósito

Ejecuta el callback medido usando la estrategia seleccionada en la configuración.

## Contrato

```cpp
TimeMeasurement measure_callback(const BenchmarkCallbacks &callbacks,
                                 const BenchmarkConfig &config);
```

| Parámetro | Significado |
| --- | --- |
| `callbacks` | Proporciona la función que se cronometra. |
| `config` | Elige ejecución en proceso o aislada y fija el umbral de cambios involuntarios. |

## Retorno y errores

Devuelve una medición detenida y calculada. En modo aislado delega a `measure_in_isolated_process`; en modo normal llama `start`, el callback, `stop` y `elapsed`. Los errores del callback o de los recolectores se propagan.
