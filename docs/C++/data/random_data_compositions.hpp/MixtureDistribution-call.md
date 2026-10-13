# `MixtureDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Selecciona componente según pesos y ejecuta su sampler con el mismo engine. Devuelve un valor `T`; consume el motor.
