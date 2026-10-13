# `detail::to_array()`

**Contrato**

```cpp
template <typename T, std::size_t N>
std::array<T, N> to_array(std::vector<T> values);
```

Copia los primeros `N` elementos del vector a un array fijo. Se usa tras generar una cantidad que ya coincide con `N`; no valida longitud por sí misma.
