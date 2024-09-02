# PA5: Diseño e Implementación de una Cadena de Procesamiento con Mecanismos de IPC

## Objetivo de Aprendizaje

Aplicar las técnicas de sincronización y comunicación entre procesos revisadas en el curso para crear una solución computacional que simule una cadena de procesamiento de datos. Esto incluye el diseño e implementación de la solución.

## Objetivos Específicos

1. **Diseño de la Cadena de Procesamiento (Pipeline):** 
   - Aplicar técnicas de IPC y sincronización usando las herramientas de la librería estándar de C.
   - Representar el diseño utilizando un Diagrama de Actividades UML junto con una descripción textual.

2. **Implementación:**
   - Escribir el programa o conjunto de programas que implementen la solución diseñada, usando técnicas de IPC y sincronización entre procesos de la librería estándar de C.

3. **Pruebas y Verificación:**
   - Realizar pruebas y recopilar los resultados para evaluar y verificar el correcto funcionamiento de la solución.

## Especificaciones

La solución debe incluir los siguientes componentes:

1. **Publicador:**
   - Programa que lee una imagen de entrada en formato BMP y la coloca en un recurso de memoria compartida.
   - Solicita recurrentemente al usuario la ruta donde se encuentra la imagen cargada.

2. **Desenfocador:**
   - Programa que lee cada imagen en el recurso compartido y aplica una transformación de desenfoque utilizando un kernel de blur.
   - El desenfoque se aplica únicamente a la primera mitad de la imagen.
   - El número de hilos es configurable y se define como parámetro antes de ejecutar el programa.

3. **Realzador:**
   - Programa que aplica un kernel de realce de bordes (edge detection) a la segunda mitad de la imagen.

### Requerimientos de Sincronización

1. Una vez que el Publicador haya cargado la imagen en el recurso compartido, se deben ejecutar como procesos independientes el Desenfocador y el Realzador.
2. Al finalizar, se deben combinar las dos porciones procesadas de la imagen en una sola imagen y guardarla en disco. La ruta de salida se define mediante un parámetro.
3. Todos los programas deben ejecutarse de forma concurrente.

## Especificaciones de las Salidas

Todos los programas deben ejecutarse desde consola o terminal, mostrando en pantalla el inicio y fin de cada etapa del procesamiento.

## Entregables

- Código estructurado y comentado adecuadamente, sin incluir código objeto o ejecutables. El archivo MAKE debe llamarse exactamente `makefile`.
- **Reporte (PDF):** 
  - (1) Antecedentes del problema.
  - (2) Diseño de la solución y su descripción.
  - (3) Limitaciones encontradas y cómo se resolvieron.
  - (4) Salidas de pantalla de las pruebas realizadas.
- **Video (3-5 minutos):** Descripción del diseño de la solución y ejecución de la cadena de procesamiento.

## Recomendaciones

- Al crear el empaquetado (`.zip`), no comprimas directamente la carpeta, selecciona los archivos y luego crea el paquete.
- No muestres, intercambies, ni copies código con nadie, y no busques una solución en la Web. ¡El plagio será detectado y severamente penalizado! Este debe ser un esfuerzo individual.

¡Gracias y buena suerte!
