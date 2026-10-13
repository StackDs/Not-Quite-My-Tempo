# `validate_config()`

**Ubicación:** `src/C++/benchmark/benchmark_runner.cpp` (función interna).

## Propósito

Valida callbacks y opciones antes de iniciar procesos de medición o crear salidas.

## Contrato

```cpp
void validate_config(const BenchmarkCallbacks &callbacks,
                     const BenchmarkConfig &config);
```

| Parámetro | Significado |
| --- | --- |
| `callbacks` | Debe contener un `function` no vacío; `prepare` es opcional. |
| `config` | Configuración: iteraciones superiores a warm-ups, unidad válida, umbral no negativo, rutas CSV distintas/no vacías, comparaciones vacías y opciones estadísticas válidas. |

## Errores

Devuelve normalmente sin valor. Lanza `std::invalid_argument` si alguna condición no se cumple, incluido un modo de ejecución inválido.
