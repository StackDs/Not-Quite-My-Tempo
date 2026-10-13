# Estadísticas

Las funciones estadísticas resumen muestras ya recolectadas; no ejecutan algoritmos. Agrupan por claves opacas, conservan auditoría de exclusiones y mantienen estados de disponibilidad.

## `statistics.cpp`

- [`count_status`](statistics.cpp/count_status.md)
- [`available_values`](statistics.cpp/available_values.md)
- [`calculate_statistics_report`](statistics.cpp/calculate_statistics_report.md)
- [`calculate_statistics`](statistics.cpp/calculate_statistics.md)

## Validación (`statistics_validation.cpp`)

- [`valid_time_unit`](statistics_validation.cpp/valid_time_unit.md)
- [`validate_options`](statistics_validation.cpp/validate_options.md)
- [`sample_id_or_index`](statistics_validation.cpp/sample_id_or_index.md)
- [`validate_and_group_samples`](statistics_validation.cpp/validate_and_group_samples.md)

## Cálculos (`bootstrap.cpp`, `quantiles.cpp`)

- [`bootstrap_interval`](bootstrap.cpp/bootstrap_interval.md)
- [`quantile`](quantiles.cpp/quantile.md), [`median`](quantiles.cpp/median.md), [`median_absolute_deviation`](quantiles.cpp/median_absolute_deviation.md)
- [`sample_standard_deviation`](quantiles.cpp/sample_standard_deviation.md), [`stable_seed`](quantiles.cpp/stable_seed.md), [`describe`](quantiles.cpp/describe.md)

## Comparación y preparación

- [`prepare_groups`](statistics_preparation.cpp/prepare_groups.md), [`add_reason`](statistics_preparation.cpp/add_reason.md)
- [`calculate_comparisons`](comparison.cpp/calculate_comparisons.md), [`compare_metric`](comparison.cpp/compare_metric.md), [`make_effect`](comparison.cpp/make_effect.md)
- [Helpers bootstrap/comparación](comparison.cpp/compare_metric_helpers.md)

## Métricas y entorno

- [Funciones de extracción de métricas](metric_values.cpp/index.md)
- [Diagnósticos de entorno](environment_diagnostics.cpp/index.md)

Tipos y políticas principales están en `include/C++/statistics/statistics.hpp`.
