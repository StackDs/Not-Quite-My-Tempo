# `count_status()`

**Ubicación:** `src/C++/statistics/statistics.cpp` (helper privado).

## Contrato

```cpp
void count_status(MetricAvailabilityCounts &counts, MetricValueStatus status);
```

`counts` es el acumulador que se modifica; `status` identifica el estado de una observación. Incrementa exactamente el contador correspondiente (`available`, no disponible, permisos, error, no medido, denominador inválido o valor inválido). No devuelve valor ni valida la observación.
