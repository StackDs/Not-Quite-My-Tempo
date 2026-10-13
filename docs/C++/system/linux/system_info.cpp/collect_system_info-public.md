# `ntq::collect_system_info()` — delegación

**Ubicación:** `src/C++/system/linux/system_info.cpp`.

## Contrato

```cpp
SystemInfo ntq::collect_system_info();
```

No recibe argumentos. Delega a `linux_system::collect_system_info()` y devuelve el resultado sin transformarlo. Es el punto de entrada público; el detalle de cada fuente de información pertenece a los colectores Linux.
