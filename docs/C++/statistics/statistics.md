# Estadísticas de mediciones

API: `include/C++/statistics/statistics.hpp`  
Implementación: `src/C++/statistics/`  
Namespace: `ntq::statistics`

`calculate_statistics()` consume mediciones que ya se ejecutaron. No ejecuta el
algoritmo, no modifica los recolectores y no interpreta el significado de los
grupos. Cada `group_key` es una clave opaca elegida por quien llama: puede
representar un parámetro, índice, tamaño u otra dimensión.

```cpp
#include "statistics/statistics.hpp"

std::vector<ntq::statistics::BenchmarkSample> samples = /* muestras */;
ntq::statistics::RunMetadata metadata;
metadata.run_id = "run-001";
metadata.clock_name = "CLOCK_MONOTONIC";
metadata.clock_resolution = ntq::TimeMeasurement::reported_clock_resolution();

ntq::statistics::StatisticsOptions options;
options.warmup_runs_per_group = 3;
const ntq::statistics::StatisticsReport report =
    ntq::statistics::calculate_statistics(samples, metadata, options);
```

## Muestras y contexto

Cada `BenchmarkSample` enlaza `ElapsedMeasurements`, `MemoryMeasurements` y
`HardwareMeasurements`, e incluye `sample_id`, `group_key`, `variant_key`,
`pair_id`, índice de repetición y `TimeUnit`. Todas las muestras de un mismo
grupo/variante deben usar la misma unidad; una mezcla se rechaza explícitamente.

`RunMetadata` guarda iteraciones configuradas, duración, reloj/resolución, versión
de librería, parámetros del algoritmo y snapshots de entorno antes/después. El
contexto contiene `SystemInfo` y valores opcionales de procesos ejecutables,
temperatura y throttling con su estado. Estos datos se conservan como contexto;
el módulo no aplica correcciones ni los usa para filtrar muestras. Las flags
exactas de compilación quedan fuera de este apartado.

`EnvironmentDiagnostics` resume cambios observables entre snapshots: delta de
procesos ejecutables, temperatura, afinidad, gobernador/frecuencia y si se
observó throttling. Si falta una lectura, el diagnóstico mantiene ese estado en
vez de inferir que no hubo cambios.

## Selección y auditoría de muestras

- Las primeras `warmup_runs_per_group` repeticiones, ordenadas por
  `repetition_index`, se apartan de los resúmenes.
- Las muestras con `elapsed.suspicious` se incluyen por defecto. La opción
  `include_suspicious_samples` permite excluirlas si el usuario lo decide. El
  conteo total de sospechosas incluye también los warm-ups.
- Si `apply_iqr_filter` está activo, el filtro se calcula sobre wall-clock dentro
  de cada grupo/variante: se apartan valores superiores a `Q3 + 1.5 * IQR`.
  Q1/Q3 usan cuantiles interpolados tipo 7.
- El mismo filtro de muestras se aplica a todas las métricas de un grupo para
  conservar las mediciones emparejadas.
- La muestra original nunca se modifica. `SampleAudit` devuelve su ID, grupo,
  variante, flag/razón/umbral de sospecha, decisión de inclusión y razones de
  exclusión: warm-up, sospechosa excluida, outlier IQR o wall-clock inválido.

## Resúmenes por métrica

Para cada valor disponible se reportan dos vistas: `before_iqr` y `after_iqr`.
Ambas ya aplican warm-up y la política de muestras sospechosas; solo la segunda
aplica el filtro IQR. Cada vista contiene:

- cantidad de observaciones;
- mínimo y máximo;
- media aritmética y desviación estándar muestral (`n-1`);
- mediana/p50 y MAD;
- p90, p95, p99 y p99.9;
- intervalos de confianza del 95 % bootstrap-percentile para media y mediana.

La media y desviación estándar son complementarias; mediana y MAD son las
medidas robustas principales. El MAD es la mediana de las distancias absolutas a
la mediana. Los intervalos incluyen método, semilla y cantidad de remuestreos.
Las métricas con estado no disponible se cuentan por estado y nunca se tratan
como cero. Los conteos de disponibilidad incluyen las muestras posteriores al
warm-up aunque se hayan excluido después por la flag o por IQR.

### Fiabilidad de intervalos y colas

Los intervalos se calculan desde dos observaciones o más; debajo de
`minimum_reliable_ci_samples` (30 por defecto) se reportan con estado de baja
fiabilidad. El valor de un percentil de cola también se conserva, pero se marca
de baja fiabilidad cuando `n * (1 - p)` es menor que
`minimum_expected_tail_observations` (5 por defecto). Por ejemplo, un p99.9
necesita alrededor de 5.000 observaciones para alcanzar ese umbral.

## Comparaciones y tamaño del efecto

El caller puede solicitar comparaciones explícitas con `ComparisonRequest`, que
identifica grupo, variante base y variante candidata. La diferencia se define
como **candidato − base**; para wall-clock un valor negativo indica menos tiempo.
Se reportan diferencias absolutas y relativas para media y mediana.

- Si hay `pair_id` coincidentes, `PairingPolicy::automatic` usa diferencias por
  pares; también se puede exigir modo pareado.
- Si no hay pares coincidentes, se usa bootstrap independiente para los grupos.
- Se reportan método, cantidad de pares/muestras e intervalos de confianza de las
  diferencias absolutas/relativas.
- Si el denominador de la diferencia relativa es cero, su estado es
  `invalid_denominator`.

## Alcance

Este módulo produce resúmenes y comparaciones; no selecciona el algoritmo
ganador, no ajusta modelos de complejidad ni calcula `R²` o constantes Big O. Un
analizador de complejidad podrá consumir después los resúmenes agrupados. Las
flags exactas de compilación también quedan pospuestas.

Prueba manual desde la raíz del repositorio:

```sh
g++ -std=c++14 -Wall -Wextra -Wpedantic -Werror \
  -Iinclude/C++ -Isrc/C++/statistics \
  tests/C++/statistics/statistics_tests.cpp \
  src/C++/statistics/statistics.cpp \
  src/C++/statistics/environment_diagnostics.cpp \
  src/C++/statistics/metric_values.cpp \
  src/C++/statistics/quantiles.cpp \
  src/C++/statistics/bootstrap.cpp \
  src/C++/statistics/comparison.cpp \
  -o /tmp/statistics_tests
/tmp/statistics_tests
```
