#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i;
	size_t priority;

	cprintf("Hello, I am environment %08x, cpu %d\n",
	        thisenv->env_id,
	        thisenv->env_cpunum);

	sys_get_env_priority(thisenv->env_id, &priority);
	cprintf("My priority is: %d\n", priority);

	size_t new_priority = 4;
	cprintf("Now I will change my priority to %d:\n", new_priority);
	int err = sys_set_env_priority(thisenv->env_id, new_priority);
	if (err) {
		cprintf("Set priority failed\n");
		return;
	}
	sys_get_env_priority(thisenv->env_id, &priority);
	cprintf("My priority is: %d\n", priority);

	new_priority = 7;
	cprintf("But it fails when I try to increase my priority back to %d:\n",
	        new_priority);
	err = sys_set_env_priority(thisenv->env_id, new_priority);
	if (err) {
		cprintf("Set priority failed\n");
		return;
	}
	cprintf("Program should never reach here...\n");
}