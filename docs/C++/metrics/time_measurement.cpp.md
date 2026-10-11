# Implementación de `time_measurement.cpp`

Archivo documentado: `src/C++/metrics/time_measurement.cpp`  
API pública relacionada: [`time_measurement.hpp.md`](time_measurement.hpp.md)

`time_measurement.cpp` implementa el ciclo de estado y combina los colectores.
`linux/monotonic_clock.cpp` lee `CLOCK_MONOTONIC`,
`linux/process_resource_snapshot.cpp` captura `getrusage(RUSAGE_SELF)`,
`linux/isolated_process.cpp` administra el proceso hijo y su pipe, y
`time_conversion.cpp` contiene las conversiones de unidades. `getrusage` entrega
tiempo de CPU en segundos y microsegundos, junto con `ru_maxrss`, `ru_minflt` y
`ru_majflt`. Los instantes de pared rodean el snapshot de recursos y su punto
medio aproxima el instante del snapshot. `elapsed()` conserva los deltas de
tiempo en nanosegundos; `obtain_elapsed(unit)` los convierte a la unidad pedida.

`linux/perf_event_group.cpp` administra grupos de hardware perf con escalado de
eventos multiplexados. `linux/hardware_counters.cpp` configura los grupos de IPC,
L1D, LLC y branch misses; `hardware_measurement_calculations.cpp` calcula IPC y
porcentajes de fallos una vez leídos los contadores. El grupo de ciclos e
instrucciones y cada par de accesos/fallos de caché comparte tiempo de ejecución
para conservar razones coherentes.

`obtain_memory()` devuelve los page faults como deltas y el valor final de
`ru_maxrss` convertido de KiB a bytes. Como `ru_maxrss` es un máximo de vida del
proceso, una llamada manual se etiqueta `process_lifetime`; el método aislado
recolecta el máximo del proceso hijo dedicado y lo etiqueta
`isolated_measurement_process`.

`reported_clock_resolution()` está implementado junto al colector de reloj y
consulta `clock_getres(CLOCK_MONOTONIC)`. Devuelve la resolución reportada por
Linux, sin calibrar empíricamente la granularidad observada.

`measure_in_isolated_process()` crea un hijo con `fork()`, inicia la medición,
ejecuta el callback y envía al padre mediante un pipe los deltas guardados en
nanosegundos. Devuelve una instancia con esos deltas, que puede convertirse a
cualquier unidad mediante `obtain_elapsed(unit)`. El callback debe ejecutarse
desde un lanzador monohilo. Los cambios de memoria del hijo no son visibles para
el padre. Si el callback lanza una excepción o la medición falla, el hijo
termina con error y el padre recibe `std::runtime_error`.

`linux/tlb_miss_counters.cpp` configura eventos DTLB sobre el mismo mecanismo
`PerfEventGroup`, con kernel e hipervisor excluidos. Cada contador puede
reportarse por separado como `available`, `unsupported`, `permission_denied`,
`read_error` o `not_measured`. La falta de permisos/soporte no interrumpe la
medición y nunca se presenta como un cero válido. Los eventos se reinician y
habilitan al iniciar, se deshabilitan/leen al detener y sus descriptores tienen
propiedad RAII. Se habilita la herencia de eventos para los hilos de trabajo
creados por el callback cuando el kernel lo soporta.

Cada instancia acepta un máximo configurable de cambios involuntarios. `elapsed()`
compara el delta capturado con ese umbral y guarda el resultado en la flag
`suspicious`. El valor predeterminado es cero. La muestra y sus métricas se
conservan independientemente de la flag; esta capa no descarta ni repite muestras.

## Conversiones

- `cpu_time_delta_nanoseconds(seconds, microseconds)`: convierte un delta de CPU
  de `getrusage` a nanosegundos.
- `nanoseconds_per_unit(TimeUnit unit)`: convierte nanosegundos a la unidad
  solicitada; una unidad no soportada genera `std::invalid_argument`.

## Funciones de `TimeMeasurement`

- `reported_clock_resolution()`: consulta la resolución reportada para
  `CLOCK_MONOTONIC` en `linux/monotonic_clock.cpp`. Si `clock_getres` falla,
  lanza `std::system_error` con el código de `errno`.
- `measure_in_isolated_process(callback)`: ejecuta el callback en un proceso hijo
  y devuelve una instancia lista para consultar. Su definición y la IPC están en
  `linux/isolated_process.cpp`; el resultado se transfiere por pipe y el hijo se
  recolecta con `waitpid`. Si el callback falla, el padre recibe
  `std::runtime_error`.
- `TimeMeasurement(max_involuntary_context_switches)`: configura el umbral de
  cambios involuntarios; rechaza valores negativos con `std::invalid_argument`.
- `capture_resources()`: llama a `getrusage(RUSAGE_SELF)` desde
  `linux/process_resource_snapshot.cpp` y devuelve un snapshot conjunto de
  tiempos, cambios de contexto, RSS y page faults. Si falla, lanza
  `std::system_error`.
- `start()`: invalida el resultado anterior, captura recursos y el instante del
  reloj monotónico, y activa el estado de medición.
- `stop()`: requiere que la medición esté activa. Captura los valores finales y
  pasa al estado detenido. Si no está activa, lanza `std::logic_error`.
- `elapsed()`: requiere una medición detenida. Calcula el delta del reloj de
  pared, los deltas de CPU y los deltas de cambios de contexto; conserva los
  tiempos en nanosegundos y habilita la lectura del resultado. Si se invoca
  antes de `stop()`, lanza `std::logic_error`.
- `obtain_elapsed(unit) const`: requiere que `elapsed()` haya calculado el
  resultado. Divide los tres tiempos por el factor de la unidad solicitada y
  devuelve los contadores sin conversión. No modifica el resultado guardado, por
  lo que las llamadas siguientes pueden elegir otra unidad.
- `obtain_memory() const`: requiere `elapsed()` y devuelve métricas RSS/page
  faults/DTLB. No convierte los valores de memoria.
- `obtain_hardware() const`: requiere `elapsed()` y devuelve contadores crudos,
  IPC y porcentajes de fallos de lectura L1D/LLC con sus estados.

## Estado interno

- `is_running_`: indica que se inició la captura y todavía no se detuvo.
- `has_stopped_`: permite distinguir una captura detenida de una instancia que
  aún no se ha iniciado.
- `has_elapsed_`: indica que los deltas están calculados y se pueden consultar.
- `resources_start_` / `resources_end_`: snapshots de `getrusage` tomados al
  iniciar y detener.
- `wall_start_` / `wall_end_`: instantes de `CLOCK_MONOTONIC` que delimitan la
  captura.
- `elapsed_nanoseconds_`: último resultado calculado; conserva los tres tiempos
  en nanosegundos, los cambios de contexto, y la flag `suspicious`.
- `memory_measurements_`: Peak RSS, deltas minor/major y valores/estados de DTLB.
- `tlb_counters_`: propietario RAII de los eventos perf activos durante la
  sección medida; el objeto es movible y no copiable.
- `hardware_counters_`: coordina grupos perf para ciclos/instrucciones, accesos y
  fallos L1D/LLC, y branch mispredictions.
- `hardware_measurements_`: lecturas crudas y valores derivados con estado por
  métrica.
- `max_involuntary_context_switches_`: máximo de cambios involuntarios permitidos
  antes de marcar el resultado como sospechoso.
