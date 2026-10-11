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

  * *Involuntarios:* El sistema operativo quita la CPU al proceso. Si el contador
    supera el umbral configurado, la muestra se marca como sospechosa. Se
    conservan y reportan sus métricas junto con la regla, el valor y el umbral que
    activaron la flag; no se descarta ni se repite automáticamente. El usuario
    decide cómo tratarla.

## 3. Métricas de Memoria

* **Peak RSS:** Máxima cantidad de memoria física (RAM) real consumida.

* **Page Faults Minor:** Asignaciones lógicas de memoria que ya estaban en la RAM.

* **Page Faults Major:** Accesos críticos a disco o swap (deberían tender a cero en algoritmos en memoria).

* **TLB Misses:** Reportar fallos de lectura y escritura del DTLB mediante contadores de hardware cuando estén disponibles; marcar explícitamente cuando el hardware, kernel o permisos impidan medirlos. Esta implementación no incluye fallos del ITLB.

## 4. Telemetría de Hardware (Microarquitectura)

* **IPC (Instructions Per Cycle):** Métrica clave de eficiencia física; valores bajos indican cuellos de botella (CPU esperando datos).

* **Retired Instructions:** Cantidad de instrucciones en ensamblador que se completaron de forma útil.

* **Cache Misses (L1D y LLC):** Reportar fallos de lectura y porcentaje sobre accesos de lectura para ambas cachés; indicar explícitamente si los eventos no están disponibles.

* **Branch Mispredictions:** Saltos condicionales fallidos que obligaron a vaciar el pipeline del procesador.

## 5. Metodología de Ejecución y Estadística

* **Fase de Warm-up:** Descartar las primeras $N$ ejecuciones de cada grupo de mediciones. El grupo lo define el usuario mediante una clave (parámetro, índice, tamaño u otra dimensión).

* **Limpieza de Ruido (Outliers):** Marcar y excluir del resumen filtrado las muestras que superen $Q3 + 1.5 \times IQR$. Conservar los datos originales, indicar el motivo por muestra y reportar resúmenes antes y después del filtro.

* **Métricas de Tendencia Central y Dispersión:** Reportar Mediana (p50) y MAD como medidas robustas principales. Añadir mínimo, máximo, media aritmética y desviación estándar muestral como medidas complementarias, identificando el conjunto de muestras usado.

* **Incertidumbre:** Reportar intervalos de confianza del 95 % para media y mediana, estimados mediante bootstrap. Registrar método, remuestreos y semilla; marcar resultados con pocas muestras como de baja confiabilidad.

* **Percentiles de Cola:** Reportar p90, p95, p99 y p99.9 para identificar la cola de la distribución. Marcar p99/p99.9 como poco fiables cuando el tamaño muestral no aporte suficientes observaciones esperadas en esa cola.

* **Tamaño del Efecto:** Al comparar variantes, reportar diferencias absolutas y relativas. Si las ejecuciones están emparejadas por el mismo caso de entrada, comparar las diferencias por pares y reportar el método utilizado.

* **Auditoría y Contexto:** Reportar iteraciones, warm-ups, duración, unidad, reloj, versión de la librería, parámetros del algoritmo y contexto del sistema. Indicar por muestra si fue apartada por warm-up, IQR u otra regla. Los indicadores de entorno son contexto, no correcciones automáticas.

* **Análisis de Complejidad Empírica:** Ejecutar el algoritmo con tamaños de datos incrementales ($10^3, 10^4, \dots$) para reportar:

  * **Ajuste de Curva (**$R^2$**):** Comparación de tiempos empíricos vs. modelos matemáticos ($O(N)$, $O(N^2)$, etc.).

  * **Constante Empírica:** El multiplicador real detrás de la notación Big O.
