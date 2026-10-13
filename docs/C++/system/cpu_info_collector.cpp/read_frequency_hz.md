# `read_frequency_hz()`

**Contrato**

```cpp
CollectedValue<std::uint64_t> read_frequency_hz(const std::string &path);
```

Lee frecuencia sysfs en kHz y la convierte a Hz. Conserva estado/fuente de lectura; texto inválido o multiplicación desbordada se marca malformed.
