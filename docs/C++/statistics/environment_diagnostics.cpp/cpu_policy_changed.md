# `cpu_policy_changed()`

**Contrato**

```cpp
CollectedValue<bool> cpu_policy_changed(const CpuRuntimeInfo &before,
                                        const CpuRuntimeInfo &after);
```

Compara snapshots por CPU: cantidad/IDs, gobernador y frecuencias actuales/mínimas/máximas. Devuelve true ante cambio conocido; propaga disponibilidad desconocida si no puede concluir. No altera snapshots.
