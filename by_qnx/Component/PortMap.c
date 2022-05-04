/*
 * PortMap.c
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#include <sys/neutrino.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <Printz.h>

#include "PortMap.h"

char	g_nDebug = 0;

void InitVectors()
{
	int n;
	for (n = 0; n < SIZE_VECTORS; n ++) {
		g_vectors[n].pExec = NULL;
		g_vectors[n].pBody = NULL;
	}
	g_map = 0;
	g_mqBell = -1;
}

int Register(BYTE ch, MAPFUNC pExec, PVOID pBody)
{
	if (ch < 0 || ch >= SIZE_VECTORS)	return -1;

	ThreadCtl(_NTO_TCTL_IO, 0);
	InterruptDisable();
	if (g_vectors[ch].pExec == NULL) {
		if (g_map == 0) {
			InitVectors();
			g_mqBell = mq_open(MQN_MFB, O_RDONLY);
			if (g_mqBell < 0) {
				ThreadCtl(_NTO_TCTL_IO, 0);
				InterruptEnable();
				MSGLOG("[INTR]ERROR:Can not open mfb queue(%s).\r\n",
						strerror(errno));
				return -1;
			}
			++ g_map;
			int res;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			if ((res = pthread_attr_setinheritsched(&attr,
								PTHREAD_EXPLICIT_SCHED)) != 0) {
				MSGLOG("[INTR]ERROR:Can not set inherit(%d).\r\n", res);
				return -1;
			}
			struct sched_param param;
			param.sched_priority = PRIORITY_INTRRECV;
			if ((res = pthread_attr_setschedparam(&attr, &param)) != 0) {
				MSGLOG("[INTR]ERROR:Can not set sch param(%d).\r\n", res);
				return -1;
			}
			if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) != 0) {
				MSGLOG("[INTR]ERROR:Can not set sch policy(%d).\r\n", res);
				return -1;
			}
			if ((res = pthread_create(&g_hThread, &attr, &MsgHandler, NULL)) != 0) {
				MSGLOG("[INTR]ERROR:Can not create thread for mfb %d(%s).\r\n",
								res, strerror(errno));
				/*MSGLOG("EAGAIN=%d EFAULT=%d EINVAL=%d EOK=%d.\r\n",
						EAGAIN, EFAULT, EINVAL, EOK);*/
				mq_close(g_mqBell);
				g_mqBell = -1;
				ThreadCtl(_NTO_TCTL_IO, 0);
				InterruptEnable();
				return -2;
			}
		}
		else	++ g_map;
	}
	g_vectors[ch].pExec = pExec;
	g_vectors[ch].pBody = pBody;
	ThreadCtl(_NTO_TCTL_IO, 0);
	InterruptEnable();
#if	DEBUG <= DEBUG_INTR
	MSGLOG("[INTR]Register channel(%d) function(%X) class(%X).\r\n",
						ch, (int)pExec, (int)pBody);
#endif
	return 0;
}

int Erasure(BYTE ch)
{
	if (ch < 0 || ch >= SIZE_VECTORS)	return -1;
	if (g_vectors[ch].pExec != NULL) {
		ThreadCtl(_NTO_TCTL_IO, 0);
		InterruptDisable();
		g_vectors[ch].pExec = NULL;
		g_vectors[ch].pBody = NULL;
		if (g_map > 0 && -- g_map == 0) {
			pthread_cancel(g_hThread);
#if	DEBUG <= DEBUG_INTR
			MSGLOG("[INTR]Remove thread for mfb.\r\n");
#endif
		}
		ThreadCtl(_NTO_TCTL_IO, 0);
		InterruptEnable();
#if	DEBUG <= DEBUG_INTR
		MSGLOG("[INTR]Remove channel(%d) from registry.\r\n", ch);
#endif
	}
	return 0;
}

void* MsgHandler()
{
	while (g_map > 0) {
		ssize_t res;
		if ((res = mq_receive(g_mqBell,
			(char*)&g_bell, sizeof(MFB_MSGARCH), NULL)) >= 0) {
			if (g_bell.ch >= 0 && g_bell.ch < SIZE_VECTORS &&
				g_vectors[g_bell.ch].pExec != NULL) {
#if	DEBUG <= DEBUG_INTR
				MSGLOG("[INTR]Handling by channel(%d).\r\n", g_bell.ch);
#endif
				if (g_bell.state != 0 && g_nDebug != 0)
					printf("[INTR]ERROR:%d %d\n", g_bell.ch, g_bell.state);
				(*(g_vectors[g_bell.ch].pExec))((BYTE*)&(g_bell.buff),
					(SHORT)g_bell.size, (BYTE)g_bell.state,
					g_vectors[g_bell.ch].pBody);
			}
		}
	}
	return 0;
}
