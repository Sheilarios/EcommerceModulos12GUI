# Motor de Personalización y Asignación Logística para E-commerce — Caso Amazon

Proyecto del curso **Programación Competitiva** (Universidad Científica del Sur, 2026-2).
Integra los **cuatro módulos** del informe EC3 y el **flujo end-to-end** (sección 10).

## Estructura

```text
EcommerceModulos12GUI.sln / .vcxproj   Solución Visual Studio 2022 (GUI Win32)
CMakeLists.txt                          Compilación alternativa (GUI + consola)
include/
  comun.hpp                 Cronómetro, tablas de resultados, formato
  modulo1_catalogo.hpp      6.1 Catálogo O(log N), índices, importación CSV
  modulo1_configurador.hpp  6.2 Backtracking con poda (presupuesto, nivel, dependencias)
  modulo1_greedy.hpp        6.3 Asignación Greedy First Fit (RN-A01..RN-A05) + óptimo exacto
  modulo2_estructuras.hpp   7.1 map/set/priority_queue, 7.2 DSU, 7.3 Segment Tree,
                            7.4 Fenwick Tree y Segment Tree con Lazy Propagation
  modulo3_grafos.hpp        8.1 BFS/DFS/ciclos/orden topológico, 8.2 Dijkstra y Bellman-Ford,
                            8.3 Kruskal y Prim, 8.4 Dinic y Edmonds-Karp (+ corte mínimo)
  modulo4_dp.hpp            9.1 Knapsack 0/1 vs Greedy, 9.2 Bitmask DP de turnos, 9.3 KMP y Trie
  pruebas.hpp               Verificaciones con ejemplos de resultado conocido + benchmarks
  integracion.hpp           10. Flujo end-to-end (Trie → catálogo → configurador → prioridad →
                            First Fit → Dijkstra → mochila → respuesta) con media/p95/p99
src/main.cpp                Interfaz gráfica Win32 (todos los módulos)
src/consola.cpp             Versión de consola multiplataforma (ejecuta todo)
data/catalogo_ejemplo.csv   Datos de prueba
sql/esquema.sql             Modelo relacional (sección 5.3)
docs/                       Arquitectura y resultados de referencia
tests/plan_pruebas.md       Plan de pruebas (casos y resultado esperado)
```

## Compilar y ejecutar

### Opción A — Visual Studio 2022 (interfaz gráfica)
1. Instala la carga **Desarrollo para el escritorio con C++** (MSVC v143, SDK de Windows 10/11).
2. Abre `EcommerceModulos12GUI.sln`, elige `Release` y `x64`.
3. `Ctrl + Shift + B` para compilar y `Ctrl + F5` para ejecutar.
4. El ejecutable queda en `bin/Release/EcommerceModulos12GUI.exe`.

### Opción B — Consola (Windows, Linux o macOS)
```bash
g++ -std=c++17 -O2 -Iinclude src/consola.cpp -o ecommerce_consola
./ecommerce_consola              # 1 000 000 de productos sintéticos
./ecommerce_consola 0 data/catalogo_ejemplo.csv   # usando un CSV
```
O con CMake: `cmake -B build && cmake --build build --config Release`.

## Uso de la interfaz
| Botón | Qué hace |
|---|---|
| Generar / Importar CSV | Crea hasta 5 000 000 de productos o importa un CSV validado (RN-C01..C04) |
| Benchmark búsqueda | Binaria O(log N) vs lineal O(N) |
| Sugerir | Autocompletado por prefijo con Trie |
| Buscar ID / Filtrar precio / Económicos / Premium | Consultas del catálogo |
| Backtracking + benchmark | Mejor configuración dentro del presupuesto; poda vs fuerza bruta |
| Asignación Greedy | First Fit por prioridad y por ratio; optimalidad frente al óptimo exacto |
| Módulo 2 / 3 / 4 | Verificación con los ejemplos del informe + pruebas de volumen |
| Escalabilidad 1k/10k/100k | IND-I03: flujo con 1 000, 10 000 y 100 000 solicitudes |
| Flujo end-to-end | 1 000 solicitudes completas con latencias por etapa, p95/p99 y RN-I03 |
| Ejecutar todo | Corre todos los módulos en una sola tabla |
| Exportar resultados CSV | Guarda la tabla actual para el informe |

La columna **Detalle / Verificación** muestra `OK` cuando el algoritmo reproduce el resultado esperado
(por ejemplo Dijkstra 0,7,9,20,20,11; flujo máximo 23; MST 14; mochila 9 vs 8; turnos 13).

## Cobertura del informe (indicadores)
| Indicador / sección | Dónde se mide |
|---|---|
| IND-C01 búsqueda < 2 µs, IND-C04 memoria ≤ 100 MB | Benchmark búsqueda |
| Rango de precio lineal vs índice; precio mínimo con Segment Tree (11.4) | Benchmark búsqueda |
| IND-F01 reducción de nodos > 40 % | Backtracking + benchmark |
| IND-A01, IND-A02; First Fit prioridad/ratio y Best Fit; 1 000 000 de solicitudes | Asignación Greedy |
| 7.1 std::map vs vector ordenado; cola de prioridad conectada al Greedy (11.4) | Módulo 2 / Flujo |
| 8.x volumen: BFS 1M/4M, Dijkstra 100k/500k, Dinic 20k/200k, Kruskal vs Prim | Módulo 3 |
| 9.x mochila, bitmask n=20, KMP peor caso, Trie 100 000 nombres | Módulo 4 |
| IND-I01, IND-I02 (p95/p99), memoria pico | Flujo end-to-end |
| IND-I03 escalabilidad | Escalabilidad 1k/10k/100k |

## Formato CSV
```csv
id,nombre,categoria,precio,nivel
100,Teclado mecanico,2,149.90,Premium
```
`id` entero único ≥ 0, `categoria` 0–19, `precio` ≥ 0 con punto decimal, `nivel` Premium/premium/1 o Económico.

## Validación
- La versión de consola se compiló con g++ (C++17, `-Wall -Wextra` sin advertencias) y todas las verificaciones dan `OK`
  (ver `docs/resultados_benchmark_referencia.txt`, corrida con 1 000 000 de productos).
- La interfaz Win32 se compiló con MinGW-w64. Debe confirmarse la primera compilación en Visual Studio 2022 del laboratorio.
- Los tiempos dependen del equipo; repetir la corrida y reportar las cifras propias.

Repositorio: https://github.com/Sheilarios/EcommerceModulos12GUI.git
