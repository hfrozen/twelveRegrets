
/*
 *  sigsleeper.c
 *
 *  This module demonstrates signal handling,
 *  and some of the problems you can run into.
 *
 */

#include <stdio.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <process.h>
#include <signal.h>
#include <stdlib.h>

char    *progname = "sigsleeper";

void    myHandler(int signo);
int     critical_operation(int);

struct  sigaction   action;

main() {
	time_t          now, then;
	unsigned int    timeleft;
	sigset_t set, oset;
	const unsigned int TIMEOUT = 20;

	setvbuf (stdout, NULL, _IOLBF, 0);

	printf("You have 60 seconds to kill me! (pid %d)\n", getpid());
	printf("Hit me with SIGTERM by doing 'slay sigsleeper'.\n");
	printf("Or by using a target navigator to send a signal.\n");

	/* set up signal handler for handling SIGTERM */
	action.sa_handler = myHandler;
	action.sa_flags = SA_SIGINFO;
	sigaction(SIGTERM, &action, NULL);

	time(&now);
	timeleft = critical_operation(TIMEOUT);
	time(&then);


	printf("Return value from critical_operation was %d\n", timeleft);
	if (then - now < TIMEOUT) {
		printf("Hmmm.... we didn't complete, only did %d seconds!\n",
				then - now);
	} else {
		printf("Done!\n");
	}
	printf("Exiting...\n");
	exit(0);
}

/* Signal handling function */
void
myHandler(int signo) {
	printf("Inside myHandler().  Got hit with signal %d.\n", signo);
}

/* Our "critical" operation function */
int critical_operation(int seconds) {
	return sleep(seconds);
}
