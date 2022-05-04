#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

#include "CWm.h"

CWm* g_pWm = NULL;

void Terminate(int sig)
{
	if (g_pWm != NULL)	delete g_pWm;
	exit(0);
}

int main(int argc, char *argv[])
{
	BYTE cDebug = 0;
	int opt = getopt(argc, argv, "d");
	if (opt > 0 && opt == 'd')	cDebug = 1;

	mqd_t que = mq_open(WMQ_STR, O_RDONLY);
	if (Validity(que)) {
		mq_close(que);
		printf("ERR:wm is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(que);

	g_pWm = new CWm();
	if (g_pWm->Launch(cDebug)) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pWm;
	return EXIT_SUCCESS;
}
