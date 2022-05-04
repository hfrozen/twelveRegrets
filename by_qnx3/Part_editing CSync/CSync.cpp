/**
 * @file	CSync.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "Face.h"
#include "CTool.h"
#include "CSync.h"

#define	IsDtbCh()		(c_fab.iCh < 2)
#define	IsMaster()		(c_fab.domi.wAddr == DTB_BROADCASTADDR)

CSync::CSync()
	: CPump()
{
	c_pTimer = NULL;
	c_bSlaveRespond = false;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = 0xff;
	memset(&c_mag.buf, 0, sizeof(SIZE_GENSBUF));
}

CSync::CSync(PVOID pParent, QHND hQue)
	: CPump(pParent, hQue)
{
	c_pTimer = NULL;
	c_bSlaveRespond = false;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = 0xff;
	memset(&c_mag.buf, 0, sizeof(SIZE_GENSBUF));
}

CSync::CSync(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CPump(pParent, hQue, pszOwnerName)
{
	c_pTimer = NULL;
	c_bSlaveRespond = false;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = 0xff;
	memset(&c_mag.buf, 0, sizeof(SIZE_GENSBUF));
}

CSync::~CSync()
{
	KILLR(c_pTimer);
	Close();
}

void CSync::Elapsed()
{
	if (IsDtbCh()) {
		if (!IsMaster() && c_bSlaveRespond) {
			c_bSlaveRespond = false;
			DoSend();
		}
		else	DoReceive(false);
	}
	else {
		c_pTimer->Release();
		DoReceive(false);
	}
}

void CSync::Receive(BYTE* pRecv, WORD wLength, WORD wStatus)
{
#if	defined(_AT_TS_)
	if (wLength > SIZE_RECIP) {
		//TRACK("%s>ERR:ch%d L%d!\n", c_szOwnerName, c_fab.iCh, wLength);
		wLength = SIZE_RECIP;
		return;
	}
#endif

	memset(c_mag.buf.c, 0, SIZE_GENSBUF);

	c_mag.wLength = wLength;
	c_mag.cStatus = (BYTE)wStatus;
	memcpy(c_mag.buf.c, pRecv, wLength);

	if (IsDtbCh()) {
		if (!IsMaster() && c_mag.buf.f.wAddr == DTB_BROADCASTADDR) {
			c_bSlaveRespond = true;
			// 181023
			//GETFSC(pFsc);
			//pFsc->TimerRun(c_fab.iCh ? true : false, c_fab.t.wResp * 10);	// 1. 슬레이브 전송 시간을 트리거한다..
			c_pTimer->Trigger(c_fab.t.wResp);
		}
	}
	else {		// 디바이스
		if (c_pTimer->GetRhythm())	c_pTimer->Release();
	}
	DoReceive(true);
}

bool CSync::Open(int iCh, WORD wAddr, CFsc::enENCMODE mode, bool bAddrMatch, DWORD dwBps, WORD wRespondTime, WORD wCycleTime)
{
	if (IsValidCh(c_fab.iCh)) {
		KILLR(c_pTimer);
		Close();
		c_fab.iCh = 0xff;
	}
	if (!IsValidCh(iCh)) {
		TRACK("%s>PORT>ERR:invalid channel %d!\n", c_szOwnerName, iCh);
		return false;
	}

	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = iCh;
	c_fab.domi.wAddr = wAddr;
	c_fab.dwBps = dwBps;
	c_fab.t.wResp = wRespondTime;
	c_fab.t.wCycle = wCycleTime;
	c_fab.bAddrMatch = bAddrMatch;

	GETFSC(pFsc);
	bool bRes = pFsc->Appear(c_fab.iCh, c_fab.domi.wAddr, mode, c_fab.dwBps, c_fab.bAddrMatch, 7,		// 171025, 4,		// 170925, cause ecu, 7,	//IsDtbCh() ? 7 : 0,
										(CFsc::PRECVINTR)(&CSync::Receive), (CPump*)this);
	if (!bRes) {
		TRACK("%s>PORT>ERR:can not open ch.%d.\n", c_szOwnerName, c_fab.iCh);
		return false;
	}

	if (wRespondTime > 0 || wCycleTime > 0) {
		int prio;
		if (c_fab.iCh < 4)	prio = PRIORITY_PERIODPORTA;
		else if (c_fab.iCh < 8)	prio = PRIORITY_PERIODPORTB;
		else	prio = PRIORITY_PERIODPORTC;

		c_pTimer = new CTimer(this);	//, buf);
		if (c_pTimer->Initial(prio, (CTimer::PTIMERFUNC)(&CSync::Elapsed), (CPump*)this, TIMERCODE_BYCH(c_fab.iCh - 1)) != 0) {
			KILL(c_pTimer);
			TRACK("%s>PORT>ERR:can not create timer for ch.%d!\n", c_szOwnerName, c_fab.iCh);
			return false;
		}
	}

	TRACK("%s>PORT:ch.%d, addr 0x%04x, bps %d, respond time %d, cycle time %d.\n",
			c_szOwnerName, c_fab.iCh, c_fab.domi.wAddr, c_fab.dwBps, c_fab.t.wResp, c_fab.t.wCycle);

	return true;
}

bool CSync::Close()
{
	if (IsValidCh(c_fab.iCh)) {
		KILLR(c_pTimer);
		GETFSC(pFsc);
		if (!pFsc->Disappear(c_fab.iCh)) {
			TRACK("%s>PORT>ERR:can not close ch.%d!\n", c_szOwnerName, c_fab.iCh);
			return -1;
		}
		memset(&c_fab, 0, sizeof(LFABRIC));
		c_fab.iCh = 0;
	}
	return 0;
}

bool CSync::ModifyAddr(WORD wAddr, WORD wRespondTime)
{
	GETFSC(pFsc);
	if (IsDtbCh()) {
		c_fab.domi.wAddr = wAddr;
		c_fab.t.wResp = wRespondTime;
		c_fab.bAddrMatch = (wAddr >= DTB_IDTOADDR(FID_PAS) && wAddr < DTB_BROADCASTADDR) ? true : false;	// tu never match
		// 그러나 CU는 어드레스를 바꾸지 않는다...
		if (pFsc->Alter(c_fab.iCh, c_fab.domi.wAddr, c_fab.bAddrMatch))	return true;
	}
	else {
		c_fab.domi.wAddr = wAddr;
		c_fab.t.wCycle = wRespondTime;
		if (pFsc->Alter(c_fab.iCh, c_fab.domi.wAddr))	return true;
	}
	return false;
}

bool CSync::Send(BYTE* pBuf, WORD wLength)
{
	if (pBuf != NULL && wLength > 0) {
		GETFSC(pFsc);
		bool bRes = pFsc->Shoot(c_fab.iCh, (PVOID)pBuf, wLength);
		if (c_fab.t.wCycle > 0) {
			// 2. 170221.보내고 사이클 시간 또는 잔여 시간을 트리거 한다.
			if (IsDtbCh()) {
				//c_itvm.IsSmall(0.06f, "PORT>WARN:send interval ");
				// 181023
				//pFsc->TimerRun(c_fab.iCh ? true : false, (c_fab.t.wCycle - c_fab.t.wResp) * 10);
				c_pTimer->Trigger(c_fab.t.wCycle - c_fab.t.wResp);
			}
			else	c_pTimer->Trigger((int)c_fab.t.wCycle);
		}
		return bRes;
	}
	return false;
}

bool CSync::SendOnly(BYTE* pBuf, WORD wLength)
{
	if (pBuf != NULL && wLength > 0) {
		GETFSC(pFsc);
		return pFsc->Shoot(c_fab.iCh, (PVOID)pBuf, wLength);
	}
	return false;
}

void CSync::KillTrig()
{
	c_pTimer->Release();

#if	defined(DEBUG_POLESEQ)
	TRACK("%s>PORT:kill trigger ch.%d!\n", c_szOwnerName, c_fab.iCh);
#endif
}
