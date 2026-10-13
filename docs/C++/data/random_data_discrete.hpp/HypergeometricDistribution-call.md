# `HypergeometricDistribution::operator()`

**Contrato**

```cpp
result_type operator()(std::mt19937_64 &engine);
```

Realiza `draws` extracciones sin reemplazo de la población descrita por `population_size` y `success_count`. Devuelve cantidad de éxitos; consume el motor. Los parámetros se validan al construir/usar el distribuidor público.
