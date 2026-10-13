# `collect_compiler_info()`

**Contrato**

```cpp
CompilerInfo collect_compiler_info();
```

No recibe parámetros. Construye `CompilerInfo` a partir de macros del compilador: familia/versión, estándar, perfil de optimización y arquitectura objetivo. Campos no detectables conservan estado `unavailable`; no cambia el proceso ni lanza por ausencia de macros.
