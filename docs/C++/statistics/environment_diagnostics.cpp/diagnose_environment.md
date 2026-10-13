# `diagnose_environment()`

**Contrato**

```cpp
EnvironmentDiagnostics diagnose_environment(
    const MeasurementContext &context);
```

Deriva deltas de procesos runnable y temperatura y flags de cambios en afinidad, política/frecuencia CPU y throttling a partir de snapshots antes/después. `context` es de solo lectura. Devuelve indicadores con estados y no corrige ni invalida muestras.
