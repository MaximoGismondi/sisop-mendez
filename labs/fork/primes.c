#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define FD_READ 0
#define FD_WRITE 1

void filter_primes(int fd_input)
{
    int prime;
    if (read(fd_input, &prime, sizeof(prime)) <= 0)
    {
        return;
    }

    printf("primo %d\n", prime);

    int fds_filter[2];
    if (pipe(fds_filter) == -1)
    {
        printf("error in pipe\n");
        exit(1);
    }

    pid_t forked_pid = fork();
    if (forked_pid == -1)
    {
        printf("error in pipe\n");
        exit(1);
    }

    if (forked_pid > 0)
    {
        // PADRE
        close(fds_filter[FD_READ]);

        int number, res;
        while (read(fd_input, &number, sizeof(number)) > 0)
        {
            if (number % prime != 0)
            {
                res = write(fds_filter[FD_WRITE],
                            &number,
                            sizeof(number));

                if (res == -1)
                {
                    printf("error in write\n");
                    exit(1);
                }
            }
        }

        close(fds_filter[FD_WRITE]);
        wait(NULL);
    }
    else
    {
        // HIJO
        close(fd_input);
        close(fds_filter[FD_WRITE]);

        filter_primes(fds_filter[FD_READ]);
        close(fds_filter[FD_READ]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Uso: %s <n>\n", argv[0]);
        exit(1);
    }

    int n = atoi(argv[1]);
    if (n < 2)
    {
        printf("El número debe ser mayor o igual a 2.\n");
        exit(1);
    }

    int fds_generator[2];
    if (pipe(fds_generator) == -1)
    {
        printf("error in pipe\n");
        exit(1);
    }

    pid_t forked_pid = fork();
    if (forked_pid == -1)
    {
        printf("error in pipe\n");
        exit(1);
    }

    if (forked_pid > 0)
    {
        // GENERADOR
        int res;

        close(fds_generator[FD_READ]);
        for (int i = 2; i <= n; ++i)
        {
            res = write(fds_generator[FD_WRITE], &i, sizeof(i));

            if (res == -1)
            {
                printf("error in write\n");
                exit(1);
            }
        }
        close(fds_generator[FD_WRITE]);
        wait(NULL);
    }
    else
    {
        // FILTROS RECURSIVOS
        close(fds_generator[FD_WRITE]);
        filter_primes(fds_generator[FD_READ]);
        close(fds_generator[FD_READ]);
    }

    return 0;
}
