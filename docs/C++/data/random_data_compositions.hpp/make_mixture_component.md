# `make_mixture_component()`

**Contrato**

```cpp
template <typename T, typename Distribution>
MixtureComponent<T> make_mixture_component(double weight,
                                           Distribution distribution);
```

`weight` debe ser finito y no negativo; `distribution` debe aceptar `std::mt19937_64&` y producir un tipo convertible a `T`; `T` numérico no-bool. Devuelve un componente con sampler adaptado al motor. Peso inválido lanza `std::invalid_argument`.
