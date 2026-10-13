# Benchmark

El runner ejecuta y mide un callback configurado por quien lo llama. Cada ejecución representa una función y grupo; comparaciones y análisis entre tamaños se realizan con las APIs de estadísticas y análisis.

## `benchmark_runner.cpp`

- [`valid_time_unit`](benchmark_runner.cpp/valid_time_unit.md)
- [`validate_config`](benchmark_runner.cpp/validate_config.md)
- [`resolved_group_key`](benchmark_runner.cpp/resolved_group_key.md)
- [`resolved_variant_key`](benchmark_runner.cpp/resolved_variant_key.md)
- [`make_run_id`](benchmark_runner.cpp/make_run_id.md)
- [`measure_callback`](benchmark_runner.cpp/measure_callback.md)
- [`run_benchmark`](benchmark_runner.cpp/run_benchmark.md)

Tipos principales: `BenchmarkCallbacks`, `BenchmarkIterationContext`, `BenchmarkConfig` y `BenchmarkResult`.
