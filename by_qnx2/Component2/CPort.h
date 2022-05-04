/*
 * CPort.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Track.h"
#include "CPop.h"
#include "CPump.h"
#include "CTimer.h"

class CPort :	public CPump
{
public:
	CPort();
	CPort(PVOID pParent, QHND hQue);
	CPort(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CPort();

	virtual void	DoSend()					{}
	virtual void	DoReceive(bool bReal)		{}

private:
	CTimer*	c_pTimer;
	bool	c_bSlaveRespond;

protected:
	LFABRIC	c_fab;

	typedef struct _tagMAGAZINE {
		WORD	wLength;
		BYTE	cStatus;
		union {
			struct {
				WORD wAddr;
				BYTE rem[SIZE_GENSBUF - sizeof(WORD)];
			} f;
			BYTE	c[SIZE_GENSBUF];
		} buf;
	} MAGAZINE;
	MAGAZINE	c_mag;

	CIntvMon	c_itvm;

public:
	void	Receive(BYTE* pRecv, WORD wLength, WORD wStatus);
	void	Elapsed();

	bool	Open(int iCh, WORD wAddr, DWORD dwBps, WORD wRespondTime, WORD wCycleTime = 0);
	bool	Close();
	bool	ModifyAddr(WORD wAddr, WORD wRespondTime);
	bool	Send(BYTE* pBuf, WORD wLength);
	bool	SendOnly(BYTE* pBuf, WORD wLength);
	void	KillTrig();
	//double	GetRespt()						{ return c_respt; }
	WORD	GetAddress()					{ return c_fab.domi.wAddr; }
	WORD	GetCycleTime()					{ return c_fab.t.wCycle; }
	int		GetTimerConnection()			{ return c_pTimer->GetConnectID(); }
};
