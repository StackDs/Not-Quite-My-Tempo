# `samples_by_pair()`

**Contrato**

```cpp
std::map<std::string, const BenchmarkSample *> samples_by_pair(
    const std::vector<const BenchmarkSample *> &samples);
```

Indexa las muestras con `pair_id` no vacío; los IDs vacíos se ignoran. Retorna mapa de punteros no propietarios que dependen de la vida de las muestras originales.
