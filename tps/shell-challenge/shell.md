# shell

## Preguntas

### Búsqueda en $PATH

> ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La diferencia principal entre `execve(2)` y `exec(3)`, es que la primera es una syscall (llamada al sistema) e interactúa directamente con el kernel de la computadora. Su funcionamiento puede variar dependiendo del sistema operativo en el que se ejecuta la llamada. En cambio, la familia de wrappers de `exec(3)` son funciones que proporciona la librería estándar de C para realizar esta misma acción desde un nivel de abstracción más alto. Es decir, las funciones de `exec(3)` utilizan internamente a `execve(2)`. Por otro lado, la familia de funciones `exec(3)` de C permiten distintos formatos para pasar argumentos/parámetros. Por último, el manejo de errores es distinto para ambas llamadas.

> ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Sí, la llamada a una función `exec(3)` puede fallar en caso de no poder ejecutar el archivo en la ruta pasada por parámetro, ya sea porque no existe, no se poseen los permisos necesarios o porque este ejecutable no está en el formato correcto.
En caso de que la ejecución fracase debido a un error, la llamada a `exec(3)` devuelve `-1`, la variable global `errno` se modifica para indicar este error y se continúa con la ejecución del programa desde donde se realizó la llamada.

En nuestra implementación de la shell, los errores en las llamadas a las funciones de la familia `exec(3)` se manejan de la siguiente forma:

``` c
// ...
execvp(e->argv[0], e->argv);

// If the program reaches here, an exec error occurred
handle_exec_error(e->argv[0]);
```

donde la función `handle_exec_error()` imprime un mensaje de error y finaliza la ejecución (con el código de error) del proceso asociado al comando que se debería haber ejecutado:

``` c
static void
handle_exec_error(char *cmd)
{
 char exec_error[BUFLEN];
 sprintf(exec_error, "Error al ejecutar el comando '%s'", cmd);
 perror(exec_error);
 exit(EXIT_FAILURE);
}
```

Luego de ejecutarse esta función, la shell (proceso padre) continúa su funcionamiento normalmente.

### Procesos en segundo plano

> Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Para implementar procesos en segundo plano, lo primero que se debe hacer es crear el proceso hijo. Una vez creado, en el proceso original (padre) en vez de hacer una llamada convencional a `waitpid`, se le debe agregar el flag `WNOHANG` provocando que el proceso hijo se ejecute en segundo plano. Esto significa que al llegar a esa línea, el proceso padre va a evaluar si su proceso hijo termino su ejecución o no, guardando esta información en la variable `status`. Sin embargo, al realizar este llamado de espera especificando el flag, no se va a bloquear su ejecución hasta que su proceso hijo termine, en cambio va a continuar con su ejecución normal, manteniendo el proceso hijo corriendo en segundo plano.  

### Flujo estándar

> Investigar el significado de 2>&1, explicar cómo funciona su forma general.

El agregado `2>&1` a un comando en la terminal Linux redirecciona la salida de error de la ejecución a la ubicación designada para la salida estándar. Por ejemplo, si típicamente el proceso que se ejecuta escribiría los errores aparecidos a un archivo `errores.txt`, y la salida del programa fuera impresa por la terminal, al ejecutarse con `2>&1` al final, ambas escrituras se realizarían imprimiendo por terminal, ya que esa era la ubicación de salida estándar.

El `2` representa el file descriptor de la salida estándar de error, el `>` indica que se debe redireccionar esta salida y el `&1` es el file descriptor de la salida estándar.

> Mostrar qué sucede con la salida de cat out.txt en el ejemplo.

```bash
 (/home/username) 
$ ls -C /home /noexiste >out.txt 2>&1
        Program: [ls -C /home /noexiste >out.txt 2>&1] exited, status: 2 
 (/home/username) 
$ cat out.txt
ls: no se puede acceder a '/noexiste': No existe el fichero o el directorio
/home:
username
        Program: [cat out.txt] exited, status: 0
```

Lo que ocurrió en este caso es que se ejecutó un comando con el modificador `>out.txt 2>&1`. La primera parte del modificador, `>out.txt`, redirecciona la salida estándar al archivo out.txt. La segunda parte, `2>&1` como se explicó anteriormente, redirecciona la salida de error al mismo lugar donde se encuentra la salida estándar: out.txt. Por lo tanto al leer el archivo, se encuentra que tanto el error producido por tratar de acceder a un archivo no existente, como la salida del comando `ls -C /home` se escribieron en el archivo `out.txt`.

> Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).

```bash
 (/home/username) 
$ ls -C /home /noexiste 2>&1 >out.txt
        Program: [ls -C /home /noexiste 2>&1 >out.txt] exited, status: 2
 (/home/username) 
$ cat out.txt
ls: no se puede acceder a '/noexiste': No existe el fichero o el directorio
/home:
username
        Program: [cat out.txt] exited, status: 0
```

Al invertir el orden de las redirecciones, nuestra implementación de shell tuvo el mismo comportamiento que en el caso anterior: tanto la salida estándar como la de error fueron redirigidas al archivo out.txt.

