# `append_confidence_interval()`

**Contrato**

```cpp
void append_confidence_interval(std::vector<std::string> &row,
                                const statistics::ConfidenceInterval &ci);
```

Agrega a `row` límites, método, parámetros y estado de `ci`. El vector es salida mutable; el intervalo es entrada de solo lectura.
