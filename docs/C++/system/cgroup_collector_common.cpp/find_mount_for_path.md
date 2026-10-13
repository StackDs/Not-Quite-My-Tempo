# `find_mount_for_path()`

**Contrato**

```cpp
const CgroupMount *find_mount_for_path(
    const std::vector<CgroupMount> &mounts, unsigned version,
    const std::string &controller, const std::string &member_path);
```

Filtra mounts por versión/controlador y devuelve el que mejor mapea `member_path`, eligiendo la raíz más específica. Retorna `nullptr` si ninguno aplica; el puntero no posee el elemento.
