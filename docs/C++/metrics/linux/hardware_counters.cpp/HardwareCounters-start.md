# `HardwareCounters::start()`

**Contrato**

```cpp
void HardwareCounters::start() noexcept;
```

Inicia los grupos de contadores de ciclos/instrucciones, caches y branch misses. No lanza; estados PMU comunican fallos.
