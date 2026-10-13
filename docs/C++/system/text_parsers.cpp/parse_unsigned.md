# `text::parse_unsigned()`

**Contrato**

```cpp
bool parse_unsigned(const std::string &text, std::uint64_t &value);
```

`text` debe contener un entero decimal sin signo, opcionalmente rodeado por espacios; `value` recibe el resultado solo si el parseo es válido. Devuelve `false` para vacío, signo negativo, caracteres no decimales o overflow. No lanza excepción por entrada inválida.
