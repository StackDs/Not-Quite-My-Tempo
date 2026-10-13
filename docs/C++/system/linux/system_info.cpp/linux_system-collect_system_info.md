# `linux_system::collect_system_info()`

**Ubicación:** `src/C++/system/linux/system_info.cpp` (coordinador interno de colectores).

## Contrato

```cpp
SystemInfo collect_system_info();
```

No recibe parámetros. Llama a los recolectores de CPU, compilador y sandbox, y combina sus resultados en `SystemInfo`. Retorna la estructura; no cambia el entorno. Los errores y ausencias de cada campo se conservan según los contratos de los colectores.
