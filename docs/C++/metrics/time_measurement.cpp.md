# Implementación de `time_measurement.cpp`

Archivo documentado: `src/C++/metrics/time_measurement.cpp`  
API pública relacionada: [`time_measurement.hpp.md`](time_measurement.hpp.md)

La implementación usa `clock_gettime(CLOCK_MONOTONIC)` para el tiempo de pared y
`getrusage(RUSAGE_SELF)` para las métricas de CPU y los cambios de contexto del
proceso. `getrusage` reporta tiempo de CPU en segundos y microsegundos. Los
instantes de pared se toman antes y después del snapshot de recursos; su punto
medio aproxima el instante del snapshot y reduce el desfase entre métricas. La
fase `elapsed()` conserva los deltas en nanosegundos; cada llamada a
`obtain_elapsed(unit)` convierte los tres tiempos juntos a la unidad solicitada.

`reported_clock_resolution()` consulta `clock_getres(CLOCK_MONOTONIC)` y devuelve
la resolución reportada por Linux en nanosegundos. Es el mismo reloj usado para
el wall-clock; la consulta no calibra la granularidad observada empíricamente.

`measure_in_isolated_process()` crea un hijo con `fork()`, inicia la medición,
ejecuta el callback y envía al padre mediante un pipe los deltas guardados en
nanosegundos. Devuelve una instancia con esos deltas, que puede convertirse a
cualquier unidad mediante `obtain_elapsed(unit)`. El callback debe ejecutarse
desde un lanzador monohilo. Los cambios de memoria del hijo no son visibles para
el padre. Si el callback lanza una excepción o la medición falla, el hijo
termina con error y el padre recibe `std::runtime_error`.

## Funciones auxiliares

- `to_nanoseconds(seconds, microseconds)`: convierte un delta de CPU capturado
  por `getrusage` a nanosegundos usando `long double` durante el cálculo.
- `nanoseconds_per_unit(TimeUnit unit)`: devuelve el divisor para convertir
  nanosegundos a la unidad seleccionada. Una unidad no soportada genera
  `std::invalid_argument`.

## Funciones de `TimeMeasurement`

- `reported_clock_resolution()`: consulta la resolución reportada para
  `CLOCK_MONOTONIC`. Si `clock_getres` falla, lanza `std::system_error` con el
  código de `errno`.
- `measure_in_isolated_process(callback)`: ejecuta el callback en un proceso hijo
  y devuelve una instancia lista para consultar. Envía los deltas en nanosegundos
  por un pipe y usa `waitpid` para recolectar el proceso hijo. Si el callback
  falla, el error se reporta en el padre como `std::runtime_error`.
- `capture_resources()`: llama a `getrusage(RUSAGE_SELF)` y copia los tiempos de
  usuario y sistema, junto con los contadores voluntario e involuntario, en un
  snapshot. Si la llamada falla, lanza `std::system_error` con el código de
  `errno`.
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

## Estado interno

- `is_running_`: indica que se inició la captura y todavía no se detuvo.
- `has_stopped_`: permite distinguir una captura detenida de una instancia que
  aún no se ha iniciado.
- `has_elapsed_`: indica que los deltas están calculados y se pueden consultar.
- `resources_start_` / `resources_end_`: snapshots de `getrusage` tomados al
  iniciar y detener.
- `wall_start_` / `wall_end_`: instantes de `steady_clock` que delimitan la
  captura.
- `elapsed_nanoseconds_`: último resultado calculado; conserva los tres tiempos
  en nanosegundos y los cambios de contexto como conteos.
