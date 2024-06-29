#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "defs.h"
#include "readline.h"
#include "ctype.h"

static char buffer[BUFLEN];

static char *read_line_interactive();
static char *read_line_not_interactive();

static char *
read_line_interactive()
{
	int cursor_position = 0;
	int cmd_length = 0;
	int history_commands = get_len_history();
	int history_index = history_commands;
	char c;

	char buffer_aux[BUFLEN];

	memset(buffer, 0, BUFLEN);

	while ((c = getchar()) != END_LINE && c != EOF && c != EOT) {
		if (c == BACKSPACE) {
			delete_character(buffer,
			                 &cursor_position,
			                 &cmd_length,
			                 &history_commands,
			                 &history_index,
			                 buffer_aux);
		} else if (c == ESC && getchar() == CSI) {
			handle_arrow_key(buffer,
			                 &cursor_position,
			                 &history_commands,
			                 &history_index,
			                 buffer_aux);
		} else {
			handle_character_input(c,
			                       buffer,
			                       &cmd_length,
			                       &cursor_position,
			                       &history_commands,
			                       &history_index,
			                       buffer_aux);
		}
	}

	if (c == EOF || c == EOT)
		return NULL;

	handle_cursor_navigation(END_KEY, buffer, &cursor_position, false);
	putchar(END_LINE);
	buffer[cursor_position] = END_STRING;

	return buffer;
}

static char *
read_line_not_interactive()
{
	int i = 0, c = 0;

	memset(buffer, 0, BUFLEN);

	c = getchar();

	while (c != END_LINE && c != EOF) {
		buffer[i++] = c;
		c = getchar();
	}

	if (c == EOF)
		return NULL;

	buffer[i] = END_STRING;

	return buffer;
}

// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *prompt)
{
#ifndef SHELL_NO_INTERACTIVE
	if (isatty(1)) {
		fprintf(stdout, "%s %s %s\n", COLOR_RED, prompt, COLOR_RESET);
		fprintf(stdout, "%s", "$ ");
	}
#endif

	if (isatty(STDIN_FILENO)) {
		return read_line_interactive();
	}
	return read_line_not_interactive();
}
