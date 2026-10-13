# `generate_seed()`

**Declaración e implementación:** `include/C++/data/random_data_core.hpp`.

## Propósito

Obtiene una semilla de 64 bits para reproducir generadores que aceptan seed. Guarda el valor devuelto junto con el experimento para repetirlo.

## Contrato

```cpp
std::uint64_t generate_seed();
```

## Parámetros y retorno

No recibe parámetros. Devuelve un valor combinado a partir de `std::random_device`. No es una API criptográfica; puede propagar una excepción de la fuente de aleatoriedad si esta falla.
