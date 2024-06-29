
#include "utils.h"
#include <stdarg.h>

// splits a string line in two
// according to the splitter character
char *
split_line(char *buf, char splitter)
{
	int i = 0;

	while (buf[i] != splitter && buf[i] != END_STRING)
		i++;

	buf[i++] = END_STRING;

	while (buf[i] == SPACE)
		i++;

	return &buf[i];
}

// looks in a block for the 'c' character
// and returns the index in which it is, or -1
// in other case
int
block_contains(char *buf, char c)
{
	for (size_t i = 0; i < strlen(buf); i++)
		if (buf[i] == c)
			return i;

	return -1;
}

// Printf wrappers for debug purposes so that they don't
// show when shell is compiled in non-interactive way
int
printf_debug(char *format, ...)
{
#ifndef SHELL_NO_INTERACTIVE
	va_list args;
	va_start(args, format);
	int ret = vprintf(format, args);
	va_end(args);

	return ret;
#else
	return 0;
#endif
}

int
fprintf_debug(FILE *file, char *format, ...)
{
#ifndef SHELL_NO_INTERACTIVE
	va_list args;
	va_start(args, format);
	int ret = vfprintf(file, format, args);
	va_end(args);

	return ret;
#else
	return 0;
#endif
}

// saves a command to the history file
void
save_cmd(char *cmd)
{
	char *path = get_hist_path();

	int fd = open(path, O_CREAT | O_WRONLY | O_APPEND, S_IWUSR | S_IRUSR);

	if (fd < 0) {
		// Si no se puede abrir el archivo (por problemas de permisos,
		// por ejemplo), no se utiliza la funcionalidad de history.
		return;
	}

	if (write(fd, cmd, strlen(cmd)) != (ssize_t) strlen(cmd)) {
		perror("save_cmd");
		return;
	}

	char end_line = END_LINE;

	if (write(fd, &end_line, 1) != 1) {
		perror("Failed to write newline to history file");
	}

	close(fd);
}

// returns the path of the history file
char *
get_hist_path()
{
	char *path = getenv("HISTFILE");

	if (path == NULL) {
		path = HISTORY_PATH;
	}

	if (path[0] == '~') {
		char *home = getenv("HOME");
		char new_path[BUFLEN];

		snprintf(new_path, BUFLEN, "%s%s", home, path + 1);
		path = strdup(new_path);

		if (path == NULL) {
			perror("Memory allocation failed");
			exit(EXIT_FAILURE);
		}
	}

	return path;
}

// Cleans the line and prints the $ sign for input
void
clear_line_stdout()
{
	printf_debug("\33[2K\r");
	putchar('$');
	putchar(SPACE);
}

// returns the index of the line next to last in the history file
int
get_len_history()
{
	char *history_path = get_hist_path();
	FILE *file = fopen(history_path, "r");

	if (!file) {
		perror("history");
		return 0;
	}

	char line[BUFLEN];

	int i = 0;

	while (fgets(line, BUFLEN, file) != NULL) {
		i++;
	}

	fclose(file);
	return i;
}

// returns the command at the cmd_index in the history file
void
get_history_command(int cmd_index, char *buffer)
{
	char *history_path = get_hist_path();
	FILE *file = fopen(history_path, "r");

	if (!file) {
		perror("history");
		return;
	}

	char line[BUFLEN];

	int i = 0;

	while (fgets(line, BUFLEN, file) != NULL) {
		if (i++ == cmd_index) {
			size_t len = strlen(line);

			if (line[len - 1] == '\n') {
				line[len - 1] = '\0';
			}

			strcpy(buffer, line);
			return;
		}
	}

	fclose(file);
}

// handles the arrow keys for history navigation
void
handle_history_navigation(int key,
                          int *history_index,
                          int history_commands,
                          char *buffer,
                          char *buffer_aux,
                          int *pos_cursor)
{
	if (key == ARROW_UP && *history_index > 0) {
		if (*history_index == history_commands) {
			strcpy(buffer_aux, buffer);
		}
		(*history_index)--;
	} else if (key == ARROW_DOWN && *history_index < history_commands) {
		(*history_index)++;
	} else {
		return;
	}

	clear_line_stdout();
	memset(buffer, 0, BUFLEN);

	if (*history_index < history_commands) {
		get_history_command(*history_index, buffer);

		for (size_t i = 0; i < strlen(buffer); i++) {
			putchar(buffer[i]);
		}

	} else if (*history_index == history_commands) {
		strcpy(buffer, buffer_aux);
		memset(buffer_aux, 0, BUFLEN);

		for (size_t i = 0; i < strlen(buffer); i++) {
			putchar(buffer[i]);
		}
	}

	*pos_cursor = strlen(buffer);
}

// updates the line with the contents of the buffer
void
update_line_from_buffer(char *buffer)
{
	for (size_t i = 0; i < strlen(buffer); i++) {
		putchar(buffer[i]);
	}
}
// moves the cursor right one position
void
move_cursor_right(int *cursor_position, char *buffer)
{
	int len = strlen(buffer);
	if (*cursor_position < len) {
		printf("\033[C");
		(*cursor_position)++;
	}
}

