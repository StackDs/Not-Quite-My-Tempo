# `bootstrap_interval()`

**Contrato**

```cpp
ConfidenceInterval bootstrap_interval(
    const std::vector<long double> &values, BootstrapEstimator estimator,
    const StatisticsOptions &options, std::uint64_t seed);
```

Remuestrea `values` con reemplazo conservando el tamaño y estima media o mediana según `estimator`. `options` aporta nivel, cantidad de remuestreos y umbral de fiabilidad; `seed` hace reproducible la secuencia. Devuelve intervalo percentile bootstrap con estado de muestra baja o disponible. Menos de dos valores produce `insufficient_samples`.
