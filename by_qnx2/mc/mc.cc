#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

#include "CMoc.h"

CMoc* g_pMoc = NULL;

void Terminate(int sig)
{
	if (g_pMoc != NULL)	delete g_pMoc;
	exit(0);
}

int main(int argc, char *argv[])
{
	BYTE cDebug = 0;
	int opt = getopt(argc, argv, "d");
	if (opt > 0 && opt == 'd')	cDebug = 1;

	mqd_t qf = mq_open((char*)MCQ_STR, O_RDONLY);
	if (qf >= 0) {
		mq_close(qf);
		printf("ERR:mc is running!\n");
		return EXIT_SUCCESS;
	}

	g_pMoc = new CMoc();
	if (g_pMoc->Launch(cDebug)) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pMoc;
	return EXIT_SUCCESS;
}
