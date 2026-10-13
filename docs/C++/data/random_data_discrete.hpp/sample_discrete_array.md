# `detail::sample_discrete_array()`

**Contrato**

Dos overloads reciben distribución, seed o motor compartido y opciones; `N` fija tamaño. Generan `N` conteos vía helper vectorial y convierten a `std::array<std::size_t,N>`. Propagan errores de muestreo.
