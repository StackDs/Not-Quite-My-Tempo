# `read_text_file()`

**Contrato**

```cpp
CollectedValue<std::string> read_text_file(const std::string &path);
```

Lee el archivo indicado por `path` en modo solo lectura y devuelve su contenido, estado y ruta como fuente. Reintenta lecturas interrumpidas. Errores se representan en `CollectionStatus` en vez de propagarse como excepción.
