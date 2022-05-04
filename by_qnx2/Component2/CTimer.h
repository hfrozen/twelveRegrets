/*
 * CTimer.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include <time.h>
#include <pthread.h>
#include <process.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>

#include "DuoTypes.h"
#include "Slight.h"
#include "CPop.h"
#include "CPump.h"
#include "CLabel.h"

#define	TIMERCODE_BASE		0
#define	TIMERCODE_BYCH(x)	(x * 2 + 8)

class CTimer :	public CLabel
{
public:
	CTimer(PVOID pVoid);
	CTimer(PVOID pVoud, PSZ pszOwnerName);
	virtual ~CTimer();

private:
	//PVOID	c_pParent;
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
	typedef void (CPump::*PTIMERFUNC)();

private:
	PTIMERFUNC	c_pTimerFunc;
	PVOID	c_pBase;

	static PVOID	TimeWatcherEntry(PVOID pVoid);
	void	Watcher();
	void	KillThread();
	void	KillConnect();
	void	KillTimer();
	void	KillChannel();

public:
	//int		Initial(int priority, void (*pTimerFunc)(PVOID), BYTE nCode, bool bCyclic = false);
	int		Initial(int priority, PTIMERFUNC pFunc, CPump* pBase, BYTE nCode, bool bCyclic = false);
	bool	Trigger(int ms);		//, bool bSysClockSync = false);
	int		Release()				{ return Trigger(0); }
	bool	GetRhythm()				{ return c_bTrig; }
	int		GetConnectID()			{ return c_nConnectID; }
	int		GetTimerID()			{ return c_timerID; }
	//void	SetParent(PVOID pVoid)	{ c_pParent = pVoid; }
	pid_t	GetPid()				{ return getpid(); }

	//static PVOID	Fob(PVOID pVoid);
};
