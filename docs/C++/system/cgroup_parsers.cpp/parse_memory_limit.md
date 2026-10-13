# `parse_memory_limit()`

**Contrato**

```cpp
CollectedValue<std::uint64_t> parse_memory_limit(
    const std::string &input, const std::string &source);
```

Parsea un límite de memoria cgroup. `input` debe ser unsigned decimal o `max`; `source` identifica el archivo. Devuelve bytes disponibles, estado `unlimited` para `max` y sentinel v1 grande, o `malformed` para texto inválido.
