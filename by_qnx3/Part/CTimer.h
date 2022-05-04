/**
 * @file	CTimer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once

#include <time.h>
#include <pthread.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>

#include "../Common/DefTypes.h"
#include "CPop.h"
#include "../Base/CLabel.h"

#define	TIMERCODE_BASE		0				// ?????
#define	TIMERCODE_BYCH(x)	(x * 2 + 8)

class CTimer :	public CLabel
{
public:
	CTimer(PVOID pVoid);
	CTimer(PVOID pVoud, PSZ pszOwnerName);
	virtual ~CTimer();

private:
	int		c_nChannel;
	int		c_nConnectID;
	timer_t	c_timerID;
	TRHND	c_hThread;
	bool	c_bCyclic;
	int		c_nCode;
	int		c_nPriority;

	bool	c_bLoop;
	bool	c_bAlive;
	bool	c_bTrig;

	CMutex	c_mtx;

public:
	typedef void (CLabel::*PTIMERFUNC)();

private:
	PTIMERFUNC	c_pTimerFunc;

	static PVOID	TimeWatcherEntry(PVOID pVoid);
	void	Watcher();
	void	KillThread();
	void	KillConnect();
	void	KillTimer();
	void	KillChannel();

public:
	int		Initial(int priority, PTIMERFUNC pFunc, BYTE nCode, bool bCyclic = false);
	bool	Trigger(int ms);		//, bool bSysClockSync = false);
	int		Release()				{ return Trigger(0); }
	bool	GetRhythm()				{ return c_bTrig; }
	int		GetConnectID()			{ return c_nConnectID; }
	int		GetTimerID()			{ return c_timerID; }
	pid_t	GetPid()				{ return getpid(); }
};
