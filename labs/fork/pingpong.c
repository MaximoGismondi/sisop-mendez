#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

#define FD_READ 0
#define FD_WRITE 1

int main()
{
    int fds_hijo_padre[2];
    int fds_padre_hijo[2];

    pipe(fds_padre_hijo);
    pipe(fds_hijo_padre);

    printf("Hola, soy PID <%d>\n", getpid());

    printf("  - IDs del primer pipe: [%d, %d]\n", fds_padre_hijo[FD_READ], fds_padre_hijo[FD_WRITE]);
    printf("  - IDs del segundo pipe: [%d, %d]\n", fds_hijo_padre[FD_READ], fds_hijo_padre[FD_WRITE]);

    pid_t forked_pid = fork();

    if (forked_pid != 0)
    {
        // PADRE
        close(fds_hijo_padre[FD_WRITE]);
        close(fds_padre_hijo[FD_READ]);

        long valor_random = random();

        printf("Donde fork me devuelve <%d>:\n", forked_pid);
        printf("  - getpid me devuelve: <%d>\n", getpid());
        printf("  - getppid me devuelve: <%d>\n", getppid());
        printf("  - valor random: <%ld>\n", valor_random);
        printf("  - envío valor <%ld> a través de fd=%d\n", valor_random, fds_padre_hijo[FD_WRITE]);

        write(fds_padre_hijo[FD_WRITE], &valor_random, sizeof valor_random);
        close(fds_padre_hijo[FD_WRITE]);

        long valor_leido;

        read(fds_hijo_padre[FD_READ], &valor_leido, sizeof valor_leido);
        close(fds_hijo_padre[FD_READ]);

        printf("Hola, de nuevo PID <%d>:\n", getpid());
        printf("  - recibí valor <%ld> vía fd=%d\n", valor_leido, fds_hijo_padre[FD_READ]);
    }
    else
    {
        // HIJO
        close(fds_hijo_padre[FD_READ]);
        close(fds_padre_hijo[FD_READ]);

        long valor_leido;

        read(fds_padre_hijo[FD_READ], &valor_leido, sizeof valor_leido);
        close(fds_padre_hijo[FD_READ]);

        printf("Donde fork me devuelve 0:\n");
        printf("  - getpid me devuelve: <%d>\n", getpid());
        printf("  - getppid me devuelve: <%d>\n", getppid());
        printf("  - recibo valor <%ld> via fd=%d\n", valor_leido, fds_padre_hijo[FD_READ]);
        printf("  - reenvío valor en fd=%d y termino\n", fds_hijo_padre[FD_WRITE]);

        write(fds_hijo_padre[FD_WRITE], &valor_leido, sizeof valor_leido);
    }

    return 0;
}