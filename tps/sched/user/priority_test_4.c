#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	size_t priority;

	if (fork() == 0) {
		cprintf("I am the child environment %08x, cpu %d\n",
		        thisenv->env_id,
		        thisenv->env_cpunum);

		sys_get_env_priority(thisenv->env_id, &priority);
		cprintf("(child) My priority is: %d\n", priority);

		for (int i = 0; i < 15; i++) {
			sys_yield();
			sys_get_env_priority(thisenv->env_id, &priority);
			cprintf("Back in child environment %08x, iteration %d, "
			        "cpu %d, priority %d\n",
			        thisenv->env_id,
			        i,
			        thisenv->env_cpunum,
			        priority);
		}
		exit();
	}

	cprintf("I am the parent environment %08x, cpu %d\n",
	        thisenv->env_id,
	        thisenv->env_cpunum);

	sys_get_env_priority(thisenv->env_id, &priority);
	cprintf("My priority is: %d\n", priority);

	for (int i = 0; i < 15; i++) {
		sys_yield();
		sys_get_env_priority(thisenv->env_id, &priority);
		cprintf("Back in parent environment %08x, iteration %d, cpu "
		        "%d, priority %d\n",
		        thisenv->env_id,
		        i,
		        thisenv->env_cpunum,
		        priority);
	}
}