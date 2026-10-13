# `apply_event()`

**Contrato**

```cpp
void apply_event(const PerfEventGroup &group, std::size_t index,
                 std::uint64_t &value, HardwareMetricStatus &status);
```

Lee evento `index` de `group`, escribiendo valor y estado en parámetros de salida. Índice fuera de rango marca `not_measured`; estados se traducen desde PerfEventStatus. No lanza.
