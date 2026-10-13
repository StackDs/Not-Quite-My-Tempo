# `calculate_hardware_derived_metrics()`

**Contrato**

```cpp
void calculate_hardware_derived_metrics(HardwareMeasurements &measurements);
```

Modifica `measurements` calculando IPC y porcentajes de cache misses si los contadores base están disponibles y denominadores no son cero. En caso contrario asigna estado de unavailable o `invalid_denominator`. No lanza.
