# `collect_cpu_runtime()`

**Contrato**

```cpp
CpuRuntimeInfo collect_cpu_runtime(const std::vector<unsigned> &cpus);
```

Recoge afinidad, estado boost y por CPU governor/frecuencias actuales y límites. `cpus` es la lista objetivo; se consulta sin modificar. Devuelve campos con estados independientes y fuente sysfs.
