
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    int pid = fork();
    if (pid == 0)
    {
        // Proceso hijo
        printf("child: exiting\n");
    }
    else if (pid > 0)
    {
        // Proceso padre
        printf("parent: child=%d\n", pid);
    }
    else
    {
        // Error al hacer fork
        printf("fork error\n");
    }
    return 0;
}
