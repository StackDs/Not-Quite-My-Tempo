# `find_mount()`

**Contrato**

```cpp
const CgroupMount *find_mount(const std::vector<CgroupMount> &mounts,
                              unsigned version,
                              const std::string &controller);
```

Busca el primer mount de `version` que incluye `controller`; para v2 se ignora el controlador. Retorna puntero no propietario al elemento o `nullptr` si no hay coincidencia.
