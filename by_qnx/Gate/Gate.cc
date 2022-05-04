#include <cstdlib>
#include <iostream>

#include <pthread.h>
#include <sys/wait.h>

#include "CGate.h"

int main(int argc, char *argv[])
{
	BYTE debug = 0;
	int opt = getopt(argc, argv, "d");
	if (opt > 0 && opt == 'd')	debug = 1;

	CGate* pGate = new CGate();
	pGate->SetOwner((PSZ)"CGate");
	if (pGate->Run(debug)) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);

		int sig;
		sigwait(&mask, &sig);
	}
	pGate->Shutoff();
	delete pGate;

	return EXIT_SUCCESS;
}
