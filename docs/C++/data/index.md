# Datos

Las funciones de `ntq::data` generan datos numéricos reproducibles para experimentos. `random_data.hpp` es el header paraguas; las familias se implementan en headers especializados. Los generadores aceptan una semilla para una secuencia reproducible o un motor `std::mt19937_64&` para compartir estado.

## `random_data_core.hpp`

- [Índice completo de funciones y helpers](random_data_core.hpp/index.md)
- [`generate_seed`](random_data_core.hpp/generate_seed.md)
- [`generate_vector` con seed](random_data_core.hpp/generate_vector-seed.md)
- [`generate_vector` con motor](random_data_core.hpp/generate_vector-engine.md)
- [`generate_array` con seed](random_data_core.hpp/generate_array-seed.md)
- [`generate_array` con motor](random_data_core.hpp/generate_array-engine.md)
- [`uniform_vector` con seed](random_data_core.hpp/uniform_vector-seed.md)
- [`uniform_vector` con motor](random_data_core.hpp/uniform_vector-engine.md)
- [`uniform_array` con seed](random_data_core.hpp/uniform_array-seed.md)
- [`uniform_array` con motor](random_data_core.hpp/uniform_array-engine.md)
- [`normal_vector` con seed](random_data_core.hpp/normal_vector-seed.md)
- [`normal_vector` con motor](random_data_core.hpp/normal_vector-engine.md)
- [`normal_array` con seed](random_data_core.hpp/normal_array-seed.md)
- [`normal_array` con motor](random_data_core.hpp/normal_array-engine.md)

## Otras familias

- `random_data_discrete.hpp`: [`bernoulli_vector`](random_data_discrete.hpp/bernoulli_vector.md), [`bernoulli_array`](random_data_discrete.hpp/bernoulli_array.md), [`binomial_vector`](random_data_discrete.hpp/binomial_vector.md), [`binomial_array`](random_data_discrete.hpp/binomial_array.md), [`geometric_vector`](random_data_discrete.hpp/geometric_vector.md), [`geometric_array`](random_data_discrete.hpp/geometric_array.md), [`poisson_vector`](random_data_discrete.hpp/poisson_vector.md), [`poisson_array`](random_data_discrete.hpp/poisson_array.md), [`hypergeometric_vector`](random_data_discrete.hpp/hypergeometric_vector.md) y [`hypergeometric_array`](random_data_discrete.hpp/hypergeometric_array.md).
- [Funciones continuas](random_data_continuous.hpp/index.md).
- [Funciones de composición](random_data_compositions.hpp/index.md).
- [Funciones de perfiles](random_data_profiles.hpp/index.md).

Los helpers de implementación `detail` no forman una API estable; sus páginas, cuando existen, describen su papel interno y sus parámetros según el header.
