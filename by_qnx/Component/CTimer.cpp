/*
 * CTimer.cpp
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#include <signal.h>
#include <sys/siginfo.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <Printz.h>

#include "CTimer.h"

void* CTimer::Handler(PVOID pVoid)
{
	CTimer* pTimer = (CTimer*)pVoid;
	struct _pulse pulse;
	//BOOL bLoop = TRUE;
	//while (bLoop) {
	BOOL valid;
	while (pTimer->c_bTimerLoop) {
		MsgReceivePulse(pTimer->c_nChannel, &pulse, sizeof(pulse), NULL);
		pthread_mutex_lock(&pTimer->c_mutex);
		valid = pTimer->c_bTimerValid;
		pthread_mutex_unlock(&pTimer->c_mutex);
		if (!pTimer->c_bCyclic)	pTimer->Release();
		if (pTimer->c_pHandler != NULL && valid) {
			(*(pTimer->c_pHandler))(pTimer->c_pParent);
		}
	}
	return NULL;
}

CTimer::CTimer()
{
	c_pParent = NULL;
	c_pOwner = NULL;
	c_pHandler = NULL;
	c_nPriority = -1;
	c_nChannel = -1;
	c_nTerminal = -1;
	c_hTimer = -1;
	c_hThread = -1;
	c_bCyclic = FALSE;
	memset((PVOID)c_pszOwner, 0, SIZE_TITLEBUF);
}

CTimer::CTimer(PVOID pVoid)
{
	c_pParent = pVoid;
	c_pOwner = NULL;
	c_pHandler = NULL;
	c_nPriority = -1;
	c_nChannel = -1;
	c_nTerminal = -1;
	c_hTimer = -1;
	c_hThread = -1;
	c_bCyclic = FALSE;
	c_bTimerLoop = FALSE;
	c_bTimerValid = FALSE;
	memset((PVOID)c_pszOwner, 0, SIZE_TITLEBUF);
}

CTimer::~CTimer()
{
	KillThread();
	KillConnect();
	KillTimer();
	KillChannel();
}

void CTimer::KillThread()
{
	if (c_hThread >= 0) {
		pthread_cancel(c_hThread);
		pthread_join(c_hThread, NULL);
		c_hThread = -1;
	}
}

int CTimer::Initial(int prio, void (*pHandler)(PVOID), BYTE nCode, BOOL bCyclic)
{
	pthread_mutex_init(&c_mutex, NULL);

	// create channel
	c_nCode = _PULSE_CODE_MINAVAIL + nCode;
	c_nChannel = ChannelCreate(0);
	if (c_nChannel < 0) {
		MSGLOG("[TIMER]ERROR:Can not create channel of %s.\r\n", c_pszOwner);
		return -1;
	}

	// thread
	int res;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if ((res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED)) != 0) {
		KillChannel();
		MSGLOG("[TIMER]ERROR:Can not set inherit(%d) of %s.\r\n", res, c_pszOwner);
		return -1;
	}
	c_nPriority = prio;
	struct sched_param param;
	param.sched_priority = c_nPriority;
	if ((res = pthread_attr_setschedparam(&attr, &param)) != 0) {
		KillChannel();
		MSGLOG("[TIMER]ERROR:Can not set sch param(%d) of %s.\r\n", res, c_pszOwner);
		return -1;
	}
	if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) != 0) {
		KillChannel();
		MSGLOG("[TIMER]ERROR:Can not set sch policy(%d) of %s.\r\n", res, c_pszOwner);
		return -1;
	}
	c_bTimerLoop = TRUE;
	if ((res = pthread_create(&c_hThread, &attr, Handler, (void*)this)) != 0) {
		c_bTimerLoop = FALSE;
		KillChannel();
		MSGLOG("[TIMER]ERROR:Can not create thread(%d) of %s.\r\n", res, c_pszOwner);
		return -1;
	}

	// connect
	if ((c_nTerminal = ConnectAttach(ND_LOCAL_NODE, 0, c_nChannel, 0, 0)) < 0) {
		KillThread();
		KillChannel();
		MSGLOG("[TIMER]ERROR:Can not open channel(%s) of %s.\r\n", strerror(errno), c_pszOwner);
		return -1;
	}
	c_pHandler = pHandler;
	c_bCyclic = bCyclic;

	struct sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = c_nTerminal;
	event.sigev_priority = c_nPriority;
	event.sigev_code = c_nCode;
	event.sigev_value.sival_ptr = NULL;
	if ((res = timer_create(CLOCK_REALTIME, &event, &c_hTimer)) < 0) {
		KillThread();
		KillChannel();
		MSGLOG("[TIMER]ERROR:Can not create timer(%s) of %s.\r\n", strerror(errno), c_pszOwner);
		return res;
	}

	struct itimerspec spec;
	spec.it_value.tv_sec = 0;
	spec.it_value.tv_nsec = 0;
	spec.it_interval.tv_sec = 0;
	spec.it_interval.tv_nsec = 0;
	if ((res = timer_settime(c_hTimer, 0, &spec, NULL)) < 0) {
		KillThread();
		KillChannel();
		KillTimer();
		MSGLOG("[TIMER]ERROR:Can not set timer of %s.\r\n", c_pszOwner);
		return res;
	}

	return 0;
}

void CTimer::SetOwnerName(PSZ pszOwner)
{
	if (strlen(pszOwner) >= SIZE_TITLEBUF)
		*(pszOwner + SIZE_TITLEBUF) = NULL;
	strcpy((char*)c_pszOwner, (char*)pszOwner);
}

int CTimer::Trigger(int ms)
{
	//KillTimer();
	//pthread_mutex_lock(&c_mutex);
	//c_bTimerValid = FALSE;
	//pthread_mutex_unlock(&c_mutex);
	//if (ms < 1)	return 0;

	int res;
	/*struct sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = c_nTerminal;
	event.sigev_priority = c_nPriority;
	event.sigev_code = c_nCode;
	event.sigev_value.sival_ptr = NULL;
	if ((res = timer_create(CLOCK_REALTIME, &event, &c_hTimer)) < 0) {
		MSGLOG("[TIMER]ERROR:Can not create timer(%s) of %s.\r\n", strerror(errno), c_pszOwner);
		return res;
	}*/

	struct itimerspec spec;
	spec.it_value.tv_sec = 0;
	spec.it_value.tv_nsec = (long)ms * 1000000;
	spec.it_interval.tv_sec = 0;
	spec.it_interval.tv_nsec = (long)ms * 1000000;
	if ((res = timer_settime(c_hTimer, 0, &spec, NULL)) < 0) {
		KillTimer();
		MSGLOG("[TIMER]ERROR:Can not set timer of %s.\r\n", c_pszOwner);
		return res;
	}
	pthread_mutex_lock(&c_mutex);
	c_bTimerValid = ms > 0 ? TRUE : FALSE;
	pthread_mutex_unlock(&c_mutex);
	return 0;
}
