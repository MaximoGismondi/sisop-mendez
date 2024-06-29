#include "builtin.h"
#include <string.h>

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit")) {
		return 0;
	}

	return 1;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strncmp(cmd, "cd", 2)) {
		return 0;
	}

	char *path = cmd + 3;

	if (path[0] == END_STRING) {
		path = getenv("HOME");
	}

	if (chdir(path) < 0) {
		perror("cd");
		return 0;
	}

	char dir[PRMTLEN] = { 0 };
	char new_promt[PRMTLEN] = { 0 };

	if (getcwd(dir, sizeof dir) == NULL) {
		perror("cd");
		return 0;
	}

	snprintf(new_promt, sizeof new_promt, "(%s)", dir);

	strncpy(prompt, new_promt, sizeof prompt);

	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd")) {
		return 0;
	}

	char current_dir[PRMTLEN] = { 0 };

	if (getcwd(current_dir, sizeof current_dir) == NULL) {
		perror("pwd");
		return 0;
	}

	printf_debug("%s\n", current_dir);

	return 1;
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	// Your code here

	return 0;
}
