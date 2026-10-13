# `available_values()`

**Ubicación:** `src/C++/statistics/statistics.cpp` (helper privado).

## Contrato

```cpp
std::vector<long double> available_values(
    const std::vector<const BenchmarkSample *> &samples, MetricId metric,
    MetricAvailabilityCounts *availability);
```

`samples` apunta a observaciones; `metric` selecciona el valor a extraer; `availability` es opcional y, si no es nulo, acumula los estados observados. Devuelve solo valores con estado disponible. No modifica las muestras.
