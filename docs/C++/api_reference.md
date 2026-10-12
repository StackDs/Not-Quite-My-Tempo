# Referencia de la API pública C++

Esta página resume qué función usar, qué parámetros recibe y qué contrato ofrece
cada función pública declarada bajo `include/C++/`. Se centra en el uso de la
API; los detalles de algoritmos estadísticos y del comportamiento de los
colectores están ampliados en las guías específicas enlazadas al final.

Los helpers privados de `src/C++/` no forman parte de esta referencia: pueden
cambiar sin ser una interfaz estable. El README menciona algunos headers de una
interfaz general (`benchmark.hpp`, `config.hpp`, `result.hpp`, `metadata.hpp` y
`types.hpp`) que todavía no existen en el árbol del proyecto; por tanto, no se
documentan aquí como funciones disponibles.

## Flujo de uso

Los módulos actuales se pueden combinar manualmente en este orden:

1. Llamar a `ntq::data::generate_seed()` o elegir una semilla fija; generar la
   entrada con una función de `ntq::data`.
2. Medir el código con `ntq::TimeMeasurement` o
   `TimeMeasurement::measure_in_isolated_process()`.
3. Crear un `ntq::statistics::BenchmarkSample` con los resultados y agruparlo
   mediante `group_key`, `variant_key`, `repetition_index` y, cuando corresponda,
   `pair_id`.
4. Llamar a `ntq::statistics::calculate_statistics()` para obtener resúmenes,
   filtros y comparaciones.
5. Asociar cada grupo con el tamaño de entrada en el código llamador y pasar los
   tamaños y tiempos representativos a `ntq::analysis::analyze_complexity()`.

La API pública actual no tiene todavía una función que ejecute automáticamente
este flujo completo.

## Datos aleatorios — `ntq::data`

Header de entrada: `data/random_data.hpp`; las familias están organizadas en
`random_data_core.hpp`, `random_data_discrete.hpp`,
`random_data_continuous.hpp`, `random_data_compositions.hpp` y
`random_data_profiles.hpp`. Las funciones aceptan una semilla para crear un
motor `std::mt19937_64`, o una referencia al motor para continuar una secuencia
compartida. Los tipos `T` deben ser aritméticos y distintos de `bool`.

### `generate_seed()`

```cpp
std::uint64_t generate_seed();
```

Obtiene una semilla de 64 bits usando `std::random_device`. Guarda la semilla
devuelta si necesitas reproducir una generación. No es una API de aleatoriedad
criptográfica; la fuente puede lanzar una excepción si no puede proporcionar un
valor.

### `uniform_vector<T>()`

```cpp
std::vector<T> uniform_vector(
    std::size_t count, T minimum, T maximum, std::uint64_t seed,
    const GenerationOptions& options = {});

std::vector<T> uniform_vector(
    std::size_t count, T minimum, T maximum, std::mt19937_64& engine,
    const GenerationOptions& options = {});
```

Genera `count` valores uniformes. Para tipos enteros el intervalo es inclusivo
`[minimum, maximum]`; para tipos de punto flotante es `[minimum, maximum)`. Los
límites invertidos generan `std::invalid_argument`; los límites reales también
deben ser finitos. Si se solicita unicidad en un rango entero insuficiente,
lanza `std::invalid_argument`. Con una distribución flotante o configuración
que no permita obtener suficientes valores únicos dentro del límite, lanza
`std::runtime_error`.

### `uniform_array<T, N>()`

```cpp
std::array<T, N> uniform_array(
    T minimum, T maximum, std::uint64_t seed,
    const GenerationOptions& options = {});

std::array<T, N> uniform_array(
    T minimum, T maximum, std::mt19937_64& engine,
    const GenerationOptions& options = {});
```

Genera un `std::array` fijo de `N` elementos con las mismas reglas de
distribución, duplicados y errores que `uniform_vector()`.

### `normal_vector<T>()` y `normal_array<T, N>()`

```cpp
std::vector<T> normal_vector(
    std::size_t count, T mean, T standard_deviation, std::uint64_t seed,
    const GenerationOptions& options = {});

std::vector<T> normal_vector(
    std::size_t count, T mean, T standard_deviation,
    std::mt19937_64& engine, const GenerationOptions& options = {});

std::array<T, N> normal_array(
    T mean, T standard_deviation, std::uint64_t seed,
    const GenerationOptions& options = {});

std::array<T, N> normal_array(
    T mean, T standard_deviation, std::mt19937_64& engine,
    const GenerationOptions& options = {});
```

