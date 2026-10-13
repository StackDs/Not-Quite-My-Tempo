# Funciones discretas

Cada función ofrece sobrecargas con seed o motor; vector usa `count`, array usa `N`, y `GenerationOptions` aplica reglas generales.

- [bernoulli_vector](bernoulli_vector.md), [bernoulli_array](bernoulli_array.md)
- [binomial_vector](binomial_vector.md), [binomial_array](binomial_array.md)
- [geometric_vector](geometric_vector.md), [geometric_array](geometric_array.md)
- [poisson_vector](poisson_vector.md), [poisson_array](poisson_array.md)
- [hypergeometric_vector](hypergeometric_vector.md), [hypergeometric_array](hypergeometric_array.md)

Helpers internos: [`validate_probability`](validate_probability.md), [`sample_discrete_vector`](sample_discrete_vector.md), [`sample_discrete_array`](sample_discrete_array.md), [`GeometricAttemptsDistribution::operator()`](GeometricAttemptsDistribution-call.md), [`HypergeometricDistribution::operator()`](HypergeometricDistribution-call.md).
