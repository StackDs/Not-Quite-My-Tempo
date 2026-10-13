# `append_descriptive()`

**Contrato**

```cpp
void append_descriptive(std::vector<std::string> &row,
                        const statistics::DescriptiveStatistics &summary);
```

Serializa en `row` los campos de `summary` (conteo, estado, estimadores, percentiles, intervalos y fiabilidad). No cambia el resumen.
