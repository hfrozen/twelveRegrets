#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

#include "../Inform2/Track.h"
#include "CFlux.h"

CFlux* g_pFlux = NULL;

void Terminate(int sig)
{
	if (g_pFlux != NULL)	delete g_pFlux;
	TRACK("mm TERMINATE ====================================\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	//SetLog(false);
	//
	bool bDebug = false;
	int opt = getopt(argc, argv, "d");
	if (opt > 0 && opt == 'd')	bDebug = true;

	//remove(LOGNOTE_PATH);
	mqd_t qf = mq_open((char*)MTQ_STR, O_RDONLY);
	if (qf >= 0) {
		mq_close(qf);
		printf("ERR:mm is running!\n");
		return EXIT_SUCCESS;
	}

	bool bClose = false;
	g_pFlux = new CFlux();
	if (g_pFlux->Launch(bDebug)) {
		if (bClose) {
			signal(SIGINT, Terminate);
			signal(SIGTERM, Terminate);
		}

		pause();
	}
	delete g_pFlux;

	TRACK("mm END ==========================================\n");
	return EXIT_SUCCESS;
}
