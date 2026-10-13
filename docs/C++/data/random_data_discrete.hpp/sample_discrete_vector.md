# `detail::sample_discrete_vector()`

**Contrato**

```cpp
template <typename Distribution>
std::vector<std::size_t> sample_discrete_vector(
    std::size_t count, Distribution distribution, std::uint64_t seed,
    const GenerationOptions &options);
```

`count` es la cantidad de resultados; `distribution` genera conteos; `seed` inicializa motor local; `options` aplica duplicados/patrones. El overload compañero sustituye seed por `std::mt19937_64& engine` y lo consume. Ambos devuelven `std::vector<std::size_t>` y delegan las validaciones generales a `generate_vector_with_distribution()`.
