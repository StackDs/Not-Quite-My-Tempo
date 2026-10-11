# Referencia de `time_measurement.hpp`

Archivo documentado: `include/C++/metrics/time_measurement.hpp`  
Namespace: `ntq`

Este header declara la API pública para medir una sección de código y obtener
tiempo de pared, tiempo de CPU, cambios de contexto y métricas de memoria. La
captura de recursos y los contadores perf están implementados para Linux.

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
- `involuntary_context_switch_threshold`: umbral que se evaluó.
- `suspicious_reason`: regla que activó la flag; por ahora identifica que el
  contador involuntario superó ese umbral.

Los tres campos de tiempo son `double` y están expresados en la unidad pedida.
Los tres tiempos devueltos por una misma llamada comparten la unidad indicada
en `obtain_elapsed()`. Los cambios de contexto son conteos `long` y no se
convierten; la unidad se conoce por el argumento pasado a ese método.
`suspicious` tampoco depende de la unidad: es una flag booleana del resultado.

## `MemoryMeasurements`

Se obtiene con `TimeMeasurement::obtain_memory()` después de `elapsed()`:

- `peak_rss_bytes`: high-water RSS del proceso, expresado en bytes.
- `minor_page_faults` / `major_page_faults`: conteos ocurridos entre `start()` y
  `stop()`.
- `data_tlb_read_misses` / `data_tlb_write_misses`: fallos de DTLB reportados por
  los eventos genéricos de Linux `perf_event_open`.
- `peak_rss_status`: disponibilidad del Peak RSS.
- `data_tlb_read_status` / `data_tlb_write_status`: estado de cada contador perf
  (`available`, `unsupported`, `permission_denied`, `read_error` o
  `not_measured`); si no está disponible, su valor numérico no debe
  interpretarse como cero.
- `peak_rss_scope`: indica si el pico corresponde a toda la vida del proceso o
  al proceso hijo aislado de una medición.

En Linux, `ru_maxrss` se reporta en KiB y se convierte a bytes. Es un máximo
acumulado y no se resta entre snapshots: en una medición manual representa el
high-water RSS de todo el proceso. Para asociarlo a una sola ejecución, usa
`measure_in_isolated_process()`. Los page faults sí se reportan como deltas de
`ru_minflt` y `ru_majflt`.

Los eventos TLB cuentan DTLB de lecturas y escrituras, en espacio de usuario. No
incluyen ITLB. El hardware, kernel, permisos o restricciones de `perf_event_open`
pueden hacer que cada contador quede como `unsupported`, `permission_denied` o
`read_error`; esto no impide obtener las otras métricas.

## `HardwareMeasurements`

Se obtiene con `TimeMeasurement::obtain_hardware()` después de `elapsed()`:

- `cpu_cycles` e `instructions_retired`: contadores usados para obtener `ipc`
  (instrucciones retiradas por ciclo).
- `l1d_read_accesses` / `l1d_read_misses` y `llc_read_accesses` /
  `llc_read_misses`: contadores crudos para calcular los porcentajes de fallos de
  lectura L1D y LLC.
- `branch_mispredictions`: predicciones de saltos fallidas.
- Los campos `*_status` identifican si el contador o valor derivado está
  `available`, `unsupported`, `permission_denied`, `read_error` o `not_measured`.
  `invalid_denominator` significa que no se puede calcular IPC o un porcentaje
  porque su denominador fue cero.

Los eventos genéricos se obtienen mediante `perf_event_open`. Ciclos/instrucciones
y accesos/fallos de cada caché se cuentan en grupos para que los términos de cada
razón compartan el mismo intervalo efectivo. Los conteos pueden escalarse si el
kernel multiplexa eventos. Las tasas de caché corresponden a accesos de lectura;
los fallos de escritura no se incluyen en esos porcentajes.

## `TimeMeasurement`

La medición se usa en este orden:

1. `start()` inicia o reinicia la medición.
2. Ejecutar el código que se quiere medir.
3. `stop()` detiene la captura.
4. `elapsed()` calcula y guarda las diferencias de los snapshots.
5. `obtain_elapsed(unit)` obtiene los resultados en la unidad deseada.
6. `obtain_memory()` obtiene métricas de memoria sin conversión de unidad.
7. `obtain_hardware()` obtiene los contadores de microarquitectura.

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
const ntq::MemoryMeasurements memory = isolated.obtain_memory();
const ntq::HardwareMeasurements hardware = isolated.obtain_hardware();
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
const ntq::MemoryMeasurements memory = measurement.obtain_memory();
const ntq::HardwareMeasurements hardware = measurement.obtain_hardware();
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
- `obtain_memory() const`: devuelve Peak RSS, deltas de page faults y
  estados/conteos DTLB después de `elapsed()`. Antes de calcular el resultado
  lanza `std::logic_error`.
- `obtain_hardware() const`: devuelve IPC, contadores de instrucciones, caché y
  branch mispredictions con su estado de disponibilidad; requiere `elapsed()`.
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
`obtain_elapsed()`/`obtain_memory()` antes de calcular el resultado lanzan
`std::logic_error`.
Una unidad fuera del enum lanza `std::invalid_argument`.

`TimeMeasurement` es movible y no copiable para que la propiedad de los
descriptores perf sea única y segura.
