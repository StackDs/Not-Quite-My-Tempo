# `append_confidence_interval_header()`

**Contrato**

```cpp
void append_confidence_interval_header(std::vector<std::string> &header,
                                       const std::string &prefix);
```

Agrega a `header` los nombres de columnas de un intervalo de confianza usando `prefix` para distinguir estimadores. Modifica solo el vector de encabezado; no calcula datos.