Sin embargo, el comportamiento de `bash(1)` es distinto al ingresar ese mismo comando:

```bash
$ ls -C /home /noexiste 2>&1 >out.txt
ls: no se puede acceder a '/noexiste': No existe el fichero o el directorio
$ cat out.txt
/home:
username
```

En este caso, primero se redirecciona la salida de error hacia la salida estándar. Luego, se redirecciona la salida del programa hacia el archivo out.txt, pero la salida de error sigue estando redirigida hacia la salida estándar. Es por esto que el mensaje de error de `ls` se muestra por pantalla, mientras que el archivo out.txt solo contiene la salida de `ls` (sin el error).

### Tuberías múltiples

> Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe. ¿Cambia en algo?

Cuando se ejecuta un pipe, el exit code que reporta la shell es aquel que retorna el comando final del pipe, es decir el último comando que se realiza en la cadena de ejecución.

> ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

Como se mencionó anteriormente, si el último comando falla, la shell reporta el exit code de ese comando:

```bash
$ ls | grep "Hipopótamos"
$ echo $?
1
```

Sin embargo, cuando falla algún comando previo al último, el flujo de ejecución se interrumpe, la shell reporta el exit code devuelto por este comando y se ignoran todos los comandos posteriores:

```bash
$ echo "Hola Mundo" | noExiste | grep "Mundo"
noExiste: command not found
$ echo $?
1
```

En nuestra implementación, en caso de ocurrir un error en algunos de los comandos intermedios, se intenta continuar el flujo de ejecución hasta el último comando:

```bash
 (/home/username) 
$ echo "Hola Mundo" | noExiste | grep "Mundo"
Error al ejecutar el comando 'noExiste': No such file or directory
 (/home/username) 
$ echo $?
0
```

De todos modos el exit code que reporta la shell es el del proceso que se encarga de gestionar los pipes, que como realizó su tarea con éxito, reporta un exit code de 0.

### Variables de entorno temporarias

> ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Las variables de entorno se heredan a los procesos hijos. En el caso de las variables de entorno temporarias, se desea que un unico proceso tenga esa variable de entorno, y que los demas procesos no. Es por esto que estas variables de entorno se deben establecer luego de la llamada `fork`, ya que nos aseguramos que solo el proceso hijo tenga estas variables.

> En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).  ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.

Al pasar un arreglo de variables de entorno a una funcion de la familia de `exec(3)` del tipo _environment_, estas variables de entorno solo se utilizan para el nuevo proceso que se inicia con `exec`, reemplazando completamente el entorno del proceso actual y no permitiendo cambiar las variables de entorno despues de `exec(3)`.

En cambio, al utilizar `setenv(3)` despues de `fork(3)`, se modifican las variables de entorno del proceso hijo, y estos cambios persisten en el proceso hijo incluso despues de que se complete la llamada a `exec`.

Esto nos proporciona un comportamiento mas dinamico, ya que permite cambios individuales y en cualquier momento en las variables de entorno del proceso hijo. A diferencia de la funciones `exec(3)` de tipo _environment_ que permiten establecer variables de entorno una sola vez al inicio.

> Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

Para obtener un comportmiento igual se podria crear un array que contenga las variables actuales, que se obtiene de la variable externa `environ`, y ademas las variables que se deseen agregar. Cada elemento del arreglo seria una cadena en el formato "variable=valor".
Con este array se llama por ejemplo a la funcion `execve(3)` como tercer argumento para ejecutar el comando actual. De esta forma el proceso hijo siempre recibira las variables de entorno necesarias.

### Pseudo-variables

> Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).

* `$!`: contiene el PID del proceso del ultimo comando ejecutado en segundo plano.
* `$$`: cotiene el PID del proceso del script o shell actual
* `$@`: contiene todos los argumentos pasados a un script o a un comando. Preserva los espacios en blanco entre ellos.

```bash
$ evince file.pdf &
 [PID=2489]

$ echo $!
2489 
```

```bash
$ ps
    PID TTY          TIME CMD
  24499 pts/0    00:00:00 bash
  24576 pts/0    00:00:00 ps

$ echo $$
24499
```

```bash
$ cat some-script
#! /bin/sh
echo "$@"

$ ./some-script "hola"
hola
```

### Comandos built-in

> ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in?

El comando `cd` no se puede implementar como un proceso separado ya que intenta cambiar el directorio de trabajo del proceso vinculado a la shell, lo cual no es posible si se ejecuta en un proceso separado.

Por otro lado `pwd` solo se encarga de consultar el directorio de trabajo del proceso principal y mostrarlo por pantalla por lo que se podría resolver con una archivo o variable de entorno que almacene el directorio de trabajo de la shell.

Si bien esto es posible, `pwd` se debería implementar como built-in para mantener la simplicidad y la encapsulación de la shell evitando así pasos adicionales a la hora de cambiar el directorio de trabajo.

### Segundo plano avanzado

> Explicar detalladamente el mecanismo completo utilizado.

