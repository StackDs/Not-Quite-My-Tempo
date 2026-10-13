# `TlbMissCounters::operator=(TlbMissCounters&&)`

**Contrato**

```cpp
TlbMissCounters &operator=(TlbMissCounters &&other) noexcept;
```

Transfiere el grupo PMU desde `other` y devuelve `*this`. Operación no lanzable.
