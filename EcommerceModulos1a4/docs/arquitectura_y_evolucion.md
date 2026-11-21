# Arquitectura del motor

## Capas
```text
Presentación   src/main.cpp (Win32)  |  src/consola.cpp (CLI)
      ↓
Aplicación     integracion.hpp (flujo end-to-end)  |  pruebas.hpp (verificación y benchmarks)
      ↓
Dominio        modulo1_* · modulo2_estructuras · modulo3_grafos · modulo4_dp
      ↓
Datos          memoria RAM, CSV (data/), esquema SQL de referencia (sql/esquema.sql)
```
La lógica no depende de Windows: la GUI y la consola usan los mismos encabezados.

## Flujo end-to-end por solicitud (sección 10.1)
```text
(0) Trie: sugerencia por prefijo
(1) Catálogo: búsqueda binaria del producto            RN-I01, RN-I02
(2) Configurador: backtracking con poda, mismo nivel   RN-F03, RN-F04
    └─ sin configuración válida → rama de error / incidencia
(3) Prioridad: urgencia, fecha, ratio                   RN-A01, RN-A05
(4) Asignación First Fit en lote                        RN-A02, RN-A03, RN-A04
(5) Dijkstra: tiempo de entrega desde el centro asignado
(6) Mochila 0/1: complementos con el presupuesto sobrante   RN-D01
(7) Respuesta; latencia total < 10 ms                   RN-I03
```

## Módulos y complejidad
| Módulo | Algoritmo / estructura | Complejidad | Uso en el motor |
|---|---|---|---|
| 1 | Búsqueda binaria (`lower_bound`) | O(log N) | Producto por ID |
| 1 | Índice por precio | O(log N + k) | Filtro por rango |
| 1 | Backtracking con poda | O(b^d) peor caso | Configurador |
| 1 | Greedy First Fit | O(n log n + n·R) | Asignación en tiempo real |
| 2 | map / set | O(log N) | Stock y segmento Premium |
| 2 | priority_queue | O(log N) | Cola de solicitudes |
| 2 | DSU | ~O(α(N)) | Regiones de centros, Kruskal |
| 2 | Segment Tree | O(log N) | Mínimo/suma por rango |
| 2 | Fenwick | O(log N) | Pedidos por hora |
| 2 | Lazy Segment Tree | O(log N) | Descuento masivo por rango |
| 3 | BFS / DFS iterativo / Kahn | O(V+E) | Dependencias de microservicios |
| 3 | Dijkstra | O((V+E) log V) | Tiempo de entrega |
| 3 | Bellman-Ford | O(V·E) | Rutas con créditos negativos |
| 3 | Kruskal / Prim | O(E log E) | Red mínima entre centros |
| 3 | Dinic / Edmonds-Karp | O(V²E) / O(VE²) | Capacidad de la cadena y cuello de botella |
| 4 | Knapsack 0/1 | O(n·W) | Complementos (cross-selling) |
| 4 | Bitmask DP | O(2^n·n) | Turnos de personal |
| 4 | KMP | O(n+m) | Búsqueda de texto interna |
| 4 | Trie | O(L) | Autocompletado |

## Evolución hacia producción
- Reemplazar el CSV por un repositorio SQL usando `sql/esquema.sql`.
- Exponer el flujo como servicio (REST/gRPC) y paralelizar la asignación por región (DSU).
- Persistir incidencias de la rama de error en la tabla `incidencia`.
