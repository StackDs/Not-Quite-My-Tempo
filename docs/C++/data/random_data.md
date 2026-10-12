# Generación de datos de prueba

API: `include/C++/data/random_data.hpp`  
Namespace: `ntq::data`

El módulo genera vectores y arreglos numéricos para pruebas y benchmarks. Usa
`std::mt19937_64` como motor y permite guardar/reutilizar semillas. Para
reproducibilidad, usa los mismos parámetros, la misma semilla y el mismo entorno
de biblioteca estándar.

`random_data.hpp` es el header de entrada. La implementación se organiza además
en `random_data_core.hpp`, `random_data_discrete.hpp`,
`random_data_continuous.hpp`, `random_data_compositions.hpp` y
`random_data_profiles.hpp`, por responsabilidad.

## Semillas

`generate_seed()` crea una semilla de 64 bits desde `std::random_device`. Guarda
el resultado junto a los parámetros del benchmark para repetir una entrada:

```cpp
#include "data/random_data.hpp"

const std::uint64_t seed = ntq::data::generate_seed();
const auto values = ntq::data::uniform_vector<int>(1000, 0, 999, seed);
```

También se puede elegir una semilla fija:

```cpp
const auto repeatable = ntq::data::uniform_vector<double>(
    1000, -1.0, 1.0, 123456);
```

La API acepta un `std::mt19937_64&` para compartir una secuencia entre varias
generaciones. Una semilla reproducirá las secuencias bajo la misma
implementación de la biblioteca estándar; las distribuciones estándar pueden
producir secuencias diferentes entre compiladores/bibliotecas. `generate_seed()`
no es un generador criptográfico.

## Distribuciones y contenedores

- `uniform_vector<T>(count, minimum, maximum, seed_or_engine, options)` genera
  enteros en el intervalo cerrado `[minimum, maximum]` o valores reales en
  `[minimum, maximum)`.
- `uniform_array<T, N>(minimum, maximum, seed_or_engine, options)` genera un
  `std::array<T, N>` con las mismas reglas.
- `normal_vector<T>(count, mean, standard_deviation, seed_or_engine, options)`
  y `normal_array<T, N>(...)` generan valores normales para tipos de punto
  flotante; la desviación debe ser positiva y finita.
- `generate_vector<T>(count, distribution, seed_or_engine, options)` y
  `generate_array<T, N>(distribution, seed_or_engine, options)` aceptan una
  distribución compatible con `std::mt19937_64` cuyo resultado se pueda
  convertir a `T`.

`T` debe ser un tipo aritmético distinto de `bool`. Límites invertidos,
parámetros no finitos para distribuciones reales/normales, o un valor no finito
producido por una distribución personalizada generan `std::invalid_argument`.

Las funciones con nombre listadas abajo están disponibles para `std::vector` y
`std::array`. Las variantes vectoriales reciben `count`; las variantes de arreglo
usan `template <std::size_t N>` y no reciben count. Cada distribución aleatoria
acepta una semilla o un `std::mt19937_64&` compartido y, al final,
`const GenerationOptions& options = {}`.

### Distribuciones discretas

| Funciones vector/arreglo | Parámetros | Resultado |
| --- | --- | --- |
| `bernoulli_vector` / `bernoulli_array` | `probability` en `[0,1]` | `std::size_t` igual a 0 o 1. |
| `binomial_vector` / `binomial_array` | `trials`, `probability` en `[0,1]` | Conteo de éxitos de 0 a `trials`. `trials` debe caber en `long long`. |
| `geometric_vector` / `geometric_array` | `probability` en `(0,1]` | Intentos hasta el primer éxito, empezando en 1. `p=1` siempre produce 1. |
| `poisson_vector` / `poisson_array` | `lambda >= 0`, finito | Conteo de eventos; `lambda=0` produce ceros. |
| `hypergeometric_vector` / `hypergeometric_array` | `population_size=N`, `success_count=K`, `draws=n` | Éxitos en `n` extracciones sin reemplazo; requiere `K <= N` y `n <= N`. |
| `zipf_vector` / `zipf_array` | `exponent > 0`, `max_rank > 0` | Rangos discretos entre 1 y `max_rank`, con pesos proporcionales a `rank^-exponent`. |

Las distribuciones discretas devuelven `std::size_t`. La geométrica cuenta
intentos, no fallos previos al éxito; si el conteo no cabe en `std::size_t`,
lanza `std::overflow_error`. La hipergeométrica reinicia la población para cada
elemento del vector: cada salida es una observación independiente de la misma
extracción sin reemplazo.

### Distribuciones continuas adicionales

Estas funciones son plantillas de punto flotante (`float`, `double` o
`long double`):

