#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/sched.h>

// Global variable used to boost all priorities periodically and to provide
// scheduling statistics.
unsigned long times_executed = 0;

// Array used to store the last `HISTORY_LENGTH` environments scheduled.
// It works as a circular array, so it is needed to store `env_history_index`
// in order to know the position of the last environment.
envid_t env_history[HISTORY_LENGTH];
uint32_t env_history_index = 0;

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
#ifdef SCHED_ROUND_ROBIN
	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running. Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here - Round robin
	int index;

	// The algorithm iterates over array `envs` like if it was a circular
	// array: The iteration starts at the next position of the current
	// environment (if there's a current environment) or 0 (if there isn't).
	int current_env_index = 0;
	if (curenv) {
		current_env_index = (ENVX(curenv->env_id) + 1) % NENV;
	}

	times_executed++;

	// Searches for the first RUNNABLE environment, starting from
	// `current_env_index`. If it finds one, it's scheduled and executed.
	for (int i = 0; i < NENV; i++) {
		index = (current_env_index + i) % NENV;
		if (envs[index].env_status == ENV_RUNNABLE) {
			env_history[env_history_index] = envs[index].env_id;
			env_history_index =
			        (env_history_index + 1) % HISTORY_LENGTH;
			env_run(&envs[index]);
		}
	}

	// If no RUNNABLE environment was found, the scheduler runs the current
	// one if it exists and is was running before `sched_yield`.
	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_history[env_history_index] = curenv->env_id;
		env_history_index = (env_history_index + 1) % HISTORY_LENGTH;
		env_run(curenv);
	}

	// Otherwise, the scheduler halts the processor.
	sched_halt();


#endif

#ifdef SCHED_PRIORITIES
	// Implement simple priorities scheduling.
	//
	// Environments now have a "priority" so it must be consider
	// when the selection is performed.
	//
	// Be careful to not fall in "starvation" such that only one
	// environment is selected and run every time.

	// Your code here - Priorities

	// Similar to the Round Robin scheduler, the priority scheduler iterates
	// over array `envs` like if it was a circular array:
	// The iteration starts at the next position of the current environment
	// (if there's a current environment) or 0 (if there isn't).
	// In addition, if there's a current environment, its priority is
	// reduced by one (unless it's already MIN_PRIORITY).
	int current_env_index = 0;
	if (curenv) {
		current_env_index = (ENVX(curenv->env_id)) % NENV;
		curenv->env_priority = curenv->env_priority > MIN_PRIORITY
		                               ? curenv->env_priority - 1
		                               : MIN_PRIORITY;
	}

	// Priority boosting: all environment priorities are boosted to
	// MAX_PRIORITY every BOOSTING_INTERVAL calls to the scheduler.
	if (times_executed && times_executed % BOOSTING_INTERVAL == 0) {
		for (int i = 0; i < NENV; i++) {
			int index = (current_env_index + i) % NENV;
			envs[index].env_priority = MAX_PRIORITY;
		}
	}
	times_executed++;

	// Searches for the RUNNABLE environment with the best priority,
	// starting from `current_env_index`.
	int best_index;
	int found = 0;
	for (int i = 0; i < NENV; i++) {
		int index = (current_env_index + i) % NENV;

		if (envs[index].env_status == ENV_RUNNABLE) {
			if (found && envs[index].env_priority <=
			                     envs[best_index].env_priority)
				continue;

			best_index = index;
			found = 1;

			if (envs[index].env_priority == MAX_PRIORITY) {
				break;
			}
		}
	}

	// If current environment's priority is better than any other RUNNABLE
	// environment, current environment is chosen to be scheduled.
	if (curenv && curenv->env_status == ENV_RUNNING) {
		if (!found ||
		    curenv->env_priority > envs[best_index].env_priority) {
			best_index = current_env_index;
			found = 1;
		}
	}

	// If a scheduleable environment was found in the previous steps, it is
	// scheduled.
	if (found) {
		env_history[env_history_index] = envs[best_index].env_id;
		env_history_index = (env_history_index + 1) % HISTORY_LENGTH;
		env_run(&envs[best_index]);
	}

	// Otherwise, the scheduler halts the processor.
	sched_halt();

#endif

	// Without scheduler, keep runing the last environment while it exists
	if (curenv) {
		env_history[env_history_index] = curenv->env_id;
		env_history_index = (env_history_index + 1) % HISTORY_LENGTH;
		env_run(curenv);
	}

	times_executed++;

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
