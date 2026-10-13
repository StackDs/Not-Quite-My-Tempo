# `collected()` — valor genérico

**Contrato**

```cpp
template <typename T> std::string collected(const CollectedValue<T> &value);
```

`value` contiene dato, estado y fuente. Devuelve estado y fuente y serializa el dato solo cuando su estado es `available`. Evita presentar el valor inicial de un campo no disponible como una medición real.
