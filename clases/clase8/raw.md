# Clase 8

## Malloc

Usa la syscall `sbrk` para pedir memoria al sistema operativo y agrandar el heap.

### Implementación

Una posible implementación es allocar un bloque de memoria de tamaño `n` y devolver un puntero a la dirección de memoria donde comienza el bloque.

Detrás del bloque se guarda un header con información sobre el tamaño del bloque, de modo que se más fácil liberar la memoria.

Cuando alocamos memoria, que la cantidad que nos pidan alocar sea más pequeña que el tamaño de los bloques que tenemos disponibles. De ser así podemos optimizar los bloques particionando el bloque en dos bloques, uno de tamaño `n` y otro de tamaño `size - n` (obviamente considerando el tamaño de ambos headers).

## Free

Libera la memoria pedida con `malloc`.

Usa la syscall `brk` para devolver la memoria al sistema operativo.

### Siguiendo la implementación de `malloc`

Para liberar la memoria, se debe buscar el header que se encuentra detrás del bloque de memoria para obtener el tamaño del bloque y devolverlo al sistema operativo.

Los bloques de memoria liberados se guardan en una lista enlazada. Es decir que cada bloque (incluyendo el header) se asocia a un puntero al siguiente bloque. Este puntero se guarda en realidad en el header del bloque.

No solo se trata de una lista simplemente enlazada, sino que también se trata de una lista doblemente enlazada. Esto es para poder liberar la memoria de manera eficiente.

## Scheduler

Se encarga de planificar el orden de ejecución de los procesos siguiendo un algoritmo de planificación o política de planificación.

### Workload

Es la carga de trabajo que se le asigna al sistema operativo.

Se hacen diferentes suposiciones:

- Los procesos llegan al mismo tiempo.
- Está definido el tiempo de ejecución de cada proceso.
- No se terminan los procesos antes de que termine el tiempo de ejecución.
- Siempre se encuentra usando la CPU.

## Métricas

### Turnaround time

Es el tiempo de media que tarda un proceso en ejecutarse.

$Turnaround Time = Completion Time - Arrival Time$

Si se tiene más de un proceso, se puede calcular el turnaround time promedio.

### Response time

Es el tiempo que tarda un proceso en empezar a ejecutarse.

$Response Time = First Response Time - Arrival Time$

## Politicas de scheduling

### FIFO: First In First Out

Es simple pero tiene el problema de que si el primer proceso es muy largo, los demás procesos tendrán que esperar mucho tiempo. y el Turnaround Time será muy alto.

### SJF: Shortest Job First

Se ejecuta el proceso que tiene el menor tiempo de ejecución hasta su finalización.
Es mejor que FIFO en términos de Turnaround Time, ya que se ejecutan primero los procesos más cortos, pero tiene el problema de que los procesos largos pueden quedar en espera infinita o que cuando este se ejecute, se pierda mucho Turnaround Time de procesos llegados después.

### STCF: Shortest Time to Completion First

Es una variante de SJF que se ejecuta el proceso que tiene el menor tiempo de ejecución restante al momento. Cuando un proceso llega, se compara su tiempo de ejecución con el tiempo de ejecución de los procesos en ejecución y se decide si se hace el contexto switch.

### Round Robin (GOAT)

Se ejecutan los procesos en orden de llegada y se les asigna un quantum de tiempo. Si el proceso no termina en el quantum, se hace un contexto switch y se le asigna el siguiente proceso.

Tiene la ventaja de que los procesos no tienen que esperar mucho tiempo para ejecutarse, pero tiene el problema de que si el quantum es muy grande, se convierte en FIFO y si es muy chico, se convierte en SJF. Además se produce un overhead por el contexto switch.

### MLFQ: Multi Level Feedback Queue

Es una combinación de Round Robin y SJF. Se tienen varias colas de Round Robin con diferentes quantums y se asigna un proceso a la cola que corresponda según su tiempo de ejecución. Cada cola tiene una prioridad y se ejecutan primero los procesos de la cola con mayor prioridad.

Las reglas de asignación de procesos a las colas son:

- Cuando un proceso llega, se le asigna a la cola de mayor prioridad.
- Si un proceso no termina en su quantum, se le asigna a la cola de menor prioridad.
- Si un proceso renuncia a la CPU, se mantiene en la misma cola.

Las reglas de ejecución de procesos son:

- Si la prioridad de un proceso es mayor a la de otro, se le asigna a la cola de mayor prioridad. Por lo tanto, se ejecuta primero.
- Si la prioridad de un proceso es igual a la de otro, entonces se ejecutan en la misma cola con round robin.

Resuelve el problema de Round Robin de que si el quantum es muy grande, se convierte en FIFO y si es muy chico, se convierte en SJF. Además, se puede asignar un proceso a una cola con un quantum más grande si este proceso es largo.

Starvation: Un proceso no puede ejecutarse porque siempre hay procesos con mayor prioridad. Esto puede ocurrir o bien porque hay muchos procesos iterativos que ceden la CPU, o bien porque un proceso malicioso cede la CPU para conservar la prioridad. Para resolver esto, se puede implementar:

- **Boost:** Cada cierto tiempo, se aumenta la prioridad de todos los procesos y el round robin licua la ventaja que tienen los procesos iterativos.

- **Aging:** No se reinicia el quantum de un proceso si este no termina en su quantum. De esta manera, los procesos iterativos que ceden la CPU terminan perdiendo prioridad.
