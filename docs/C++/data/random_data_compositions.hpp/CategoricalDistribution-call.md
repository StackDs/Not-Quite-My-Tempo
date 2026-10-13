# `CategoricalDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Muestrea índice con distribución discreta ponderada y retorna categoría correspondiente. Consume engine; los vectores se validan al construir.
