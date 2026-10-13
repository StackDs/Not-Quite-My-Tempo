# `write_row()`

**Ubicación:** `src/C++/output/benchmark_csv.cpp` (helper privado).

## Contrato

```cpp
void write_row(std::ofstream &file, const std::vector<std::string> &fields);
```

`file` es el stream CSV abierto; `fields` son los campos de la fila. Los serializa separados por coma, escapando cada campo, y agrega salto de línea. Si el stream falla al escribir, lanza `std::runtime_error`.
