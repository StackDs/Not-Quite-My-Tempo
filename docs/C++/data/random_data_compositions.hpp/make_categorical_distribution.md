# `detail::make_categorical_distribution()`

**Contrato**

```cpp
template <typename T>
CategoricalDistribution<T> make_categorical_distribution(
    const std::vector<T> &categories, const std::vector<double> &weights);
```

Requiere categorías y pesos de misma longitud no vacía, categorías finitas, pesos no negativos finitos con suma positiva y cantidad representable. Devuelve sampler ponderado; entrada inválida lanza `std::invalid_argument`.
