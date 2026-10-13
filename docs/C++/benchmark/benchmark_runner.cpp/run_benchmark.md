# `run_benchmark()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` —
**Declaración:** `include/C++/benchmark/benchmark_runner.hpp`
**Namespace:** `ntq::benchmark`

## Propósito

Ejecuta repetidamente una función configurada por el caller, mide cada ejecución, reúne estadísticas de un grupo y escribe los CSV de iteraciones y resúmenes.

## Contrato

```cpp
BenchmarkResult run_benchmark(const BenchmarkCallbacks &callbacks,
                              const BenchmarkConfig &config);
```

| Parámetro | Tipo | Contrato |
| --- | --- | --- |
| `callbacks` | `const BenchmarkCallbacks&` | `function` es obligatoria y debe ser invocable sin argumentos como `void()`. `prepare`, si existe, se invoca antes de cada medición con índice, tamaño, seed derivado y marca de warm-up. |
| `config` | `const BenchmarkConfig&` | Define iteraciones totales, warm-ups, tamaño, seed, claves, unidad, modo, opciones estadísticas, metadatos y rutas CSV distintas/no vacías. Los directorios de salida deben existir. |

## Funcionamiento

Valida la configuración, obtiene metadatos y snapshots del sistema, deriva un seed por iteración, llama a `prepare` fuera del temporizador y mide `function`. Calcula estadísticas y escribe ambos CSV al terminar correctamente.

## Retorno y errores

Devuelve `BenchmarkResult` con tamaño, semillas, muestras, reporte y rutas. Configuración inválida lanza `std::invalid_argument`; errores del callback, medición, estadística o escritura se propagan. Los CSV se escriben solo al final del flujo satisfactorio.

## Límites

Una llamada representa una función/variante y un grupo. Comparaciones entre llamadas y complejidad entre tamaños se calculan después. El modo aislado usa `fork()` en Linux y requiere caller monohilo; cambios de memoria del hijo no vuelven al padre. El runner observa el entorno, pero no fija afinidad ni frecuencia.

## Ejemplo

```cpp
ntq::benchmark::BenchmarkCallbacks callbacks;
callbacks.prepare = [&](const ntq::benchmark::BenchmarkIterationContext &) {
  input = make_input();
};
callbacks.function = [&] { algorithm(input); };
ntq::benchmark::BenchmarkConfig config;
config.total_iterations = 100;
config.iterations_csv_path = "out/iterations.csv";
config.statistics_csv_path = "out/statistics.csv";
auto result = ntq::benchmark::run_benchmark(callbacks, config);
```

Haz observable el resultado dentro del callback cuando sea necesario para evitar que el optimizador elimine el trabajo.
