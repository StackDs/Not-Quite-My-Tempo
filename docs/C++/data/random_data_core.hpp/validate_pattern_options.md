# `detail::validate_pattern_options()`

**Contrato**

```cpp
void validate_pattern_options(const GenerationOptions &options);
```

Valida compatibilidad entre patrón y parámetros: repeated no admite duplicados prohibidos; block size, run length y partial key width deben ser válidos cuando aplica. No modifica opciones; lanza `std::invalid_argument` si son incompatibles.
