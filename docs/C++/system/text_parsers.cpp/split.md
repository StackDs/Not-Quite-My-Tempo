# `text::split()`

**Contrato**

```cpp
std::vector<std::string> split(const std::string &value, char delimiter);
```

Divide `value` usando `delimiter` y devuelve los campos. Conserva un campo vacío final cuando el texto termina en delimitador; una cadena vacía da vector vacío.
