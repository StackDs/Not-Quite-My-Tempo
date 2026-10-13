# `model_basis()`

**Ubicación:** `src/C++/analysis/complexity.cpp` (función interna de `ntq::analysis`).

## Propósito

Calcula la función base `f(N)` para un modelo de complejidad y tamaño concretos. La constante multiplicativa se ajusta posteriormente en `fit_model()`.

## Contrato

```cpp
bool model_basis(ComplexityModel model, std::size_t input_size,
                 long double &basis);
```

| Parámetro | Significado |
| --- | --- |
| `model` | Modelo de crecimiento solicitado. |
| `input_size` | Tamaño `N`; debe ser distinto de cero para las llamadas de ajuste. |
| `basis` | Referencia de salida; recibe `f(N)` cuando el cálculo es válido. |

## Retorno y límites

Devuelve `true` si el modelo es reconocido y el resultado es finito y no negativo; en caso contrario devuelve `false`. `n_log_log_n` requiere `N >= 3`. Los modelos factorial y exponencial pueden exceder el rango de `long double`.

Es una función interna sin efectos externos; no modifica los datos de entrada.
