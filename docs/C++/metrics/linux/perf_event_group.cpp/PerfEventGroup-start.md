# `PerfEventGroup::start()`

**Contrato**

```cpp
void start() noexcept;
```

Reinicia y habilita atómicamente el grupo mediante el líder. Si no está preparado o no tiene líder no hace nada; un fallo de ioctl marca el grupo no disponible. No lanza.
