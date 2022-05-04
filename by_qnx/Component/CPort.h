/*
 * CPort.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	CPORT_H_
#define	CPORT_H_

#include <Mfb.h>
#include <Draft.h>

enum  {
	PORTATTR_NON = 0,
	PORTATTR_DEVICE,
	PORTATTR_CC,
	PORTATTR_TCSLAVE,
	PORTATTR_TCMASTER
};

typedef struct _tagPORTSTATE {
	BYTE	uCh;
	UCURV	uMode;
	WORD	wAddr;
	UCURV	uAttribute;
	UCURV	uBps;
	UCURV	uRecvLength;
	UCURV	uRespondTime;
	UCURV	uWatchTime;
} PORTSTATE, *PPORTSTATE;

enum  {	PORTERROR_NON = 0,
		PORTERROR_BUSINTR,
		PORTERROR_MODE,
		PORTERROR_BPS,
		PORTERROR_RECVLENGTH,
		PORTERROR_ADDR,
		PORTERROR_CHANNEL,
		PORTERROR_MALLOC,
		PORTERROR_CANNOTREG
};

#include <CAnt.h>
#include "CTimer.h"

class CPort	: public CAnt
{
public:
	CPort();
	CPort(PVOID pParent, int mq);
	CPort(PVOID pParent, int mq, PSZ pszOwner);
	virtual ~CPort();

private:
	PVOID	c_pParent;

public:
	PORTSTATE	c_portState;

private:
	CTimer*		c_pTimeOut;
	BOOL		c_bDirection;
	BOOL		c_bAutoTrigOut;
	int			c_prio;
	BYTE		c_pszOwner[SIZE_TITLEBUF];

protected:
	typedef struct _tagRECVFRAME {
		BYTE*	pBuf;
		BYTE	nState;
		WORD	wLength;
	} RECVFRAME;
	RECVFRAME	c_recv;
	void	Porter(BYTE* pRecv, WORD wLength, BYTE nState);
	void	TimeOutter();

	void	PrintMfError(int res, PSZ msg);

public:
	int		Begin(PPORTSTATE pState);
	int		End();
	int		GetStatus(PMFSTATUS pStatus);
	int		SetStatus(PMFSTATUS pStatus);
	int		EnableIntr(BOOL intr);
//	int		Readp(BYTE* pBuf, int iReadLength);
	int		Writep(BYTE* pBuf, int iWriteLength);
	int		WriteWithoutTrigger(BYTE* pBuf, int iWriteLength);
	BOOL	SetTrig();
	void	KillTrig();
	void	SetOwnerName(PSZ pszOwner);
	int		GetTimerConnection();
	void	SetAutoTrigOut(BOOL bAutoTrigOut)	{ c_bAutoTrigOut = bAutoTrigOut; }
	UCURV	GetRespondTime()	{ return c_portState.uRespondTime; }
	UCURV	GetWatchTime()		{ return c_portState.uWatchTime; }
	void	SetPrio(int prio)	{ c_prio = prio; }
	static void	PortHandler(BYTE* pRecv, WORD wLength, BYTE nState, PVOID pVoid);
	static void	TimeOutHandler(PVOID pVoid);
	virtual void	DoSend(int res);
	virtual void	DoReceive(BOOL respond);
};

#endif	/* CPORT_H_ */
