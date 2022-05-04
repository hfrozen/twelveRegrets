#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>

#include "../Common/Refer.h"
#include "SmRefer.h"
#include "CSm.h"

CSm* g_pSm = NULL;

void Terminate(int sig)
{
	if (g_pSm != NULL)	delete g_pSm;
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t que = mq_open(STQ_STR, O_RDONLY);
	if (Validity(que)) {
		mq_close(que);
		printf("ERR:sm is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(que);

	g_pSm = new CSm();
	if (g_pSm->Launch()) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pSm;
	return EXIT_SUCCESS;
}
