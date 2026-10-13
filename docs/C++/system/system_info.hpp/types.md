# Tipos de información del sistema

**Declaración:** `include/C++/system/system_info.hpp`.

- `CollectionStatus`: `available`, `unavailable`, `permission_denied`, `malformed`, `read_error` o `unlimited`.
- `CollectedValue<T>`: valor, estado y fuente. Leer `value` según su estado; `unlimited` no significa cero.
- `CacheInfo`, `CpuInfo`, `CpuFrequencyInfo` y `CpuRuntimeInfo`: cachés, identidad CPU, frecuencias, gobernador, boost y afinidad.
- `CompilerInfo`: familia/versión, estándar, perfil de optimización y arquitectura objetivo.
- `SandboxInfo`: cgroup, cuota/periodo de CPU, CPUs permitidas y límite de memoria.
- `SystemInfo`: agrega CPU, runtime, compilador y sandbox.

La obtención se realiza mediante [`collect_system_info()`](../system_info.cpp/collect_system_info.md), de forma de solo lectura y con disponibilidad parcial.
