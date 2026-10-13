# `sample_standard_deviation()`

**Contrato**

```cpp
long double sample_standard_deviation(
    const std::vector<long double> &values);
```

Calcula desviación estándar muestral con denominador `n-1`. Con menos de dos elementos devuelve 0; no modifica la entrada.
