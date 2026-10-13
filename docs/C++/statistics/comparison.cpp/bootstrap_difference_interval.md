# `bootstrap_difference_interval()`

**Contrato**

```cpp
ConfidenceInterval bootstrap_difference_interval(
    const std::vector<long double> &baseline,
    const std::vector<long double> &candidate, BootstrapEstimator estimator,
    bool relative, const StatisticsOptions &options, std::uint64_t seed);
```

Remuestrea base y candidata independientemente, calcula la diferencia candidata menos base para media/mediana y opcionalmente la expresa como porcentaje relativo a la base. `options` configura bootstrap; `seed` lo hace reproducible. Valores menores que dos en cada grupo o denominator base cero en todos los remuestreos dejan CI insuficiente.
