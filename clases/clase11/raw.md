# Clase 11 - FileSystem

## Very simple life the one that maxi has when learning file system

El más simple está compuesto por 64 bloques de 4KB cada uno.

La estructura es la siguiente:

1. Superbloque (Metadata del filesystem)
2. Bitmap de inodos (que inodos están siendo utilizados y cuales no)
3. Bitmap de bloques (que bloques están siendo utilizados y cuales no)
4. Tabla de inodos (64 inodos de X bytes cada uno, en este caso 256 bytes) -> Implica que usamos 64*X/4KB bloques, en este caso 4 bloques
5. Bloques de datos (64 - 3 (SB, BI, BB) - (Bloques de la tabla de inodos)) -> En este caso 57 bloques

Del 1-4 todo es metadata, del 5 en adelante son bloques de datos.

## VFS: Virtual File System

Es el pegamento entre distintos sistemas de archivos. Permite que el kernel pueda acceder a distintos sistemas de archivos de manera transparente.

Por ejemplo en el user space se hace un `write`, luego se llama a `sys_write` en VFS y luego llama al correspondiente `write` del otro sistema de archivos.

Fat32: File Allocation Table 32

## API del FileSystem

Hay 2 tipos de API: las de metadata y las de datos

### open()

Esta función se encarga de abrir un archivo. Te devuelve el file descriptor correspondientes. Si no existe el archivo, lo crea. Open falla si no tenes permisos para abrir o si el archivo no existe y no se especifican los flags para crearlo.

### read()

Lee un archivo. Se le pasa el file descriptor, un buffer donde se va a guardar lo leído y la cantidad de bytes a leer. NUNCA asegura que lea la cantidad de bytes que le pedís. Intenta hacer el intento de lectura y lee como mucho la cantidad de bytes que le pedís.

### write()

Escribe en un archivo. Se le pasa el file descriptor, un buffer con los datos a escribir y la cantidad de bytes a escribir. NUNCA asegura que escriba la cantidad de bytes que le pedís. Intenta hacer el intento de escritura y escribe como mucho la cantidad de bytes que le pedís.

### lseek()

Mueve el cursor de lectura/escritura en el archivo. Se le pasa el file descriptor, la posición a la que se quiere mover y el modo en el que se quiere mover (desde el principio, desde el final, desde la posición actual). Devuelve la posisición relativa al principio del archivo.

3 -> ARCHIVO_PTR

### dup() y dup2()

Tenemos una tabla de files descriptors en donde se especifica para cada fd unas flags y el puntero al archivo. `dup` duplica un file descriptor y `dup2` duplica un file descriptor en un file descriptor específico. En ambos casos se mantienen las propiedades del file descriptor original por ejemplo, la posición del cursor. Se devuelve el nuebo fd a no ser que de un erros.

### link()

Crea un hardlink, es decir un puntero directamente al inodo. Este hardlink se guarda

### unlink()
