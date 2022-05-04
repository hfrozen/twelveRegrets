#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>

#include "../Common/Refer.h"
#include "CDn.h"

CDn* g_pDn = NULL;

void Terminate(int sig)
{
	if (g_pDn != NULL)	delete g_pDn;
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t que = mq_open(DTQ_STR, O_RDONLY);
	if (Validity(que)) {
		mq_close(que);
		printf("ERR:dn is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(que);

	g_pDn = new CDn();
	if (g_pDn->Launch()) {
		signal(SIGINT, Terminate);
		signal(SIGTERM, Terminate);

		pause();
	}
	delete g_pDn;
	return EXIT_SUCCESS;
}
