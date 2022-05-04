#include <cstdlib>
#include <iostream>
#include <mqueue.h>
#include <fcntl.h>

#include "Track.h"
#include "CRtd.h"

#if	defined(_AT_RS_)
CRtd*	g_pRtd = NULL;

void Destroy(int sig)
{
	if (g_pRtd != NULL)	delete g_pRtd;
	TRACK("rs TERMINATE ====================================\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	mqd_t hReservoir = mq_open((char*)MAINQ_STR, O_RDONLY);
	if (hReservoir >= 0) {
		mq_close(hReservoir);
		TRACK("ERR:rs is running!\n");
		return EXIT_SUCCESS;
	}
	mq_close(hReservoir);

	bool bClose = false;
	g_pRtd = new CRtd();
	if (g_pRtd->Launch()) {
		if (bClose) {
			signal(SIGINT, Destroy);
			signal(SIGTERM, Destroy);
		}
		pause();
	}
	delete g_pRtd;

	TRACK("rs END ==========================================\n");
	return EXIT_SUCCESS;
}
#endif
