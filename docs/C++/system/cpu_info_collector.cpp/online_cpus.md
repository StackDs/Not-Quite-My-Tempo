# `online_cpus()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> online_cpus();
```

Lee `/sys/devices/system/cpu/online` y delega su formato a `parse_cpu_list()`. Devuelve CPUs online y estado/fuente; propaga fallos de lectura.
