# `low_cardinality_array()`

**Contrato**

Construye `std::array<T,N>` a partir de `palette`, opcionalmente ponderada por `weights`. `N` es longitud fija; seed o engine controla aleatoriedad; options define patrón/duplicados. Sin pesos las categorías son uniformes. Paleta/pesos inválidos lanzan `std::invalid_argument` mediante `categorical_array()`.
