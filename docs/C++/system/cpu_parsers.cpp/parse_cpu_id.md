# `parse_cpu_id()`

**Contrato**

```cpp
bool parse_cpu_id(const std::string &text_value, unsigned &value);
```

Parsea el identificador unsigned `text_value`, escribe `value` si cabe en `unsigned` y retorna true; texto inválido u overflow retorna false. No lanza.
