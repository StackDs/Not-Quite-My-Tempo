# `detail::checked_positive_parameter()`

**Contrato**

```cpp
template <typename T> T checked_positive_parameter(double value,
                                                   const char *name);
```

Valida finitud, rango representable y positividad de `value` para `T`; `name` etiqueta la excepción. Devuelve valor convertido o lanza `std::invalid_argument`.
