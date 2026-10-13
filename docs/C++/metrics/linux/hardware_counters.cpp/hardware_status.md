# `hardware_status()`

**Contrato**

```cpp
HardwareMetricStatus hardware_status(PerfEventStatus status);
```

Traduce el estado común de un evento PMU al estado público de métrica hardware. `status` es entrada; devuelve equivalente y clasifica estado no reconocido como read error.
