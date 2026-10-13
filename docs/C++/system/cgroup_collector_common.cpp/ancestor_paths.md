# `ancestor_paths()`

**Contrato**

```cpp
std::vector<std::string> ancestor_paths(const std::string &leaf,
                                        const std::string &mount_point);
```

Devuelve la ruta hoja y sus padres hasta `mount_point`. `leaf` y `mount_point` se normalizan; si la hoja no pertenece al punto de montaje, retorna vector vacío.
