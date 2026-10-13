# `detail::domain_can_hold()`

**Contrato**

```cpp
template <typename T>
bool domain_can_hold(std::size_t count, T minimum, T maximum,
                     std::true_type /* integral */);
template <typename T>
bool domain_can_hold(std::size_t count, T minimum, T maximum,
                     std::false_type /* floating point */);
```

Para tipos integrales determina si el intervalo inclusivo contiene al menos `count` valores distintos. Para tipos no integrales devuelve true y deja la limitación de unicidad al muestreador. El tag de tipo selecciona overload.
