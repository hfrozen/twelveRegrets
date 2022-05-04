/*
 * CTerm.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once
#include <termios.h>

#include "CPump.h"
#include "CTimer.h"


class CTerm :	public CPump
{
public:
	CTerm();
	CTerm(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CTerm();

private:
	CTimer*		c_pWatcher;
	CTimer*		c_pGappy;
	TRHND		c_hThread;

	struct termios	c_tmo;

protected:
	LFABRIC		c_fab;

	typedef struct _tagMAGAZINE {
		WORD	wi;
		WORD	wLength;
		BYTE	cBuf[SIZE_GENSBUF];
	} MAGAZINE;
	MAGAZINE	c_mag;

	void	Receive();

public:
	static PVOID	ReceiveEntry(PVOID pVoid);
	void	Snatched();
	void	Elapsed();

	bool	Open(int iCh, DWORD dwBps, WORD wCycleTime);
	bool	Close();
	bool	Send(BYTE* pBuf, WORD wLength);
	bool	FreeSend(BYTE* pBuf, WORD wLength);
	void	Bunch(BYTE* pBuf, WORD wLength);
	int		GetWatcherConnection();
	int		GetGappyConnection();
	void	KillWatcherTrigger()			{ if (c_pWatcher != NULL) c_pWatcher->Release(); }
	void	KillGappyTrigger()				{ if (c_pGappy != NULL) c_pGappy->Release(); }
	WORD	GetCycleTime()					{ return c_fab.t.wCycle; }
};
