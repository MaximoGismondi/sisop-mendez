#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

char *split_line(char *buf, char splitter);

int block_contains(char *buf, char c);

int printf_debug(char *format, ...);
int fprintf_debug(FILE *file, char *format, ...);

void save_cmd(char *cmd);

char *get_hist_path();

int navigate_history(int direction, char *buffer);

void clear_last_line_stdout();

void clear_line_stdout();

int get_len_history();

void get_history_command(int index, char *buffer);

void handle_history_navigation(int key,
                               int *history_index,
                               int history_commands,
                               char *buffer,
                               char *buffer_aux,
                               int *pos_cursor);

void update_line_from_buffer(char *buffer);

void move_cursor_right(int *cursor_position, char *buffer);

void move_cursor_left(int *cursor_position);

void move_cursor_home(int *cursor_position);

void move_cursor_end(int *cursor_position, char *buffer);

void handle_cursor_navigation(int key,
                              char *buffer,
                              int *cursor_position,
                              bool ctrl_pressed);

void delete_character(char *buffer,
                      int *cursor_position,
                      int *cmd_length,
                      int *history_commands,
                      int *history_index,
                      char *buffer_aux);

void handle_arrow_key(char *buffer,
                      int *cursor_position,
                      int *history_commands,
                      int *history_index,
                      char *buffer_aux);

void handle_character_input(int c,
                            char *buffer,
                            int *cmd_length,
                            int *cursor_position,
                            int *history_commands,
                            int *history_index,
                            char *buffer_aux);

#endif  // UTILS_H
