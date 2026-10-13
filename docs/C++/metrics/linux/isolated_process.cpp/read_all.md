# `read_all()`

**Ubicación:** `src/C++/metrics/linux/isolated_process.cpp` (helper privado).

## Contrato

```cpp
bool read_all(int descriptor, void *data, std::size_t size, int &error);
```

`descriptor` es el extremo de lectura; `data` es el buffer destino; `size` es la longitud esperada; `error` recibe `errno` si falla `read()` o cero ante cierre prematuro del pipe. Reintenta `EINTR`, retorna `true` solo tras leer todos los bytes y no lanza excepción.
