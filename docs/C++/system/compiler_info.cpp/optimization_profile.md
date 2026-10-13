# `optimization_profile()`

**Contrato**

```cpp
std::string optimization_profile();
```

No recibe parámetros. Inspecciona macros de optimización para reportar si se detecta optimización de tamaño, optimización general o perfil equivalente a `-O0`. No puede inferir el nivel exacto ni las flags exactas.
