# `map_cgroup_path()`

**Contrato**

```cpp
bool map_cgroup_path(const CgroupMount &mount, const std::string &member,
                     std::string &mapped);
```

Mapea la ruta de miembro `member` dentro de `mount` a una ruta bajo el punto de montaje. `mapped` es parámetro de salida y se escribe solo si la raíz del mount contiene el miembro. Devuelve `false` si no puede mapearlo.
