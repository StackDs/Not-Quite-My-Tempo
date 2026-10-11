# Referencia de `time_measurement.hpp`

Archivo documentado: `include/C++/metrics/time_measurement.hpp`  
Namespace: `ntq`

Este header declara la API pública para medir una sección de código y obtener
tiempo de pared, tiempo de CPU del proceso y cambios de contexto. La captura de
CPU y cambios de contexto está implementada para Linux.

## `TimeUnit`

Enum que indica la unidad de los tres tiempos que devuelve
`TimeMeasurement::obtain_elapsed()`:

| Valor | Unidad |
| --- | --- |
| `nanoseconds` | Nanosegundos |
| `microseconds` | Microsegundos |
| `milliseconds` | Milisegundos |
| `seconds` | Segundos |
| `minutes` | Minutos |
| `hours` | Horas |

Los contadores de cambios de contexto no cambian con la unidad seleccionada.

## `ElapsedMeasurements`

Estructura que contiene los valores calculados:

- `wall_clock`: tiempo real transcurrido.
- `cpu_user`: tiempo de CPU en modo usuario.
- `cpu_system`: tiempo de CPU empleado por el kernel.
- `voluntary_context_switches`: cambios de contexto voluntarios.
- `involuntary_context_switches`: cambios de contexto involuntarios.

Los tres campos de tiempo son `double` y están expresados en la unidad pedida.
Los tres tiempos devueltos por una misma llamada comparten la unidad indicada
en `obtain_elapsed()`. Los cambios de contexto son conteos `long` y no se
convierten; la unidad se conoce por el argumento pasado a ese método.

## `TimeMeasurement`

La medición se usa en este orden:

1. `start()` inicia o reinicia la medición.
2. Ejecutar el código que se quiere medir.
3. `stop()` detiene la captura.
4. `elapsed()` calcula y guarda las diferencias de los snapshots.
5. `obtain_elapsed(unit)` obtiene los resultados en la unidad deseada.

```cpp
#include "metrics/time_measurement.hpp"

ntq::TimeMeasurement measurement;
measurement.start();

// Código a medir.

measurement.stop();
measurement.elapsed();
const ntq::ElapsedMeasurements result =
    measurement.obtain_elapsed(ntq::TimeUnit::microseconds);
const ntq::ElapsedMeasurements result_in_seconds =
    measurement.obtain_elapsed(ntq::TimeUnit::seconds);
```

Se puede llamar a `obtain_elapsed()` varias veces para una medición. Cada
resultado contiene todos los tiempos en la unidad indicada en esa llamada; por
ejemplo, `result` está íntegramente en microsegundos y `result_in_seconds`
íntegramente en segundos. La estructura no almacena la unidad; quien consulta
debe conservar la unidad que solicitó para interpretar los tres campos.

### Métodos

- `start()`: captura los valores iniciales y marca la instancia como activa.
  Llamarlo de nuevo reinicia el ciclo y descarta resultados anteriores.
- `stop()`: captura los valores finales y marca la instancia como detenida.
- `elapsed()`: calcula y almacena los deltas desde `start()` hasta `stop()`.
- `obtain_elapsed(TimeUnit unit) const`: devuelve los resultados calculados,
  convirtiendo los tres tiempos a `unit` y conservando los conteos de contexto.
  No cambia ni consume el resultado almacenado, así que se puede consultar en
  otra unidad después; los campos del resultado retornado no incluyen la unidad.

`stop()` si la medición no está activa, `elapsed()` antes de detenerla y
`obtain_elapsed()` antes de calcular el resultado lanzan `std::logic_error`.
Una unidad fuera del enum lanza `std::invalid_argument`.
