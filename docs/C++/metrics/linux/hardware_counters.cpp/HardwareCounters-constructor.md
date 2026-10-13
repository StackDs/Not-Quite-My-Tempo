# `HardwareCounters::HardwareCounters()`

**Contrato**

```cpp
HardwareCounters::HardwareCounters();
```

No recibe argumentos. Construye cuatro grupos PMU: ciclos/instrucciones, accesos y misses L1D, accesos y misses LLC, branch misses. No los abre hasta `prepare()`.
