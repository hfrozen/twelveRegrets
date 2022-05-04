/* CDtb.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include "Track.h"
#include "CBand.h"
#include "CFio.h"
#include "CFsc.h"
#include "CPaper.h"
#include "../ts/CLand.h"
#include "CDtb.h"

//#define	DTBADD_RECFAILS

#define	GETFIO(p)\
	CFio* p = NULL;\
	do {\
		CLand* pLand = (CLand*)c_pParent;\
		ASSERTP(pLand);\
		pFio = pLand->GetFio();\
		ASSERTP(pFio);\
	} while (0)

CDtb::CDtb(PVOID pParent, QHND hReservoir)
	: CPort(pParent, hReservoir)	//(PSZ)"DTB")
{
	c_bLine = false;
	c_cProperID = 0xff;
	c_cID = 0xff;
	c_cCarLength = DEFAULT_CARLENGTH;
	c_dwOrder = 0;
	ClearAllDozen();
	c_wSendWatch = 0;
	c_wErrBitmap = 0;
}

CDtb::CDtb(PVOID pParent, QHND hReservoir, PSZ pszOwnerName)	// must remove this
	: CPort(pParent, hReservoir, pszOwnerName)
{
	c_bLine = false;
	c_cProperID = 0xff;
	c_cID = 0xff;
	c_cCarLength = DEFAULT_CARLENGTH;
	c_dwOrder = 0;
	ClearAllDozen();
	c_wSendWatch = 0;
	c_wErrBitmap = 0;
}

CDtb::~CDtb()
{
	CloseBus();
}

void CDtb::DoSend()
{
#if	defined(_DIRECT_CALL_)
	CLand* pLand = (CLand*)c_pParent;
	ASSERTP(pLand);
	pLand->SendDtbBus((DWORD)c_bLine);
#else
	SendTag(TAG_SENDBUS, c_pParent, (DWORD)c_bLine);
#endif
	GETBAND(pBand);
	pBand->Section(c_bLine, c_cID);
}

void CDtb::DoReceive(bool bReal)
{
	BYTE cCarLength = c_cCarLength;

	GETBAND(pBand);
	// check timeout
	if (!bReal) {
		KillTrig();
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTBF_RTIMEOUT, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTBF_RTIMEOUT, c_bLine));
		c_wErrBitmap = (1 << DTBF_RTIMEOUT);
#endif
		// FOR_SAFETY, to receive everything
		//// by timeout
		//if (c_cID < FID_PAS) {		// CU는 timeout에 의해 버스 사이클을 끝내므로 램프는 조용히...
			GETFIO(pFio);
			pFio->Lamp(c_bLine ? EPOS_RECEIVEBTIMEOUT : EPOS_RECEIVEATIMEOUT);
		//}

		pBand->Section(c_bLine, PERIOD_TIMEOUT);
		return;
	}

	GETPAPER(pPaper);
	// check size
	if (c_mag.wLength != SIZE_RECIP) {
		if (pPaper->GetFloatID() != FID_HEAD)	KillTrig();		// slave do not response to invalid frame
		c_wErrBitmap = (1 << DTBF_RSIZE);

#if	defined(DTBADD_RECFAILS)
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTBF_RSIZE, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTBF_RSIZE, c_bLine));
#endif
#endif
		return;
	}

	PROLOG prol;
	memcpy(&prol, c_mag.buf.c, sizeof(PROLOG));

	// check address range
	if (prol.wAddr != DTB_BROADCASTADDR && (prol.wAddr <= DTB_BASEADDR || prol.wAddr > DTB_LASTADDR(cCarLength))) {
		if (pPaper->GetFloatID() != FID_HEAD)	KillTrig();		// slave do not response to invalid frame
		c_wErrBitmap = (1 << DTBF_RADDR);

#if	defined(DTBADD_RECFAILS)
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTBF_RADDR, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTBF_RADDR, c_bLine));
#endif
#endif
		return;
	}

	// check echo
	if (prol.wAddr == c_fab.domi.wAddr && prol.cProperID == c_cProperID) {	// aux가 main이 되는 경우도 있으므로 고유 번호까지 검사한다.
		if (pPaper->GetFloatID() != FID_HEAD)	KillTrig();		// slave do not response to own address
		c_wErrBitmap = (1 << DTBF_RECHO);

#if	defined(DTBADD_RECFAILS)
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTBF_RECHO, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTBF_RECHO, c_bLine));
#endif
#endif
		return;
	}

	if (c_fab.bAddrMatch && prol.wAddr != DTB_BROADCASTADDR) {
		if (pPaper->GetFloatID() != FID_HEAD)	KillTrig();		// slave do not response to invalid address
		c_wErrBitmap = (1 << DTBF_RADDRB);

#if	defined(DTBADD_RECFAILS)
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTBF_RADDRB, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTBF_RADDRB, c_bLine));
#endif
#endif
		return;
	}

	// check order
	//if ((ImBroadcast && SendOrder != RecvOrder) ||
	//	(ImSlave && SendOrder == RecvOrder)) {
	if ((c_fab.domi.wAddr == (WORD)DTB_BROADCASTADDR && c_dwOrder != prol.dwOrder) ||
		(c_fab.domi.wAddr != (WORD)DTB_BROADCASTADDR &&
		(prol.wAddr == (WORD)DTB_BROADCASTADDR && c_dwOrder == prol.dwOrder)) ||
		(prol.wAddr != (WORD)DTB_BROADCASTADDR && c_dwOrder != prol.dwOrder)) {
		if (pPaper->GetFloatID() != FID_HEAD)	KillTrig();		// slave do not response to invalid order
		c_wErrBitmap = (1 << DTBF_RORDER);

#if	defined(DTBADD_RECFAILS)
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTBF_RORDER, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTBF_RORDER, c_bLine));
#endif
#endif
		return;
	}

	//if (ImSlave && Recved broadcast)
	if (c_fab.domi.wAddr != (WORD)DTB_BROADCASTADDR && prol.wAddr == (WORD)DTB_BROADCASTADDR)
		c_dwOrder = prol.dwOrder;	// recharge order at slave

	// 정상 수신 프레임이므로 각 버퍼에 저장하고 랜드에 통보한다.
	if (prol.wAddr == DTB_BROADCASTADDR) {
		ClearAllDozen();
		memcpy(&c_doz.recip[0], c_mag.buf.c, SIZE_RECIP);
#if	defined(_DIRECT_CALL_)
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ReceiveDtbBus(MAKEDWORD(DTB_RBROADCAST, c_bLine));
#else
		SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTB_RBROADCAST, c_bLine));
		//c_wErrBitmap = (1 << DTB_RBROADCAST);
#endif
		pBand->Synchro(c_bLine);		// sync. by slave
	}
	else {
		int id = DTB_ADDRTOID(prol.wAddr);
		memcpy(&c_doz.recip[id], c_mag.buf.c, SIZE_RECIP);
		if (cCarLength != 0) {
			if (prol.wAddr == DTB_LASTADDR(cCarLength)) {
				// FOR_SAFETY, to receive everything
				//if (pPaper->GetFloatID() < LENGTH_TU)
				KillTrig();		// prevent timeout
#if	defined(_DIRECT_CALL_)
				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				pLand->ReceiveDtbBus(MAKEDWORD(DTB_RFULL, c_bLine));
#else
				SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTB_RFULL, c_bLine));
				//c_wErrBitmap = (1 << DTB_RFULL);
#endif
			}
		}
		pBand->Section(c_bLine, id);
	}
}

void CDtb::Arteriam()
{
}

bool CDtb::OpenBus(bool bLine, int iProperID, int iID, int iCarLength, WORD wCycleTime)
{
	if (iID >= SIZE_MAXCARLENGTH) {
		TRACK("%s>ERR:invalid id!(%d)\n", c_szOwnerName, iID);
		return false;
	}
	if (iID < LENGTH_TU && (iCarLength < 4 || iCarLength >= SIZE_MAXCARLENGTH)) {
		TRACK("%s>ERR:invalid car length!(%d)\n", c_szOwnerName, iCarLength);
		return false;
	}

	// rest time is needed for bus cycle close at tu slave!
	WORD wRespondTime = 0;
	WORD wAddr = DTB_BROADCASTADDR;
	if (iID > 0) {
		wAddr = DTB_IDTOADDR(iID);
		wRespondTime =  iID * DTBPERIOD_SECTION + DTBPERIOD_MARGIN;
		if (iID >= LENGTH_TU)	wRespondTime += DTBPERIOD_CUMARGIN;
	}

	if (!(Open(bLine ? 1 : 0, wAddr, DTB_BAUDRATE, wRespondTime, wCycleTime))) {
		TRACK("%s>ERR:open failure!\n", c_szOwnerName);
		return false;
	}
	c_bLine = bLine;
	c_cProperID = (BYTE)iProperID;
	c_cID = (BYTE)iID;
	c_cCarLength = (BYTE)iCarLength;

	return true;
}

bool CDtb::ModifyBus(int iID)
{
	if (iID >= SIZE_MAXCARLENGTH) {
		TRACK("%s>ERR:invalid id!(%d)\n", c_szOwnerName, iID);
		return false;
	}

	WORD wRespondTime = 0;
	WORD wAddr = DTB_BROADCASTADDR;
	if (iID > 0) {
		wRespondTime = iID * DTBPERIOD_SECTION + DTBPERIOD_MARGIN;
		if (iID >= LENGTH_TU)	wRespondTime += DTBPERIOD_CUMARGIN;		// 실행되지 않는 코드...
		wAddr = DTB_IDTOADDR(iID);
	}
	if (!(ModifyAddr(wAddr, wRespondTime))) {
		TRACK("%s>ERR:modify failure!\n", c_szOwnerName);
		return false;
	}
	c_cID = (BYTE)iID;

	return true;
}

void CDtb::CloseBus()
{
	Close();
	c_cID = 0xff;
	c_cCarLength = DEFAULT_CARLENGTH;
}

bool CDtb::SendBus(PVOID pVoid, DWORD dwOrder, bool bSend)
{
	_TWATCHCLICK();

	if (c_cID == FID_HEAD)	ClearAllDozen();

	memcpy(&c_doz.recip[c_cID], pVoid, SIZE_RECIP);
	c_doz.recip[c_cID].real.prol.wAddr = c_fab.domi.wAddr;
	c_doz.recip[c_cID].real.prol.dwOrder = c_dwOrder = dwOrder;
	// 190110
	bool bRes = true;
	// FOR_SAFETY, to receive everything
	//if (bSend)	bRes = Send((BYTE*)&c_doz.recip[c_cID], SIZE_RECIP);
	if (bSend) {
		BYTE cCarLength = c_cCarLength;
		if (c_fab.domi.wAddr == DTB_LASTADDR(cCarLength)) {
			bRes = SendOnly((BYTE*)&c_doz.recip[c_cID], SIZE_RECIP);
			SendTag(TAG_RECEIVEBUS, c_pParent, MAKEDWORD(DTB_RFULL, c_bLine));
		}
		else	bRes = Send((BYTE*)&c_doz.recip[c_cID], SIZE_RECIP);
	}

	_TWATCHMEASURE(WORD, c_wSendWatch, 6);
	return bRes;
}

ENTRY_CONTAINER(CDtb)
	SCOOP(&c_fab.domi.wAddr,	sizeof(WORD),			"DTB")
	SCOOP(&c_fab.t.wResp,		sizeof(WORD),			"")
	SCOOP(&c_fab.t.wCycle,		sizeof(WORD),			"")
	SCOOP(&c_fab.iCh,			sizeof(BYTE),			"ch")
	SCOOP(&c_wSendWatch,		sizeof(WORD),			"")	//"DTB")
	SCOOP(&c_wErrBitmap,		sizeof(WORD),			"")
EXIT_CONTAINER()
