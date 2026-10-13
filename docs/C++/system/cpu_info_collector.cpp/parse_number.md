# `parse_number()`

**Contrato**

```cpp
CollectedValue<std::uint64_t> parse_number(const std::string &value,
                                           const std::string &source);
```

Parsea unsigned decimal `value`, asociando `source`. Retorna available con el número o malformed si la sintaxis/rango no es válida.
