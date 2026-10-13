# `invalid_fit()`

**Ubicación:** `src/C++/analysis/complexity.cpp` (función interna de `ntq::analysis`).

## Propósito

Construye un `ComplexityFit` que identifica el modelo evaluado y marca su estado como `AnalysisStatus::invalid_input`. Centraliza la respuesta a entradas o cálculos inválidos.

## Contrato

```cpp
ComplexityFit invalid_fit(ComplexityModel model, std::size_t count);
```

| Parámetro | Significado |
| --- | --- |
| `model` | Modelo que no pudo ajustarse; se copia al resultado. |
| `count` | Número de observaciones que se conserva en `observation_count`. |

## Retorno

Devuelve el resultado con `status == invalid_input`, el modelo solicitado y el conteo indicado. La constante y `R²` permanecen con sus valores iniciales.

No modifica estado externo ni lanza errores como parte de su contrato normal.
