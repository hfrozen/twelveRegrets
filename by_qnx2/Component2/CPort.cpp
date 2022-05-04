/*
 * CPort.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "Arrange.h"
#include "../Inform2/Fpga/Face.h"
#include "CTool.h"
#include "CBand.h"
#include "CFsc.h"
#include "CPort.h"

#include "../ts/CLand.h"

#define	IsDtbCh()		(c_fab.iCh < 2)
#define	IsMaster()		(c_fab.domi.wAddr == DTB_BROADCASTADDR)
#define	IsValidCh(x)	((x >= 0 && x < 2) || (x >= PSCCHB_RTD && x <= PSCCHB_MAX))

CPort::CPort()
	: CPump()
{
	c_pTimer = NULL;
	c_bSlaveRespond = false;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = 0xff;
	memset(&c_mag.buf, 0, sizeof(SIZE_GENSBUF));
	//c_itvm.Initial();
	//c_respt = 0.f;
}

CPort::CPort(PVOID pParent, QHND hQue)
	: CPump(pParent, hQue)
{
	c_pTimer = NULL;
	c_bSlaveRespond = false;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = 0xff;
	memset(&c_mag.buf, 0, sizeof(SIZE_GENSBUF));
	//c_itvm.Initial();
	//c_respt = 0.f;
}

CPort::CPort(PVOID pParent, QHND hQue, PSZ pszOwnerName)
	: CPump(pParent, hQue, pszOwnerName)
{
	c_pTimer = NULL;
	c_bSlaveRespond = false;
	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = 0xff;
	memset(&c_mag.buf, 0, sizeof(SIZE_GENSBUF));
	//c_itvm.Initial();
	//c_respt = 0.f;
}

CPort::~CPort()
{
	KILLR(c_pTimer);
	Close();
}

void CPort::Elapsed()
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

void CPort::Receive(BYTE* pRecv, WORD wLength, WORD wStatus)
{
	if (wLength > SIZE_RECIP) {
		//TRACK("%s>ERR:ch%d L%d!\n", c_szOwnerName, c_fab.iCh, wLength);
		wLength = SIZE_RECIP;
		return;
	}
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

bool CPort::Open(int iCh, WORD wAddr, DWORD dwBps, WORD wRespondTime, WORD wCycleTime)
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

	if (iCh < MAX_MANCHESTER)	dwBps = DTB_BAUDRATE;
	//if (dwBps < MIN_BAUDRATE || dwBps > MAX_BAUDRATE) {
	//	TRACK("%s>PORT>ERR:ch.%d invalid baudrate!(%ld).\n", c_szOwnerName, iCh, dwBps);
	//	return false;
	//}

	memset(&c_fab, 0, sizeof(LFABRIC));
	c_fab.iCh = iCh;
	c_fab.domi.wAddr = wAddr;
	c_fab.dwBps = dwBps;
	c_fab.t.wResp = wRespondTime;
	c_fab.t.wCycle = wCycleTime;

	// FOR_SAFETY, to receive everything
	//// bAddrMatch when device or cu
	//c_fab.bAddrMatch = ((!IsDtbCh() && c_fab.domi.wAddr < 0xffff) ||
	//		(c_fab.domi.wAddr >= DTB_IDTOADDR(FID_PAS) && c_fab.domi.wAddr < DTB_BROADCASTADDR)) ? true : false;
	//
	c_fab.bAddrMatch = (IsDtbCh() || c_fab.domi.wAddr == DTB_BROADCASTADDR) ? false : true;
	// ?????
	if (IsDtbCh()) {
		TRACK("%s>PORT>dtb %d(%#x) %s addressmatch.\n", c_szOwnerName, iCh, c_fab.domi.wAddr, c_fab.bAddrMatch ? "is" : "is not");
	}

	GETFSC(pFsc);
	bool bRes = pFsc->Appear(c_fab.iCh, c_fab.domi.wAddr, c_fab.dwBps, c_fab.bAddrMatch, 7,		// 171025, 4,		// 170925, cause ecu, 7,	//IsDtbCh() ? 7 : 0,
										(CFsc::PRECVINTR)(&CPort::Receive), (CPump*)this);
										// 181023
										//IsDtbCh() ? (CFsc::PONESINTR)(&CPort::Elapsed) : (CFsc::PONESINTR)NULL, (CPump*)this);
	if (!bRes) {
		TRACK("%s>PORT>ERR:can not open ch.%d.\n", c_szOwnerName, c_fab.iCh);
		return false;
	}

	//if (!IsDtbCh() && c_fab.t.wCycle > 0) {		// use timer?
	int prio;
	if (c_fab.iCh < MAX_MANCHESTER)	prio = PRIORITY_PERIODPORTA;
	else if (c_fab.iCh < 8)	prio = PRIORITY_PERIODPORTB;
	else	prio = PRIORITY_PERIODPORTC;

	c_pTimer = new CTimer(this);	//, buf);
	if (c_pTimer->Initial(prio, (CTimer::PTIMERFUNC)(&CPort::Elapsed), (CPump*)this, TIMERCODE_BYCH(c_fab.iCh - 1)) != 0) {
		KILL(c_pTimer);
		TRACK("%s>PORT>ERR:can not create timer for ch.%d!\n", c_szOwnerName, c_fab.iCh);
		return false;
	}
	//}

	if (IsDtbCh())
		TRACK("%s>PORT:DTB ch.%d, respond time %d, cycle time %d.\n", c_szOwnerName, c_fab.iCh, c_fab.t.wResp, c_fab.t.wCycle);
	else	TRACK("%s>PORT:ch.%d, addr 0x%04x, bps %d.\n", c_szOwnerName, c_fab.iCh, c_fab.domi.wAddr, c_fab.dwBps);

	return true;
}

bool CPort::Close()
{
	if (IsValidCh(c_fab.iCh)) {
		// 190621
		KILLR(c_pTimer);
		GETFSC(pFsc);
		if (!pFsc->Disappear(c_fab.iCh)) {
			TRACK("%s>PORT>ERR:can not close ch.%d!\n", c_szOwnerName, c_fab.iCh);
			return -1;
		}
		// 181023
		//if (IsDtbCh())	pFsc->TimerStop(c_fab.iCh ? true : false);
		//else if (c_pTimer != NULL)	c_pTimer->Release();
		// 190621
		//c_pTimer->Release();
		memset(&c_fab, 0, sizeof(LFABRIC));
		c_fab.iCh = 0;
	}
	return 0;
}

bool CPort::ModifyAddr(WORD wAddr, WORD wRespondTime)
{
	GETFSC(pFsc);
	if (IsDtbCh()) {
		c_fab.domi.wAddr = wAddr;
		c_fab.t.wResp = wRespondTime;
		// FOR_SAFETY, to receive everything
		//c_fab.bAddrMatch = (wAddr >= DTB_IDTOADDR(FID_PAS) && wAddr < DTB_BROADCASTADDR) ? true : false;	// tu never match
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

bool CPort::Send(BYTE* pBuf, WORD wLength)
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

bool CPort::SendOnly(BYTE* pBuf, WORD wLength)
{
	if (pBuf != NULL && wLength > 0) {
		GETFSC(pFsc);
		return pFsc->Shoot(c_fab.iCh, (PVOID)pBuf, wLength);
	}
	return false;
}

void CPort::KillTrig()
{
	// 181023
	//if (IsDtbCh()) {
	//	GETFSC(pFsc);
	//	pFsc->TimerStop(c_fab.iCh ? true : false);
	//}
	//else {
	//	if (c_pTimer != NULL)	c_pTimer->Release();
	//}
	c_pTimer->Release();

#if	defined(DEBUG_POLESEQ)
	TRACK("%s>PORT:kill trigger ch.%d!\n", c_szOwnerName, c_fab.iCh);
#endif
}
