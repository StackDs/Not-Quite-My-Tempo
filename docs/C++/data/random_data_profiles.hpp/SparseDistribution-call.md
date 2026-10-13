# `SparseDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Con probabilidad almacenada devuelve cero; de lo contrario invoca sampler no nulo. Consume engine y devuelve valor de tipo `T`.
