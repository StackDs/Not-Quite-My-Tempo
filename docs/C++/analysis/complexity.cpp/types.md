# Tipos de análisis de complejidad

**Declaración:** `include/C++/analysis/complexity.hpp`.

- `ComplexityModel`: modelos de crecimiento aceptados por [`analyze_complexity()`](analyze_complexity.md): constante, logarítmico, raíz, lineal, linealítmico, `N log log N`, cuadrático, cuadrático-logarítmico, cúbico, exponencial base 2 y factorial.
- `AnalysisStatus`: `available`, `insufficient_samples`, `invalid_input` o `undefined_r_squared`.
- `ComplexityObservation`: entrada `{input_size, elapsed_time}`. Tiempo en unidad común, positivo y finito; tamaño distinto de cero.
- `ComplexityFit`: `{model, status, empirical_constant, r_squared, observation_count}`. Interpretar constante y R² conforme al estado.
