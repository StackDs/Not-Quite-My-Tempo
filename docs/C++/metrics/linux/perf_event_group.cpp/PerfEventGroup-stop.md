# `PerfEventGroup::stop()`

**Contrato**

```cpp
void stop() noexcept;
```

Deshabilita y lee el grupo, escala valores por tiempo enabled/running y los asocia con IDs. Si formato, tiempos, IDs, desbordamiento, syscall o lectura no son válidos, conserva estado no disponible. Cierra descriptores al final; no lanza.
