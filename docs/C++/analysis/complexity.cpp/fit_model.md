# `fit_model()`

**Ubicación:** `src/C++/analysis/complexity.cpp` (función interna de `ntq::analysis`).

## Propósito

Ajusta un único modelo a las observaciones mediante mínimos cuadrados sin intercepto: `elapsed_time ≈ c * f(N)`. Calcula la constante empírica `c` y el `R²` centrado.

## Contrato

```cpp
ComplexityFit fit_model(
    const std::vector<ComplexityObservation> &observations,
    ComplexityModel model);
```

| Parámetro | Significado |
| --- | --- |
| `observations` | Pares de tamaño y tiempo. Se requieren al menos dos; los tamaños deben ser positivos y los tiempos positivos y finitos. Todos los tiempos deben estar en la misma unidad. |
| `model` | Modelo cuya base `f(N)` se ajusta. |

## Retorno y estados

Devuelve un `ComplexityFit` con el modelo y el número de observaciones. Menos de dos observaciones produce `insufficient_samples`; datos inválidos, bases no representables o acumulaciones no finitas producen `invalid_input`. Si la varianza de tiempos es cero, la constante queda calculada y el estado es `undefined_r_squared`; de lo contrario se devuelve `available` con `R²`.

La función no modifica el vector recibido. Es un detalle interno que no forma parte de la API pública.
