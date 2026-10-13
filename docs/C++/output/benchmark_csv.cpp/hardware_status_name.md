# `hardware_status_name()`

**Contrato**

```cpp
std::string hardware_status_name(HardwareMetricStatus status);
```

Convierte `status`, el estado de disponibilidad de un contador hardware, a su etiqueta textual para CSV. No convierte estados no disponibles en valores numéricos.
