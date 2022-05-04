#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>

#include "../Common/Refer.h"
#include "CUd.h"

CUp* g_pUd = NULL;

void Terminate(int sig)
{
	if (g_pUd != NULL)	delete g_pUd;
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t que = mq_open(UTQ_STR, O_RDONLY);
	if (Validity(que)) {
		mq_close(que);
		printf("ERR:up is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(que);

	g_pUp = new CUp();
	if (g_pUp->Launch()) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pUp;
	return EXIT_SUCCESS;
}
