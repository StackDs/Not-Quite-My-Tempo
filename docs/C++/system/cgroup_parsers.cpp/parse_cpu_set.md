# `parse_cpu_set()`

**Contrato**

```cpp
CollectedValue<std::vector<unsigned>> parse_cpu_set(
    const std::string &input, const std::string &source);
```

Interpreta una lista/rangos de CPUs y devuelve IDs junto con estado y fuente. `input` es texto de cpuset; `source` identifica el recurso. Delega el análisis a `parse_cpu_list()`.
