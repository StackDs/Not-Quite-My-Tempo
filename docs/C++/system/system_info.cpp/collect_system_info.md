# `ntq::collect_system_info()`

**Declaración:** `include/C++/system/system_info.hpp` · **Implementación:** `src/C++/system/linux/system_info.cpp`.

## Propósito y contrato

```cpp
SystemInfo collect_system_info();
```

No recibe parámetros. Devuelve `SystemInfo` con información recolectada sobre CPU/cachés, runtime (afinidad/frecuencias), compilador y sandbox/cgroups. Cada valor lleva un estado de disponibilidad y, donde aplica, fuente. La recolección puede ser parcial; el caller debe revisar `CollectionStatus` antes de usar `value`.

## Efectos y límites

No modifica la configuración del sistema. La implementación actual es Linux y la información faltante se conserva como no disponible, no como cero. Puede haber errores de lectura representados por estado.