Generan muestras de `std::normal_distribution<T>`; `T` debe ser de punto
flotante. `mean` debe ser finita y `standard_deviation` debe ser positiva y
finita. Si no se cumple, lanzan `std::invalid_argument`.

### Distribuciones discretas nombradas

Cada función vectorial tiene sobrecargas con semilla y `std::mt19937_64&`; la
forma array usa `template <std::size_t N>` y omite el parámetro `count`. Todas
aceptan `const GenerationOptions& options = {}`.

| Función | Parámetros de distribución | Contrato |
| --- | --- | --- |
| `bernoulli_vector` / `bernoulli_array` | `double probability` | Devuelve `std::size_t` 0/1; probabilidad en `[0,1]`. |
| `binomial_vector` / `binomial_array` | `std::size_t trials`, `double probability` | Éxitos de 0 a `trials`; `trials` cabe en `long long`, probabilidad en `[0,1]`. |
| `geometric_vector` / `geometric_array` | `double probability` | Intentos hasta primer éxito (soporte desde 1); probabilidad en `(0,1]`. Conteos fuera de `size_t` lanzan `std::overflow_error`. |
| `poisson_vector` / `poisson_array` | `double lambda` | Conteos con media `lambda >= 0`; `lambda=0` devuelve ceros. |
| `hypergeometric_vector` / `hypergeometric_array` | `population_size`, `success_count`, `draws` (`std::size_t`) | Éxitos al extraer sin reemplazo; exige `success_count <= population_size` y `draws <= population_size`. |
| `zipf_vector` / `zipf_array` | `double exponent`, `std::size_t max_rank` | Devuelve rangos `[1,max_rank]` con pesos `rank^-exponent`; exponente y rango positivos. |

Las funciones discretas devuelven `std::vector<std::size_t>` o
`std::array<std::size_t, N>`.

### Distribuciones continuas nombradas

Estas funciones requieren tipo de salida `T` de punto flotante. Las vectoriales
aceptan `count`; las de array especifican `N` como parámetro de plantilla. Cada
una ofrece semilla o motor compartido y `GenerationOptions`.

| Función | Parámetros | Dominio de parámetros / salida |
| --- | --- | --- |
| `exponential_vector<T>` / `exponential_array<T,N>` | `rate` | `rate > 0`; valores positivos. |
| `gamma_vector<T>` / `gamma_array<T,N>` | `shape`, `scale` | Ambos positivos; valores positivos. |
| `beta_vector<T>` / `beta_array<T,N>` | `alpha`, `beta` | Ambos positivos; valores en `(0,1)`. |
| `lognormal_vector<T>` / `lognormal_array<T,N>` | `log_mean`, `log_stddev` | Media logarítmica finita y desviación positiva; valores positivos. |
| `weibull_vector<T>` / `weibull_array<T,N>` | `shape`, `scale` | Ambos positivos; valores no negativos. |
| `pareto_vector<T>` / `pareto_array<T,N>` | `xmin`, `alpha` | Ambos positivos; cola no acotada desde `xmin`, los resultados no finitos se rechazan. |
| `power_law_vector<T>` / `power_law_array<T,N>` | `exponent`, `xmin`, `xmax` | Exponente positivo y `0 < xmin < xmax`; densidad proporcional a `x^-exponent` en ese intervalo. |

Un parámetro fuera del dominio o no finito lanza `std::invalid_argument`.

### Categóricas, mezclas y perfiles

- `categorical_vector<T>(count, categories, weights, seed_or_engine, options)` y
  `categorical_array<T,N>(categories, weights, seed_or_engine, options)` eligen
  categorías numéricas con pesos configurables. Los vectores deben ser no vacíos
  y de igual longitud; los pesos finitos y no negativos deben sumar más que cero.
- `MixtureComponent<T>` describe un sampler numérico y su peso.
  `make_mixture_component<T>(weight, distribution)` adapta una distribución
  invocable con `std::mt19937_64&`; `mixture_vector<T>` y
  `mixture_array<T,N>` componen una lista no vacía de esos samplers. Pesos
  inválidos o un sampler vacío lanzan `std::invalid_argument`.
- `constant_vector<T>(count, value)` y `constant_array<T,N>(value)` generan un
  valor fijo, sin semilla; el valor debe ser finito.
