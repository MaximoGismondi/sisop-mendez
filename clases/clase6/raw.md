# Clase 6

## Memoria Fisica y Virtual

La MMU se la puede pensar como una función matemática que tracuce las direcciones virtuales a direcciones físicas. La MMU se encarga de mapear todas las direcciones virtuales a direcciones físicas.

La memoria virtual es como el CPU ve la memoria. Y cada proceso tiene su propia tabla de páginas que mapea las direcciones virtuales. Cuando se hace un cambio de contexto, se cambia entre otras cosas la tabla de páginas.

## Sistemas de memoria virtual

### Early Days

La memoria estaba dividida en la sección del SO, y la sección de usuario. En la sección de usuario, cada vez que se quería hacer un cambio de contexto, se reemplazaba toda la memoria del proceso. Esto era muy ineficiente.

### Base and Bounds

Cada Address Space tiene un base y un bound. El base es la dirección física donde empieza la memoria del proceso, y el bound es el tamaño de la memoria del proceso. Esto introduce la protección de memoria pero no es muy eficiente, ya que no tiene la capacidad de crecer o decrecer la memoria de un proceso. Por esto mismo la fragmentación es un problema ya que no se aprovecha efientemente la memoria.

### Tabla de segmentos

Se divide la memoria en segmentos, HEAP, STACK, DATA, TEXT. Cada segmento tiene un base y un bound. Cada dirección virtual se compone de un indice de segmento y un offset. El indice de segmento se usa para buscar en la tabla de segmentos y obtener el base y el bound del segmento. Esto permite que exista seguridad pero seguía habiendo problemas de fragmentación.

### Paginación

La memoria de divide en FRAMES y cada frame tiene 4Kbits. Ahora tenemos una tabla de páginas que cada entrada (página) tiene asociada un frame. Cada dirección virtual se compone de un indice de página y un offset. El indice de página se usa para buscar en la tabla de páginas y obtener el frame asociado. Esto permite que la memoria sea más eficiente y que no haya fragmentación.

### Paginación (moderno)

Por último el sistema usado actualmente es el de paginación, pero por etapas donde cada proceso tiene un page directory (una tabla de tabla de páginas) y 1 o más page tables. Cada page table tiene 1024 entradas y cada entrada tiene 4 bytes. Cada dirección virtual se compone de un indice de page directory, uno o más indices de page table y un offset. El indice de page directory se usa para buscar en el page directory y obtener el page table. Luego se usa el indice de page table para buscar en la page table y obtener el frame asociado. Esto permite que el proceso crea que tiene toda la memoria para el solo y que a su vez la fragmentación sea mínima.

En 32 bits, la dirección virtual se compone de 10 bits para el indice de page directory, 10 bits para el indice de page table y 12 bits para el offset. (32 bits para cada dirección virtual)
En 64 bits, la dirección virtual se compone de 9 bits para el indice de page directory, 3 direcciones de page tables de 9 bits cada una y 12 bits para el offset. (48 bits para cada dirección virtual)
