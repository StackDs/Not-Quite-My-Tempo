# `throttling_observed()`

**Contrato**

```cpp
CollectedValue<bool> throttling_observed(
    const CollectedValue<bool> &before,
    const CollectedValue<bool> &after);
```

Devuelve disponible/true si cualquiera de los snapshots informa throttling; disponible/false si ambos se conocen y ninguno indica throttling; en otro caso preserva el estado desconocido.
