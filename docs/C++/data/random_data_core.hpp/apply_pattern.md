# `detail::apply_pattern()`

**Contrato**

```cpp
template <typename T>
void apply_pattern(std::vector<T> &values,
                   const GenerationOptions &options,
                   std::mt19937_64 &engine);
```

Reordena `values` en sitio según `options.pattern`, consumiendo `engine` para patrones aleatorios. Algunos patrones ordenan o permutan; parámetros específicos están en `GenerationOptions`. No modifica cardinalidad ni multiconjunto salvo patrón `repeated` ya tratado en generación.
