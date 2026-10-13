# `append_environment_diagnostics()`

**Contrato**

```cpp
void append_environment_diagnostics(
    std::vector<std::string> &row,
    const statistics::StatisticsReport &report);
```

Agrega a la fila mutable los diagnósticos de cambios ambientales incluidos en `report`. No vuelve a recolectar el sistema ni altera el reporte.
