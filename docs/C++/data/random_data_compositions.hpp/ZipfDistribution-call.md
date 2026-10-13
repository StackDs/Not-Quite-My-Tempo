# `ZipfDistribution::operator()`

**Contrato**

```cpp
std::size_t operator()(std::mt19937_64 &engine);
```

Muestrea índice discreto con la distribución Zipf almacenada y desplaza resultado a rango desde 1. Consume `engine`; devuelve rank.
