# Backend E-commerce — Módulos 1 y 2 

Esta entrega contiene el **Módulo 1: Catálogo** y el **Módulo 2: Configurador**. El Módulo 3 (Asignación Greedy) no forma parte de esta versión y no se ejecuta desde la interfaz; se deja previsto para integrarlo en una siguiente etapa.

## Requisitos
- Windows 10/11.
- Visual Studio 2022.
- Carga de trabajo **Desarrollo para el escritorio con C++**.
- MSVC v143, Windows 10/11 SDK y compatibilidad con C++17.

## Abrir y ejecutar
1. Descomprime toda la carpeta del ZIP.
2. Abre `EcommerceModulos12GUI.sln`.
3. En la barra superior selecciona `Release` y `x64`.
4. Ejecuta **Compilar > Compilar solución** o presiona `Ctrl + Shift + B`.
5. Ejecuta con `Ctrl + F5`.

El ejecutable se genera en `bin/Release/EcommerceModulos12GUI.exe`.

## Módulo 1 — Catálogo
- Genera de 1 a 5,000,000 productos sintéticos reproducibles.
- Importa productos desde CSV.
- Valida campos, rangos e IDs duplicados antes de reemplazar el catálogo actual.
- Busca por ID usando `std::lower_bound`: O(log n), luego de ordenar.
- Filtra por rango de precio usando un índice de posiciones ordenado por precio: O(log n + k).
- Filtra por categoría (0–19) y nivel Económico/Premium usando índices precalculados.
- La lista visual limita la salida a 500 resultados para no saturar la interfaz; el catálogo en memoria conserva todos los registros.

### Formato CSV
Encabezado obligatorio:

```csv
id,nombre,categoria,precio,nivel
100,Teclado mecanico,2,149.90,Premium
101,Mouse optico,3,49.90,Economico
```

- `id`: entero no negativo y único.
- `nombre`: texto simple sin comas.
- `categoria`: entero de 0 a 19.
- `precio`: número no negativo, usar punto decimal.
- `nivel`: `Premium`, `premium`, `1` o cualquier otro valor para Económico.

Se incluye `data/catalogo_ejemplo.csv`.

## Módulo 2 — Configurador
- Construye un escenario de 4 categorías con 10 alternativas por categoría.
- Recibe un presupuesto desde la interfaz.
- Evalúa configuraciones mediante backtracking.
- Poda ramas que superan el presupuesto.
- Impone consistencia de nivel: una configuración no mezcla alternativas Premium y Económicas.
- Evalúa dependencias entre opciones. El escenario de ejemplo exige la opción 1 de la categoría 1 si se elige la opción 3 de la categoría 0.
- Entre las configuraciones válidas, muestra la de menor costo.
- Informa los nodos evaluados y el tiempo de ejecución.

Los componentes y dependencias del configurador son datos de demostración generados en memoria; pueden reemplazarse por datos leídos desde CSV o una base de datos en una siguiente iteración.

## Arquitectura actual y crecimiento
Esta versión prioriza que los dos módulos estén integrados y ejecutables en una aplicación Win32 nativa. La lógica se encuentra en `src/main.cpp` para facilitar la apertura inmediata del proyecto. La documentación de `docs/arquitectura_y_evolucion.md` explica cómo separar presentación, aplicación, dominio e infraestructura sin cambiar el comportamiento.

El catálogo usa memoria RAM y estructuras de índices. La generación de cinco millones de productos puede consumir bastante memoria; el resultado depende del equipo. La importación actual es CSV. **No hay conexión a SQL en esta entrega**: se documenta cómo añadir un repositorio SQL posteriormente.

## Pruebas manuales sugeridas
Consulta `tests/plan_pruebas.md` para verificar generación, importación, búsquedas, filtros, validaciones y configurador.

## Alcance y validación
- Incluido: módulos 1 y 2, interfaz gráfica, datos de ejemplo, solución y proyecto de Visual Studio.
- Excluido: Módulo 3, conexión a base de datos real, autenticación, servicios web y despliegue.
- El ZIP se revisó estructuralmente, pero no se compiló con Visual Studio 2022 en este entorno. La primera compilación debe realizarse en tu PC; si aparece un error, comparte una captura o el texto de salida.
