# Tipos del runner

**Declaración:** `include/C++/benchmark/benchmark_config.hpp`, `benchmark_result.hpp`.

- `BenchmarkCallbacks`: `prepare(context)` opcional, fuera del timer, y `function()` obligatoria, medida como `void()`.
- `BenchmarkIterationContext`: índice, `input_size`, seed derivado y flag de warm-up entregados a `prepare`.
- `ExecutionMode`: `in_process` o `isolated_process` (Linux/fork, launcher monohilo).
- `BenchmarkConfig`: cantidad total de iteraciones, tamaño, seed, unidad, umbral de switches, claves, rutas CSV, opciones estadísticas y metadatos. Parámetros del algoritmo quedan en las capturas del caller.
- `BenchmarkResult`: tamaño, rutas, seeds por iteración, muestras y reporte estadístico.

Las rutas deben ser distintas, no vacías y apuntar a directorios existentes. `total_iterations` debe superar los warm-ups.
