# `comparison_seed()`

**Contrato**

```cpp
std::uint64_t comparison_seed(const StatisticsOptions &options,
                              const ComparisonRequest &request,
                              MetricId metric, const char *estimator);
```

Deriva un seed reproducible usando seed bootstrap, grupo, variantes, estimador y métrica. `estimator` identifica media/mediana y modo; devuelve `uint64_t`.
