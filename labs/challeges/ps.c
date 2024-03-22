#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

int main()
{
    DIR *dir;
    struct dirent *entrada;

    dir = opendir("/proc");
    if (dir == NULL)
    {
        fprintf(stderr, "No se pudo abrir el directorio /proc\n");
        return 1;
    }

    printf("%7s %s\n", "PID", "COMMAND");

    while ((entrada = readdir(dir)) != NULL)
    {
        int PID = atoi(entrada->d_name);

        if (PID == 0)
        {
            continue;
        }

        char path[PATH_MAX];
        FILE *fp;

        snprintf(path, sizeof(path), "/proc/%d/comm", PID);
        fp = fopen(path, "r");

        if (!fp)
        {
            fprintf(stderr, "No se pudo abrir %s\n", path);
            continue;
        }

        char nombre[NAME_MAX];

        if (fgets(nombre, sizeof(nombre), fp) != NULL)
        {
            printf("%7d %s", PID, nombre);
        }
        else
        {
            fprintf(stderr, "No se pudo leer el nombre del proceso %d\n", PID);
        }

        fclose(fp);
    }

    closedir(dir);
    return 0;
}