#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <origen> <destino>\n", argv[0]);
        exit(1);
    }

    char *origen = argv[1];
    char *destino = argv[2];

    int fd_origen = open(origen, O_RDONLY);

    if (fd_origen == -1)
    {
        fprintf(stderr, "Error el archivo %s no existe\n", origen);
        exit(1);
    }

    struct stat statbuf;

    if (fstat(fd_origen, &statbuf) == -1)
    {
        fprintf(stderr, "Error al obtener el tamaño del archivo %s\n", origen);
        exit(1);
    }

    void *origen_mmap = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd_origen, 0);

    if (origen_mmap == MAP_FAILED)
    {
        fprintf(stderr, "Error al mapear el archivo %s\n", origen);
        exit(1);
    }

    int fd_destino = open(destino, O_RDWR | O_CREAT | O_EXCL, 0666);

    if (fd_destino == -1)
    {
        fprintf(stderr, "Error el archivo %s ya existe\n", destino);
        exit(1);
    }

    if (ftruncate(fd_destino, statbuf.st_size) == -1)
    {
        fprintf(stderr, "Error al truncar el archivo %s\n", destino);
        exit(1);
    }

    void *destino_mmap = mmap(NULL, statbuf.st_size, PROT_WRITE, MAP_SHARED, fd_destino, 0);

    if (destino_mmap == MAP_FAILED)
    {
        perror("Error al mapear el archivo destino");
        fprintf(stderr, "Error al mapear el archivo %s\n", destino);
        exit(1);
    }

    memcpy(destino_mmap, origen_mmap, statbuf.st_size);

    munmap(origen_mmap, statbuf.st_size);
    munmap(destino_mmap, statbuf.st_size);

    close(fd_origen);
    close(fd_destino);

    return 0;
}