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

	for (i = 0; i < 10; i++) {
		sys_yield();
		cprintf("Back in environment %08x, iteration %d, cpu %d\n",
		        thisenv->env_id,
		        i,
		        thisenv->env_cpunum);
		sys_get_env_priority(thisenv->env_id, &priority);
		cprintf("My priority is: %d\n", priority);
	}
	cprintf("All done in environment %08x, cpu %d\n",
	        thisenv->env_id,
	        thisenv->env_cpunum);
}