# `read_cpuset_v2()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> read_cpuset_v2(
    const std::vector<std::string> &paths);
```

Prefiere el primer `cpuset.cpus.effective` disponible; si no, lee cpusets configurados y resuelve intersección de ancestros. Errores de permisos/lectura/malformación se conservan.
