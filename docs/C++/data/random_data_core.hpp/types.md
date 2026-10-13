# Tipos de generación base

**Declaración:** `include/C++/data/random_data_core.hpp`.

- `DuplicatePolicy`: `allow` conserva duplicados; `forbid` solicita valores únicos y puede fallar al agotar intentos.
- `DataPattern`: forma del vector: aleatoria, ordenada, inversa, casi ordenada, repetida, bloques ordenados/alternados, diente de sierra, extremos alternados, rotada, pocos elementos fuera de lugar, agrupada o clave parcial.
- `GenerationOptions`: política de duplicados, patrón, `max_attempts`, `block_size`, `sawtooth_run_length`, swaps casi ordenados, cantidad desplazada, offset de rotación y ancho de clave parcial. Solo aplican restricciones de los parámetros correspondientes al patrón activo.

Los generadores numéricos admiten tipos aritméticos distintos de `bool`. La política de igualdad para valores flotantes es exacta.
