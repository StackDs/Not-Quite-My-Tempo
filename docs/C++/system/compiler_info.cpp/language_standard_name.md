# `language_standard_name()`

**Contrato**

```cpp
std::string language_standard_name();
```

No recibe parámetros. Devuelve el estándar detectado mediante `__cplusplus` y el prefijo `c` o `gnu` según `__STRICT_ANSI__`.
