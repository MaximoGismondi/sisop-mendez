#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

#include <termios.h>
#include <signal.h>

char prompt[PRMTLEN] = { 0 };
struct termios saved_attributes;

static void reset_input_mode();

static void set_input_mode();

static void handle_init_shell_error(char *error_message, stack_t *handler_stack);

// reset the terminal to its original state
static void
reset_input_mode()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

// set the noncanonical input mode, without echo
static void
set_input_mode()
{
	if (!isatty(STDIN_FILENO))
		return;

	tcgetattr(STDIN_FILENO, &saved_attributes);

	struct termios tattr;

	tcgetattr(STDIN_FILENO, &tattr);
	tattr.c_lflag &= ~(ICANON | ECHO);
	tattr.c_cc[VMIN] = 1;
	tattr.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

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

	set_input_mode();
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

	if (isatty(STDIN_FILENO))
		reset_input_mode();

	free(handler_stack.ss_sp);

	return 0;
}
