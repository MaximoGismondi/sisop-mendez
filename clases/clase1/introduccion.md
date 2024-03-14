# Introducción a los Sistemas Operativos

Los sistemas operativos (SO) son el nexo esencial entre el hardware de una computadora y el usuario final, facilitando la interacción mediante una capa de software que gestiona eficientemente los recursos del sistema. A través de una interfaz o API, el SO ofrece servicios expuestos por el hardware, permitiendo a las aplicaciones realizar operaciones complejas sin necesidad de manejar directamente el hardware subyacente.

## Fundamentos de los Sistemas Operativos

### Componentes del Sistema Operativo

- **Kernel:** Es el núcleo central del SO, responsable de las operaciones de bajo nivel, como la gestión de memoria y procesos, y la comunicación con el hardware.
- **Aplicaciones de Usuario:** Conjunto de herramientas y programas que, junto al kernel, conforman lo que comúnmente se denomina una distribución en entornos Linux.

### Distribuciones

Las distribuciones (o "distros") se refieren a combinaciones específicas del kernel con un conjunto de aplicaciones diseñadas para ofrecer una experiencia completa al usuario. Ejemplos notables incluyen:

- **Linux:** Ubuntu, una distribución basada en el kernel de Linux.
- Aclaración: MacOS y Windows, mencionados previamente como distribuciones, son más adecuadamente clasificados como sistemas operativos completos desarrollados por Apple y Microsoft, respectivamente.

## Pioneros de la Computación

- **Alan Turing:** Reconocido como el padre de la ciencia de la computación, sus teorías sentaron las bases de la computación moderna.
- **John Von Neumann:** Implementó las teorías computacionales de Turing, contribuyendo al diseño arquitectónico de las computadoras actuales.

## Roles de un Sistema Operativo

El SO sirve múltiples propósitos esenciales, actuando como un orquestador que gestiona los recursos entre diversas aplicaciones, un ilusionista que crea abstracciones del hardware para facilitar su uso, y como pegamento que proporciona servicios comunes, como el copiar y pegar, a todas las aplicaciones.

## Conceptos Clave

### Sistema Operativo

Definido como la capa de software que administra los recursos hardware para los usuarios y sus aplicaciones, el SO es indispensable para la operación eficiente de una computadora.

### Virtualización

La virtualización abstrae el hardware para crear entornos de fácil uso, permitiendo a los usuarios ejecutar múltiples sistemas operativos o instancias de aplicaciones en una sola máquina física.

## Unix vs Linux

- **Unix:** Presentado como el sistema operativo esencial, proporciona todos los componentes necesarios de un SO pero es de pago.
- **Filosofía de Linux:** Se centra en la modularidad y eficiencia, donde cada programa realiza una función específica y se comunica con otros mediante pipes.

## Estructura de un Sistema Operativo

El SO se organiza en distintas capas, desde los usuarios hasta el hardware, pasando por modos de usuario y kernel que permiten una ejecución segura y eficiente de aplicaciones y servicios del sistema.

### Llamadas al Sistema (System Calls)

Las syscalls son interfaces fundamentales entre el software de aplicación y el kernel del SO, permitiendo a las aplicaciones solicitar servicios del kernel, como operaciones de entrada/salida, creación y gestión de procesos, y comunicaciones de red. Son esenciales para el desarrollo de software a nivel de sistema, especialmente en lenguajes como C.

#### Ejemplos de Llamadas al Sistema

- **fork():** Crea un nuevo proceso como copia del proceso padre.
- **exec():** Reemplaza el programa ejecutado por el proceso con un nuevo programa.
- **wait():** Espera a que un proceso hijo cambie de estado.
- **pipe():** Crea un canal de comunicación unidireccional entre procesos.
- **read(), write():** Operaciones de lectura y escritura en archivos o descriptores de archivo.

## Gestión de Procesos

Los procesos son programas en ejecución, cada uno con su propio espacio de dirección y recursos asignados. El kernel gestiona estos procesos, asignándoles CPU y memoria según sea necesario, y facilitando la comunicación entre ellos a través de mecanismos como pipes y señales.

### Componentes de un Proceso

Cada proceso se identifica por un PID (Identificador de Proceso) y tiene acceso a recursos como memoria (dividida en stack, heap, datos y código) y descriptores de archivo (stdin, stdout, stderr).

## Conclusión

Los sistemas operativos son la base sobre la que se ejecutan todas las aplicaciones, proporcionando una interfaz crítica entre el hardware y el software. Su comprensión es fundamental para cualquier persona que trabaje en el campo de la tecnología, especialmente para aquellos involucrados en el desarrollo de software a nivel de sistema.
