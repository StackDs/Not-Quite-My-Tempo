# `HardwareCounters::apply()`

**Contrato**

```cpp
void HardwareCounters::apply(HardwareMeasurements &measurements) const noexcept;
```

Escribe valores y estados de cada evento en `measurements`. No deriva IPC/porcentajes; estos se calculan después con `calculate_hardware_derived_metrics()`. No lanza.