- `low_cardinality_vector` / `low_cardinality_array` reciben una paleta y pueden
  usar pesos uniformes o explícitos.
- `clustered_vector<T>` / `clustered_array<T,N>` reciben centros, desviaciones y
  pesos de igual longitud; construyen una mezcla de normales y requieren
  desviaciones positivas.
- `sparse_vector<T>(count, zero_probability, distribution, seed_or_engine,
  options)` y `sparse_array<T,N>(...)` agregan una masa puntual en cero; la
  probabilidad debe estar en `[0,1]`.
- Para alta cardinalidad, usa una distribución de soporte amplio con
  `DuplicatePolicy::forbid`; puede fallar si el dominio o `max_attempts` no
  permiten completar la muestra.

Estas funciones de composición conservan las opciones de duplicados/patrones y
el error por salidas no finitas del generador común.

### `generate_vector<T>()` y `generate_array<T, N>()`

```cpp
template <typename T, typename Distribution>
std::vector<T> generate_vector(
    std::size_t count, Distribution distribution, std::uint64_t seed,
    const GenerationOptions& options = {});

template <typename T, typename Distribution>
std::vector<T> generate_vector(
    std::size_t count, Distribution distribution, std::mt19937_64& engine,
    const GenerationOptions& options = {});

template <typename T, std::size_t N, typename Distribution>
std::array<T, N> generate_array(
    Distribution distribution, std::uint64_t seed,
    const GenerationOptions& options = {});

template <typename T, std::size_t N, typename Distribution>
std::array<T, N> generate_array(
    Distribution distribution, std::mt19937_64& engine,
    const GenerationOptions& options = {});
```

Usa estas funciones cuando necesites una distribución distinta de las
conveniencias uniforme/normal. `Distribution` debe poder invocarse con el motor
`std::mt19937_64&` y su resultado debe poder convertirse a `T`. Si produce un
valor no finito para el tipo generado, la llamada lanza
`std::invalid_argument`.

### `GenerationOptions`, `DuplicatePolicy` y `DataPattern`

`GenerationOptions` tiene estos campos:

| Campo | Predeterminado | Contrato |
| --- | --- | --- |
| `duplicates` | `DuplicatePolicy::allow` | `allow` conserva repeticiones; `forbid` exige valores únicos según igualdad exacta del tipo almacenado. |
| `pattern` | `DataPattern::random` | `random`, `sorted`, `reversed`, `nearly_sorted` o `repeated`. |
| `max_attempts` | `10000` | Máximo de extracciones al generar valores únicos; al agotarlo se lanza `std::runtime_error`. |

Para valores `float`/`double` la igualdad es exacta, sin tolerancia. La política
`forbid` condiciona el muestreo y puede alterar las probabilidades relativas.
`repeated` genera una sola extracción y la copia; por ello es incompatible con
`DuplicatePolicy::forbid` y lanza `std::invalid_argument` en esa combinación.
`nearly_sorted` ordena el resultado y luego realiza `max(1, count / 10)`
intercambios aleatorios cuando hay al menos dos valores.

Para importes o valores decimales exactos, genera enteros escalados y conviértelos
después: los valores de punto flotante representan aproximaciones finitas.

## Medición — `ntq::TimeMeasurement`

Header: `metrics/time_measurement.hpp`. Implementación de colectores: Linux.

### Construcción, movimiento y ciclo de vida

```cpp
explicit TimeMeasurement(long max_involuntary_context_switches = 0);
TimeMeasurement(TimeMeasurement&& other) noexcept;
TimeMeasurement& operator=(TimeMeasurement&& other) noexcept;
```

El constructor establece el umbral de cambios involuntarios que marcará una
muestra como sospechosa; un valor negativo lanza `std::invalid_argument`. La
clase es movible y no copiable, porque posee recursos de contadores de hardware.
El destructor libera dichos recursos.

Para una medición en el proceso actual:

```cpp
ntq::TimeMeasurement measurement(/* umbral opcional */);
measurement.start();
algorithm();
measurement.stop();
measurement.elapsed();
const auto elapsed =
    measurement.obtain_elapsed(ntq::TimeUnit::microseconds);
const auto memory = measurement.obtain_memory();
const auto hardware = measurement.obtain_hardware();
```

