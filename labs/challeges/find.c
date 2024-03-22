#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int is_special_directory(const char *name)
{
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

void recursive_search(int dirfd, const char *path, const char *word, char *(*contains)(const char *, const char *))
{
    DIR *dir = fdopendir(dirfd);
    if (dir == NULL)
    {
        fprintf(stderr, "No se pudo abrir el directorio %s\n", path);
        close(dirfd);
        return;
    }

    struct dirent *entrada;
    while ((entrada = readdir(dir)) != NULL)
    {
        if (entrada->d_type == DT_DIR)
        {
            // DIRECTORIO
            if (is_special_directory(entrada->d_name))
            {
                continue;
            }

            char new_path[PATH_MAX];
            snprintf(new_path, PATH_MAX, "%s/%s", path, entrada->d_name);

            int new_dirfd = openat(dirfd, entrada->d_name, O_RDONLY | O_DIRECTORY);
            if (new_dirfd == -1)
            {
                fprintf(stderr, "No se pudo abrir el directorio %s\n", new_path);
                continue;
            }

            recursive_search(new_dirfd, new_path, word, contains);
            close(new_dirfd);
        }
        else
        {
            // ARCHIVO
            if (contains(entrada->d_name, word) != NULL)
            {
                char result_path[PATH_MAX];
                snprintf(result_path, PATH_MAX, "%s/%s", path, entrada->d_name);
                printf("%s\n", result_path + 2);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        fprintf(stderr, "Uso: %s [-i] <palabra>\n", argv[0]);
        exit(1);
    }

    char *word;
    char *(*contains)(const char *, const char *);

    if (argc == 3)
    {
        if (strcmp(argv[1], "-i") != 0)
        {
            fprintf(stderr, "Uso: %s [-i] <palabra>\n", argv[0]);
            exit(1);
        }

        word = argv[2];
        contains = &strcasestr;
    }
    else
    {
        word = argv[1];
        contains = &strstr;
    }

    DIR *dir;
    struct dirent *entrada;

    dir = opendir(".");
    if (dir == NULL)
    {
        printf("No se pudo abrir el directorio actual\n");
        exit(1);
    }

    int directory_fd = dirfd(dir);
    recursive_search(directory_fd, ".", word, contains);

    closedir(dir);
}
