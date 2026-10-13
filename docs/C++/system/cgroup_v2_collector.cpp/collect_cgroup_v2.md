# `collect_cgroup_v2()`

**Contrato**

```cpp
SandboxInfo collect_cgroup_v2(const CgroupMount &mount,
                              const std::string &member_path);
```

Mapea ruta miembro al mount, calcula ancestors y recolecta cuota CPU, memoria y cpuset efectivos. `mount` describe montaje; `member_path` viene de membership. Si no se puede mapear, devuelve estado malformed; de lo contrario status por recurso.
