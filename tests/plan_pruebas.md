# Plan de pruebas — Módulos 1 a 4 e integración

Las pruebas marcadas como **auto** se ejecutan con el botón correspondiente o con `ecommerce_consola`; la columna de verificación muestra `OK`.

| ID | Módulo | Prueba | Resultado esperado | Tipo |
|---|---|---|---|---|
| CAT-01 | 1 | Generar 1 000 000 productos | Catálogo e índices creados | manual |
| CAT-02 | 1 | Buscar ID existente / inexistente | Un producto / "No se encontró" | manual |
| CAT-03 | 1 | Filtrar precio 100–500 | Hasta 500 productos ordenados por precio | manual |
| CAT-04 | 1 | Importar `data/catalogo_ejemplo.csv` | 20 productos validados | manual |
| CAT-05 | 1 | CSV con ID duplicado o categoría 25 | Error con número de línea; catálogo intacto | manual |
| CAT-06 | 1 | Benchmark búsqueda | Binaria < 2 µs por consulta | auto |
| CFG-01 | 1 | Presupuesto 900 | Configuración de menor costo, mismo nivel | auto |
| CFG-02 | 1 | Presupuesto 100 | "Sin configuración válida" | manual |
| CFG-03 | 1 | Poda vs fuerza bruta | Mismo óptimo, menos nodos | auto |
| GRD-01 | 1 | 10 000 solicitudes / 300 recursos | < 10 ms por solicitud | auto |
| GRD-02 | 1 | RN-A02 capacidad | Ningún recurso excedido | auto |
| GRD-03 | 1 | Optimalidad ratio + First Fit | ≥ 90 % del óptimo | auto |
| M2-01 | 2 | map/set reserva de stock | Reserva válida y rechazo por falta de stock | auto |
| M2-02 | 2 | priority_queue | Orden 3, 2, 4, 1 | auto |
| M2-03 | 2 | DSU 8 centros | 4 regiones | auto |
| M2-04 | 2 | Segment Tree [120,80,300,45,210,95] | 80, 45, 425; tras actualizar 95 | auto |
| M2-05 | 2 | Fenwick horas 17–19 | 109 | auto |
| M2-06 | 2 | Lazy descuento 15 en IDs 2–6 | Suma 925; suma(3,4) = 170 | auto |
| M3-01 | 3 | BFS 6 servicios | 0,1,1,2,3,4 | auto |
| M3-02 | 3 | Ciclo 5→1 | Detectado; topológico ordena 2 de 6 | auto |
| M3-03 | 3 | Dijkstra red clásica | 0,7,9,20,20,11 | auto |
| M3-04 | 3 | Bellman-Ford | 0,4,1,3; ciclo negativo detectado | auto |
| M3-05 | 3 | Kruskal y Prim | 14 | auto |
| M3-06 | 3 | Dinic y Edmonds-Karp | 23 | auto |
| M4-01 | 4 | Mochila W=7 | DP 9, Greedy 8 | auto |
| M4-02 | 4 | Turnos 4×4 | Costo 13 = fuerza bruta | auto |
| M4-03 | 4 | KMP "abab" | 0, 2, 9, 11 | auto |
| M4-04 | 4 | Trie "la" | lampara, laptop, laptop gamer | auto |
| CAT-07 | 1 | Memoria con 1M productos | ≤ 100 MB (IND-C04) | auto |
| CAT-08 | 1 | Rango por índice vs lineal y Segment Tree vs lineal | Mismos resultados | auto |
| GRD-04 | 1 | Best Fit (comparación) | Optimalidad reportada; no reemplaza RN-A03 | auto |
| M2-07 | 2 | map vs vector ordenado | Vector más rápido | auto |
| M3-07 | 3 | Dijkstra 100k/500k y Dinic 20k/200k | Termina en decenas de ms | auto |
| INT-02 | Int. | Escalabilidad 1k/10k/100k | Costo por solicitud estable, < 10 ms | auto |
| INT-03 | Int. | Cola de prioridad alimenta First Fit | Igual número de asignaciones que el ordenamiento | auto |
| INT-01 | Int. | Flujo 1 000 solicitudes | 0 productos no encontrados; RN-D01 OK; máx < 10 ms | auto |
