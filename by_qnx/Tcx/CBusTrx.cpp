/*
 * CBusTrx.cpp
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#include <string.h>
#include <sys/neutrino.h>
#include <inttypes.h>

#include <TrainArch.h>

#include "CBusTrx.h"
#include "CTcx.h"

CBusTrx::CBusTrx()
	: CPort()
{
	c_pParent = NULL;
	c_bLine = FALSE;
	c_nDuty = 0xff;
	c_uMfch = 0xff;
	c_wAddr = INITIAL_ADDR;
	c_uAttribute = 0;
	c_uCarLength = 0;
	c_uRespondTime = 0;
	c_uWatchTime = 0;
	c_wCycleTime = CYCLETIME_TBUS;
	c_dwOrderID = 0;
	ClearFirm();
}

CBusTrx::CBusTrx(PVOID pParent, int mq)
	: CPort(pParent, mq)
{
	c_pParent = pParent;
	c_bLine = FALSE;
	c_nDuty = 0xff;
	c_uMfch = 0xff;
	c_wAddr = INITIAL_ADDR;
	c_uAttribute = 0;
	c_uCarLength = 0;
	c_uRespondTime = 0;
	c_uWatchTime = 0;
	c_wCycleTime = CYCLETIME_TBUS;
	c_dwOrderID = 0;
	ClearFirm();
}

CBusTrx::CBusTrx(PVOID pParent, int mq, PSZ pszOwner)
	: CPort(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	c_bLine = FALSE;
	c_nDuty = 0xff;
	c_uMfch = 0xff;
	c_wAddr = INITIAL_ADDR;
	c_uAttribute = 0;
	c_uCarLength = 0;
	c_uRespondTime = 0;
	c_uWatchTime = 0;
	c_wCycleTime = CYCLETIME_TBUS;
	c_dwOrderID = 0;
	ClearFirm();
}

CBusTrx::~CBusTrx()
{
	if (c_wAddr < 0xff)	Close();
}

int CBusTrx::Open(UCURV uMfch, BYTE nDuty, WORD wAddr, UCURV uCarLength, BOOL bLine)
{
	ASSERT(uMfch < MF_HDLCA);
	c_uMfch = uMfch;
	ASSERT(nDuty < LENGTH_TC);
	c_bLine = bLine;
	c_nDuty = nDuty;	// 0 ~ 3
	c_wAddr = wAddr;			// 1 ~ 4
	c_uCarLength = uCarLength;	// 4, 6, 8
	c_uWatchTime = (UCURV)c_wCycleTime * (LENGTH_TC + c_uCarLength) + 5;
	if (c_nDuty == 0) {	// bus master
		c_uAttribute = PORTATTR_TCMASTER;
		c_uRespondTime = 0;
	}
	else {
		c_uAttribute = PORTATTR_TCSLAVE;
		c_uRespondTime = (UCURV)c_wCycleTime * c_nDuty;
		c_uWatchTime -= c_uRespondTime;
	}

	PORTSTATE state;
	memset(&state, 0, sizeof(PORTSTATE));
	state.uCh = c_uMfch;
	state.wAddr = BROADCAST_ADDR | BUSADDR_HIGH;
	// TC can take all message without position(wAddr=id)
	state.uAttribute = c_uAttribute;
	state.uRespondTime = c_uRespondTime;
	state.uWatchTime = c_uWatchTime;
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

int CBusTrx::Close()
{
	int res = End();
	c_uCarLength = 0;
	c_uWatchTime = 0;
	if (res == PORTERROR_NON)	return res;
	// Trace error
	return res;
}

int CBusTrx::Send(PVOID pVoid, DWORD dwOrderID)
{
	memcpy((PVOID)&c_doz.tcs[c_nDuty], pVoid, sizeof(TCFIRM));
	c_doz.tcs[c_nDuty].real.nFlowNo = c_nDuty + 1;
	if (c_nDuty == 0) {	// master-main
		c_doz.tcs[0].real.wAddr = GETLWORD(BROADCAST_ADDR | BUSADDR_HIGH);
		if (dwOrderID != 0)	c_dwOrderID = dwOrderID;
		else if (++ c_dwOrderID == 0)	++ c_dwOrderID;
		c_doz.tcs[0].real.dwOrderID = c_dwOrderID;
	}
	else {
		c_doz.tcs[c_nDuty].real.wAddr = GETLWORD(c_wAddr | BUSADDR_HIGH);
		c_doz.tcs[c_nDuty].real.dwOrderID = c_dwOrderID;
	}

	int res;
	if ((res = Writep((BYTE*)&c_doz.tcs[c_nDuty], sizeof(TCFIRM))) == PORTERROR_NON)
		return res;
	// Trace error
	return res;
}

int CBusTrx::Dispatch(WORD wAddr, PVOID pVoid)
{
	PTCPERT pPert = (PTCPERT)pVoid;
	pPert->nFlowNo = c_nDuty + 1;
	pPert->wAddr = GETLWORD(wAddr | BUSADDR_HIGH);
	if (++ c_dwOrderID == 0)	++ c_dwOrderID;
	pPert->dwOrderID = c_dwOrderID;

	int res;
	if ((res = WriteWithoutTrigger((BYTE*)pPert, sizeof(TCPERT))) != PORTERROR_NON)
		MSGLOG("[BUS]ERROR:dispatch at WriteWithoutTrigger() addr=%d res=%d.\r\n", wAddr, res);
	return res;
}

void CBusTrx::CombineZero(BYTE* pDest)
{
	memcpy((PVOID)pDest, (PVOID)&c_doz.tcs[0], sizeof(TCFIRM));
}

void CBusTrx::Combine(BYTE* pDest)
{
	int size;
	UCURV n;
	if (c_nDuty == 0)	n = 0;
	else {
		n = 1;
		pDest += sizeof(TCFIRM);
	}
	for ( ; n < LENGTH_TC; n ++) {
		size = n != c_nDuty ? sizeof(TCFIRM) : 8;
		// 8 byte include wAddr, dwOrderID, nFlowNo, nHcrInfo, make at here
		memcpy((PVOID)pDest, (PVOID)&c_doz.tcs[n], size);
		pDest += sizeof(TCFIRM);
	}
	memcpy((PVOID)pDest, (PVOID)&c_doz.ccs, sizeof(CCFIRM) * c_uCarLength);
	ClearFirm();
}

void CBusTrx::ClearFirm()
{
	memset((PVOID)&c_doz, 0, sizeof(TCDOZ));
}

void CBusTrx::ClearFirmWithoutZero()
{
	memset((PVOID)&c_doz.tcs[CAR_HEADBK], 0, sizeof(TCFIRM) * (LENGTH_TC - 1));
	memset((PVOID)&c_doz.ccs, 0, sizeof(CCFIRM) * LENGTH_CC);
}

void CBusTrx::DoSend(int res)
{
	SendMail(MA_SENDBUS, c_pParent, (DWORD)c_bLine);
	//CTcx* pTcx = (CTcx*)c_pParent;
	//pTcx->FlagDoSend(c_bLine);
}

void CBusTrx::DoReceive(BOOL state)
{
	TCFIRM tcf;
	CTcx* pTcx = (CTcx*)c_pParent;

	if (!state) {
		pTcx->RecordTime(TTIME_BUSTIMEOUT);
		SendMail(MA_RECEIVEBUS, c_pParent, MAKEDWORD(RECEIVE_TIMEOUT, c_bLine));
	}
	else {
		if (c_recv.wLength != SIZE_FIRM) {	//sizeof(TCFIRM) && c_recv.wLength != sizeof(CCFIRM)) {
			// FAULT CASE A
			//*MSGLOG("[BUS]ERROR:Unknown size(%d-%d)(%02X-%02X).\r\n",
			//*		c_recv.wLength, c_recv.nState, *c_recv.pBuf, *(c_recv.pBuf + 1));
			SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 9, c_bLine));
			return;
		}
		memcpy((PVOID)&tcf, (PVOID)c_recv.pBuf, c_recv.wLength);
		WORD cur_add = GETLWORD(tcf.real.wAddr);
		if ((cur_add & 0xff00) != BUSADDR_HIGH) {
			// FAULT CASE B
			//*MSGLOG("[BUS]ERROR:unformat high address(%04X).\r\n", cur_add);
			SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 8, c_bLine));
			return;
		}

		if (tcf.real.nHcrInfo >= REQ_PERTDIR) {
			KillTrig();			// inhibit timeout
			// Modified 2013/03/04
			//if ((cur_add & 0xff) == c_wAddr) {
			//	memcpy((PVOID)&c_pert, (PVOID)&tcf, sizeof(TCPERT));
			//	//pTcx->VestigesPertA();
			//	SendMail(MA_RECEIVEPERT, c_pParent, MAKEDWORD(0, c_bLine));
			//}
			memcpy((PVOID)&c_pert, (PVOID)&tcf, sizeof(TCPERT));
			SendMail(MA_RECEIVEPERT, c_pParent, MAKEDWORD(0, c_bLine));
			return;
		}
		cur_add &= 0xff;
		if (cur_add == BROADCAST_ADDR)	cur_add = 1;
		-- cur_add;
		if (cur_add >= CAR_HEAD && cur_add < CAR_MAX) {
			if (cur_add < LENGTH_TC) {	// TC frame
				BYTE cur_duty = tcf.real.nFlowNo - 1;	// same each Tc's nDuty
				if (cur_duty > 3) {
					// FAULT CASE C
					//*MSGLOG("[BUS]ERROR:Received other(add=%d duty=%d).\r\n", cur_add, cur_duty);
					SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 7, c_bLine));
					return;
				}
				if (c_nDuty == 0) {		// master
					memcpy((PVOID)&c_doz.tcs[cur_duty], (PVOID)&tcf, sizeof(TCFIRM));
					c_dwOrderID = tcf.real.dwOrderID;	// ?????
					if (cur_duty == 0) {	// need at poweron and Tc0 master, !!!!! NEVER EXCUTE CODE !!!!!
						SendMail(MA_RECEIVEBUS, c_pParent,
								MAKEDWORD(RECEIVE_FRAMEZERO, c_bLine));
						c_syncTime = ClockCycles();
						MSGLOG("[BUS]CAUTION:Receive frame0 at master.\r\n");
					}
					pTcx->RecordTime(cur_duty + TTIME_BUSRESPOND);
					pTcx->BusTrace(cur_duty, c_bLine);
				}
				else {		// slave
					if (cur_duty == 0 && cur_add == 0) {	// receive TcFrame0
						memcpy((PVOID)&c_doz.tcs[cur_duty], (PVOID)&tcf, sizeof(TCFIRM));
						if (tcf.real.dwOrderID != c_dwOrderID) {
							c_dwOrderID = tcf.real.dwOrderID;
							SendMail(MA_RECEIVEBUS, c_pParent,
									MAKEDWORD(RECEIVE_FRAMEZERO, MAKEDWORD(0, c_bLine)));
							c_syncTime = ClockCycles();
							pTcx->RecordTime(TTIME_BUSRESPOND);
							pTcx->BusTrace(0, c_bLine);
						}
						else {
							// FAULT CASE D
							//*MSGLOG("[BUS]ERROR:Received matched frame(%d-%d).\r\n",
							//*						c_dwOrderID, tcf.real.dwOrderID);
							SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 6, c_bLine));
							return;
						}
					}
					else if (cur_duty != c_nDuty) {				// receive another TcFrame
						memcpy((PVOID)&c_doz.tcs[cur_duty], (PVOID)&tcf, sizeof(TCFIRM));
						if (tcf.real.dwOrderID == c_dwOrderID) {
							pTcx->RecordTime(cur_duty + TTIME_BUSRESPOND);
							pTcx->BusTrace(cur_duty, c_bLine);
						}
						else {
							// FAULT CASE E
							//*MSGLOG("[BUS]ERROR:received unmatched frame(%x:%x-%X).\r\n",
							//*				c_dwOrderID, tcf.real.dwOrderID, cur_add);
							SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 5, c_bLine));
							return;
						}
					}
					else {
						// FAULT CASE F
						//*MSGLOG("[BUS]ERROR:Received own frame.\r\n", 0);
						SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 4, c_bLine));
						return;
					}
				}
			}
			else {		// CC frame
				if (tcf.real.dwOrderID == c_dwOrderID) {
					pTcx->RecordTime(cur_add + TTIME_BUSRESPOND);
					pTcx->BusTrace(cur_add, c_bLine);
					memcpy((PVOID)&c_doz.ccs[cur_add - LENGTH_TC], (PVOID)&tcf, sizeof(CCFIRM));
					if (cur_add == (c_uCarLength + 3)) {
						KillTrig();
						pTcx->RecordTime(TTIME_BUSCLOSE);
						SendMail(MA_RECEIVEBUS, c_pParent, MAKEDWORD(0, c_bLine));
						// Copy all frame
					}
				}
				else {
					// FAULT CASE G
					//*MSGLOG("[BUS]ERROR:Unmatch order(%x:%x-%X)[%d].\r\n",
					//*		c_dwOrderID, tcf.real.dwOrderID, cur_add, c_bLine);	//, GetInterval());
					SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 3, c_bLine));
					return;
				}
			}
		}
		else {
			// FAULT CASE H
			// receive any address
			//*MSGLOG("[BUS]ERROR:Unknown address(%04X).\r\n", cur_add);
			SendMail(MA_RECEIVEFAULT, c_pParent, MAKEDWORD(SIZE_BUSFAULT - 2, c_bLine));
			return;
		}
	}
}
