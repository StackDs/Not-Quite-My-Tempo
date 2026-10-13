# `median()`

**Contrato**

```cpp
long double median(const std::vector<long double> &values);
```

Devuelve el cuantil p50 tipo 7 de `values`. El vector vacío devuelve 0 conforme al comportamiento de `quantile()`; no modifica la entrada.
