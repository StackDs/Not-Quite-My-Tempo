# `sample_id_or_index()`

**Contrato**

```cpp
std::string sample_id_or_index(const BenchmarkSample &sample,
                               std::size_t index);
```

Devuelve `sample.sample_id` si está definido; de lo contrario produce `sample-<index>`. `sample` es de solo lectura e `index` es la posición original usada como fallback.