| Funciones vector/arreglo | Parámetros | Soporte |
| --- | --- | --- |
| `exponential_vector<T>` / `exponential_array<T,N>` | `rate > 0`, finito | Valores positivos. |
| `gamma_vector<T>` / `gamma_array<T,N>` | `shape > 0`, `scale > 0` | Valores positivos. |
| `beta_vector<T>` / `beta_array<T,N>` | `alpha > 0`, `beta > 0` | Valores estrictamente entre 0 y 1. Si el redondeo llega a un extremo, se usa el valor representable más cercano dentro del intervalo. |
| `lognormal_vector<T>` / `lognormal_array<T,N>` | `log_mean` finito, `log_stddev > 0` | Valores positivos. Los parámetros describen la normal en el espacio logarítmico. |
| `weibull_vector<T>` / `weibull_array<T,N>` | `shape > 0`, `scale > 0` | Valores no negativos. |
| `pareto_vector<T>` / `pareto_array<T,N>` | `xmin > 0`, `alpha > 0` | Cola no acotada desde `xmin`; valores no finitos se rechazan. |

Las distribuciones continuas usan el parámetro de la forma indicada: `rate` para
la exponencial; `shape` y `scale` para gamma/Weibull; `alpha` y `beta` para Beta;
`log_mean` y `log_stddev` para log-normal; `xmin` y `alpha` para Pareto.

### Power law, categorías y mezclas

- `power_law_vector<T>` / `power_law_array<T,N>` recibe `exponent > 0` y finitos
  `0 < xmin < xmax`; muestrea una ley continua con densidad proporcional a
  `x^-exponent` dentro del intervalo. El caso `exponent == 1` usa la forma
  logarítmica de la inversa de la CDF.
- `categorical_vector<T>` / `categorical_array<T,N>` recibe dos vectores de igual
  longitud no vacía: categorías numéricas finitas y pesos `double`. Los pesos
  deben ser finitos, no negativos y al menos uno debe ser positivo; se normalizan
  internamente.
- `MixtureComponent<T>` contiene `weight` y un `sample` invocable con
  `std::mt19937_64&`. `make_mixture_component<T>(weight, distribution)` adapta
  distribuciones estándar o personalizadas cuyo resultado se convierte a `T`.
- `mixture_vector<T>` / `mixture_array<T,N>` recibe una lista no vacía de
  componentes; cada sampler debe existir, los pesos deben ser finitos/no
  negativos y su suma positiva. Cada salida elige primero un componente y luego
  toma su muestra usando el mismo motor.

Zipf modela rangos discretos y power law el dominio continuo acotado; Pareto
mantiene la cola continua no acotada.

### Perfiles de datasets

- `constant_vector<T>(count, value)` y `constant_array<T,N>(value)` rellenan con
  un valor numérico finito y no necesitan semilla.
- `low_cardinality_vector<T>` / `low_cardinality_array<T,N>` reciben una paleta
  numérica y asignan pesos iguales; la sobrecarga ponderada recibe además un
  vector de pesos y delega a la distribución categórica.
- `clustered_vector<T>` / `clustered_array<T,N>` reciben centros, desviaciones
  estándar y pesos de igual longitud no vacía. Cada componente es una normal con
  el centro/desviación correspondientes; requiere `T` de punto flotante.
- `sparse_vector<T>` / `sparse_array<T,N>` recibe `zero_probability` en `[0,1]` y
  una distribución para el resto de extracciones. Esta probabilidad añade una
  masa en cero; si la distribución no nula también produce cero, la frecuencia
  total de ceros puede ser mayor.
- Para alta cardinalidad, usa una distribución con soporte amplio junto con
  `DuplicatePolicy::forbid`. La unicidad depende de que el dominio y
  `max_attempts` permitan completarla.

## Duplicados y patrones

`GenerationOptions` controla el comportamiento:

```cpp
ntq::data::GenerationOptions options;
options.duplicates = ntq::data::DuplicatePolicy::forbid;
options.max_attempts = 100000;
options.pattern = ntq::data::DataPattern::sorted;
const auto unique = ntq::data::uniform_vector<int>(100, 1, 1000, 42, options);
```

- `DuplicatePolicy::allow` es el valor predeterminado.
- `DuplicatePolicy::forbid` conserva valores únicos según igualdad exacta del
  tipo almacenado. Para punto flotante no se usa tolerancia aproximada.
- `max_attempts` limita el total de extracciones al pedir valores únicos. Si no
  se consigue la cantidad solicitada, se lanza `std::runtime_error`. Para
  uniformes enteras, un rango demasiado pequeño se detecta antes y lanza
  `std::invalid_argument`.
- El muestreo sin duplicados está condicionado y no equivale a muestras
  independientes sin restricción. En tipos de punto flotante, la precisión
  finita puede causar colisiones.
- `DataPattern::random` conserva el orden de generación; `sorted` ordena
  ascendentemente; `reversed` ordena descendentemente; `nearly_sorted` ordena y
  realiza `max(1, count / 10)` intercambios aleatorios si hay al menos dos
  valores; `repeated` rellena el contenedor con una sola extracción.
- `repeated` y `DuplicatePolicy::forbid` son opciones incompatibles.

Los números de punto flotante son aproximaciones representables y no garantizan
decimales exactos. Para importes u otros valores decimales exactos, genera
enteros escalados (por ejemplo, centavos) y conviértelos después.

Prueba manual desde la raíz del repositorio:

```sh
g++ -std=c++14 -Wall -Wextra -Wpedantic -Werror -Iinclude/C++ \
  tests/C++/data/random_data_tests.cpp -o /tmp/random_data_tests
/tmp/random_data_tests
```
