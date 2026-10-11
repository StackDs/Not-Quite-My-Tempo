# Recolector de información del sistema Linux

API: `include/C++/system/system_info.hpp`  
Implementación Linux: `src/C++/system/linux/`  
Namespace: `ntq`

`collect_system_info()` devuelve hardware, estado de CPU, propiedades detectables
del compilador y límites del sandbox/cgroup en una instancia de `SystemInfo`.
La recolección es de solo lectura: no cambia gobernador, frecuencia, Turbo/boost,
afinidad ni límites.

```cpp
#include "system/system_info.hpp"

const ntq::SystemInfo info = ntq::collect_system_info();
if (info.cpu.model.status == ntq::CollectionStatus::available) {
  // info.cpu.model.value contiene el modelo visible para Linux.
}
if (info.sandbox.effective_memory_limit_bytes.status ==
    ntq::CollectionStatus::available) {
  // El valor está expresado en bytes.
}
```

## Estado de los valores

Los campos se envuelven en `CollectedValue<T>`:

- `value`: valor recolectado. Solo debe interpretarse cuando el estado lo permita.
- `status`: `available`, `unavailable`, `permission_denied`, `malformed`,
  `read_error` o `unlimited`.
- `source`: ruta o interfaz de donde se obtuvo el dato.

Un valor ausente no se convierte en cero, y un límite ilimitado no se confunde
con un límite numérico igual a cero. Si una fuente falla, el resto de categorías
se sigue recolectando.

## Datos recolectados

### CPU y hardware visible

`SystemInfo::cpu` informa arquitectura (`uname`), fabricante/modelo de
`/proc/cpuinfo`, CPUs lógicas en línea y cachés L1/L2/L3. Las cachés incluyen
nivel, tipo, tamaño en bytes y lista de CPUs que las comparten; entradas
equivalentes se deduplican.

### Estado de CPU

`SystemInfo::cpu_runtime` informa por CPU los valores de gobernador y frecuencia
que expone `cpufreq` (actual y límites de la política), el estado de Turbo/boost
si hay una interfaz conocida y la afinidad efectiva obtenida mediante
`sched_getaffinity`. En Linux la afinidad pertenece a cada hilo; este campo
corresponde al hilo que llama a `collect_system_info()`, no a una unión de todos
los hilos del proceso. Frecuencias se reportan en Hz. Si el kernel o el huésped
no exponen una interfaz, su estado se marca como no disponible en vez de inferir
un valor.

### Compilador

`SystemInfo::compiler` informa familia, versión, dialecto/estándar, arquitectura
objetivo y perfil de optimización inferible mediante macros predefinidas del
compilador con que se compiló el recolector. El perfil puede indicar que la
optimización está desactivada (equivalente efectivo a `-O0`), que está habilitada
sin poder distinguir el nivel exacto, o que está orientada a tamaño. No se puede
reconstruir una línea de comandos exacta desde esas macros.

### Sandbox y cgroups

`SystemInfo::sandbox` intenta detectar cgroups v1/v2, la ruta del proceso, la
cuota/periodo efectivo de CPU, CPUs permitidas y el límite de memoria efectivo.
Se consideran los límites visibles en los ancestros del cgroup. Cuotas se
expresan en microsegundos y el límite de memoria en bytes; una cuota de CPU
también se ofrece como número de núcleos equivalente. Los límites ilimitados
tienen estado `unlimited`.

## Alcance y límites

Los datos describen lo que ve el proceso en Linux. En una VM o contenedor, el
modelo de CPU, las cachés y los límites pueden estar virtualizados, ocultos o
restringidos; el recolector no los presenta como datos físicos del host. Los
resultados pueden ser parciales si faltan interfaces o permisos.

El colector compila como C++14 y no usa dependencias externas. Prueba manual del
módulo desde la raíz del repositorio:

```sh
g++ -std=c++14 -Wall -Wextra -Wpedantic -Werror \
  -Iinclude/C++ -Isrc/C++/system/linux \
  tests/C++/system/system_info_tests.cpp \
  src/C++/system/linux/compiler_info.cpp \
  src/C++/system/linux/parsers.cpp \
  src/C++/system/linux/system_info.cpp \
  -o /tmp/system_info_tests
/tmp/system_info_tests
```
