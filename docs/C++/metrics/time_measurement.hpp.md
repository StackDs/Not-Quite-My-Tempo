# Referencia de `time_measurement.hpp`

Archivo documentado: `include/C++/metrics/time_measurement.hpp`  
Namespace: `ntq`

Este header declara la API pública para medir una sección de código y obtener
tiempo de pared, tiempo de CPU del proceso y cambios de contexto. La captura de
CPU, cambios de contexto y resolución reportada del reloj está implementada para
Linux.

## Resolución del reloj

`TimeMeasurement::reported_clock_resolution()` devuelve, en nanosegundos, la
resolución que Linux reporta para `CLOCK_MONOTONIC`, el mismo reloj utilizado
para capturar el wall-clock. Este valor describe la resolución reportada por el
sistema; no garantiza la precisión de cada medición ni mide empíricamente la
granularidad observada.

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
- `suspicious`: indica si los cambios involuntarios superaron el umbral
  configurado.

Los tres campos de tiempo son `double` y están expresados en la unidad pedida.
Los tres tiempos devueltos por una misma llamada comparten la unidad indicada
en `obtain_elapsed()`. Los cambios de contexto son conteos `long` y no se
convierten; la unidad se conoce por el argumento pasado a ese método.
`suspicious` tampoco depende de la unidad: es una flag booleana del resultado.

## `TimeMeasurement`

La medición se usa en este orden:

1. `start()` inicia o reinicia la medición.
2. Ejecutar el código que se quiere medir.
3. `stop()` detiene la captura.
4. `elapsed()` calcula y guarda las diferencias de los snapshots.
5. `obtain_elapsed(unit)` obtiene los resultados en la unidad deseada.

Para medir en un proceso dedicado se puede usar
`TimeMeasurement::measure_in_isolated_process(callback)`. El método crea un hijo
con `fork()` y devuelve un `TimeMeasurement` listo para consultar. Los tiempos se
conservan internamente en nanosegundos y se convierten al pedir el resultado:

```cpp
const ntq::TimeMeasurement isolated =
    ntq::TimeMeasurement::measure_in_isolated_process([] {
      // Ejecutar el algoritmo.
    }, 1); // Permitir como máximo un cambio involuntario.

const ntq::ElapsedMeasurements in_microseconds =
    isolated.obtain_elapsed(ntq::TimeUnit::microseconds);
const ntq::ElapsedMeasurements in_seconds =
    isolated.obtain_elapsed(ntq::TimeUnit::seconds);
```

El proceso que lo invoca debe ser monohilo. El estado de memoria modificado en
el hijo no se refleja en el padre; solo se devuelven las métricas. Los efectos
externos, como escribir en un archivo, sí pueden persistir. Si no se pasa un
umbral, se usa cero y cualquier cambio involuntario marca el resultado como
sospechoso.

También se puede configurar el umbral para el flujo manual mediante el
constructor `TimeMeasurement(max_involuntary_context_switches)`. Un umbral
negativo lanza `std::invalid_argument`.

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

- `reported_clock_resolution()`: consulta la resolución reportada de
  `CLOCK_MONOTONIC`. Es un método estático y puede invocarse sin iniciar una
  medición. Si la consulta del sistema falla, lanza `std::system_error`.
- `measure_in_isolated_process(callback)`: ejecuta el callback en un proceso
  hijo dedicado y devuelve una instancia lista para consultar con
  `obtain_elapsed(unit)`. El argumento opcional `max_involuntary_context_switches`
  establece el umbral para la flag `suspicious` y vale cero por defecto. Los
  tiempos se conservan en nanosegundos hasta que se solicita una unidad. Lanza
  `std::system_error` si fallan las operaciones POSIX de proceso o comunicación,
  y `std::runtime_error` si el callback o la medición falla en el hijo. No debe
  invocarse desde un proceso multihilo.
- `TimeMeasurement(max_involuntary_context_switches)`: configura el umbral del
  flujo manual; el valor por defecto es cero. Un valor negativo lanza
  `std::invalid_argument`.
- `start()`: captura los valores iniciales y marca la instancia como activa.
  Llamarlo de nuevo reinicia el ciclo y descarta resultados anteriores.
- `stop()`: captura los valores finales y marca la instancia como detenida.
- `elapsed()`: calcula y almacena los deltas desde `start()` hasta `stop()`.
- `obtain_elapsed(TimeUnit unit) const`: devuelve los resultados calculados,
  convirtiendo los tres tiempos a `unit` y conservando los contadores y la flag
  `suspicious`.
  No cambia ni consume el resultado almacenado, así que se puede consultar en
  otra unidad después; los campos del resultado retornado no incluyen la unidad.

`stop()` si la medición no está activa, `elapsed()` antes de detenerla y
`obtain_elapsed()` antes de calcular el resultado lanzan `std::logic_error`.
Una unidad fuera del enum lanza `std::invalid_argument`.