| Función | Contrato, parámetros y errores |
| --- | --- |
| `start()` | Captura snapshots y activa/reinicia el ciclo. Descarta resultados de una captura anterior. Fallos de lectura de recursos pueden lanzar `std::system_error`. |
| `stop()` | Captura los valores finales. Requiere que `start()` haya activado la medición; si no, lanza `std::logic_error`. |
| `elapsed()` | Calcula y guarda los deltas entre `start()` y `stop()`. Requiere una medición detenida; de lo contrario lanza `std::logic_error`. |
| `obtain_elapsed(TimeUnit unit) const` | Devuelve wall-clock, CPU user/system y cambios de contexto. Convierte los tiempos a `unit`, no consume el resultado y puede llamarse con varias unidades. Antes de `elapsed()` lanza `std::logic_error`; una unidad inválida lanza `std::invalid_argument`. |
| `obtain_memory() const` | Devuelve Peak RSS, page faults, DTLB y estados. Requiere `elapsed()`; si se llama antes, lanza `std::logic_error`. |
| `obtain_hardware() const` | Devuelve ciclos, instrucciones, IPC, contadores de caché, porcentajes y branch mispredictions con su disponibilidad. Requiere `elapsed()`; si se llama antes, lanza `std::logic_error`. |

`TimeUnit` admite `nanoseconds`, `microseconds`, `milliseconds`, `seconds`,
`minutes` y `hours`. Esta unidad solo afecta los tres tiempos de
`ElapsedMeasurements`; los contadores permanecen en sus unidades naturales. La
estructura retornada no conserva `unit`, así que el caller debe recordar el
argumento utilizado.

`stop()` también puede lanzar `std::system_error` si falla la captura final de
recursos.

### Funciones estáticas

```cpp
static std::chrono::nanoseconds reported_clock_resolution();

static TimeMeasurement measure_in_isolated_process(
    const std::function<void()>& algorithm,
    long max_involuntary_context_switches = 0);
```

- `reported_clock_resolution()` consulta la resolución reportada por Linux para
  `CLOCK_MONOTONIC`; no es una garantía de precisión. Si la consulta falla,
  lanza `std::system_error`.
- `measure_in_isolated_process()` ejecuta `algorithm` en un hijo `fork()` y
  devuelve un `TimeMeasurement` ya detenido y calculado, listo para consultar.
  El segundo parámetro es el umbral de cambios involuntarios y vale cero por
  defecto. El proceso llamador debe ser monohilo; los cambios de memoria del hijo
  no se reflejan en el padre. Fallos POSIX pueden lanzar `std::system_error` y
  fallos del callback/medición se propagan como `std::runtime_error`.

Los contadores de hardware pueden quedar no disponibles por soporte, permisos o
restricciones del sistema. Sus campos de estado deben consultarse antes de
interpretar el valor; no disponible no significa cero.

## Información del sistema — `ntq::collect_system_info()`

Header: `system/system_info.hpp`.

```cpp
SystemInfo collect_system_info();
```

No recibe parámetros. Devuelve los datos visibles para el proceso: arquitectura,
CPU/cachés, afinidad, frecuencias/gobernador, Turbo/boost, macros del compilador
y límites de sandbox/cgroups. Es de solo lectura: no fija afinidad ni cambia
frecuencia o gobernador. La recolección puede ser parcial.

Los campos se expresan como `CollectedValue<T>`:

- constructor por defecto: valor inicializado y estado `unavailable`;
- `CollectedValue(const T& value, CollectionStatus status,
  const std::string& source)`: establece valor, estado y origen;
- solo debe leerse `value` según el `CollectionStatus`;
- `unlimited` expresa que un límite no está configurado, no que sea cero.

Los estados posibles son `available`, `unavailable`, `permission_denied`,
`malformed`, `read_error` y `unlimited`. Esta función recolecta el contexto; no
configura el entorno para el benchmark.

## Estadísticas — `ntq::statistics::calculate_statistics()`

Header: `statistics/statistics.hpp`.

```cpp
StatisticsReport calculate_statistics(
    const std::vector<BenchmarkSample>& samples,
    const RunMetadata& metadata,
    const StatisticsOptions& options);
```

- `samples`: mediciones ya ejecutadas, cada una con ID, claves de grupo/variante,
  repetición, unidad, tiempos, memoria y hardware.
- `metadata`: contexto de la corrida, duración, reloj, versión y parámetros del
  algoritmo; contiene snapshots ambientales antes y después.
