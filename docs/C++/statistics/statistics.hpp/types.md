# Tipos de estadísticas

**Declaración:** `include/C++/statistics/statistics.hpp`.

- `BenchmarkSample`: muestra identificada y agrupada con tiempos, memoria y hardware.
- `StatisticsOptions`: warm-ups, política de sospechosas, IQR, nivel/resamples/seed bootstrap, umbrales de fiabilidad y solicitudes de comparación.
- `RunMetadata`: ID, iteraciones, duración, reloj/resolución, versión, parámetros y snapshots del entorno.
- `StatisticsReport`: metadatos, opciones, diagnósticos, grupos, comparaciones y auditoría.
- `DescriptiveStatistics`: mínimo/máximo, media, mediana, MAD, desviación muestral, percentiles, intervalos y estados de fiabilidad.
- `ComparisonRequest`: grupo, variante base/candidata y política de emparejamiento.
- Los enums de estado identifican disponibilidad, fiabilidad, método de CI, exclusión y emparejamiento; consultar sus valores directamente en el header.

La API pública de cálculo es [`calculate_statistics()`](../statistics.cpp/calculate_statistics.md). Las muestras conservan valores originales; los filtros solo gobiernan las vistas estadísticas.
