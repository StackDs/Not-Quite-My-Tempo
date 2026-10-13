# `parse_cgroup_membership()`

**Contrato**

```cpp
CgroupMembership parse_cgroup_membership(const std::string &input);
```

Interpreta el texto de `/proc/self/cgroup`. `input` contiene líneas de jerarquías; devuelve rutas de controladores y ruta unificada cgroup v2, decodificando escapes de mount path. Las líneas mal formadas se omiten.
