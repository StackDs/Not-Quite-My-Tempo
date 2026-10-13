# `boost_state()`

**Contrato**

```cpp
CollectedValue<bool> boost_state();
```

Prueba interfaces sysfs Intel, genérica y AMD para detectar turbo/boost. Devuelve true/false disponible, malformed para contenido inesperado o estado unavailable/permisos/lectura si no puede conocerlo. No modifica el estado de boost.
