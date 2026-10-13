# `calculate_comparisons()`

**Contrato**

```cpp
std::vector<GroupComparison> calculate_comparisons(
    const std::vector<PreparedGroup> &groups,
    const std::vector<ComparisonRequest> &requests,
    const StatisticsOptions &options);
```

Ejecuta las solicitudes explícitas de comparación. `groups` son grupos preparados y filtrados; `requests` indica grupo/base/candidata/política; `options` configura bootstrap. Emparejamiento automático usa pares coincidentes cuando existen. Devuelve una comparación por solicitud y métricas incluidas.
