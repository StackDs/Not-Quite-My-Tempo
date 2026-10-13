# `unavailable_sandbox()`

**Contrato**

```cpp
SandboxInfo unavailable_sandbox(CollectionStatus status,
                                const std::string &source);
```

Construye sandbox donde versión, ruta y límites llevan el mismo estado `status` y fuente `source`. Los valores numéricos inicializados son placeholders que no deben interpretarse como mediciones disponibles.
