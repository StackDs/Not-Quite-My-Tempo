# Not-Quite-My-Tempo

<div align="center">
<img src = "assets/main.jpg" alt="Not Quite My Tempo" width="746" height="413"/>
</div>

Herramientas independientes para medir algoritmos y procesos en C, C++ y Python.


### C++

- `include/C++/benchmark.hpp`, `config.hpp`, `result.hpp`, `metadata.hpp` y
  `types.hpp`: interfaz principal, opciones, resultados y tipos comunes.
- `include/C++/data/`: API para semillas y generación de datos de prueba.
- `include/C++/metrics/`: contratos de colectores de tiempo, proceso y hardware.
- `include/C++/system/system_info.hpp`: API para recolectar metadatos visibles
  del sistema Linux; `src/C++/system/linux/` contiene sus colectores.
- `include/C++/statistics/`: resúmenes estadísticos y ajuste de complejidad.
- `include/C++/output/`: interfaz para escribir resultados CSV.
- `include/C++/system/`: interfaz para registrar información del equipo.
- `src/C++/`: archivos de implementación C++ organizados por esas mismas
  responsabilidades. `metrics/linux/` y `system/linux/` aíslan las partes que
  dependen de Linux.
