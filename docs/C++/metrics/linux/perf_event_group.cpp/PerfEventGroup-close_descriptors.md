# `PerfEventGroup::close_descriptors()`

**Contrato**

```cpp
void close_descriptors() noexcept;
```

Cierra cada descriptor válido y restablece el líder y flags de preparación/ejecución. Método interno de limpieza, idempotente y `noexcept`.
