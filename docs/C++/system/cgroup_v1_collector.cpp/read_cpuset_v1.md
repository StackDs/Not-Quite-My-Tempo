# `read_cpuset_v1()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> read_cpuset_v1(
    const std::vector<std::string> &paths);
```

Lee `cpuset.cpus` de los ancestros disponibles y resuelve su intersección efectiva mediante `resolve_effective_cpu_set()`. Devuelve lista con estado y fuente.
