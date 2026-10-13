# `write_all()`

**Ubicación:** `src/C++/metrics/linux/isolated_process.cpp` (helper privado).

## Contrato

```cpp
bool write_all(int descriptor, const void *data, std::size_t size);
```

`descriptor` es el extremo de escritura; `data` señala los bytes; `size` es su longitud. Escribe hasta completar el buffer y reintenta si `write()` se interrumpe por `EINTR`. Devuelve `false` ante error permanente o escritura de cero bytes; no lanza excepción.
