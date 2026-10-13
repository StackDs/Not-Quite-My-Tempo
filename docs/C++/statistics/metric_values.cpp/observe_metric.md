# `observe_metric()`

**Contrato**

```cpp
MetricObservation observe_metric(const BenchmarkSample &sample,
                                 MetricId metric);
```

Extrae una métrica escalar de `sample` y traduce los estados de memoria/hardware. `metric` selecciona el campo; métricas desconocidas resultan inválidas. Devuelve valor y estado sin mutar la muestra.
