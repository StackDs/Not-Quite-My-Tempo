# `PowerLawDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Muestrea la distribución power-law truncada con `xmin`, `xmax` y exponente almacenados, usando transformación logarítmica estable. Consume engine y devuelve valor acotado.
