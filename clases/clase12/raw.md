# Clase 12 - Maquinas Virtuales

Tratamos de virtualizar un sistema de computación completo donde corremos un sistema operativo completo casi sin modificaciones.

La idea es que podamos sobre el mismo hardware correr varios sistemas operativos al mismo tiempo.

Para esto se usa un VMM (Virtual Machine Monitor) que es un software que se encarga de virtualizar el hardware y permitir que varios sistemas operativos corran al mismo tiempo.

## Para qué sirve?

- **Diversidad de sistemas operativos**: Correr varios sistemas operativos en un solo hardware.
- **Consolidación de servidores**: Correr varios servidores en un solo hardware.
- **Aprovisionamiento rápido**: Crear máquinas virtuales es mucho más rápido que comprar hardware.
- **Alta disponibilidad**: Si una máquina virtual falla, se puede migrar a otra máquina virtual.
- **Seguridad**: Aislar aplicaciones y sistemas operativos.
- **Scheduling de recursos distribuidos**: Asignar recursos a las máquinas virtuales.
- **Cloud computing**: Correr máquinas virtuales en la nube.

## Definición

Una **máquina virtual** es un entorno de computación completo con sus propias capacidades de procesamiento, memoria y canales de comunicación, aisladas unas de otras.

## Requerimientos

- **Fidelidad**: La máquina virtual debe comportarse como si fuera una máquina real. Es decir un sistema operativo que corre en una máquina virtual no debería saber si está corriendo en una máquina virtual o en una máquina real.
- **Eficiencia**: La máquina virtual debe ser eficiente en términos de recursos. No debería haber una gran pérdida de performance al correr en una máquina virtual vs correr en una máquina real.
- **Seguridad**: Las máquinas virtuales deben estar aisladas unas de otras. No debería ser posible que una máquina virtual acceda a la memoria de otra máquina virtual.

## Tipos de VMM

### Tipo 1: Bare Metal

El VMM corre directamente sobre el hardware. Es decir, no hay un sistema operativo que corre sobre el hardware. El VMM es el que corre directamente sobre el hardware y luego corre las máquinas virtuales. Tenemos un Control Domain que es el SO que nos permite administrar las máquinas virtuales, probablemente por SSH. Este Control Domain es un SO (tipo Linux) con algunos comandos especiales para administrar las máquinas virtuales.

### Tipo 2: Hosted

El VMM corre sobre un sistema operativo. Es decir, primero corre un sistema operativo (**host**) y luego corren múltples VMM que se parecen a procesos los cuales permiten levantar un SO completo sobre cada uno de ellos (**guest**). Estos VMM no son procesos normales, sinó que tienen privilegios especiales para poder virtualizar el hardware.

## Técnicas de virtualización

### Multiplexado de recursos

Permite que varias máquinas virtuales compartan los recursos de la máquina física. Por ejemplo, si una máquina virtual no está usando la CPU, otra máquina virtual puede usarla.

### Agregación de recursos

Permite que una máquina virtual pueda usar más recursos de los que tiene la máquina física. Por ejemplo, si una máquina virtual necesita más CPU de la que tiene la máquina física, se le puede asignar más CPU.

### Emulación

Permite que una máquina virtual pueda correr en un hardware diferente al que fue diseñado. Por ejemplo, una máquina virtual diseñada para correr en un procesador Intel, puede correr en un procesador AMD.

## Ideas de como hacerlo

### Idea 1: Emular todas las instrucciones

- Ejemplo: DOSBox
- Emula una PC vieja con DOS
- Popular para ejecutar juegos viejos en plataformas modernas

Lo que hacemos es emular todas las instrucciones del procesador. Esto es muy lento porque cada instrucción del procesador tiene que ser traducida a al menos una o varias instrucciones del procesador host.

### Idea 2: Ejecución directa

- Ejemplo: Wine

Lo que hacemos es interceptar las llamadas al sistema operativo y traducirlas a llamadas al sistema operativo host. Esto se parece al concepto de las llamadas al sistema operativo en un proceso normal.
