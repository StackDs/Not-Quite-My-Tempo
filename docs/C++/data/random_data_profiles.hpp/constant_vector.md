# `constant_vector()`

**Contrato**

```cpp
template <typename T> std::vector<T> constant_vector(std::size_t count, T value);
```

`count` es la longitud; `value` es el valor replicado y debe ser finito; `T` es numérico no-bool. Devuelve vector de `count` copias, incluso vacío si count es cero. Valor no finito lanza `std::invalid_argument`.
