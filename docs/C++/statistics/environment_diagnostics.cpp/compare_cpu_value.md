# `compare_cpu_value()`

**Contrato**

```cpp
void compare_cpu_value(const CollectedValue<T> &before,
                       const CollectedValue<T> &after,
                       bool &changed, CollectionStatus &unknown_status);
```

Sobrecargas para `std::string` y `std::uint64_t`. Compara valores disponibles y actualiza `changed`; si falta información, registra el primer estado desconocido en `unknown_status`.
