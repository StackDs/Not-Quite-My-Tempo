# `PerfEventGroup::PerfEventGroup()`

**Contrato**

```cpp
explicit PerfEventGroup(std::vector<PerfEventSpec> events);
```

Toma posesión de las especificaciones `events` por movimiento y prepara buffers/IDs/resultados del tamaño correspondiente. Aún no abre eventos; `prepare()` realiza esa operación. No devuelve valor.
