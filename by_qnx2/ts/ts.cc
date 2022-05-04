#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

#include "Track.h"
#include "CLand.h"

#if	defined(_AT_TS_)
CLand*	g_pLand = NULL;

void Destroy(int sig)
{
	if (g_pLand != NULL)	delete g_pLand;
	TRACK("ts TERMINATE ====================================\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t hReservoir = mq_open((char*)MAINQ_STR, O_RDONLY);
	if (hReservoir >= 0) {
		mq_close(hReservoir);
		TRACK("ERR:ts is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(hReservoir);

	bool bClose = false;
	g_pLand = new CLand();
	if (g_pLand->Launch()) {
		//sigset_t mask;
		//sigemptyset(&mask);
		//sigaddset(&mask, SIGINT);
		//sigaddset(&mask, SIGTERM);

		//int sig;
		//sigwait(&mask, &sig);

		if (bClose) {
			signal(SIGINT, Destroy);
			signal(SIGTERM, Destroy);
		}

		pause();
	}
	delete g_pLand;

	TRACK("ts END ==========================================\n");
	return EXIT_SUCCESS;
}
#endif
