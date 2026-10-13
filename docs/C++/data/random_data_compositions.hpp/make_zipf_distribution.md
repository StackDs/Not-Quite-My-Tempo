# `detail::make_zipf_distribution()`

**Contrato**

```cpp
ZipfDistribution make_zipf_distribution(std::size_t max_rank,
                                        double exponent);
```

Construye pesos `rank^-exponent` para ranks 1..max_rank. `max_rank` debe ser positivo y caber en `long long`; `exponent` positivo y finito. Devuelve distribuidor discreto o lanza `std::invalid_argument`.
