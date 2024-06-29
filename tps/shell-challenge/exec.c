#include "exec.h"

#define WRITE_PIPE_CLOSED 0
#define WRITE_PIPE_OPEN 1
#define STDERR_TO_STDOUT_MOD "&1"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}


// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		int equals_index = block_contains(eargv[i], '=');
		if (equals_index == -1)
			continue;

		char key[ENV_MAX_SIZE];
		char value[ENV_MAX_SIZE];
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, equals_index);

		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file

static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags == O_WRONLY) {
		fd = open(file, flags | O_CREAT | O_TRUNC, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}

	if (fd < 0) {
		perror("No se pudo crear el file descriptor.\n");
		exit(EXIT_FAILURE);
	} else
		return fd;
}

// redirects a file desriptor - does not return
static void
redirect_fd(char *file_name, int fd_redirect, int flags)
{
	int opened_fd = open_redir_fd(file_name, flags);
	int new_fd = dup2(opened_fd, fd_redirect);

	if ((new_fd) < 0) {
		perror("Error al duplicar un file descriptor.\n");
		exit(EXIT_FAILURE);
	}

	close(opened_fd);
}

// handles a fork error by closing opened pipes and freeing parsed_pipe
// allocated memory and exiting. Does not return.
static void
handle_fork_error(int pipefd[2], int write_pipe_status)
{
	if (write_pipe_status == WRITE_PIPE_OPEN) {
		close(pipefd[WRITE]);
	}
	close(pipefd[READ]);
	perror("Error al crear un proceso hijo.\n");
	free_command(parsed_pipe);
	exit(EXIT_FAILURE);
}

// shows an exec error message - does not return
static void
handle_exec_error(char *cmd)
{
	char exec_error[BUFLEN];
	sprintf(exec_error, "Error al ejecutar el comando '%s'", cmd);
	perror(exec_error);
	exit(EXIT_FAILURE);
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		e = (struct execcmd *) cmd;

		set_environ_vars(e->eargv, e->eargc);

		execvp(e->argv[0], e->argv);

		// If the program reaches here, an exec error occurred
		handle_exec_error(e->argv[0]);
		break;
	}

	case BACK: {
		b = (struct backcmd *) cmd;
		e = (struct execcmd *) b->c;

		set_environ_vars(e->eargv, e->eargc);

		execvp(e->argv[0], e->argv);

		// If the program reaches here, an exec error occurred
		handle_exec_error(e->argv[0]);
		break;
	}

	case REDIR: {
		r = (struct execcmd *) cmd;

		if (strlen(r->in_file) > 0) {
			redirect_fd(r->in_file, STDIN_FILENO, O_RDONLY);
		}

		if (strlen(r->out_file) > 0) {
			redirect_fd(r->out_file, STDOUT_FILENO, O_WRONLY);
		}

		if (strlen(r->err_file) > 0) {
			if (strcmp(r->err_file, STDERR_TO_STDOUT_MOD) == 0) {
				int new_fd = dup2(STDOUT_FILENO, STDERR_FILENO);
				if ((new_fd) < 0) {
					perror("Error al duplicar un file "
					       "descriptor.\n");
					exit(EXIT_FAILURE);
				}
			} else {
				redirect_fd(r->err_file, STDERR_FILENO, O_WRONLY);
			}
		}

		execvp(r->argv[0], r->argv);

		// If the program reaches here, an exec error occurred
		handle_exec_error(r->argv[0]);
		break;
	}

	case PIPE: {
		p = (struct pipecmd *) cmd;

		int pipefd[2];

		if (pipe(pipefd) < 0) {
			perror("Error al crear un pipe.\n");
			free_command(parsed_pipe);
			exit(EXIT_FAILURE);
		}

		pid_t pid_left = fork();

		if (pid_left < 0) {
			handle_fork_error(pipefd, WRITE_PIPE_OPEN);
		}

		if (pid_left == 0) {
			close(pipefd[READ]);
			int out_fd = dup2(pipefd[WRITE], STDOUT_FILENO);
			if ((out_fd) < 0) {
				perror("Error al duplicar un file "
				       "descriptor.\n");
				exit(EXIT_FAILURE);
			}

			close(pipefd[WRITE]);
			setpgid(0, 0);
			exec_cmd(p->leftcmd);
		}

		close(pipefd[WRITE]);

		pid_t pid_right = fork();

		if (pid_right < 0) {
			handle_fork_error(pipefd, WRITE_PIPE_CLOSED);
		}

		if (pid_right == 0) {
			int in_fd = dup2(pipefd[READ], STDIN_FILENO);
			if ((in_fd) < 0) {
				perror("Error al duplicar un file "
				       "descriptor.\n");
				exit(EXIT_FAILURE);
			}

			close(pipefd[READ]);
			setpgid(0, 0);
			exec_cmd(p->rightcmd);
		}

		close(pipefd[READ]);

		waitpid(pid_left, NULL, 0);
		waitpid(pid_right, NULL, 0);

		free_command(parsed_pipe);

		exit(EXIT_SUCCESS);

		break;
	}
	}
}
