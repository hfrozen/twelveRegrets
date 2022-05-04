#include <cstdlib>
#include <iostream>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include "CBeil.h"

int main(int argc, char *argv[])
{
	BYTE debug, cap;
	debug = cap = 0;
	int opt;
	while ((opt = getopt(argc, argv, "cd")) > 0) {
		switch (opt) {
		case 'c' :	cap = 1;	break;
		case 'd' :	debug = 1;	break;
		default :	break;
		}
	}

	CBeil* pBeil = new CBeil();
	if (pBeil->Run(debug, cap)) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);

		int sig;
		sigwait(&mask, &sig);
	}
	pBeil->Shutoff();
	delete pBeil;

	return EXIT_SUCCESS;
}
