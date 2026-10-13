# `detail::checked_finite_parameter()`

**Contrato**

```cpp
template <typename T> T checked_finite_parameter(double value,
                                                 const char *name);
```

Valida finitud y representabilidad de `value` en `T`; `name` se usa en error. Devuelve conversión a `T`; inválido lanza `std::invalid_argument`.
