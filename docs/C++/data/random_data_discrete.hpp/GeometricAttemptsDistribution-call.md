# `GeometricAttemptsDistribution::operator()`

**Contrato**

```cpp
std::size_t operator()(std::mt19937_64 &engine);
```

Muestrea fallos geométricos y devuelve intentos sumando uno. `engine` se consume; desbordamiento del resultado en `size_t` lanza `std::overflow_error`.
