# `PositiveOutputDistribution::operator()`

**Contrato**

```cpp
T operator()(std::mt19937_64 &engine);
```

Invoca la distribución interna con `engine`; si la salida finita es no positiva, la sustituye por el siguiente representable positivo. Devuelve valor; la verificación general de finitud se realiza en el generador común.
