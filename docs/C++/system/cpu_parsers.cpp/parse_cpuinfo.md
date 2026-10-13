# `parse_cpuinfo()`

**Contrato**

```cpp
CpuIdentity parse_cpuinfo(const std::string &text_value);
```

Busca fabricante y modelo en texto de `/proc/cpuinfo`, incluyendo nombres de campos de x86/ARM. Campos ausentes retornan `unavailable`; no lanza por líneas mal formadas.
