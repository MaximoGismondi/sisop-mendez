#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

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

    int fd_destino = open(destino, O_WRONLY | O_CREAT | O_EXCL, 0666);

    if (fd_destino == -1)
    {
        fprintf(stderr, "Error el archivo %s ya existe\n", destino);
        exit(1);
    }

    char buffer[BUFSIZ];
    ssize_t bytes_leidos;

    while ((bytes_leidos = read(fd_origen, buffer, BUFSIZ)) > 0)
    {
        write(fd_destino, buffer, bytes_leidos);
    }

    close(fd_origen);
    close(fd_destino);

    return 0;
}