# `status_from_errno()`

**Contrato**

```cpp
CollectionStatus status_from_errno(int error);
```

Mapea un `errno` de lectura a estado de colección: permisos para `EACCES`/`EPERM`; no disponible para rutas o capacidades ausentes; `read_error` para otros fallos. No recibe estado global.
