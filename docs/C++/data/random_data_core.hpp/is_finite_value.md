# `detail::is_finite_value()`

**Contrato**

```cpp
template <typename T> bool is_finite_value(T value);
```

Convierte `value` a `long double` y comprueba finitud. Devuelve booleano; no modifica ni valida rango adicional.
