# `detail::validate_uniform_bounds()`

**Contrato**

```cpp
template <typename T>
void validate_uniform_bounds(T minimum, T maximum, std::true_type);
void validate_uniform_bounds(T minimum, T maximum, std::false_type);
```

Valida límite mínimo/máximo según si `T` es integral. Integers exigen orden; reales exigen además finitud. El tag selecciona overload; incumplimiento lanza `std::invalid_argument`.
