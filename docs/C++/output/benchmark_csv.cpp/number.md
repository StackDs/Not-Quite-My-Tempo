# `number()`

**Ubicación:** `src/C++/output/benchmark_csv.cpp` (template privado).

## Contrato

```cpp
template <typename T> std::string number(T value);
```

`value` es el número a serializar. Devuelve su representación textual con locale clásico y precisión máxima de `T`; no agrega escape CSV porque los campos numéricos no requieren comillas.
