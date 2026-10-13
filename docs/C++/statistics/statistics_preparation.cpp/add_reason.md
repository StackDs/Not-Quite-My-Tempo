# `add_reason()`

**Contrato**

```cpp
void add_reason(SampleAudit &audit, SampleExclusionReason reason);
```

Agrega `reason` a la auditoría mutable y marca `included_after_filters` como falso. No altera la muestra medida.
