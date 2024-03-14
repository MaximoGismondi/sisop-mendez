# Clase 1 - Introducción a Sistemas Operativos

Sys. OP = Kernel

Linux es un Kernel

Kernel + Apps = Distribucion

Distribucion o Distro:

- Ubuntu
- MacOS
- Windows

## Los Procer

Alan Turing: creador de la ciencia de la computación

John Von Neuman: implementa la computación de Alan

## Rol

Compartir una computadora entre varias programas de forma tal de prooveer un conjunto de servicios expuestos por el hardware

Los expone mediante una interfaz o API es decir abstrae el hardware de modo que los usuarios puedan usarlos.

API &rarr; System Calls

Por ejemplo:

- Pantalla
- E/S devices

### Orquestador

Gestiona los recursos entre varias aplicaciones, ejecutandosle en la misma comptadora física

### Ilusionista

Crea Abastracciones / Ilusiones de modo que sea fácil de usar

### Pegamento

Proove servicios comunes a todos los programas, por ejemplo el "copy-paste". Algo cohesivo para todo el sistema.

## Definiciones

### Sistema Operativo

Es la capa de sofware que maneja los recursos de una computadora que maneja los recursos de una computadora para sus usuarios y aplicaciones

### Virtualización

Trata de abstraer el hardware hacia algo facil de usar en el mundo virtual o informático.

## Unix

- "El" sisop
- Tiene todo lo que un OS necesita
- Es pago

## Filosofía LINUX

Modularizar, hacer que cada programa haga una cosa y lo haga bien.

Se usan pipes, la salida de un programa es la entrada del siguiente.

## Estructura

- Usuarios
- User Mode
  - Librerias.
  - Aplicaciones
- Kernel Mode
  - Subsystem
    - FS
    - VM
    - TCP/IP networking
    - Scheduling
  - Drivers
- Harware

Podemos estar en DUAL mode (User y Kernel al mismos tiempo) con una syscall

### Kernel Land

Kernel ejecutandose

### User Land

Donde viven las apps, se las denominan procesos.

Cada proceso en ejecución posee memoria con las instrucciones, los datos, el stack y el heap. Ademas

El contexto es aislado, protegido, restringido, y mediante bibliotecas se accede al hardware.

## System Calls

Es un punto de entrada controlado, al kernel. El KERNEL expone una API que provee diversos servicios.

## Procesos

Programa en ejecución
Es dinámico
Estructura interna propia
Viven en user-land

El Kernel tiene una Process Tables donde guarda la info de cada proceso por PID

### Partes básicas

- PID
- Nombre
- File Descriptor
- Memoria

### Memoria

- Stack: programa en ejecución
- Heap: memoria dinamica
- Data: variables globales (y variables normales ??)
- Code: instrucciones

### File descriptor

Identifican por un numero entero postivo a los archivos abiertos que se están usando. y 3 tipos:

- fd = 0 &rarr; Stdin, solo lectura
- fd = 1 &rarr; Stdout, solo escritura
- fd = 2 &rarr; StdErr, solo escritura

## API Resumida

- fork() -> pid : Crea un proceso y devuelve su id.
- wait(): Espera por un proceso hijo.
- getpid() -> pid: Devuelve el pid del proceso actual.
- exec(filename, argv): Carga un archivo y lo ejecuta.
- exit(): Termina el proceso actual.
- kill(pid): Termina el proceso cuyo pid es el parámetro.
- pipe(): abre un buffer de memoria en el cual el proceso puede leer por un extremo y escribir por el otro.
- dup(): Duplica el fd
- getppid() -> pid: Te da el pid del padre

### Fork

Un proceso SOLO se puede crear forkeando el proceso PADRE de todos

Crea una copia de un proceso padre el cual se denomina hijo y este a priori es una COPIA EXACTA

### Wait

Espera que cambie el estado el proceso hijo, pasado por parametro y obtiene info.

Cambio:

- termina
- parado tras señal
- continua tras señal

### getpid

Devuelve el pid del proceso actual

### exit

Para finalizar el proceso

### execve

Permite cambiar el contenido del hijo

Mediante argumentos: argv[]

Esto hace que el programa actual sea remplazado por el pasado por parametros

### dup

Duplica un fd el cual puede ser utilizado indifenrentemente entre el y el duplicado.

Mete el nuevo fd en donde haya lugar en la tabla de fd

### pipe

Es un buffer en el kernel que conecta 2 fd 1 para escritura y el otro para lectura. Al ir escribiendo por un extremo permite la lectura por el otro.

Recibe un arreglo de 2 enteros ccon los fdid

Leo el [0] y escribo en el [1]

Lo bueno es que se puede hacer un pipe y luego un fork y ahora tenes 2 procesos conectados.
