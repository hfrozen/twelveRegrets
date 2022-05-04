/**
 * @file	CAsync.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include <termios.h>

#include "../Base/CPump.h"
#include "CTimer.h"

enum enTAGASYNC {
	TAG_RECEIVEASYNC = 110
};

enum enRECVSTATE {
	RECV_NORMAL,
	RECV_TIMEOUT
};

class CAsync :	public CPump
{
public:
	CAsync();
	CAsync(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CAsync();

	enum enPARITY {
		PARITY_NON = 0,
		PARITY_EN,
		PARITY_ODD,
	};

private:
	CTimer*		c_pWatcher;
	CTimer*		c_pSettler;
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
	void	Settle();
	void	Watch();

	bool	Open(int iCh, DWORD dwBps, enPARITY parity, WORD wCycleTime);
	bool	Close();
	bool	Send(BYTE* pBuf, WORD wLength);
	bool	FreeSend(BYTE* pBuf, WORD wLength);
	void	Bunch(BYTE* pBuf, WORD wLength);
	int		GetWatcherConnection();
	int		GetSettlerConnection();
	void	KillWatcherTrigger()			{ if (c_pWatcher != NULL) c_pWatcher->Release(); }
	void	KillSettlerTrigger()			{ if (c_pSettler != NULL) c_pSettler->Release(); }
	WORD	GetCycleTime()					{ return c_fab.time.wCycle; }
};
