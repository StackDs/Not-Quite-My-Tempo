# `number()`

**Contrato**

```cpp
MetricObservation number(long double value, MetricValueStatus status);
```

Empaqueta valor y estado. Si se marca disponible pero es negativo o no finito, cambia estado a `invalid_value` y valor a cero placeholder. No confundir placeholder con una medición cero.
