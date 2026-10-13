# `ParetoDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Muestrea transformación inversa Pareto con parámetros almacenados `xmin` y `alpha`. `engine` se consume; devuelve valor en cola desde `xmin`.
