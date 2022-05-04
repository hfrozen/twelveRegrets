#include <pthread.h>
#include <sys/wait.h>

#include "CCcx.h"

CCcx ccx;

int main(int argc, char *argv[])
{
	int aich;
	aich = 0;
	int opt;
	while ((opt = getopt(argc, argv, "a:")) > 0) {
		switch (opt) {
		case 'a' :	aich = 1;	break;
		default :	break;
		}
	}

	ccx.SetOwner((PSZ)"CCcx");
	if (ccx.Run(aich)) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);

		int sig;
		sigwait(&mask, &sig);
	}
	ccx.Destroy();
	printf("Exit Ccx.\r\n");

	return EXIT_SUCCESS;
}
