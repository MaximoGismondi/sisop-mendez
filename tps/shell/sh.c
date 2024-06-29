#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"
#include <signal.h>

char prompt[PRMTLEN] = { 0 };

static void handle_init_shell_error(char *error_message, stack_t *handler_stack);

static void
child_signal_handler()
{
	pid_t pid;
	int status;

	if ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		printf("==> terminado: PID=%i\n", pid);
	}
}

// runs a shell command
static void
run_shell()
{
	char *cmd;

	while ((cmd = read_line(prompt)) != NULL)
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
}

// initializes the shell
// with the "HOME" directory
static void
init_shell(stack_t handler_stack)
{
	char buf[BUFLEN] = { 0 };
	char *home = getenv("HOME");
	struct sigaction sa;

	sa.sa_handler = child_signal_handler;
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

	/* Indicates that all signals should be blocked during the execution of
	 * the child signal handler */
	sigfillset(&sa.sa_mask);

	if (sigaltstack(&handler_stack, NULL) == -1) {
		handle_init_shell_error(
		        "Falló la asignación del stack de manejo de señales.",
		        &handler_stack);
	}

	// Register signal handler for SIGCHLD (child process status change)
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		handle_init_shell_error("Falló la creación del handler para "
		                        "procesos en segundo plano.",
		                        &handler_stack);
	}

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(prompt, sizeof prompt, "(%s)", home);
	}
}

static void
handle_init_shell_error(char *error_message, stack_t *handler_stack)
{
	perror(error_message);
	free(handler_stack->ss_sp);
	exit(EXIT_FAILURE);
}

int
main(void)
{
	/* Initializes the handler dedicated stack */
	stack_t handler_stack;
	handler_stack.ss_sp = malloc(SIGSTKSZ);
	handler_stack.ss_size = SIGSTKSZ;
	handler_stack.ss_flags = SS_ONSTACK;

	init_shell(handler_stack);

	run_shell();

	free(handler_stack.ss_sp);

	return 0;
}
