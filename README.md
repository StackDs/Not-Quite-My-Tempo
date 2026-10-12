# Not-Quite-My-Tempo

<div align="center">
<img src = "assets/main.jpg" alt="Not Quite My Tempo" width="746" height="413"/>
</div>

Herramientas independientes para medir algoritmos y procesos en C, C++ y Python.


### C++

- [Referencia de la API pública C++](docs/C++/api_reference.md): contratos,
  parámetros y flujo de uso recomendado.
- La interfaz general de benchmark (`benchmark.hpp`, `config.hpp`, `result.hpp`,
  `metadata.hpp` y `types.hpp`) y la exportación CSV (`output/`) están pendientes.
- `include/C++/data/random_data.hpp`: generación reproducible de vectores y
  arreglos numéricos con `std::mt19937_64`, semillas guardables, distribuciones
  discretas/continuas, categorías, mezclas y perfiles de datos; las familias se
  organizan en headers específicos bajo `include/C++/data/`.
- `include/C++/metrics/`: contratos de colectores de tiempo, proceso y hardware.
- `include/C++/metrics/hardware_measurements.hpp`: resultados de IPC, fallos de
  caché y branch mispredictions; `src/C++/metrics/linux/` contiene los
  colectores Linux basados en `perf_event_open`.
- `include/C++/system/system_info.hpp`: API para recolectar metadatos visibles
  del sistema Linux; `src/C++/system/linux/` contiene sus colectores.
- `include/C++/statistics/statistics.hpp`: resumen configurable de muestras
  agrupadas, intervalos bootstrap, comparaciones pareadas/no pareadas y auditoría
  de exclusiones; `src/C++/statistics/` contiene sus cálculos.
- `include/C++/analysis/complexity.hpp`: ajuste empírico de tiempos frente a
  modelos de crecimiento, con constante multiplicativa y R²; consume
  observaciones existentes y permanece separado de las estadísticas.
- `src/C++/`: archivos de implementación C++ organizados por esas mismas
  responsabilidades. `metrics/linux/` y `system/linux/` aíslan las partes que
  dependen de Linux.
