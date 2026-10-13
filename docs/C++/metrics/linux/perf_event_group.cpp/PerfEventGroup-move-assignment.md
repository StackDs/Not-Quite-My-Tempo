# `PerfEventGroup::operator=(PerfEventGroup&&)`

**Contrato**

```cpp
PerfEventGroup &operator=(PerfEventGroup &&other) noexcept;
```

Libera recursos previos del destino, transfiere el estado desde `other` y devuelve `*this`. Gestiona autoasignación; `other` queda vacío. No lanza.
