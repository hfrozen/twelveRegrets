/*
 * CTimer.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	CTIMER_H_
#define	CTIMER_H_

#include <time.h>
#include <process.h>
#include <pthread.h>
#include <sys/neutrino.h>

#include <Define.h>

class CTimer
{
public:
	CTimer();
	CTimer(PVOID pVoid);
	virtual ~CTimer();

private:
	PVOID	c_pParent;
	PVOID	c_pOwner;
	int		c_nPriority;
	int		c_nChannel;
	int		c_nTerminal;
	timer_t	c_hTimer;
	pthread_t	c_hThread;
	pthread_mutex_t	c_mutex;
	BOOL	c_bCyclic;
	BYTE	c_nCode;
	void	(*c_pHandler)(PVOID);
#define	SIZE_TITLEBUF	256
	BYTE	c_pszOwner[SIZE_TITLEBUF];

	void	KillThread();

	void	KillConnect() {
		if (c_nTerminal >= 0) {
			ConnectDetach(c_nTerminal);
			c_nTerminal = -1;
		}
	}

	void	KillTimer() {
		if (c_hTimer >= 0) {
			timer_delete(c_hTimer);
			c_hTimer = -1;
		}
	}

	void	KillChannel() {
		if (c_nChannel >= 0) {
			ChannelDestroy(c_nChannel);
			c_nChannel = -1;
		}
	}

public:
	BOOL	c_bTimerLoop;
	BOOL	c_bTimerValid;

	int		Initial(int prio, void (*pHandler)(PVOID), BYTE nCode, BOOL bCyclic = FALSE);
	int		Trigger(int ms);
	void	SetOwnerName(PSZ pszOwner);
	int		Release()				{ return Trigger(0); }
	int		GetConnection()			{ return c_nTerminal; }
	void	SetOwner(PVOID pVoid)	{ c_pOwner = pVoid; }
	pid_t	GetPid()	{ return getpid(); }

	static void*	Handler(PVOID pVoid);
};

#endif	/* CTIMER_H_ */
