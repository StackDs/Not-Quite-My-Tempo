# `PerfEventGroup::results()`

**Contrato**

```cpp
const std::vector<PerfEventResult> &results() const noexcept;
```

Devuelve referencia constante a los valores/estados de eventos. La referencia pertenece a la instancia y deja de ser válida al destruirla o moverla.
