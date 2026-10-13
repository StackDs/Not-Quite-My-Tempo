# `calculate_statistics_report()`

**Ubicación:** `src/C++/statistics/statistics.cpp` (coordinador interno de `ntq::statistics::detail`).

## Contrato

```cpp
StatisticsReport calculate_statistics_report(
    const std::vector<BenchmarkSample> &samples,
    const RunMetadata &metadata,
    const StatisticsOptions &options);
```

`samples` son mediciones a validar y agrupar; `metadata` se copia al reporte y sirve para diagnósticos de entorno; `options` controla warm-ups, sospechosas, IQR, bootstrap y comparaciones. Devuelve reporte con cobertura, resúmenes, comparaciones y auditoría. Para entrada vacía retorna un reporte vacío con metadatos y diagnósticos. Las opciones/muestras inválidas generan `std::invalid_argument` en la validación delegada.
