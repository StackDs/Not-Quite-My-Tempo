# `parse_cpu_list()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> parse_cpu_list(
    const std::string &text_value, const std::string &source);
```

Interpreta IDs/rangos separados por coma, como `0-3,6`. `text_value` es el contenido; `source` se conserva en el resultado. Devuelve lista ordenada y deduplicada con estado available o malformed; rangos invertidos, vacíos, inválidos o mayores a un millón de CPUs se rechazan como malformed.
