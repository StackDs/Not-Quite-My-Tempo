# `path_is_within()`

**Contrato**

```cpp
bool path_is_within(const std::string &path, const std::string &root);
```

Comprueba si `path` coincide con `root` o es descendiente por límite de directorio. `root == "/"` acepta rutas absolutas. Devuelve booleano sin modificar rutas.
