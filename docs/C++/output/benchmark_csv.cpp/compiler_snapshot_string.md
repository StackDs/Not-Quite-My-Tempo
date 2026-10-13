# `compiler_snapshot_string()`

**Contrato**

```cpp
std::string compiler_snapshot_string(const SystemInfo &system);
```

Serializa desde `system.compiler` familia, versión, estándar, perfil de optimización y arquitectura objetivo. Campos faltantes conservan su estado de colección.
