// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/env.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/sched.h>
#include <kern/env.h>

#define CMDBUF_SIZE 80  // enough for one VGA text line

extern envid_t env_history[HISTORY_LENGTH];
extern uint32_t env_history_index;

#define SEPARATOR "=================================================\n"
#define SCHEDULER_STATS_TITLE                                                  \
	"------------- SCHEDULING STADISTICS -------------\n"
#define ENVIRONMENT_HISTORY_TITLE                                              \
	"------------- LAST ENVIRONMENTS RUN: ------------\n"

struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char **argv, struct Trapframe *tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
};

void
swap(struct Env *arr[NENV], int i, int j)
{
	struct Env *aux;

	aux = arr[i];
	arr[i] = arr[j];
	arr[j] = aux;
}

void
get_most_run_envs(struct Env *envs_aux[NENV])
{
	// copy envs to envs_aux
	for (int i = 0; i < NENV; i++) {
		envs_aux[i] = &envs[i];
	}

	// get most run envs
	int max;

	for (int i = 0; i < TOP_RUNS_COUNT; i++) {
		max = i;
		for (int j = i + 1; j < NENV; j++) {
			if (envs_aux[j]->env_runs > envs_aux[max]->env_runs) {
				max = j;
			}
		}
		swap(envs_aux, i, max);
	}
}

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
	        ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	return 0;
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS - 1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
print_scheduling_statistics_header()
{
	cprintf("\n");
	cprintf(SEPARATOR);
	cprintf(SCHEDULER_STATS_TITLE);
	cprintf(SEPARATOR);
}

void
print_environment_history()
{
	cprintf("\n");
	cprintf(ENVIRONMENT_HISTORY_TITLE);
	cprintf("\n");
	for (int i = HISTORY_LENGTH - 1; i >= 0; i--) {
		int index = (env_history_index + i) % HISTORY_LENGTH;
		if (env_history[index] == 0) {
			break;
		}
		cprintf("Environment: %d\n", env_history[index]);
	}
	cprintf("\n");
}

void
print_top_runs()
{
	struct Env *top_envs[NENV];
	get_most_run_envs(top_envs);
	cprintf("------------- MOST RUN ENVIRONMENTS: ------------\n\n");
	for (int i = 0; i < TOP_RUNS_COUNT; i++) {
		if (top_envs[i]->env_id == 0) {
			break;
		}
		cprintf("%d. Env %d: ran %d times\n",
		        i + 1,
		        top_envs[i]->env_id,
		        top_envs[i]->env_runs);
	}
}

void
print_sched_runs()
{
	cprintf("\n");
	cprintf("------------- SCHEDULER RUNS: %d -----------------\n",
	        times_executed);
	cprintf("\n");
}

void
print_scheduling_statistics()
{
	print_scheduling_statistics_header();
	print_environment_history();
	print_top_runs();
	print_sched_runs();
	cprintf(SEPARATOR);
	cprintf("\n");
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	// Scheduling statistics
	print_scheduling_statistics();

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
