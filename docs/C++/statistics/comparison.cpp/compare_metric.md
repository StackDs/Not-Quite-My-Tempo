# `compare_metric()`

**Contrato**

```cpp
MetricComparison compare_metric(
    const ComparisonRequest &request, MetricId metric,
    const PreparedGroup *baseline, const PreparedGroup *candidate,
    bool paired, const StatisticsOptions &options);
```

Compara una métrica entre grupos preparados, usando solo `after_iqr`. `request` define identidad, `metric` selecciona variable, los punteros pueden ser nulos, `paired` selecciona emparejamiento por pair_id y `options` controla bootstrap. Devuelve conteos, modo de pareado y efectos mean/median; ausencia de grupos produce estado unavailable.
