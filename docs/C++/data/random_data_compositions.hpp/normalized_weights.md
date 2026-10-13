# `detail::normalized_weights()`

**Contrato**

```cpp
std::vector<double> normalized_weights(
    const std::vector<double> &weights, const char *name);
```

Valida vector no vacío, pesos finitos/no negativos y total positivo; escala por el máximo para evitar overflow y retorna pesos normalizados. `name` etiqueta errores; entradas inválidas lanzan `std::invalid_argument`.
