# `detail::make_mixture_distribution()`

**Contrato**

```cpp
template <typename T>
MixtureDistribution<T> make_mixture_distribution(
    const std::vector<MixtureComponent<T>> &components);
```

Valida lista no vacía, samplers presentes y pesos finitos/no negativos con suma positiva. Devuelve distribuidor de mezcla; componentes inválidos lanzan `std::invalid_argument`.
