# `BetaDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Muestrea dos gamma con engine y calcula su razón estable para Beta. Protege límites abiertos `(0,1)` mediante valores representables próximos. Resultado no finito se rechaza aguas arriba.
