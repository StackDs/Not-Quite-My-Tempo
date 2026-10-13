# `parse_cache_size()`

**Contrato**

```cpp
CollectedValue<std::uint64_t> parse_cache_size(
    const std::string &text_value, const std::string &source);
```

Parsea tamaño de caché con sufijos B/K/KiB/M/MiB/G/GiB y devuelve bytes. `source` identifica el dato. Texto/sufijo desconocido o overflow da estado malformed.
