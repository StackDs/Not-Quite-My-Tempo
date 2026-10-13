# `normalized_path()`

**Contrato**

```cpp
std::string normalized_path(std::string path);
```

Normaliza rutas de cgroup: cadena vacía se convierte en `/` y se eliminan barras finales salvo en la raíz. Recibe y devuelve por valor.
