# `collect_cpu_info()`

**Contrato**

```cpp
CpuCollection collect_cpu_info();
```

Combina `uname`, `/proc/cpuinfo`, CPUs online, caches y runtime por CPU en `CpuCollection`. No recibe argumentos ni modifica sistema; fallos de fuentes quedan en los estados de cada campo.
