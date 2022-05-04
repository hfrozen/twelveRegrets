/*
 * CTimer.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "../Inform2/Track.h"
//#include "CTool.h"
#include "CFsc.h"
#include "CPump.h"
#include "CTimer.h"

PVOID CTimer::TimeWatcherEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CTimer* pTimer = (CTimer*)pVoid;
	if (pTimer != NULL)	pTimer->Watcher();
	return NULL;
}

CTimer::CTimer(PVOID pVoid)
	: CLabel(pVoid)
{
	//c_pParent = pVoid;
	c_pTimerFunc = NULL;
	c_pBase = NULL;
	c_nChannel = INVALID_HANDLE;
	c_nConnectID = INVALID_HANDLE;
	c_timerID = INVALID_HANDLE;
	c_hThread = INVALID_HANDLE;
	c_bCyclic = false;
	c_nCode = _PULSE_CODE_MINAVAIL;
	c_nPriority = PRIORITY_DEFAULT;
	c_bLoop = false;
	c_bAlive = false;
	c_bTrig = false;
	//memset(c_szOwnerName, 0, SIZE_OWNERBUFF);
}

CTimer::CTimer(PVOID pVoid, PSZ pszOwnerName)
	: CLabel(pVoid, pszOwnerName)
{
	//c_pParent = pVoid;
	c_pTimerFunc = NULL;
	c_pBase = NULL;
	c_nChannel = INVALID_HANDLE;
	c_nConnectID = INVALID_HANDLE;
	c_timerID = INVALID_HANDLE;
	c_hThread = INVALID_HANDLE;
	c_bCyclic = false;
	c_nCode = _PULSE_CODE_MINAVAIL;
	c_nPriority = PRIORITY_DEFAULT;
	c_bLoop = false;
	c_bAlive = false;
	c_bTrig = false;
	//strncpy(c_szOwnerName, pszOwnerName, SIZE_OWNERBUFF);
}

CTimer::~CTimer()
{
	c_bLoop = false;
	c_bTrig = false;
	KillThread();
	KillConnect();
	KillTimer();
	KillChannel();
}

void CTimer::KillThread()
{
	if (Validity(c_hThread)) {
		c_bLoop = false;
		c_bTrig = false;
		if (pthread_cancel(c_hThread) == EOK) {
			PVOID pResult;
			pthread_join(c_hThread, &pResult);
		}
		c_hThread = INVALID_HANDLE;
	}
}

void CTimer::Watcher()
{
	struct _pulse pulse;

#if	_DEBL(DEBUG_HANDLER)
	TRACK("%s>TIMER:handler start.\n", c_szOwnerName);
#endif
	c_bAlive = true;
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	while (c_bLoop) {
		int res = MsgReceivePulse(c_nChannel, &pulse, sizeof(pulse), NULL);
		if (res == 0 && pulse.code == c_nCode) {
			c_mtx.Lock();
			bool bTrig = c_bTrig;
			c_mtx.Unlock();
			if (!c_bCyclic)	Release();		// reset c_bTrig
			if (c_pTimerFunc != NULL && c_pBase != NULL && bTrig) {
				CPump* pPump = (CPump*)c_pBase;
				(pPump->*c_pTimerFunc)();
			}
		}
	}
	c_bAlive = false;

#if	_DEBL(DEBUG_HANDLER)
	TRACK("%s>TIMER:handler exit.\n", c_szOwnerName);
#endif
}

void CTimer::KillConnect()
{
	if (Validity(c_nConnectID)) {
		ConnectDetach(c_nConnectID);
		c_nConnectID = INVALID_HANDLE;
	}
}

void CTimer::KillTimer()
{
	if (Validity(c_timerID)) {
		timer_delete(c_timerID);
		c_timerID = INVALID_HANDLE;
	}
}

void CTimer::KillChannel()
{
	if (Validity(c_nChannel)) {
		ChannelDestroy(c_nChannel);
		c_nChannel = INVALID_HANDLE;
	}
}

int CTimer::Initial(int priority, PTIMERFUNC pFunc, CPump* pBase, BYTE nCode, bool bCyclic)
{
	ASSERTP(pFunc);

	// create channel
	if (!Validity(c_nChannel = ChannelCreate(0))) {
		TRACK("%s>TIMER>ERR:can not create channel!(%s)\n", c_szOwnerName, strerror(errno));
		return 1;
	}

	// thread
	c_nPriority = priority;
	c_bLoop = true;
	int res = CreateThread(&c_hThread, &CTimer::TimeWatcherEntry, (PVOID)this, priority, (PSZ)"TIMER");
	if (!Validity(res)) {
		c_bLoop = false;
		KillChannel();
		return 2;
	}

	// connect
	if (!Validity(c_nConnectID = ConnectAttach(ND_LOCAL_NODE, 0, c_nChannel, 0, 0))) {
		c_bLoop = false;
		KillThread();
		KillChannel();
		TRACK("%s>TIMER>ERR:can not open channel!(%d-%s)\n", c_szOwnerName, c_nConnectID, strerror(errno));
		return 3;
	}
	c_pTimerFunc = pFunc;
	c_pBase = (PVOID)pBase;
	c_bCyclic = bCyclic;

	struct sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = c_nConnectID;
	event.sigev_priority = priority;
	event.sigev_code = c_nCode = _PULSE_CODE_MINAVAIL + nCode;
	event.sigev_value.sival_ptr = NULL;
	if (!Validity(res = timer_create(CLOCK_MONOTONIC, &event, &c_timerID))) {
		c_bLoop = false;
		KillThread();
		KillChannel();
		TRACK("%s>TIMER>ERR:can not create timer!(%d-%s)\n", c_szOwnerName, res, strerror(errno));
		return res;
	}

	struct itimerspec spec;
	spec.it_value.tv_sec = 0;
	spec.it_value.tv_nsec = 0;
	spec.it_interval.tv_sec = 0;
	spec.it_interval.tv_nsec = 0;
	if (!Validity(res = timer_settime(c_timerID, 0, &spec, NULL))) {
		c_bLoop = false;
		KillThread();
		KillChannel();
		KillTimer();
		TRACK("%s>TIMER>ERR:can not set timer!(%d-%s)\n", c_szOwnerName, res, strerror(errno));
		return res;
	}

	return 0;
}

bool CTimer::Trigger(int ms)	//, bool bSysClockSync)
{
	//if (bSysClockSync) {
	//	int msec;
	//	do {
	//		struct timeval tv;
	//		gettimeofday(&tv, NULL);
	//		//struct tm* pTm = localtime(&tv.tv_sec);
	//		msec = tv.tv_usec / 1000;	// / 10000;
	//									// msec * 10;
	//	} while (msec % 10);			// % 100);
	//}

	if (!Validity(c_timerID)) {
		TRACK("%s>TIMER>ERR:illegal timer id!(%d)\n", c_szOwnerName, c_timerID);
		return false;
	}

	c_mtx.Lock();
	c_bTrig = false;
	c_mtx.Unlock();

	struct itimerspec spec;
	spec.it_value.tv_sec = 0;
	spec.it_value.tv_nsec = (long)ms * 1e6;
	spec.it_interval.tv_sec = 0;
	spec.it_interval.tv_nsec = (long)ms * 1e6;
	if (!Validity(timer_settime(c_timerID, TIMER_PRECISE, &spec, NULL))) {
		KillTimer();
		TRACK("%s>TIMER>ERR:can not set timer!!(%d-%s)\n", c_szOwnerName, ms, strerror(errno));
		return false;
	}
	c_mtx.Lock();
	c_bTrig = ms > 0 ? true : false;
	c_mtx.Unlock();

	return true;
}
