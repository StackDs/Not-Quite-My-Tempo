# `detail::validate_probability()`

**Contrato**

```cpp
void validate_probability(double probability, bool allow_zero,
                          const char *name);
```

Valida probabilidad finita en `[0,1]` si `allow_zero`, o `(0,1]` si no. `name` etiqueta el error. Lanza `std::invalid_argument` fuera de dominio.
