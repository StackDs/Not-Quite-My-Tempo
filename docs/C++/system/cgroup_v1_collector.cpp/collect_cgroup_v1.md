# `collect_cgroup_v1()`

**Contrato**

```cpp
SandboxInfo collect_cgroup_v1(const CgroupMembership &membership,
                              const std::vector<CgroupMount> &mounts);
```

Busca controladores y rutas v1, inspecciona ancestros de CPU/memoria/cpuset y construye límites efectivos en `SandboxInfo`. `membership` y `mounts` son entrada de solo lectura; ausencia/malformación queda en estados. No aplica cuotas.
