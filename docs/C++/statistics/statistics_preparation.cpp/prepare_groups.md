# `prepare_groups()`

**Contrato**

```cpp
std::vector<PreparedGroup> prepare_groups(
    const std::vector<BenchmarkSample> &samples,
    const StatisticsOptions &options, ValidatedSamples &validated);
```

Ordena índices por repetición, aparta warm-ups y muestras sospechosas según política, rechaza wall-clock inválido y calcula la vista antes/después de IQR. `samples` permanece inmutable; `options` controla reglas; `validated` se actualiza con razones de auditoría. Devuelve grupos con cobertura y vistas filtradas.
