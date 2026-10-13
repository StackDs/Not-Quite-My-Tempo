# `metric_values()`

**Contrato**

```cpp
std::vector<long double> metric_values(
    const std::vector<const BenchmarkSample *> &samples, MetricId metric);
```

Extrae valores numéricos disponibles para `metric` e ignora observaciones con estado no disponible. Devuelve los valores en orden de muestras; no altera las entradas.
