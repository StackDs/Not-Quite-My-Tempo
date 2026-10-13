# `TlbMissCounters::apply()`

**Contrato**

```cpp
void apply(MemoryMeasurements &measurements) const noexcept;
```

Transfiere valores y disponibilidad de misses DTLB a `measurements`. Si no hay exactamente dos eventos, ambos estados quedan `not_measured`; un cero no disponible no se considera medición. No lanza.
