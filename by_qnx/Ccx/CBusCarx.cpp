/*
 * CBusCarx.cpp
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#include <string.h>
#include <sys/neutrino.h>
#include <inttypes.h>

#include <TrainArch.h>

#include "CBusCarx.h"
#include "CCcx.h"

CBusCarx::CBusCarx()
	: CPort()
{
	c_pParent = NULL;
	c_bLine = TRUE;
	c_uMfch = 0xff;
	c_wAddr = INITIAL_ADDR;
	c_uAttribute = 0;
	c_uCarLength = 0;
	c_uRespondTime = 0;
	c_wCycleTime = CYCLETIME_TBUS;
	c_dwOrderID = 0;
	ClearFrame();
}

CBusCarx::CBusCarx(PVOID pParent, int mq)
	: CPort(pParent, mq)
{
	c_pParent = pParent;
	c_bLine = TRUE;
	c_uMfch = 0xff;
	c_wAddr = INITIAL_ADDR;
	c_uAttribute = 0;
	c_uCarLength = 0;
	c_uRespondTime = 0;
	c_wCycleTime = CYCLETIME_TBUS;
	c_dwOrderID = 0;
	ClearFrame();
}

CBusCarx::CBusCarx(PVOID pParent, int mq, PSZ pszOwner)
	: CPort(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	c_bLine = TRUE;
	c_uMfch = 0xff;
	c_wAddr = INITIAL_ADDR;
	c_uAttribute = 0;
	c_uCarLength = 0;
	c_uRespondTime = 0;
	c_wCycleTime = CYCLETIME_TBUS;
	c_dwOrderID = 0;
	ClearFrame();
}

CBusCarx::~CBusCarx()
{
	if (c_wAddr < 0xff)	Close();
}

int CBusCarx::Open(UCURV uMfch, WORD wAddr, BOOL bLine)
{
	ASSERT(uMfch < MF_HDLCA);
	c_bLine = bLine;
	c_uMfch = uMfch;
	c_wAddr = wAddr;	// 5 ~
	c_uAttribute = PORTATTR_CC;
	c_uRespondTime = (UCURV)c_wCycleTime * (c_wAddr - 1);

	PORTSTATE state;
	memset(&state, 0, sizeof(PORTSTATE));
	state.uCh = c_uMfch;
	state.wAddr = c_wAddr | BUSADDR_HIGH;
	state.uAttribute = c_uAttribute;
	state.uRespondTime = c_uRespondTime;
	//state.uWatchTime = 0;
	int res;
	if ((res = Begin(&state)) == PORTERROR_NON) {
		SetAutoTrigOut(FALSE);
		return res;
	}
	else {
		// Trace error
		c_wAddr = INITIAL_ADDR;
		return res;
	}
}

int CBusCarx::Close()
{
	int res = End();
	if (res == PORTERROR_NON)	return res;
	// Trace error
	return res;
}

int CBusCarx::Send(PVOID pVoid)
{
	memcpy((PVOID)&c_doz.cc, pVoid, sizeof(CCFIRM));
	//c_doz.cc.wAddr = GETLWORD(c_wAddr | BUSADDR_HIGH);
	//c_doz.cc.dwOrderID = c_dwOrderID;
	int res;
	if ((res = Writep((BYTE*)&c_doz.cc, sizeof(CCFIRM))) == PORTERROR_NON)
		return res;
	// Trace error
	return res;
}

void CBusCarx::Combine(BYTE* pDest)
{
	memcpy((PVOID)pDest, (PVOID)&c_doz.tc, sizeof(TCFIRM));
}

void CBusCarx::DoSend(int res)
{
	SendMail(MA_SENDBUS, c_pParent, (long)c_bLine);
	//CCcx* pCcx = (CCcx*)c_pParent;
	//pCcx->SendBus(c_bLine);
}

void CBusCarx::DoReceive(BOOL state)
{
	TCFIRM tcf;
	if (c_recv.wLength != sizeof(TCFIRM)) {
		// Trace error
		MSGLOG("[BUS]ERROR:size(%d).\r\n", c_recv.wLength);
		SendMail(MA_RECEIVEFAULT, c_pParent, (long)c_bLine);
		return;
	}
	memcpy((PVOID)&tcf, (PVOID)c_recv.pBuf, c_recv.wLength);

	if (tcf.real.nHcrInfo >= REQ_PERTDIR) {
		KillTrig();
		return;
	}

	if (GETLWORD(tcf.real.wAddr) == (BUSADDR_HIGH | BROADCAST_ADDR) &&
			c_dwOrderID != tcf.real.dwOrderID) {
		c_dwOrderID = tcf.real.dwOrderID;
		memcpy((PVOID)&c_doz.tc, (PVOID)&tcf, sizeof(TCFIRM));
		SendMail(MA_RECEIVEBUS, c_pParent, (long)c_bLine);
		c_syncTime = ClockCycles();
	}
	else {
		// Trace error
		SendMail(MA_RECEIVEFAULT, c_pParent, (long)c_bLine);
	}
}

void CBusCarx::ClearFrame()
{
	memset((PVOID)&c_doz, 0, sizeof(CCDOZ));
}
