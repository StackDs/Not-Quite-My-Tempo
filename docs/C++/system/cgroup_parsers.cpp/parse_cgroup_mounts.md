# `parse_cgroup_mounts()`

**Contrato**

```cpp
std::vector<CgroupMount> parse_cgroup_mounts(const std::string &input);
```

Parsea mountinfo y devuelve mounts cgroup v1/v2 válidos con raíz, mount point y controladores reconocidos. `input` es el texto completo; las líneas no cgroup o mal formadas se ignoran.
