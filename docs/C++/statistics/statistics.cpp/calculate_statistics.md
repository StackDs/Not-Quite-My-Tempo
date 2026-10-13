# `calculate_statistics()`

**Implementación:** `src/C++/statistics/statistics.cpp` · **Declaración:** `include/C++/statistics/statistics.hpp`.

## Propósito

Calcula estadísticas, fiabilidad, comparaciones solicitadas y auditoría para muestras existentes.

## Contrato

```cpp
StatisticsReport calculate_statistics(
    const std::vector<BenchmarkSample> &samples,
    const RunMetadata &metadata,
    const StatisticsOptions &options);
```

| Parámetro | Significado |
| --- | --- |
| `samples` | Muestras medidas; cada una incluye grupo, variante, unidad y métricas. |
| `metadata` | Contexto de corrida, parámetros y snapshots ambientales. |
| `options` | Políticas de selección, warm-up, filtro IQR, bootstrap y comparación. |

Devuelve `StatisticsReport`. No modifica muestras ni ejecuta algoritmos. Lanza `std::invalid_argument` para opciones inválidas, IDs/repeticiones/pares duplicados o unidades incompatibles.
