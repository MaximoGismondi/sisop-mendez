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

## Locks

Cuando necesitamos que ciertas secciones de código se ejecuten con exclusión mutua, recurrimos a los locks. Estos se encargan de bloquear el acceso a ciertas secciones de código para que no se ejecuten de forma concurrente.

Los locks permiten sincronizar el acceso a recursos compartidos, evitando que dos threads accedan al mismo tiempo a un recurso compartido.

En terminos de implementación, se pueden crear de la siguiente forma:

```c
pthread_mutex_t lock;

int main() {
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_lock(&lock);
    // Sección crítica
    pthread_mutex_unlock(&lock);
}
```

En definitiva los locks tienen 2 operaciones básicas:

- **Lock:** bloquea la sección crítica para que otro thread no se ejecute.
- **Unlock:** desbloquea la sección crítica para que otro thread pueda ejecutarla.

### Propiedades de los Locks

- **Exclusión mutua:** solo un thread puede acceder a la sección crítica.
- **Progreso:** si nadie posee el lock, un thread puede tomarlo.
- **Espera limitada:** si un thread quiere tomar el lock, lo toma en un tiempo finito. Eventualmente el procesador le cede el lock a todos los que están esperando. Esto se llama "liveliness".

### Spinlocks

Los spinlocks son una variante de los locks que no bloquean el thread, sino que lo hacen esperar. Esto se logra con un ciclo que verifica si el lock está disponible. Si no lo está, el thread se queda ciclando hasta que lo esté.

Se usa cuando se espera que la espera sea corta, ya que si es larga, el thread se queda ciclando y desperdicia recursos. También cuando hay poca contención y cuando hay paralismo real (múltiples procesadores). Sinó no habría otro thread realmnente corriendo que pueda liberar el lock.

### Operaciones atómicas

Las operaciones atómicas son operaciones que se ejecutan de forma indivisible, es decir que no se pueden interrumpir. Estas operaciones son muy útiles para implementar locks. Estas operaciones se implementan a nivel de hardware. Unas de ellas son:

- *T _sync_lock_test_and_set(T \*ptr, T value, ...):* setea el valor de value en ptr y devuelve el valor anterior de ptr.
- *void __sync_lock_release(T \*ptr, ...):* setea el valor de ptr en 0.

### Sleep lock

El proceso que no puede tomar el lock, se duerme / blockea y se pone en una cola de espera. Cuando el proceso que tiene el lock lo libera, el SO puede hacer 2 cosas:

- Pueden despertar al primer proceso que se quedo esperando en la cola. Esto garantiza el liveliness.
- Pueden despertar a todos los procesos que se quedaron esperando en la cola y dejar que el race condition decida quien toma el lock.

Una implementación en xv6 puede ser:

```c
void
acquiresleep(struct sleeplock *lk)
{
    acquire(&lk->lk);

    while (lk->locked) {
        sleep(lk, &lk->lk);
    }

    lk->locked = 1;

    release(&lk->lk);
}

void
releasesleep(struct sleeplock *lk)
{
    acquire(&lk->lk);

    lk->locked = 0;
    wakeup(lk);

    release(&lk->lk);
}
```

### Spinlock vs Sleeplock

- **Spinlock:** el thread se queda ciclando hasta que el lock esté disponible.
- **Sleeplock:** el thread se duerme hasta que el lock esté disponible.

Se podría decir que spinlock es más proactivo y sleeplock es más reactivo.

### Deadlock

Un deadlock ocurre cuando 2 threads se quedan esperando mutuamente. Por ejemplo, si un thread 1 tiene el lock 1 y quiere el lock 2, y el thread 2 tiene el lock 2 y quiere el lock 1, ambos se quedan esperando mutuamente, freezeando la ejecución del programa. A esto se le llama deadlock.
