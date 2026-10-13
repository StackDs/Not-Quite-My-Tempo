# `sparse_array()`

**Contrato**

Genera `std::array<T,N>` disperso. `N` fija longitud; `zero_probability` en `[0,1]`; `nonzero_distribution` invocable con motor y convertible a `T`; seed/engine gobierna azar; options controla salida. Devuelve array numérico no-bool; parámetros inválidos lanzan `std::invalid_argument`.
