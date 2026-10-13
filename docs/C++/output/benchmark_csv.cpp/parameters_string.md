# `parameters_string()`

**Contrato**

```cpp
std::string parameters_string(
    const std::vector<std::pair<std::string, std::string>> &parameters);
```

`parameters` son pares nombre/valor de configuración. Devuelve una representación concatenada para metadatos CSV; el escape de campos se aplica al escribir la fila.
