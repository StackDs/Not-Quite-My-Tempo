# `combined_status()`

**Contrato**

```cpp
HardwareMetricStatus combined_status(HardwareMetricStatus first,
                                     HardwareMetricStatus second);
```

Devuelve el primer estado no disponible; si el primero es `available`, devuelve el segundo. Se usa para propagar el estado de una métrica derivada que depende de dos contadores.
