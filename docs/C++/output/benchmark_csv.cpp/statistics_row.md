# `statistics_row()`

**Contrato**

```cpp
std::vector<std::string> statistics_row(
    const statistics::GroupStatistics &group,
    const statistics::MetricSummary &metric,
    const statistics::StatisticsReport &report,
    const statistics::DescriptiveStatistics &summary);
```

Combina grupo, métrica, reporte y resumen descriptivo para devolver los campos de una fila estadística. Los cuatro parámetros son de solo lectura; el resultado sigue el orden de `statistics_header()`.
