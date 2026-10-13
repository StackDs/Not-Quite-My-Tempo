# `exclusion_reason_name()`

**Contrato**

```cpp
std::string exclusion_reason_name(statistics::SampleExclusionReason reason);
```

`reason` es la regla de exclusión de una muestra (warm-up, sospecha, outlier IQR o tiempo inválido). Devuelve el nombre de la regla para su columna de auditoría.
