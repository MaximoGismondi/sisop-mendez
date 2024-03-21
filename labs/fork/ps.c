#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>

int isNumber(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (!isdigit(str[i]))
            return 0;
    }
    return 1;
}

int toNumber(char *str)
{
    if (!isNumber(str))
    {
        return -1;
    }

    int result = 0;
    for (int i = 0; str[i] != '\0'; i++)
    {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}

int main()
{

    DIR *dir;
    struct dirent *entrada;

    dir = opendir("/proc");
    if (dir == NULL)
    {
        printf("No se pudo abrir el directorio /proc\n");
        return 1;
    }

    printf("%7s %s\n", "PID", "COMMAND");

    while ((entrada = readdir(dir)) != NULL)
    {
        int PID = toNumber(entrada->d_name);

        if (PID == -1)
        {
            continue;
        }

        char path[256];
        FILE *fp;

        snprintf(path, sizeof(path), "/proc/%d/comm", PID);
        fp = fopen(path, "r");

        if (!fp)
        {
            printf("No se pudo abrir %s\n", path);
            continue;
        }

        char nombre[256];

        if (fgets(nombre, sizeof(nombre), fp) != NULL)
        {
            printf("%7d %s", PID, nombre);
        }
        else
        {
            printf("%7d [No se pudo leer el nombre del proceso]\n", PID);
        }

        fclose(fp);
    }

    closedir(dir);
    return 0;
}