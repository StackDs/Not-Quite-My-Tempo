# Lenguajes de Evaluación

* C / C++

* Python

* Rust

## 1. Configuración y Aislamiento del Entorno (Reproducibilidad)

* **Hardware:** Registrar modelo exacto de CPU, arquitectura y tamaño de cachés (L1, L2, L3).

* **Estabilización de CPU:** Fijar la frecuencia del procesador (gobernador performance), desactivar el Turbo Boost y fijar el proceso a un núcleo específico (CPU pinning / taskset).

* **Compilación:** Registrar compilador (GCC/Clang), versión exacta y banderas de optimización (ej. `-O3`, `-march=native`, `-flto`).

* **Sandboxing:** Si usas herramientas como isolate o contenedores, registrar las cuotas de CPU (`cfs_quota`) y los límites estrictos de memoria.

## 2. Métricas de Tiempo e Interrupciones

* **Wall-clock time:** Tiempo real total transcurrido (capturado con alta resolución, ej. `CLOCK_MONOTONIC`).

* **CPU User Time:** Tiempo absoluto ejecutando únicamente la lógica del algoritmo.

* **CPU System Time:** Tiempo gastado por el kernel en llamadas al sistema (I/O, memoria).

* **Context Switches:**

  * *Voluntarios:* El algoritmo cede la CPU intencionalmente.

  * *Involuntarios:* El sistema operativo quita la CPU al proceso (picos altos indican ruido externo y anulan la medición).

## 3. Métricas de Memoria

* **Peak RSS:** Máxima cantidad de memoria física (RAM) real consumida.

* **Page Faults Minor:** Asignaciones lógicas de memoria que ya estaban en la RAM.

* **Page Faults Major:** Accesos críticos a disco o swap (deberían tender a cero en algoritmos en memoria).

* **TLB Misses:** Costo acumulativo por traducir direcciones virtuales a físicas (común en accesos dispersos a grandes estructuras).

## 4. Telemetría de Hardware (Microarquitectura)

* **IPC (Instructions Per Cycle):** Métrica clave de eficiencia física; valores bajos indican cuellos de botella (CPU esperando datos).

* **Retired Instructions:** Cantidad de instrucciones en ensamblador que se completaron de forma útil.

* **Cache Misses (L1D y LLC):** Porcentaje de fallos al buscar datos en las cachés del procesador (penaliza la latencia).

* **Branch Mispredictions:** Saltos condicionales fallidos que obligaron a vaciar el pipeline del procesador.

## 5. Metodología de Ejecución y Estadística

* **Fase de Warm-up:** Descartar las primeras $N$ ejecuciones para asegurar que los binarios estén cargados y las cachés llenas.

* **Limpieza de Ruido (Outliers):** Descartar mediciones atípicas que superen $Q3 + 1.5 \times IQR$ (interrupciones del sistema operativo).

* **Métricas de Tendencia Central:** Usar la Mediana (p50) y la Desviación Absoluta de la Mediana (MAD). Evitar la media/desviación estándar por su sensibilidad al ruido.

* **Percentiles de Cola:** Reportar p90, p95, p99 y p99.9 para identificar el rendimiento en el "peor caso real".

* **Análisis de Complejidad Empírica:** Ejecutar el algoritmo con tamaños de datos incrementales ($10^3, 10^4, \dots$) para reportar:

  * **Ajuste de Curva (**$R^2$**):** Comparación de tiempos empíricos vs. modelos matemáticos ($O(N)$, $O(N^2)$, etc.).

  * **Constante Empírica:** El multiplicador real detrás de la notación Big O.