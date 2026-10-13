# `csv_escape()`

**Ubicación:** `src/C++/output/benchmark_csv.cpp` (helper privado).

## Contrato

```cpp
std::string csv_escape(const std::string &value);
```

`value` es un campo textual CSV. Devuelve una cadena escapada: duplica barras invertidas y comillas, representa CR/LF como secuencias literales y entrecomilla campos que contienen delimitadores o comillas. No modifica el original.
