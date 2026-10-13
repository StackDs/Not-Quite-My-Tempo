# `make_hardware_event()`

**Contrato**

```cpp
PerfEventSpec make_hardware_event(std::uint64_t config);
```

Construye especificación `PERF_TYPE_HARDWARE` con el `config` de evento recibido. Devuelve valor; no abre descriptores.
