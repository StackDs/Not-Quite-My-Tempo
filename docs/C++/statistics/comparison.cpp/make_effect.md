# `make_effect()`

**Contrato**

```cpp
EffectEstimate make_effect(
    const std::vector<long double> &baseline,
    const std::vector<long double> &candidate, BootstrapEstimator estimator,
    bool paired, const std::vector<long double> &paired_differences,
    const std::vector<long double> &paired_percentages,
    const StatisticsOptions &options, std::uint64_t seed);
```

Construye diferencias absolutas/relativas e intervalos de confianza. Si `paired`, estima sobre diferencias y porcentajes ya alineados; si no, resume grupos independientes. `options` fija bootstrap y `seed` reproducibilidad. Resultado expresa estados unavailable o invalid denominator cuando no hay datos/base válida.
