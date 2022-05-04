#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>

#include "../Common/Refer.h"
#include "PiRefer.h"
#include "CPi.h"

CPi* g_pPi = NULL;

void Terminate(int sig)
{
	if (g_pPi != NULL)	delete g_pPi;
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t que = mq_open(PTQ_STR, O_RDONLY);
	if (Validity(que)) {
		mq_close(que);
		printf("ERR:pi is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(que);

	g_pPi = new CPi();
	if (g_pPi->Launch()) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pPi;
	return EXIT_SUCCESS;
}
