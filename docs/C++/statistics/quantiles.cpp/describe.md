# `describe()`

**Contrato**

```cpp
DescriptiveStatistics describe(const std::vector<long double> &values,
                               const StatisticsOptions &options,
                               const std::string &group_key, MetricId metric);
```

Resume valores disponibles con mínimo/máximo, media, mediana, MAD, desviación muestral, percentiles, fiabilidad de colas e intervalos bootstrap. `options` fija políticas y seeds; `group_key` y `metric` estabilizan bootstrap. Entrada vacía devuelve estado unavailable; entradas válidas no se modifican.
