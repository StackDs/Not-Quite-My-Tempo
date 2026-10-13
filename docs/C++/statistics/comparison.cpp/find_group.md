# `find_group()`

**Contrato**

```cpp
const PreparedGroup *find_group(const std::vector<PreparedGroup> &groups,
                                const std::string &group_key,
                                const std::string &variant_key);
```

Busca la combinación de clave de grupo y variante. Devuelve puntero no propietario o `nullptr` si no existe; el puntero depende de la vida de `groups`.
