# `join_path()`

**Contrato**

```cpp
std::string join_path(const std::string &directory, const std::string &name);
```

Une `name` al `directory` normalizado, evitando doble barra para la raíz. Devuelve la ruta; no consulta el sistema de archivos.
