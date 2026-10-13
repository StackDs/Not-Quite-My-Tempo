# `CollectedValue<T>::CollectedValue(value, status, source)`

**Declaración:** `include/C++/system/system_info.hpp`.

## Contrato

```cpp
CollectedValue(const T &collected_value, CollectionStatus collected_status,
               const std::string &collected_source);
```

Copia el valor `collected_value`, almacena su estado y copia la etiqueta `collected_source`. Los tres parámetros son explícitos; no valida coherencia entre estado y valor, por lo que el caller debe inspeccionar `status` antes de usarlo.