- `options`: warm-ups, política de muestras sospechosas, filtro IQR, bootstrap,
  umbrales de fiabilidad y solicitudes de comparación. Debe pasarse
  explícitamente; `StatisticsOptions{}` usa los valores predeterminados del
  header.

Campos de entrada principales:

| Tipo | Campos que completa el caller |
| --- | --- |
| `BenchmarkSample` | `sample_id`, `group_key`, `variant_key`, `pair_id`, `repetition_index`, `time_unit`, `elapsed`, `memory`, `hardware`. El ID vacío se genera desde la posición; `pair_id` vacío indica que no hay par. |
| `RunMetadata` | `run_id`, `configured_iterations`, `total_duration`, `clock_name`, `clock_resolution`, `library_version`, `algorithm_parameters`, `environment`. |
| `StatisticsOptions` | `warmup_runs_per_group` (0), `apply_iqr_filter` (true), `include_suspicious_samples` (true), `confidence_level` (0.95), `bootstrap_resamples` (10000), `bootstrap_seed` (42), `minimum_reliable_ci_samples` (30), `minimum_expected_tail_observations` (5) y `comparisons` (vacío). |

`confidence_level` debe ser finito y estar estrictamente entre 0 y 1;
`bootstrap_resamples` debe ser mayor que cero y `minimum_expected_tail_observations`
debe ser finito y no negativo. Las primeras `warmup_runs_per_group` muestras de
cada serie se identifican por `repetition_index`, no por el orden del vector.

Devuelve `StatisticsReport` con grupos, resúmenes, comparaciones, diagnósticos y
auditoría de exclusiones. No ejecuta algoritmos ni interpreta `group_key` como
tamaño de entrada. Lanza `std::invalid_argument` si las opciones son inválidas,
hay IDs/repeticiones/pares duplicados, se mezclan unidades dentro de un grupo o
las unidades de grupos comparados son incompatibles.

Los tiempos de todas las muestras de cada pareja grupo/variante deben usar la

Para solicitar comparación, agrega `ComparisonRequest` a
`StatisticsOptions::comparisons`:

- `group_key`: grupo a comparar;
- `baseline_variant`: variante base;
- `candidate_variant`: variante candidata;
- `pairing`: `automatic`, `require_paired` o `unpaired`.

La diferencia se calcula como candidato menos base. El modo automático utiliza
pares coincidentes cuando existen; de lo contrario usa muestras independientes.

## Complejidad empírica — `ntq::analysis::analyze_complexity()`

Header: `analysis/complexity.hpp`.

```cpp
std::vector<ComplexityFit> analyze_complexity(
    const std::vector<ComplexityObservation>& observations,
    const std::vector<ComplexityModel>& models);
```

- `observations`: puntos `(input_size, elapsed_time)`; los tiempos deben usar
  una unidad común.
- `models`: modelos a ajustar, en el orden deseado: `constant`, `logarithmic`,
  `square_root`, `linear`, `linearithmic`, `n_log_log_n`, `quadratic`,
  `quadratic_logarithmic`, `cubic`, `exponential_2` y `factorial`.

Cada salida conserva el modelo solicitado, número de observaciones, constante
empírica y R². El ajuste usa `elapsed_time ≈ c * f(N)` sin intercepto. La función
no ejecuta el algoritmo ni toma directamente `StatisticsReport`; el caller debe
asociar su tamaño de entrada a cada observación.

`AnalysisStatus` describe cada resultado:

- `available`: constante y R² calculados;
- `insufficient_samples`: menos de dos observaciones;
- `invalid_input`: tamaño cero, tiempo no positivo/no finito, base singular o
  cálculo no finito;
- `undefined_r_squared`: la constante se calculó, pero los tiempos tienen
  varianza cero; el R² no está definido.

Los resultados son ajustes empíricos sobre los datos entregados, no una prueba
de complejidad asintótica ni una selección automática del algoritmo ganador.
`n_log_log_n` requiere `input_size >= 3`; los modelos `exponential_2` y
`factorial` pueden quedar como `invalid_input` si la base o los cálculos exceden
el rango de `long double`.

## Guías detalladas

- [Generación de datos](data/random_data.md)
- [Medición de tiempo y recursos](metrics/time_measurement.hpp.md)
- [Información del sistema Linux](system/system_info.md)
- [Estadísticas de mediciones](statistics/statistics.md)
- [Análisis empírico de complejidad](analysis/complexity.md)
