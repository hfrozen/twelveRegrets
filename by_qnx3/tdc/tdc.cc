#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

#include "CTdc.h"

CTdc* g_pTdc = NULL;

void Terminate(int sig)
{
	if (g_pTdc != NULL)	delete g_pTdc;
	TRACK("TERMINATE ========================\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t que = mq_open(MAINQ_STR, O_RDONLY);
	if (Validity(que)) {
		mq_close(que);
		printf("ERR:tdc is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(que);

	g_pTdc = new CTdc();
	if (g_pTdc->Launch()) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pTdc;

	TRACK("END ==============================\n");
	return EXIT_SUCCESS;
}
