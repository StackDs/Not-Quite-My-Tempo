# `PerfEventGroup::prepare()`

**Contrato**

```cpp
void prepare() noexcept;
```

Reinicia el grupo y abre eventos PMU deshabilitados, guardando IDs y líder. Fallos de permisos/soporte/lectura se guardan en `PerfEventResult` en lugar de lanzarse. Con build sin `perf_event_open`, marca eventos unsupported.
