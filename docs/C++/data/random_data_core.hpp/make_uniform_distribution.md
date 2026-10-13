# `detail::make_uniform_distribution()`

**Contrato**

```cpp
template <typename T>
std::uniform_int_distribution<T> make_uniform_distribution(
    T minimum, T maximum, std::true_type);
template <typename T>
std::uniform_real_distribution<T> make_uniform_distribution(
    T minimum, T maximum, std::false_type);
```

Elige distribución uniforme entera o real en compilación mediante el tag. `minimum`/`maximum` son límites previamente validados. Devuelve objeto de distribución sin consumir motor.
