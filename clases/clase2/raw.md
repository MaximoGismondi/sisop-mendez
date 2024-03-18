# Clase práctica

## Definiciones

### Sistema Operativo

Conjunto de KERNEL + Interfaz de usuario + apps

### Ubuntu, Debian, Fedora, Red Hat, CentOS, Windows, MacOS

Son sistemas operativos con kernels algunos comunes otros distinos

### UNIX

Es un KERNEL que es el padre de todos los kernels. Es el primer kernel que se hizo. Es el padre de todos los sistemas operativos.

### GNU

### Linux

Es un Kernel basado de una forma u otra en UNIX. Es el kernel de Ubuntu, Debian, Fedora, Red Hat, CentOS, etc.

### Kenrel

Interactua con el hardware y le da una interfaz al cliente.

### Drivers

Son los pedacitos de KERNEL que interactuan con el hardware y le dan una interfaz al KERNEL. Estan en la frontera en el software y el harware

## Estándares

### POSIX

### SUSv4

## Filososfía UNIX

Hacer bien una cosa y hacerla bien. Todo es un archivo. Piping, la salida de un programa es la entrada de otro.

## Proceso

Es la estructura que abstrae y modela al procesador. Se identifican con un PID.

Tiene un binario que está ejecutandose, tiene un Heap para alocar memoria un Stack donde viven las variables y las funciones, Data donde viven las variables globales y variables estáticas, y un código que es el binario que está ejecutandose.

Por otro lado tiene unos files descriptors que son "puntero" que apuntan a entradas y salidas estandar de datos. Por un lado del 0, 1 y 2 que son la entrada, salida y error estandar respectivamente. Además los siguientes números son otras entradas y salidas abiertas como un archivo. Estas entrada y salidas son una API a un archivo, es decir pueden ser o no un archivo, pero se comportan como tal.

## Pipe

Cuando creamos unpipe, estamos creado un par de file descriptors conectados entre si. Esta conexión es unidireccional, y además bloqueante en ambos sentidos, es decir si el pipe está lleno el que escribe se bloquea y si el pipe está vacio el que lee se bloquea. Estos pipes funcionan como archivos "virtuales" que se comportan como un archivo pero no lo son.

## Execve

Es una llamada al sistema que reemplaza el proceso actual por otro proceso. Es decir, el proceso que llama a execve muere y es reemplazado por el proceso que se le pasa como argumento. Lo que se mantiene son los file descriptors y el PID.
