# `analyze_complexity()`

**Ubicación:** `src/C++/analysis/complexity.cpp` —
**Declaración:** `include/C++/analysis/complexity.hpp`
**Namespace:** `ntq::analysis`

## Propósito

Ajusta observaciones ya medidas a varios modelos de crecimiento. No ejecuta el algoritmo ni toma mediciones; el caller asocia cada tamaño con un tiempo representativo, por ejemplo la mediana wall-clock del benchmark.

## Contrato

```cpp
std::vector<ComplexityFit> analyze_complexity(
    const std::vector<ComplexityObservation> &observations,
    const std::vector<ComplexityModel> &models);
```

### Parámetros

| Parámetro | Tipo | Contrato |
| --- | --- | --- |
| `observations` | `const std::vector<ComplexityObservation>&` | Puntos `(input_size, elapsed_time)`. El tamaño debe ser mayor que cero y el tiempo positivo y finito. Se necesitan dos o más observaciones y los tiempos deben compartir unidad. |
| `models` | `const std::vector<ComplexityModel>&` | Modelos a evaluar: `constant`, `logarithmic`, `square_root`, `linear`, `linearithmic`, `n_log_log_n`, `quadratic`, `quadratic_logarithmic`, `cubic`, `exponential_2` y `factorial`. |

### Retorno

Devuelve un `ComplexityFit` por cada modelo, en el orden solicitado. Cada ajuste contiene el modelo, estado, constante empírica, `R²` y cantidad de observaciones. El modelo es `elapsed_time ≈ c * f(N)`, sin intercepto.

## Estados y límites

`insufficient_samples` indica menos de dos observaciones; `invalid_input` señala tamaños/tiempos inválidos o cálculo no representable; `undefined_r_squared` indica varianza de tiempos cero; `available` indica que se obtuvieron constante y `R²`. `n_log_log_n` requiere tamaños de al menos 3. Factorial y exponencial pueden exceder `long double`. Los resultados describen el ajuste empírico y no prueban una cota asintótica.

## Ejemplo

```cpp
std::vector<ntq::analysis::ComplexityObservation> points{
    {100, 0.0012L}, {200, 0.0025L}, {400, 0.0051L}};
std::vector<ntq::analysis::ComplexityModel> models{
    ntq::analysis::ComplexityModel::linear,
    ntq::analysis::ComplexityModel::quadratic};
auto fits = ntq::analysis::analyze_complexity(points, models);
```
