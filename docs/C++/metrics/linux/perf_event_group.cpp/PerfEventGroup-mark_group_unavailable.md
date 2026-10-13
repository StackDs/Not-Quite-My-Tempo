# `PerfEventGroup::mark_group_unavailable()`

**Contrato**

```cpp
void mark_group_unavailable(PerfEventStatus status) noexcept;
```

Asigna `status` a todos los resultados, deja valores en cero como placeholders y libera descriptores. `status` explica indisponibilidad; los valores no se consideran mediciones válidas.
