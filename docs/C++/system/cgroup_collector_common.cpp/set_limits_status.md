# `set_limits_status()`

**Contrato**

```cpp
void set_limits_status(SandboxInfo &sandbox, CollectionStatus status,
                       const std::string &source);
```

Asigna el mismo `status` y `source` a las métricas efectivas de cuota, periodo, cores, CPUs permitidas y memoria de `sandbox`. Usa valores inicializados como placeholder; el estado determina si son utilizables.
