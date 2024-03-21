# Clase teórica

## Arquitecturas

### ARM

Una de las arquitecturas más usadas en el día a día en dispositivos móviles. Es una arquitectura RISC, es decir, tiene instrucciones simples y pocas instrucciones. Es una arquitectura abierta.

### INTEL x86

Una de las arquitecturas más usadas en el día a día en computadoras de escritorio y servidores. Es una arquitectura CISC, es decir, tiene instrucciones complejas y muchas instrucciones. Es una arquitectura cerrada, es decir, no se puede hacer un procesador x86 sin pagarle a Intel.

### RISC V

Desarollado de modo educativo por el MIT, pensado para el ámbtio educativo. Es una arquitectura abierta, es decir, cualquiera puede hacer un procesador RISC V. Tiene una documentación muy buena y es muy fácil de entender de tan solo 200 páginas.

## POV: Usuario cuando ve un Sistema Operativo

Ve 3 cosas: el fyle system, el interprete de comandos y los bloques de contrucción.

## Protección

Lo mas importante que debe darte un Sistema Operativo es la protección. -> Un sistema operativo serio debe limitar la ejecución directa de código.

### Modo dual

El procesador tiene dos modos de ejecución: el modo usuario y el modo kernel. El modo kernel es el que tiene acceso a todas las instrucciones del procesador, mientras que el modo usuario tiene acceso a un subconjunto de las instrucciones del procesador.

Estas instrucciones especiales, que solo las puede ejecutar el kernel, se denominan instrucciones privilegiadas.

Debe haber un mecanismo por hardware que permita proteger a la memoria de que alguien que no tenga permisos no pueda acceder a ella.

Este modo se va intercambiando mediante un módulo llamado scheduler. Se encarga de despertar a los procesos que están dormidos y de dormir a los procesos que están despiertos cuando correspondan.

Cada vez que el kernel sale de modo kernel a modo usuario empieza un timer que cuando se termina, le devuelve el control al kernel. Esto se llama interrupcion por timer. Esto asegura que el kernel siempre tenga el control tras un tiempo determinado.

El kernel al final al final del día, es un proceso que tiene los privilegios para ejecutar instrucciones privilegiadas. El resto de procesos no.

para ver el timer `sudo cat /proc/timer_list`

### Handler

Cada Syscall tiene asosciado un handler que se encarga de verificar que el proceso que está pidiendo la syscall tenga los permisos necesarios para ejecutarla.

El KERNEL hace el setup del stack y le pasa el mando de la ejecución al programa en ejecución hasta que se llama un syscall. En ese momento se le envia una seña al KERNEL para que tome el control de la ejecución. El Kernel hace las verificaciones necesarias mediante el handler y luego le devuelve el control al programa en ejecución (user mode) nuevamente.

## Privilegios

Para saber que privilegios, revisa una flag que se llaman EFLAGS. Estas tienen 2 bits por lo que pueden tener 4 valores posibles.

Normalmente el 0 es el kernel mode y el 3 es el user mode.

Estos privilegios se checkean muchas veces a nivel hardware para evitar la ejecución directa de instrucciones privilegiadas para procesos que no tengan los permisos necesarios.

Podemos decir que hay rings o lands de privilegios que van desde el 0 al 3. El 0 es el más privilegiado y el 3 el menos privilegiado. Pudiendo el 0 usar todo el set de instrucciones del procesador y el 3 solo un subconjunto.

## Garantías

Multiplexación, aislamiento e interacción.

## Protección de memoria

Cada proceso, cree que tiene todo el espacio de memoria para el solo. El sistema operativo se encarga de que cada proceso no pueda acceder a la memoria de otro proceso. Esto se logra virtualizando, creando un espacio de memoria virtual que se le disponibiliza a cada proceso. Este mapeo lo hace la MMU (Memory Management Unit) a nivel hardware.

## Sandbox

El modo usuario y el modo kernel son entornos distintos, totalmente aislados.

## Mecanismos para pasar de modo usuario a modo kernel

### Syscall

### Excepción

### Interrupción

## Llamadas a SYSCALL

En C cuando llamamos a una syscall, en realidad estamos llamando a una función que se encarga de hacer la syscall, un wrapper. Esta función se llama `syscall` y se encuentra en la librería `libc`.

<https://x86.syscall.sh/>

Acá se detallan todas las syscalls que tiene un procesador x86, y como el procesador se debe preparar llamarlas a nivel registros.

Existe una syscall que se llama `syscall` que verifica que la syscall que se quiere hacer sea válida o no, en terminos de argumentos y permisos. Además trappea la ejecución del programa y le pasa el control al kernel. De una u otra forma llama a todas las demas syscalls por lo que siempre se llama a `syscall` para hacer una syscall.

La respuesta de una syscall es un número que se encuentra en el registro `rax`.

## Booteo

La palabra viene de 'boostrap' que significa 'levantarse uno mismo' y es la tirita que tienen los zapatos o botas. El booteo es el proceso de levantar el sistema operativo.

Este carga en la BIOS (Basic Input Output System) que es un programa que se encuentra en un chip de la placa madre. Este se encarga de ejecutar rutinas que inicializan la computadora

Entre ellos, deshabilita las interrupciones, setea algunos registros en valores por defecto, y pasa de modo real a modo protegido. Pone en un registro la direccion de inicio y llama al bootmain un programa que carga el kernel.
