# `compare_values()`

**Contrato**

```cpp
template <class T>
CollectedValue<bool> compare_values(const CollectedValue<T> &before,
                                    const CollectedValue<T> &after,
                                    const std::string &source);
```

Compara valores solo cuando ambos snapshots están disponibles. `source` etiqueta el resultado. Si falta un snapshot, devuelve falso placeholder con el estado faltante; no infiere cambio.
