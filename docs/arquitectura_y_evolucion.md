# Arquitectura y evolución del backend

## Flujo de ejecución

```text
Usuario
  ↓
Interfaz gráfica Win32 (controles y eventos)
  ↓
Operaciones del catálogo / configurador
  ↓
Lógica de dominio y estructuras de datos
  ↓
Memoria RAM o archivo CSV
```

## Responsabilidad de cada módulo

### Módulo 1 — Catálogo
- `Producto`: ID, nombre, categoría, precio y nivel.
- `Catalogo`: almacena los productos, valida reemplazos e importa CSV.
- Índice principal: vector ordenado por ID.
- Índice secundario: posiciones ordenadas por precio.
- Índice por categoría y nivel: vectores de posiciones para filtros rápidos.

La clase conserva un único vector de productos y los índices almacenan posiciones, evitando duplicar objetos completos.

### Módulo 2 — Configurador
- `Componente`: categoría, alternativa, precio y nivel.
- `ReglaDependencia`: relación entre una alternativa de una categoría y la alternativa requerida en otra.
- `Configurador`: recorre combinaciones mediante backtracking y descarta ramas inválidas tan pronto como detecta exceso de presupuesto o incompatibilidad de nivel.

## Próxima separación por capas

Para una siguiente versión, cuando se requiera mantener y probar cada parte de forma independiente, se recomienda mover el código a esta estructura:

```text
src/
├── presentation/       # Ventana principal y vistas Win32
├── application/        # Servicios de casos de uso
├── domain/
│   ├── entities/       # Producto, Componente, Configuracion
│   ├── catalogo/       # Reglas e índices del catálogo
│   └── configurador/   # Backtracking, restricciones y dependencias
└── infrastructure/
    ├── csv/            # Lectura y escritura CSV
    └── database/       # Adaptador SQL futuro
```

## Preparación para base de datos

La GUI no debería consultar directamente SQL. La evolución recomendada es definir una abstracción `IRepositorioProductos` con operaciones como `obtenerPorId`, `listarPorRangoPrecio` e `importar`. En la primera etapa se implementaría `RepositorioCSV`; después se agregaría `RepositorioSQL` sin modificar las reglas de negocio del catálogo. Para millones de registros, la base de datos deberá aportar índices por ID, precio y categoría, y la interfaz deberá paginar resultados en lugar de cargar toda la tabla.

## Integración futura del Módulo 3

El módulo de asignación Greedy debe añadirse como módulo independiente que consuma las solicitudes y recursos del sistema. No debe mezclarse con la lógica de búsqueda del catálogo ni con las restricciones del configurador. Su integración se hará mediante un servicio/caso de uso nuevo y una vista propia.
