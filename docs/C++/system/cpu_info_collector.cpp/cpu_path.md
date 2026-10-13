# `cpu_path()`

**Contrato**

```cpp
std::string cpu_path(unsigned cpu);
```

Devuelve la ruta sysfs `/sys/devices/system/cpu/cpu<N>` para el ID recibido. No consulta el sistema.
