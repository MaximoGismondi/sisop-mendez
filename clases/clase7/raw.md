# Clase 7 -

## Modo Protegido x86

### Global Descriptor Table (GDT)

Tabla que guarda en mmeoria, apuntanda por el registro GDTR. Hay una en todo el sistema y siempre es accesible.

### Local Descriptor Table (LDT)

Tabla que guarda en memoria, apuntada por el registro LDTR. Es opcional y es accesible solo por el proceso que la carga.

### Segment Selectors

### Segment Descriptors

## Cache de Memoria

La cache de memoria es una memoria de acceso rápido que se utiliza para almacenar temporalmente los datos que se utilizan con mayor frecuencia. La cache de memoria es mucho más rápida que la memoria principal, por lo que el acceso a los datos almacenados en la cache es mucho más rápido que el acceso a los datos almacenados en la memoria principal.

Hay varios niveles de cache, cada uno de los cuales es más rápido que el anterior. La cache de nivel 1 (L1) es la más rápida y la cache de nivel 3 (L3) es la más lenta. La cache se encuentra dentro del procesador.

### Cache a nivel MMU

Dentro de la MMU, hay varias caches. Entre ellas:

- **TLB (Translation Lookaside Buffer):** Cache de traducción de direcciones. Por cada página de memoria, se guarda la traducción de la entrada de la tabla de páginas.
