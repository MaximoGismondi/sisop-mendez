#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

void execute_command(char *args[])
{
    pid_t forked_pid = fork();
    if (forked_pid == -1)
    {
        printf("Error en fork\n");
        exit(-1);
    }

    if (forked_pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            printf("Error en execvp\n");
            exit(-1);
        }
    }
    else
    {
        wait(NULL);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s <command>\n", argv[0]);
        exit(-1);
    }

    char *command = argv[1];
    char *args[NARGS + 2];

    size_t len;
    ssize_t line_size;

    args[0] = command;
    int args_index = 1;

    while ((line_size = getline(&args[args_index], &len, stdin)) > 0)
    {
        args[args_index][line_size - 1] = '\0';

        args_index++;

        if (args_index == NARGS + 1)
        {
            args[args_index] = NULL;
            execute_command(args);
            args_index = 1;
        }
    }

    if (args_index > 1)
    {
        args[args_index] = NULL;
        execute_command(args);
    }

    return 0;
}
