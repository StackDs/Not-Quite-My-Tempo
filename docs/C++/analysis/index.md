# Análisis

El módulo `ntq::analysis` ajusta observaciones de tiempo existentes a modelos de crecimiento. No ejecuta algoritmos ni recolecta mediciones.

## `complexity.cpp`

- [`model_basis`](complexity.cpp/model_basis.md) — evalúa la base de un modelo para un tamaño.
- [`invalid_fit`](complexity.cpp/invalid_fit.md) — construye un resultado con entrada inválida.
- [`fit_model`](complexity.cpp/fit_model.md) — ajusta un modelo a observaciones.
- [`analyze_complexity`](complexity.cpp/analyze_complexity.md) — interfaz pública para ajustar varios modelos.

Tipos públicos: `ComplexityModel`, `AnalysisStatus`, `ComplexityObservation` y `ComplexityFit`, declarados en `include/C++/analysis/complexity.hpp`.
