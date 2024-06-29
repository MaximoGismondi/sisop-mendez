#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i, scheduling_counter = 0;
	size_t priority;

	cprintf("Hello, I am environment %08x, cpu %d\n",
	        thisenv->env_id,
	        thisenv->env_cpunum);

	sys_get_env_priority(thisenv->env_id, &priority);
	cprintf("My priority is: %d\n", priority);

	for (i = 0; i < 400; i++) {
		sys_yield();
		sys_get_env_priority(thisenv->env_id, &priority);
		if (priority == MIN_PRIORITY) {
			scheduling_counter++;
			continue;
		}
		if (scheduling_counter > 0) {
			cprintf("Environment %08x ran %d times on cpu %d with "
			        "priority %d\n\n",
			        thisenv->env_id,
			        scheduling_counter,
			        thisenv->env_cpunum,
			        MIN_PRIORITY);
			scheduling_counter = 0;
		}
		cprintf("Back in environment %08x, iteration %d, cpu %d\n",
		        thisenv->env_id,
		        i,
		        thisenv->env_cpunum);
		cprintf("My priority is: %d\n", priority);
	}
	cprintf("All done in environment %08x, cpu %d\n",
	        thisenv->env_id,
	        thisenv->env_cpunum);
}