// moves the cursor left one position
void
move_cursor_left(int *cursor_position)
{
	if (*cursor_position > 0) {
		printf("\033[D");
		(*cursor_position)--;
	}
}

// moves the cursor to the beginning of the line
void
move_cursor_home(int *cursor_position)
{
	if (*cursor_position > 0) {
		printf("\033[%dD", *cursor_position);
		*cursor_position = 0;
	}
}

// moves the cursor to the end of the line
void
move_cursor_end(int *cursor_position, char *buffer)
{
	int len = strlen(buffer);
	if (*cursor_position < len) {
		printf("\033[%dC", len - *cursor_position);
		*cursor_position = len;
	}
}

void
handle_cursor_navigation(int key, char *buffer, int *cursor_position, bool ctrl_pressed)
{
	int len = strlen(buffer);

	if (ctrl_pressed) {
		if (key == ARROW_RIGHT) {
			while (*cursor_position < len &&
			       buffer[*cursor_position] != SPACE &&
			       buffer[*cursor_position] != '\0') {
				move_cursor_right(cursor_position, buffer);
			}
		} else if (key == ARROW_LEFT) {
			while (*cursor_position > 0 &&
			       buffer[*cursor_position - 1] != SPACE) {
				move_cursor_left(cursor_position);
			}
		}
	}

	if (key == ARROW_RIGHT)
		move_cursor_right(cursor_position, buffer);
	else if (key == ARROW_LEFT)
		move_cursor_left(cursor_position);
	else if (key == HOME_KEY)
		move_cursor_home(cursor_position);
	else if (key == END_KEY)
		move_cursor_end(cursor_position, buffer);
}

// deletes a character from the input line and updates the buffer accordingly
void
delete_character(char *buffer,
                 int *cursor_position,
                 int *cmd_length,
                 int *history_commands,
                 int *history_index,
                 char *buffer_aux)
{
	if (*cursor_position > 0) {
		putchar(CURSOR_BACK);

		(*cmd_length)--;
		(*cursor_position)--;

		int aux_cursor_position = *cursor_position;

		while (aux_cursor_position < *cmd_length) {
			buffer[aux_cursor_position] =
			        buffer[aux_cursor_position + 1];

			putchar(buffer[aux_cursor_position]);

			if (*history_commands == *history_index) {
				buffer_aux[aux_cursor_position] =
				        buffer[aux_cursor_position + 1];
			}

			aux_cursor_position++;
		}

		putchar(SPACE);
		aux_cursor_position++;

		while (aux_cursor_position > *cursor_position) {
			handle_cursor_navigation(
			        ARROW_LEFT, buffer, &aux_cursor_position, false);
		}

		buffer[*cmd_length] = END_STRING;
		if (*history_commands == *history_index) {
			buffer_aux[*cmd_length] = END_STRING;
		}
	}
}

// handles the arrow keys for history and cursor navigation
void
handle_arrow_key(char *buffer,
                 int *cursor_position,
                 int *history_commands,
                 int *history_index,
                 char *buffer_aux)
{
	int key = getchar();

	if (key == ARROW_UP || key == ARROW_DOWN) {
		handle_history_navigation(key,
		                          history_index,
		                          *history_commands,
		                          buffer,
		                          buffer_aux,
		                          cursor_position);
	} else if (key == HOME_KEY || key == END_KEY || key == ARROW_RIGHT ||
	           key == ARROW_LEFT) {
		handle_cursor_navigation(key, buffer, cursor_position, false);
	} else if (getchar() != MARGIN_KEYS) {
		getchar();
		key = getchar();
		if (key == ARROW_RIGHT || key == ARROW_LEFT) {
			handle_cursor_navigation(key, buffer, cursor_position, true);
		}
	}
}

// handles the normal character input and updates the buffer accordingly
void
handle_character_input(int c,
                       char *buffer,
                       int *cmd_length,
                       int *cursor_position,
                       int *history_commands,
                       int *history_index,
                       char *buffer_aux)
{
	int aux_cursor_position = *cursor_position;

	handle_cursor_navigation(END_KEY, buffer, &aux_cursor_position, false);

	while (aux_cursor_position > *cursor_position) {
		buffer[aux_cursor_position] = buffer[aux_cursor_position - 1];
		if (*history_commands == *history_index) {
			buffer_aux[aux_cursor_position] =
			        buffer[aux_cursor_position - 1];
		}

		putchar(buffer[aux_cursor_position]);
		handle_cursor_navigation(
		        ARROW_LEFT, buffer, &aux_cursor_position, false);
		aux_cursor_position++;
		handle_cursor_navigation(
		        ARROW_LEFT, buffer, &aux_cursor_position, false);
	}

	buffer[*cursor_position] = c;
	if (*history_commands == *history_index) {
		buffer_aux[*cursor_position] = c;
	}
	putchar(c);

	(*cursor_position)++;
	(*cmd_length)++;
}
