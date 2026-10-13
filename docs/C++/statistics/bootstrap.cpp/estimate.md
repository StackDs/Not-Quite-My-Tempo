# `estimate()`

**Contrato**

```cpp
long double estimate(const std::vector<long double> &values,
                     BootstrapEstimator estimator);
```

Calcula la media aritmética o mediana según `estimator` para un remuestreo bootstrap. `values` es la réplica y `estimator` selecciona el estimador central; devuelve `long double`.
