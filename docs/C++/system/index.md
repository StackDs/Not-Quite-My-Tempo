# Sistema

`collect_system_info()` recopila el contexto visible para el proceso en Linux. Es de solo lectura: no cambia afinidad, gobernador, frecuencia ni límites del sandbox.

## `system_info.cpp`

- [`collect_system_info` (API)](system_info.cpp/collect_system_info.md)
- [`linux_system::collect_system_info`](linux/system_info.cpp/linux_system-collect_system_info.md)
- [`ntq::collect_system_info` (delegación)](linux/system_info.cpp/collect_system_info-public.md)

## Parsers y colectores internos

- [Tipos/constructores de `system_info.hpp`](system_info.hpp/index.md)
- [Parsers de texto](text_parsers.cpp/index.md)
- [Lectura de archivos](file_reader.cpp/index.md)
- [Parsers cgroup](cgroup_parsers.cpp/index.md)
- [Utilidades de rutas cgroup](cgroup_collector_common.cpp/index.md)
- [Metadatos del compilador](compiler_info.cpp/index.md)
- [Parser CPU/cachés](cpu_parsers.cpp/index.md)
- [Colector CPU](cpu_info_collector.cpp/index.md)
- [Colector de sandbox/cgroups](cgroup_info_collector.cpp/index.md), [v1](cgroup_v1_collector.cpp/index.md), [v2](cgroup_v2_collector.cpp/index.md)

Tipos de resultados y estados están declarados en `include/C++/system/system_info.hpp`.
