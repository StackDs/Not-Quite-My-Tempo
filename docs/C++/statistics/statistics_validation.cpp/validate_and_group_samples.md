# `validate_and_group_samples()`

**Contrato**

```cpp
ValidatedSamples validate_and_group_samples(
    const std::vector<BenchmarkSample> &samples,
    const StatisticsOptions &options);
```

Valida opciones, IDs, repeticiones, parejas y unidades; construye índices por grupo/variante y auditoría inicial. `samples` contiene observaciones; `options` define reglas y comparaciones. Devuelve `ValidatedSamples`; configuraciones inconsistentes lanzan `std::invalid_argument`.
