# `reliability()`

**Contrato**

```cpp
TailReliabilityStatus reliability(std::size_t count, long double percentile,
                                  long double minimum_tail_observations);
```

Estima observaciones esperadas en la cola superior como `count * (1-percentile)`. Compara con `minimum_tail_observations`; devuelve `reliable` o `low_sample_count`.