El mecanismo utilizado se basa en el uso de señales y el manejo de los `PGID` de los procesos, el cual se detalla mas adelante.  La señal  utilizada es `SIGCHLD`, la cual es enviada al proceso padre cuando un proceso hijo cambia de estado. Para poder atrapar esta señal se utiliza la syscall `sigaction`, la cual configura una función handler para la señal pasada por parametro. Para esto se le pasa un struct con la función handler y ciertos flags, que indican como se debe manejar la señal. En este caso se utiliza `SA_RESTART` para que si la syscall es interrumpida por la señal, se vuelva a ejecutar. Además se utiliza `SA_NOCLDSTOP` para que no se envíe la señal cuando un hijo se detiene.

```c
struct sigaction sa;
sa.sa_handler = child_signal_handler;
sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

...

if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        // Error handling
}
```

Donde la función `child_signal_handler` es:

```c
static void
child_signal_handler(int action_number)
{
	pid_t pid;
	int status;

	if ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf("==> terminado: PID=%i\n", pid);
	}
}
```

Es importante destacar que cualquier proceso hijo emitirá la señal, por lo que nos interesa diferenciar los procesos en segundo plano de los de primer plano. Una solución a esto es cambiar el `PGID` de todos los procesos que no esten en segundo plano y dejar a los procesos en segundo plano con el mismo `PGID` que la shell, ya que por default todos los procesos hijos de la shell heredan su `PGID`. Para lograr esto, se utiliza la función `setpgid` con la cual se setea el `PGID` del proceso en primer plano a su propio `PID` pasando cero en ambos argumentos.

```c
if (parsed->type != BACK) {
 setpgid(0, 0);
}
```

Esto se hace despues de hacer fork en el proceso hijo, y antes de ejecutar el comando. De esta manera, todos los procesos que no esten en segundo plano tendran un `PGID` distinto al de la shell, y los procesos en segundo plano tendran el mismo `PGID` que la shell.

Adicionalmente, es deseable que el handler de `SIGCHLD` se ejecute en un stack distinto al utilizado por la shell. Esto se debe a que, en caso contrario, un error en el manejo de la señal puede provocar fallos en la ejecución de toda la shell.

Para crear un stack alternativo, debemos crear un struct de tipo `stack_t`. Uno de sus campos (`ss_sp`) contiene el stack pointer del nuevo stack, el cual se puede obtener reservando una porción de memoria en el heap:

```c
stack_t handler_stack;
handler_stack.ss_sp = malloc(SIGSTKSZ);
handler_stack.ss_size = SIGSTKSZ;
handler_stack.ss_flags = SS_ONSTACK;
```

`SIGSTKSZ` está definido como un tamaño estándar lo suficientemente grande para un stack de manejo de señales. El flag `SS_ONSTACK` le dice al sistema que el stack alternativo está disponible para utilizarse.

Una vez hecho esto, se le asigna el stack alternativo al manejo de señales con la syscall `sigaltstack(2)`:

```c
if (sigaltstack(&handler_stack, NULL) == -1) {
        // Error handling
}
```

> ¿Por qué es necesario el uso de señales?

El uso de señales es necesario ya que estas proveen un forma de comunicacion entre procesos y control de los mismos. A su vez, funcionan como notficiaciones de eventos asicronicos que han sucedido, como la notificacion al proceso padre de la terminacion de un proceso hijo con la señal `SIGCHLD` o la notificacion para interrumpir la ejecución de un programa en la terminal con Ctrl+C mediante la señal `SIGINT`.


### Historial

> ¿Cuál es la función de los parámetros `MIN` y `TIME` del modo no canónico?

A la hora de configurar el modo no canónico, estos dos parámetros determinan en qué circunstancia la shell considera que la lectura está completa y debe ser ejecutada.
El parametro `MIN` determina cuántos caracteres se deben recibir como mínimo para que la lectura se complete. Hasta que no se cumple esta condición, la lectura de la entrada se bloquea y el comando no se ejecuta.
El parámetro `TIME` determina cuánto tiempo va a pasar desde que se recibe el primer caracter hasta que se completa la lectura. Esta medida se expresa en décimas de segundos. Si se recibe otro carácter antes de que transcurra el tiempo especificado por `TIME`, el temporizador de `TIME` se reinicia.

Hace falta que se cumpla una de las dos condiciones para que la lectura se considere completa. En otras palabras, una vez transcurridas las `TIME` decimas de segundo, no se espera a que se cumpla la condición de `MIN` caracteres para completar la lectura, ni viceversa.

> ¿Qué se logra en el ejemplo dado al establecer a `MIN` en 1 y a `TIME` en 0?

Establecer `TIME` en 0, provoca que no se temporice el tiempo desde que se escribe el primer caracter, es decir que el instante que se cumpla la condición de `MIN`, la lectura se considerará completa.

De todos modos, al establecerse un valor de `MIN` igual a `1` el valor de `TIME` deja de tener efecto, ya que al escribirse el primer caracter la lectura se considerará completa por lo que no hay posiibilidad de que el temporizador de `TIME` interfiera en la lectura de caracteres.

Está implementación es ideal para una shell ya que el comportamiento deseado, es del de leer caracter por caracter sin retraso alguno.
