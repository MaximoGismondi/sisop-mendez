# Clase 9 - Concurrencia

Cuando queremos tener cosas aisladas recurrimos a los procesos, ya que no comparten memorias, variables, etc.

Ahora si queremos cosas que NO estén aisladas, entonces recurrimos a los threads. Estos viven dentro de un proceso y comparten memoria, variables, etc.

El objetivo entonces, es escribir programas que se puedan ejecutar de forma simultanea y que puedan compartir recursos. Esto optimiza la velocidad aunque aumenta la complejidad del mismo.

## Threads

Un thread es una secuencia de ejecución atómica que representa una tarea planificable, es decir que se puede suspender y reanudar por el sistema operativo.

Se puede pensar que cada bloque de código `{ ... }` es un thread, ya que se ejecuta de forma atómica. Aún así se pueden crear threads a mano que permiten ejecutar tareas mientras otras se encuentran esperando (por ejemplo I/O).

### Elementos

- Thread id
- Conjunto los valores de registros
- Stack propio
- Política y prioridad de ejecución
- Propio errno
- Datos específicos del thread

### Threads vs Procesos

- **Proceso:** un programa en ejecución con
derechos restringidos.

- **Thread:** una secuencia independiente de
instrucciones ejecutándose dentro de un
programa.

### Thread scheduler

Al igual que con los procesos, existe un scheduler de threads que se encarga de dirigir la ejecución de los threads.

### Relación entre threads

- **Multi thread cooperativo:** no se interrumpen entre sí, a menos que lo hagan explícitamente.
- **Multi thread preemptivo:** se interrumpen entre sí, y el scheduler decide cuál sigue.

### Creación de Threads

Para crear un thread con la librería de C se utiliza la función `pthread_create`. Esta función recibe 4 parámetros:

1. Un puntero a la variable donde se guardará el id del thread.
2. Atributos del thread (NULL si no se quiere especificar).
3. La función que se ejecutará en el thread.
4. Argumentos de la función.

```c
#include <pthread.h>

void *func(void *arg) {
    // Hacer algo
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, func, NULL);
}
```

A nivel syscall, se utiliza `clone` para crear un thread. Esta también se usa para hacer fork. La diferencia entre fork y clone es que fork crea un proceso nuevo, mientras que clone crea un thread. Estos se diferencias por que memoria comparten.

Clone para crear un thread:

```C
clone(CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND, 0);
```
