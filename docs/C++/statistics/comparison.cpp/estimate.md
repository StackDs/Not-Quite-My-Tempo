# `estimate()`

**Contrato**

```cpp
long double estimate(const std::vector<long double> &values,
                     BootstrapEstimator estimator);
```

Selecciona media o mediana según `estimator` y devuelve el estimador de `values`. Helper interno del bootstrap de comparaciones.
