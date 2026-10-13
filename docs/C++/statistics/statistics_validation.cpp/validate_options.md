# `validate_options()`

**Contrato**

```cpp
void validate_options(const StatisticsOptions &options);
```

Valida que el nivel de confianza sea finito y entre 0 y 1, los remuestreos bootstrap sean positivos y el mínimo esperado en cola sea finito y no negativo. `options` es de solo lectura. Lanza `std::invalid_argument` en caso inválido.
