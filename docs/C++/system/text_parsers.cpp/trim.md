# `text::trim()`

**Ubicación:** `src/C++/system/linux/text_parsers.cpp`.

## Contrato

```cpp
std::string trim(const std::string &value);
```

`value` es texto de entrada. Devuelve una copia sin espacios de locale al principio ni al final; el contenido interior permanece sin cambios. No modifica el argumento.
