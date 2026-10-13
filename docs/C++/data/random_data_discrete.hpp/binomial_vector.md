# `binomial_vector()`

**Contrato**

```cpp
std::vector<std::size_t> binomial_vector(
    std::size_t count, std::size_t trials, double probability,
    std::uint64_t seed, const GenerationOptions &options = {});
```

Existe overload con engine. `count` cuenta muestras; `trials` es ensayos por muestra y debe caber en `long long`; `probability` es finita en `[0,1]`; `seed` o `engine` controla aleatoriedad; `options` aplica políticas comunes. Retorna conteos de éxitos; dominio inválido lanza `std::invalid_argument`.
