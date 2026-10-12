# Análisis empírico de complejidad

API: `include/C++/analysis/complexity.hpp`  
Implementación: `src/C++/analysis/complexity.cpp`  
Namespace: `ntq::analysis`

`analyze_complexity()` ajusta mediciones existentes a modelos de crecimiento.
No ejecuta el algoritmo, no recolecta mediciones y no depende de los tipos
internos de `ntq::statistics`.

## Observaciones y uso

Cada punto contiene el tamaño de entrada y un tiempo en una unidad común:

```cpp
#include "analysis/complexity.hpp"

std::vector<ntq::analysis::ComplexityObservation> observations{
    {100, 0.0012L}, {200, 0.0025L}, {400, 0.0051L}, {800, 0.0103L}};
const std::vector<ntq::analysis::ComplexityModel> models{
    ntq::analysis::ComplexityModel::linear,
    ntq::analysis::ComplexityModel::linearithmic,
    ntq::analysis::ComplexityModel::quadratic};
const auto fits = ntq::analysis::analyze_complexity(observations, models);
```

Para analizar un `StatisticsReport`, el caller asocia cada `group_key` opaca con
el tamaño de entrada correspondiente y copia, por ejemplo, la mediana de
`MetricId::wall_clock` en una `ComplexityObservation`. La asociación queda en el
caller porque `statistics` no interpreta el significado de `group_key`.

## Modelos y cálculo

Los modelos disponibles son:

| Modelo | Base `f(N)` |
| --- | --- |
| `constant` | 1 |
| `logarithmic` | ln(N) |
| `square_root` | √N |
| `linear` | N |
| `linearithmic` | N ln(N) |
| `n_log_log_n` | N ln(ln(N)), para N ≥ 3 |
| `quadratic` | N² |
| `quadratic_logarithmic` | N² ln(N) |
| `cubic` | N³ |
| `exponential_2` | 2ᴺ |
| `factorial` | N! |

Para cada modelo se ajusta una constante multiplicativa sin intercepto,
`elapsed_time ≈ c * f(N)`, mediante mínimos cuadrados:

`c = Σ(f(N) * elapsed_time) / Σ(f(N)²)`

Se reporta el R² centrado: `1 - SSE/SST`. Un R² alto describe el ajuste a los
puntos suministrados; no prueba una cota asintótica ni selecciona
automáticamente un algoritmo ganador. Los tiempos de todas las observaciones
deben estar en la misma unidad.

## Estados y validación

- `AnalysisStatus::available`: constante y R² calculados.
- `AnalysisStatus::insufficient_samples`: menos de dos observaciones.
- `AnalysisStatus::invalid_input`: tamaño cero, tiempo no positivo/no finito,
  base singular o cálculo intermedio no finito. El estado es independiente por
  modelo.
- `AnalysisStatus::undefined_r_squared`: la constante pudo ajustarse, pero la
  varianza de los tiempos es cero; el R² no está definido. La constante sigue
  disponible, mientras `r_squared` queda en su valor inicial.

Los resultados conservan el orden de los modelos solicitados y el número de
observaciones. Las estructuras de entrada originales no se modifican.

Los modelos factorial y exponencial crecen muy rápido. Si su base o un cálculo
intermedio no cabe en `long double`, ese ajuste devuelve
`AnalysisStatus::invalid_input`; esto puede suceder incluso con tamaños de
entrada relativamente pequeños para `factorial`. `n_log_log_n` requiere tamaños
enteros de al menos 3, donde ln(ln(N)) es positiva.

Prueba manual desde la raíz del repositorio:

```sh
g++ -std=c++14 -Wall -Wextra -Wpedantic -Werror -Iinclude/C++ \
  tests/C++/analysis/complexity_tests.cpp src/C++/analysis/complexity.cpp \
  -o /tmp/complexity_tests
/tmp/complexity_tests
```
