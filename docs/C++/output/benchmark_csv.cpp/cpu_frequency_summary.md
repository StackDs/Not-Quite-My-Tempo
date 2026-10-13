# `cpu_frequency_summary()`

**Contrato**

```cpp
std::string cpu_frequency_summary(
    const CollectedValue<std::vector<CpuFrequencyInfo>> &frequencies);
```

`frequencies` contiene los datos por CPU junto con su estado. Devuelve gobernador y frecuencias actual/mínima/máxima en una representación compacta de contexto CSV.
