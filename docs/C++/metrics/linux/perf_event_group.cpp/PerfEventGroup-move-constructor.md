# `PerfEventGroup::PerfEventGroup(PerfEventGroup&&)`

**Contrato**

```cpp
PerfEventGroup(PerfEventGroup &&other) noexcept;
```

Transfiere especificaciones, descriptores, IDs, resultados, buffer y estado desde `other`; deja al origen sin líder/ownership activo. Operación `noexcept`.
