# `constant_array()`

**Contrato**

```cpp
template <typename T, std::size_t N> std::array<T, N> constant_array(T value);
```

`N` fija el tamaño y `value` se copia en todas las posiciones; `T` debe ser numérico no-bool y `value` finito. Devuelve array; valor no finito lanza `std::invalid_argument`.
