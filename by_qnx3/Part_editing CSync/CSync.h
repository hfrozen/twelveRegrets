/**
 * @file	CSync.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once

#include "Track.h"
#include "CPop.h"
#include "CPump.h"
#include "CTimer.h"
#include "CFsc.h"

class CSync :	public CPump
{
public:
	CSync();
	CSync(PVOID pParent, QHND hQue);
	CSync(PVOID pParent, QHND hQue, PSZ pszOwnerName);
	virtual ~CSync();

	virtual void	DoSend()					{}
	virtual void	DoReceive(bool bReal)		{}

private:
	CTimer*	c_pTimer;
	bool	c_bSlaveRespond;

public:
	typedef struct _tagLINEFABRIC {
		union {
			int		fd;			// for /dev/ser1...5
			WORD	wAddr;
		} domi;
		int		iCh;
		bool	bAddrMatch;
		DWORD	dwBps;
		struct {
			WORD	wResp;
			WORD	wCycle;
		} t;
	} LFABRIC, *PLFABRIC;

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

	bool	Open(int iCh, WORD wAddr, CFsc::enENCMODE mode, bool bAddrMatch, DWORD dwBps, WORD wRespondTime, WORD wCycleTime = 0);
	bool	Close();
	bool	ModifyAddr(WORD wAddr, WORD wRespondTime);
	bool	Send(BYTE* pBuf, WORD wLength);
	bool	SendOnly(BYTE* pBuf, WORD wLength);
	void	KillTrig();
	WORD	GetAddress()					{ return c_fab.domi.wAddr; }
	WORD	GetCycleTime()					{ return c_fab.t.wCycle; }
	int		GetTimerConnection()			{ return c_pTimer->GetConnectID(); }
};
