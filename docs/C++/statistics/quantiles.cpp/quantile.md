# `quantile()`

**Contrato**

```cpp
long double quantile(std::vector<long double> values,
                     long double probability);
```

Calcula un cuantil lineal tipo 7. `values` se recibe por valor porque se particiona; `probability` expresa p entre 0 y 1. Vector vacío retorna 0; p≤0 retorna mínimo, p≥1 máximo. Devuelve `long double`.
