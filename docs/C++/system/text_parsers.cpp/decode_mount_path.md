# `text::decode_mount_path()`

**Contrato**

```cpp
std::string decode_mount_path(std::string path);
```

Decodifica secuencias octales de mountinfo (`\040`, `\011`, `\012`, `\134`) a espacio, tab, newline y barra invertida. Recibe una copia por valor y devuelve la ruta decodificada.
