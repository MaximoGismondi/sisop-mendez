#ifndef BUILTIN_H
#define BUILTIN_H

#include "defs.h"

extern char prompt[PRMTLEN];

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);

int history(char *cmd);

int printf_debug(char *format, ...);

#endif  // BUILTIN_H
