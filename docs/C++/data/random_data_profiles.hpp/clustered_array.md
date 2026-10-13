# `clustered_array()`

**Contrato**

Genera `std::array<T,N>` de una mezcla de normales. `N` fija longitud; `centers`, `standard_deviations` y `weights` deben alinearse y no estar vacíos; centros finitos, desviaciones positivas finitas y pesos válidos. Seed/engine controla azar; options configura salida. Solo tipo flotante; parámetros inválidos lanzan `std::invalid_argument`.
