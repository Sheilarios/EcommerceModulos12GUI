-- Modelo relacional (seccion 5.3 del informe). Compatible con PostgreSQL.
CREATE TABLE categoria (
  id_categoria INT PRIMARY KEY,
  nombre VARCHAR(60) NOT NULL);
CREATE TABLE vendedor (
  id_vendedor INT PRIMARY KEY,
  nombre VARCHAR(100) NOT NULL);
CREATE TABLE producto (
  id_producto INT PRIMARY KEY,
  nombre VARCHAR(120) NOT NULL,
  id_categoria INT NOT NULL REFERENCES categoria(id_categoria),
  id_vendedor INT REFERENCES vendedor(id_vendedor),
  precio NUMERIC(10,2) NOT NULL CHECK (precio >= 0),
  nivel_comercial VARCHAR(10) NOT NULL CHECK (nivel_comercial IN ('Premium', 'Economico')),
  stock INT NOT NULL DEFAULT 0 CHECK (stock >= 0));
CREATE TABLE componente (
  id_componente INT PRIMARY KEY,
  id_categoria INT NOT NULL REFERENCES categoria(id_categoria),
  indice_opcion INT NOT NULL,
  precio NUMERIC(10,2) NOT NULL,
  nivel_comercial VARCHAR(10) NOT NULL CHECK (nivel_comercial IN ('Premium', 'Economico')));
CREATE TABLE regla_dependencia (
  id_regla INT PRIMARY KEY,
  id_comp_origen INT NOT NULL REFERENCES componente(id_componente),
  id_comp_requerido INT NOT NULL REFERENCES componente(id_componente));
CREATE TABLE solicitud (
  id_solicitud INT PRIMARY KEY,
  id_producto INT NOT NULL REFERENCES producto(id_producto),
  urgencia SMALLINT NOT NULL CHECK (urgencia BETWEEN 1 AND 5),
  fecha_entrega DATE NOT NULL,
  presupuesto NUMERIC(10,2) NOT NULL,
  ganancia NUMERIC(10,2) NOT NULL,
  costo NUMERIC(10,2) NOT NULL CHECK (costo > 0));
CREATE TABLE configuracion (
  id_configuracion INT PRIMARY KEY,
  id_solicitud INT NOT NULL UNIQUE REFERENCES solicitud(id_solicitud),
  costo_total NUMERIC(10,2) NOT NULL);
CREATE TABLE configuracion_componente (
  id_configuracion INT REFERENCES configuracion(id_configuracion),
  id_componente INT REFERENCES componente(id_componente),
  PRIMARY KEY (id_configuracion, id_componente));
CREATE TABLE recurso (
  id_recurso INT PRIMARY KEY,
  capacidad_maxima NUMERIC(10,2) NOT NULL,
  capacidad_disponible NUMERIC(10,2) NOT NULL,
  CHECK (capacidad_disponible >= 0 AND capacidad_disponible <= capacidad_maxima));
CREATE TABLE asignacion (
  id_asignacion INT PRIMARY KEY,
  id_solicitud INT NOT NULL UNIQUE REFERENCES solicitud(id_solicitud),
  id_recurso INT NOT NULL REFERENCES recurso(id_recurso),
  cantidad NUMERIC(10,2) NOT NULL,
  fecha_hora TIMESTAMP NOT NULL);
CREATE TABLE incidencia (
  id_incidencia INT PRIMARY KEY,
  tipo VARCHAR(40) NOT NULL,
  descripcion VARCHAR(255),
  fecha_hora TIMESTAMP NOT NULL);
