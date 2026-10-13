# `collect_sandbox_info()`

**Contrato**

```cpp
SandboxInfo collect_sandbox_info();
```

Lee membership y mountinfo cgroup, determina versión/montaje y delega en colector v1 o v2. Si falta entrada o no hay mount usable, devuelve campos unavailable con su fuente. No cambia límites.
