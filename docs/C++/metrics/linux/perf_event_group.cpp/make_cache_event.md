# `make_cache_event()`

**Contrato**

```cpp
PerfEventSpec make_cache_event(unsigned cache, unsigned operation,
                               unsigned result);
```

Combina cache, operación y resultado en la codificación `PERF_TYPE_HW_CACHE`. Devuelve especificación del evento; la disponibilidad se conoce al prepararlo.
