# Plan de pruebas manuales — Módulos 1 y 2

| ID | Módulo | Prueba | Resultado esperado |
|---|---|---|---|
| CAT-01 | Catálogo | Generar 1,000 productos | El estado informa 1,000 productos generados. |
| CAT-02 | Catálogo | Generar 1,000,000 productos | Se genera el catálogo y sus índices; el tiempo depende del equipo. |
| CAT-03 | Catálogo | Buscar un ID existente | Se muestra un único producto con sus datos. |
| CAT-04 | Catálogo | Buscar un ID inexistente | Se informa que no se encontró el producto. |
| CAT-05 | Catálogo | Filtrar precio 100–500 | Solo aparecen productos dentro del rango. |
| CAT-06 | Catálogo | Filtrar categoría 0, Económico y Premium | Se muestran resultados de la categoría y nivel elegidos. |
| CAT-07 | Catálogo | Importar `data/catalogo_ejemplo.csv` | Se reemplaza el catálogo con los registros del CSV. |
| CAT-08 | Catálogo | Importar CSV con IDs duplicados | Se muestra error y no se reemplaza el catálogo. |
| CAT-09 | Catálogo | Ingresar categoría 20 | La interfaz rechaza el dato. |
| CFG-01 | Configurador | Presupuesto amplio (por ejemplo 900) | Se obtiene una configuración válida de menor costo. |
| CFG-02 | Configurador | Presupuesto muy bajo (por ejemplo 1) | Se informa que no existe configuración válida. |
| CFG-03 | Configurador | Presupuesto vacío o texto | La interfaz solicita un valor numérico mayor que cero. |
| CFG-04 | Configurador | Revisar salida | Las opciones pertenecen a cuatro categorías y respetan el mismo nivel. |

La prueba de cinco millones de productos debe realizarse solo si el equipo dispone de memoria suficiente. Cerrar otras aplicaciones y usar `Release | x64`.
