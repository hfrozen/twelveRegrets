/*
 * CStick.cpp
 *
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include "Track.h"
#include "CPaper.h"
#include "CArch.h"
#include "CLand.h"

#include "CStick.h"

const WORD	CStick::c_wMutualPattern[20][2] = {
							// bit order = 		ts - hs
	{	MUTUALPAT_UNKNOWN,	21	},	// 0000:	NON-NON,	비 활성  -,
	{	MUTUALPAT_CONTINUE,	26	},	// 0001:	NON-HCR,	현상 유지+,	후미 회로 이상
	{	MUTUALPAT_DOWN,		27	},	// 0010:	NON-TCR,	현상 포기,	후미 회로 이상
	{	MUTUALPAT_DOWN,		28	},	// 0011:	NON-H&T,	현상 포기,	선두 회로 이상
	{	MUTUALPAT_CHANGE,	25	},	// 0100:	HCR-NON,	방향 전환*,	선두 회로 이상
	{	MUTUALPAT_DOWN,		29	},	// 0101:	HCR-HCR,	현상 포기?,	양 쪽 회로 이상
	{	MUTUALPAT_CHANGE,	24	},	// 0110:	HCR-TCR,	방향 전환,	정상
	{	MUTUALPAT_CHANGE,	25	},	// 0111:	HCR-H&T,	방향 전환*,	선두 회로 이상
	{	MUTUALPAT_DOWN,		28	},	// 1000:	TCR-NON,	현상 포기-,	선두 회로 이상
	{	MUTUALPAT_CONTINUE,	22	},	// 1001:	TCR-HCR,	현상 유지+,	정상
	{	MUTUALPAT_DOWN,		29	},	// 1010:	TCR-TCR,	현상 포기,	양쪽 회로 이상
	{	MUTUALPAT_DOWN,		28	},	// 1011:	TCR-H&T,	현상 포기,	선두 회로 이상
	{	MUTUALPAT_DOWN,		29	},	// 1100:	H&T-NON,	현상 포기-,	양쪽 회로 이상
	{	MUTUALPAT_CONTINUE,	26	},	// 1101:	H&T-HCR,	현상 유지+,	후미 회로 이상
	{	MUTUALPAT_DOWN,		27	},	// 1110:	H&T-TCR,	현상 포기,	후미 회로 이상
	{	MUTUALPAT_DOWN,		29	},	// 1111:	H&T-H&T,	현상 포기,	양쪽 회로 이상
	{	MUTUALPAT_UNKNOWN,	30	},	// XX00:	NON-NON,	비 활성  -,	통신 이상
	{	MUTUALPAT_CONTINUE,	31	},	// XX01:	NON-HCR,	현상 유지+,	통신 이상
	{	MUTUALPAT_DOWN,		32	},	// XX10:	NON-TCR,	현상 포기,	통신 이상
	{	MUTUALPAT_DOWN,		33	}	// XX11:	NON-H&T,	현상 포기,	통신 이상
};

CStick::CStick()
{
	c_pParent = NULL;
	Initial();
}

CStick::~CStick()
{
}

void CStick::Initial()
{
	c_bDomin = false;
	c_bHcrOff = false;
	c_wWarmupTimer = TPERIOD_WARMUPB;
	memset(&c_reversal, 0, sizeof(REVERSAL));
	//c_reversal.d.SetDebounceTime(DEB_REVERSALB);
	memset(&c_mut, 0, sizeof(MUTUAL));
	c_mut.d.buf = 0xff;
	c_mut.d.SetDebounceTime(DEB_MUTUAL);
	memset(&c_rud, 0, sizeof(RUDDER));
	c_rud.d.SetDebounceTime(DEB_RUDDER);
	TRACK("STCK:begin warm %d\n", TPERIOD_WARMUPB);
}

// 1 : HCR/TCR을 검사하다가 마스터가 전환 동기를 발견하면...
// 2 : prol.cmd.b.mcd를 통해 이를 서로에게 알린다, 그러나 마스터의 recipe만 사용한다.
// 3 : 마스터와 슬레이브는 이 명령을 디바운스한 후 전환 작업을 시작한다.
void CStick::Control(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	CLand* pLand = (CLand*)c_pParent;
	c_mtx.Lock();
	if (c_wWarmupTimer > 0 && -- c_wWarmupTimer == 0) {
		if (pLand->GetColdWakeup() && CAR_FSELF() == FID_HEAD) {
			pLand->StirTimeMatch(false);
			if (pPaper->GetLabFromRecip())	TRACK("STCK:time match.\n");
		}
	}
	c_mtx.Unlock();

	GETARCHIVE(pArch);
	PDOZEN pDoz = pPaper->GetDozen();

	// 171226
	//bool bHcr = GETTI(FID_HEAD, TUDIB_HCR) ? true : false;
	//if (c_hm.b.Stabilize(bHcr)) {
	//	if (c_hm.b.cur && !c_hm.bPrev) {
	//		c_hm.bPrev = true;
	//		// 171221
	//		pArch->Conceal(CID_ALL, 100);
	//		pArch->Conceal(CID_ALL, 101);
	//		pArch->Conceal(CID_ALL, 102);
	//		pArch->Conceal(CID_ALL, 103);
	//		pArch->Conceal(CID_ALL, 104);
	//		pArch->Conceal(CID_ALL, 105);
	//		pArch->Conceal(CID_ALL, 325);
	//		//if (c_hm.bCatchDown) {
	//		//	c_hm.bCatchDown = false;
	//		//	pLand->SetNewSheetFromHcr();
	//		//}
	//	}
	//	else if (!c_hm.b.cur) {
	//		//if (c_hm.bPrev) {
	//		//	c_hm.bCatchDown = true;
	//			c_hm.bPrev = false;
	//		//}
	//	}
	//}
	// 171221
	//if (!c_hm.b.cur) {
	//	pLand->SetEcuBcfPrevent();
	//	pLand->SetAtoLinePrevent();
	//}

	// HCR, TCR Check
	BYTE state = (GETTIS(FID_HEAD, TUDIB_HCR) >> 5) & 3;		// 선두 차 HCR과 TCR만 참조
	if (IsCorrAddr(pDoz->recip[FID_TAIL].real.prol.wAddr))
		state |= ((GETTIS(FID_TAIL, TUDIB_HCR) >> 3) & 0xc);	// 후미 차 HCR과 TCR만 참조
	else	state |= 0x10;

	if (c_mut.d.Stabilize(state)) {
		c_mut.cPattern = c_wMutualPattern[c_mut.d.cur][0];
		c_mut.cError = c_wMutualPattern[c_mut.d.cur][1];
		pArch->Closeup(CID_BOW, c_wMutualPattern[c_mut.d.cur][1], 21, 33);
#if	defined(_DEBUG_TURN_)
		TRACK("STCK:dir. change 0x%02x - 0x%02x - 0x%02x\n", c_mut.d.cur, pDoz->recip[FID_HEAD].real.prol.cProperID, pDoz->recip[FID_TAIL].real.prol.cProperID);
#endif
		// 171226
		//if (c_mut.cPattern == MUTUALPAT_CONTINUE) {
		//	if (!pPaper->IsActive()) {
		//		// active가 아닐 때 라는 것은 전원 투입 후... (cold start)
		//		c_wWarmupTimer = TPERIOD_WARMUPA;
		//		pArch->SetWarmup();
		//		pPaper->SetOblige(OBLIGE_ACTIVE);
		//		//DTSTIME* pDtst = pArch->GetDtsTime();
		//		//memcpy(&c_stime, pDtst, sizeof(DTSTIME));
		//		TRACK("STCK:capture active.(0x%02X) begin warm %d\n", pPaper->GetOblige(), TPERIOD_WARMUPA);
		//	}
		//}
		//else {
		//	// 여기가 F/R이 중립이 되는 시점인데.. 무엇을 할 수 있을까???
		//}
		//c_reversal.bFlow = (c_mut.cPattern == MUTUALPAT_CHANGE) ? true : false;
		//if (c_reversal.bFlow)	TRACK("STCK>TSP1:reversal begin.(%d)\n", pDoz->recip[FID_HEAD].real.prol.dwOrder);
		// TSP1. 처음 감지 - DTB를 통해 마스터 체인지 사이클을 알린다.
		if (c_mut.cPattern == MUTUALPAT_CHANGE) {
			c_reversal.bFlow = true;
			TRACK("STCK>TSP1:reversal begin.(%d)\n", pDoz->recip[FID_HEAD].real.prol.dwOrder);
			// TSP1. 처음 감지 - DTB를 통해 마스터 체인지 사이클을 알린다.
		}
		else {
			c_reversal.bFlow = false;
			if (c_mut.cPattern == MUTUALPAT_CONTINUE && !pPaper->IsActive()) {	// active가 아닐 때 라는 것은 전원 투입 후... (cold start)
				c_wWarmupTimer = TPERIOD_WARMUPA;
				pArch->SetWarmup();
				pPaper->SetOblige(OBLIGE_ACTIVE);
				TRACK("STCK:capture active.(0x%02X) begin warm %d\n", pPaper->GetOblige(), TPERIOD_WARMUPA);
			}
			if (!(state & 1)) {		// HEAD HCR off
				pLand->SetEcuBcfPrevent();
				pLand->SetAtoLinePrevent();
				c_bHcrOff = false;
			}
			else {					// HEAD HCR on
				if (!c_bHcrOff) {
					c_bHcrOff = true;
					pArch->ConcealAtoBcf();
				}
			}
		}
	}
	pRecip->real.sign.gcmd.b.mcd = c_reversal.bFlow;	// reform
	//pLand->SetChapID(602);

	// 여기서 F/R을 검사하여 열차 방향을 설정한다.
	state = (GETTIS(FID_HEAD, TUDIB_FORWARD) >> 1) & 3;
	if (c_rud.d.Stabilize(state)) {
		switch (c_rud.d.cur) {
		case 1 :
			pRecip->real.motiv.lead.b.r = false;
			pRecip->real.motiv.lead.b.f = true;
			break;
		case 2 :
			pRecip->real.motiv.lead.b.f = false;
			pRecip->real.motiv.lead.b.r = true;
			break;
		default :
			pRecip->real.motiv.lead.b.f =
			pRecip->real.motiv.lead.b.r = false;
			break;
		}
	}
	// 180911
	//if (pPaper->GetRouteState(FID_TAIL) &&
	if (pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL) &&
		(GETTI(FID_TAIL, TUDIB_FORWARD) || GETTI(FID_TAIL, TUDIB_REVERSE)))	pArch->Shot(CID_BOW, 36);
	else	pArch->Cut(CID_BOW, 36);
	//pLand->SetChapID(603);

	c_reversal.md.bBuf = pDoz->recip[FID_HEAD].real.sign.gcmd.b.mcd;
	if (c_reversal.md.bCur != c_reversal.md.bBuf) {
		c_reversal.md.bCur = c_reversal.md.bBuf;
		if (c_reversal.md.bCur) {
			c_reversal.md.cDeb = CAR_FSELF() == FID_HEAD ? DEB_REVERSALA : DEB_REVERSALB;
			// 179822
			pPaper->SetOblige(OBLIGE_ACTIVE, false);	// dominate를 풀어서 trouble code를 잡지 않게 한다.
			TRACK("STCK>TSP2:reversal catch.(%d)\n", pDoz->recip[FID_HEAD].real.prol.dwOrder);
			// TSP2. 감지가 안정됨 - 자신의 감지를 알린다.
		}
	}
	else if (c_reversal.md.cDeb != 0 && -- c_reversal.md.cDeb == 0) {	// && c_reversal.md.bCur) {
		if (c_reversal.md.bCur) {
			pArch->DownloadCancel();
			pLand->TurnObserve();
			TRACK("STCK>TSP3:reversal agree.(%d)\n", pDoz->recip[FID_HEAD].real.prol.dwOrder);
			// TSP3. 모두가 감지하여 환경과 이력을 저장하고 모두 슬레이브로 전환한다. 마스터 없음.
			// 500ms 동안 DTB에 아무런 통신이 없어야 하나, 하나라도 포착이되면 100ms씩 늘어난다.
		}
	}

	if (CAR_FSELF() == FID_HEAD) {
		pRecip->real.sign.gcmd.b.domin = (c_wWarmupTimer == 0 && pPaper->IsActive()) ? true : false;
		if (pRecip->real.sign.gcmd.b.domin && !c_bDomin) {
			c_bDomin = true;
			TRACK("STCK:dominate.\n");
		}
		// 170822
		else if (!pRecip->real.sign.gcmd.b.domin && c_bDomin) {
			c_bDomin = false;
			TRACK("STCK:release.\n");
		}
	}
}

void CStick::Monitor()
{
	GETPAPER(pPaper);
	CUCUTOFF();
}

void CStick::CheckDomination()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin && !c_bDomin) {
		c_bDomin = true;
		TRACK("STCK:be dominated.\n");
	}
}

ENTRY_CONTAINER(CStick)
	SCOOP(&c_mut.cPattern,		sizeof(BYTE),	"STCK")
	SCOOP(&c_mut.cError,		sizeof(BYTE),	"")
	SCOOP(&c_mut.d.cur,			sizeof(BYTE),	"")
	SCOOP(&c_mut.d.buf,			sizeof(BYTE),	"")
	SCOOP(&c_wWarmupTimer,		sizeof(WORD),	"")
EXIT_CONTAINER()
