/*
 * CVerse.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "Track.h"
#include "../Component2/CTool.h"
#include "CPaper.h"
#include "CArch.h"
#include "CSch.h"
#include "CProse.h"

#include "CVerse.h"

const WORD CVerse::c_wFlapLength[PDEVID_MAX][LL_MAX] = {	// 200
	{	SIZE_ATOSR,	SIZE_ATOSA,	LINEDEB_POLEB	},
	{	SIZE_HDUSR,	SIZE_HDUSA,	LINEDEB_POLE	},
	{	SIZE_RTDSB,	SIZE_RTDSB,	LINEDEB_POLE	},
	{	SIZE_PAUSR,	SIZE_PAUSA,	LINEDEB_POLEB	},
	{	SIZE_PISSR,	SIZE_PISSA,	LINEDEB_POLEB	},
	{	SIZE_TRSSR,	SIZE_TRSSA,	LINEDEB_POLEB	},
	{	SIZE_FDUSR,	SIZE_FDUSA,	LINEDEB_POLEC	},
	{	SIZE_BMSSR,	SIZE_BMSSA,	LINEDEB_POLEC	},
	{	0,			0,			0	}
};

const WORD CVerse::c_wFlapLength2[PDEVID_MAX][LL_MAX] = {	// 24
	{	SIZE_ATOSR,	SIZE_ATOSA,	LINEDEB_POLEA	},	//D	},
	{	SIZE_HDUSR,	SIZE_HDUSA,	LINEDEB_POLE	},
	{	SIZE_RTDSB,	SIZE_RTDSB,	LINEDEB_POLEC	},		//201207	LINEDEB_POLE	},
	{	SIZE_PAUSR,	SIZE_PAUSA,	LINEDEB_POLEC	},
	{	SIZE_PISSR,	SIZE_PISSA,	LINEDEB_POLEC	},
	{	SIZE_TRSSR,	SIZE_TRSSA,	LINEDEB_POLEC	},
	{	SIZE_FDUSR,	SIZE_FDUSA,	LINEDEB_POLEC	},
	{	SIZE_BMSSR,	SIZE_BMSSA,	LINEDEB_POLEC	},
	{	0,			0,			0	}
};

CVerse::CVerse()
{
	c_wHduRecvLength = 0;
	c_cPermNo = 0;
	c_iServicedID = 0xff;
	// 200218
	c_seqCnts.wFdu = c_seqCnts.wBms = 0;
	c_cPoleRedID = 0;

	memset(&c_hdu, 0, sizeof(HDUFLAP) * LI_MAX);
	memset(&c_rtd, 0, sizeof(RTDFLAP) * LI_MAX);
	memset(&c_pau, 0, sizeof(PAUFLAP) * LI_MAX);
	memset(&c_pis, 0, sizeof(PISFLAP) * LI_MAX);
	memset(&c_trs, 0, sizeof(TRSFLAP) * LI_MAX);
	memset(&c_fdu, 0, sizeof(FDUFLAP) * LI_MAX);
	memset(&c_bms, 0, sizeof(BMSFLAP) * LI_MAX);
	c_wCurOtrn = DEFAULT_OTRN;
	// 200218
	memset(&c_ritem, 0, sizeof(REDITEM));
	Initial();
}

CVerse::~CVerse()
{
}

void CVerse::Initial()
{
	c_cAtoSeqNo = c_cAtoChgNo = 0;
	c_cLineState = c_cLineBlocks = 0;
	memset(&c_pisInfo, 0, sizeof(PISINFO));
	// PIS CONTROL CODE #1
	c_pisInfo.cNorCtrlCode = c_pisInfo.cCurCtrlCode = ((SVCCODEB_TAIL << 4) | SVCCODEB_HEAD);
	memset(&c_wDebs, 0, sizeof(WORD) * PDEVID_MAX);
	memset(&c_wBlocks, 0, sizeof(WORD) * PDEVID_MAX);
	memset(&c_ato, 0, sizeof(ATOFLAP) * LI_MAX);		// because update cycle count
	// 200218
	//memset(&c_wRed, 0, sizeof(WORD) * PDEVID_MAX);
	memset(&c_sheaf, 0, sizeof(_SHEAF));
	memset(&c_psv, 0, sizeof(PSHARE));

	c_psv.atocf.wWarmUpTimer[0] = TIME_ATOWARMUP;
	c_psv.atocf.wWarmUpTimer[1] = TIME_ATOWARMUP;
	// 210218
	//c_psv.rtdcf.wWarmUpTimer = TIME_RTDWARMUP;
	// 210803
	//c_psv.rtdcf.wWarmUpTimer = TIME_RTDWARMUPA;
	//c_psv.paucf.wWarmUpTimer = TIME_PAUWARMUP;
	//c_psv.piscf.wWarmUpTimer = TIME_PISWARMUP;
	//c_psv.trscf.wWarmUpTimer = TIME_TRSWARMUP;
	//c_psv.rtdcf.wWarmUpTimer = 0xffff;  //211112 delete
#if 1	//---hroy 20211223
	GETPAPER(pPaper);
	c_psv.rtdcf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
							TIME_RTDWARMUPC : TIME_RTDWARMUPA;
	c_psv.paucf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
							TIME_PAUWARMUPB : TIME_PAUWARMUPA;
	c_psv.piscf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
							TIME_PISWARMUPB : TIME_PISWARMUPA;
	c_psv.trscf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
							TIME_TRSWARMUPB : TIME_TRSWARMUPA;
#else
	c_psv.rtdcf.wWarmUpTimer = TIME_RTDWARMUPA;  //211112
	c_psv.paucf.wWarmUpTimer = 0xffff;
	c_psv.piscf.wWarmUpTimer = 0xffff;
	c_psv.trscf.wWarmUpTimer = 0xffff;
#endif
	c_psv.fducf.wWarmUpTimer = TIME_FDUWARMUP;
	c_psv.bmscf.wWarmUpTimer = TIME_BMSWARMUP;

	memset(&c_rtdRef, 0, sizeof(RTDREF));
	memset(&c_cDump[0], 0, 512);
}

// 180511
void CVerse::InitAtoWarmup(int si)
{
	c_psv.atocf.wWarmUpTimer[si != 0 ? 1 : 0] = TIME_ATOWARMUP;
}

int CVerse::GetValidPisSide()
{
	if (c_psv.piscf.fl[0].r.t.s.ais.cCode == SVCCODEH_HEAD || c_psv.piscf.fl[0].r.t.s.ais.cCode == SVCCODEH_XCHANGE)	return 0;
	return 1;
}

PVOID CVerse::GetInfo(bool bDir, int pdi, int li)
{
	if (li < LI_MAX) {
		switch (pdi) {
		case PDEVID_ATO :	return bDir ? (PVOID)&c_ato[li].a : (PVOID)&c_ato[li].r;	break;
		case PDEVID_HDU :	return bDir ? (PVOID)&c_hdu[li].a : (PVOID)&c_hdu[li].r;	break;
		case PDEVID_RTD :	return bDir ? (PVOID)&c_rtd[li].a : (PVOID)&c_rtd[li].r;	break;
		case PDEVID_PAU :	return bDir ? (PVOID)&c_pau[li].a : (PVOID)&c_pau[li].r;	break;
		case PDEVID_PIS :	return bDir ? (PVOID)&c_pis[li].a : (PVOID)&c_pis[li].r;	break;
		case PDEVID_TRS :	return bDir ? (PVOID)&c_trs[li].a : (PVOID)&c_trs[li].r;	break;
		case PDEVID_FDU :	return bDir ? (PVOID)&c_fdu[li].a : (PVOID)&c_fdu[li].r;	break;
		case PDEVID_BMS :	return bDir ? (PVOID)&c_bms[li].a : (PVOID)&c_bms[li].r;	break;
		default :	break;
		}
	}
	TRACK("VERSE>ERR:impossible reference!(pdi=%d,vi=%d)\n", pdi, li);
	return NULL;
}

WORD CVerse::GetLength(bool bDir, int pdi)
{
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip())
		return c_wFlapLength2[pdi][bDir ? 1 : 0];
	return c_wFlapLength[pdi][bDir ? 1 : 0];
}

WORD CVerse::GetWarmup(int pdi, int si)
{
	switch (pdi) {
	case PDEVID_ATO :	return c_psv.atocf.wWarmUpTimer[si != 0 ? 1 : 0];	break;
	case PDEVID_RTD :	return c_psv.rtdcf.wWarmUpTimer;	break;
	case PDEVID_PAU :	return c_psv.paucf.wWarmUpTimer;	break;
	case PDEVID_PIS :	return c_psv.piscf.wWarmUpTimer;	break;
	case PDEVID_TRS :	return c_psv.trscf.wWarmUpTimer;	break;
	case PDEVID_FDU :	return c_psv.fducf.wWarmUpTimer;	break;
	case PDEVID_BMS :	return c_psv.bmscf.wWarmUpTimer;	break;
	default :	break;
	}
	return 0;
}

void CVerse::ClearAtoFlap()
{
	PVOID pCurr = GetInfo(false, PDEVID_ATO, LI_CURR);
	memset(pCurr, 0, c_wFlapLength[PDEVID_ATO][LL_REQ]);
	pCurr = GetInfo(false, PDEVID_ATO, LI_WORK);
	memset(pCurr, 0, c_wFlapLength[PDEVID_ATO][LL_REQ]);
	pCurr = GetInfo(true, PDEVID_ATO, LI_CURR);
	memset(pCurr, 0, c_wFlapLength[PDEVID_ATO][LL_ANS]);
	pCurr = GetInfo(true, PDEVID_ATO, LI_WORK);
	memset(pCurr, 0, c_wFlapLength[PDEVID_ATO][LL_ANS]);
}

void CVerse::Backup(int pdi)
{
	// ATO는 seqcnt를 위해 송신 프레임을 백업한다. (백업 내용과 다르면 ++ chgcnt)
	PVOID pCurr = GetInfo(false, pdi, LI_CURR);
	PVOID pBkup = GetInfo(false, pdi, LI_BKUP);
	memcpy(pBkup, pCurr, c_wFlapLength[pdi][LL_REQ]);
	if (pdi != PDEVID_ATO)	memset(pCurr, 0, c_wFlapLength[pdi][LL_REQ]);	// ATO는 decorate를 하고 백업을 하므로 지울 수는 없다.
	// 응답 디바운스를 위해 송신 전에 응답 프레임을 백업한다
	pCurr = GetInfo(true, pdi, LI_CURR);
	pBkup = GetInfo(true, pdi, LI_BKUP);
	memcpy(pBkup, pCurr, c_wFlapLength[pdi][LL_ANS]);
	memset(pCurr, 0, c_wFlapLength[pdi][LL_ANS]);
}

void CVerse::Closure(int pdi)
{
	c_mtx.Lock();
	PVOID pCurr = GetInfo(false, pdi, LI_CURR);
	PVOID pWork = GetInfo(false, pdi, LI_WORK);
	memcpy(pWork, pCurr, c_wFlapLength[pdi][LL_REQ]);
	pCurr = GetInfo(true, pdi, LI_CURR);
	pWork = GetInfo(true, pdi, LI_WORK);
	memcpy(pWork, pCurr, c_wFlapLength[pdi][LL_ANS]);
	//if (pdi >= PDEVID_PAU && pdi <= PDEVID_TRS)	c_iServicedID = pdi;		// 따라올 reform에서 참조
	c_mtx.Unlock();
}

bool CVerse::RtdClosure(int iType)
{
	c_rtdRef.cResult = (BYTE)iType;
	GETARCHIVE(pArch);
	switch (iType) {
	case RCLSTYPE_SUCCESS : {
			Closure(PDEVID_RTD);		// curr -> work
			c_rtdRef.bSdrHold = false;
			c_cLineState |= (1 << PDEVID_RTD);
			c_wDebs[PDEVID_RTD] = 0;
			PRTDBINFO pRtdr = (PRTDBINFO)GetInfo(false, PDEVID_RTD, LI_WORK);
			PRTDBINFO pRtda = (PRTDBINFO)GetInfo(true, PDEVID_RTD, LI_WORK);
			if (pRtda->t.s.pack.cItem == RTDITEM_CLOSECURRENT) {
				if (pArch->GetRtdNewSheetState())	pArch->SetRtdNewSheetState(false);
			}
			else if (pRtdr->t.s.pack.cItem == RTDITEM_DESKREPORT)
				memcpy(&c_rtdRef.state, &pRtda->t.s.pack.pad.tp.tin.c[0], sizeof(RTDSTATE));
		}
		break;
	default : {		//	RCLSTYPE_FAIL,
			GETPAPER(pPaper);
			WORD wDeb = pPaper->GetDeviceExFromRecip() ? LINEDEB_POLEC : LINEDEB_POLEA;
			if (++ c_wDebs[PDEVID_RTD] >= wDeb) {
				c_rtdRef.bSdrHold = false;
				c_wDebs[PDEVID_RTD] = 0;
				#if 0
				c_cLineState &= ~(1 << PDEVID_RTD);
				#else
				if (GetWarmup(PDEVID_RTD) == 0) {  //20220125
					c_mtxRed.Lock();
					INCWORD(c_ritem.wRed[PDEVID_RTD]);
					c_mtxRed.Unlock();
					c_cLineState &= ~(1 << PDEVID_RTD);
				}
				#endif
				if (!pArch->GetRtdNewSheetState()) {
					pArch->SetRtdNewSheetState(true);		// 통신이 끊어졌다는 것은 RTD가 OFF 되었을 수도 있다는 것이므로 head를 재 전송하여야 한다.
					TRACK("VERS>ERR:rtd line!\n");			// 171206
				}
			}
			else {
				c_rtdRef.bSdrHold = true;
				//TRACK("VERS>ERR:rtd line off!\n");	// 180118		// 171206
			}
		}
		break;
	}
	PRTDBINFO pRtdac = (PRTDBINFO)GetInfo(true, PDEVID_RTD, LI_CURR);
	//GETPAPER(pPaper);
	//pPaper->Shoot((PVOID)pRtdac, sizeof(RTDBINFO));
	c_rtdRef.wAddr = pRtdac->wAddr;

	return true;
}

void CVerse::Success(int pdi)
{
	if (c_wDebs[pdi] > c_wFlapLength[pdi][LL_DEB] && -- c_wDebs[pdi] > c_wFlapLength[pdi][LL_DEB]) {
		PVOID pCurr = GetInfo(true, pdi, LI_CURR);
		*((BYTE*)pCurr + 2) |= 0x70;
		c_cLineState &= ~(1 << pdi);
	}
	else {
		c_cLineState |= (1 << pdi);
		//c_cLineBlocks &= ~(1 << pdi);
		c_wDebs[pdi] = 0;
		c_wBlocks[pdi] = 0;
	}

	Closure(pdi);
}

bool CVerse::Repair(int pdi, int recf)
{
	PVOID pCurr = GetInfo(true, pdi, LI_CURR);
	PVOID pBkup = GetInfo(true, pdi, LI_BKUP);
	bool bRes = true;
	if (++ c_wDebs[pdi] >= c_wFlapLength[pdi][LL_DEB]) {
		c_wDebs[pdi] = c_wFlapLength[pdi][LL_DEB];
		if (pdi == PDEVID_ATO)	*((BYTE*)pCurr + 5) = (BYTE)((0 - recf) & 0xff);
		else	*((BYTE*)pCurr + 2) = (BYTE)((0 - recf) & 0xff);	// cCtrl
		if (GetWarmup(pdi) == 0) {
			c_mtxRed.Lock();
			INCWORD(c_ritem.wRed[pdi]);
			c_mtxRed.Unlock();
			c_cLineState &= ~(1 << pdi);
			bRes = false;
			// 200218
			//if (pdi != PDEVID_ATO) {
			if (pdi > PDEVID_ATO && pdi < PDEVID_TRS) {
				if (++ c_wBlocks[pdi] >= BLOCK_CYCLES) {
					c_wBlocks[pdi] = BLOCK_CYCLES;
					// PIS CONTROL CODE #2
					c_cLineBlocks |= (1 << pdi);
				}
			}
			//else	c_cLineBlocks &= ~(1 << pdi);	 // 한번 블럭되면 다시 깨우지는 않는다. 운전실 전환이나 전원 재 투입 때 지워진다.
		}
	}
	else {
		PVOID pSdr = GetInfo(false, pdi, LI_CURR);
		if (*((BYTE*)pSdr + 3) != DEVTRACE_REQ)	memcpy(pCurr, pBkup, c_wFlapLength[pdi][LL_ANS]);
		c_cLineState |= (1 << pdi);
		//c_cLineBlocks &= ~(1 << pdi);
		bRes = true;
	}
	Closure(pdi);

	return bRes;
}

void CVerse::Reform(PRECIP pRecip)
{
	// RTD는 하지않는다. 크다..
	c_mtx.Lock();
	// 180704
	// pRecip->real.sign.pcmd.b.dow = c_pisInfo.ref.bDow;
	int vi = GetValidPisSide();
	pRecip->real.sign.pcmd.b.dow = c_psv.piscf.fl[vi].a.t.s.ref.b.dow;

	memcpy(&pRecip->real.ato, &c_ato[LI_WORK], sizeof(ATOFLAP));
	//GETPAPER(pPaper);
	//if (!(pPaper->GetOblige() & (1 << OBLIGE_HEAD)))
	// 180123
	//if (!pPaper->IsHead())	pRecip->real.ato.r.t.s.cReserve2[0] = ATO_BLINDCODE;
	switch (c_iServicedID) {
	case PDEVID_PAU :
		memcpy(&pRecip->real.svc.u, &c_pau[LI_WORK], sizeof(PAUFLAP));
		break;
	case PDEVID_PIS :
		memcpy(&pRecip->real.svc.i, &c_pis[LI_WORK], sizeof(PISFLAP));
		break;
	case PDEVID_TRS :
		memcpy(&pRecip->real.svc.t, &c_trs[LI_WORK], sizeof(TRSFLAP));
		break;
	// 200218
	case PDEVID_FDU : {
			memcpy(&pRecip->real.svc.f.r, &c_fdu[LI_WORK].r, sizeof(FDURINFO));
			memcpy(&pRecip->real.svc.f.a, &c_fdu[LI_WORK].a, 4);		// wAddr-cCtrl-cFlow
			pRecip->real.svc.f.a.t.s.det.a = c_fdu[LI_WORK].a.t.s.det.a;
			pRecip->real.svc.f.a.t.s.uf.a = c_fdu[LI_WORK].a.t.s.uf.a;
			pRecip->real.svc.f.a.t.s.selft.a = c_fdu[LI_WORK].a.t.s.selft.a;
			pRecip->real.svc.f.a.t.s.stat.a = c_fdu[LI_WORK].a.t.s.stat.a;
			memcpy(&pRecip->real.svc.f.a.t.s.susage, &c_fdu[LI_WORK].a.t.s.susage, 4);
			memcpy(&pRecip->real.svc.f.a.t.s.sstate, &c_fdu[LI_WORK].a.t.s.sstate, 8);
			memcpy(&pRecip->real.svc.f.a.t.s.sclean, &c_fdu[LI_WORK].a.t.s.sclean, 4);
			memcpy(&pRecip->real.svc.f.a.t.s.curs, &c_fdu[LI_WORK].a.t.s.curs, 6);	// untill wVer
			GETPAPER(pPaper);
			pRecip->real.svc.f.a.t.s.uf.b.cid = (pPaper->GetCarID() & 0xf);
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			pRecip->real.svc.f.a.t.s.wRecover = pLsv->fire.wRecoverMap;
			// FD STEP 5: reform driver action to dtb
			pRecip->real.sign.scmdEx.b.fireAck = pLsv->fire.bAck;		// must occur at head
			// FD STEP 9: reform fire sequence close cycle
			pRecip->real.sign.scmdEx.b.fireMbc = pLsv->fire.bMbCancel;
		}
		break;
	case PDEVID_BMS :
		memcpy(&pRecip->real.svc.m, &c_bms[LI_WORK], sizeof(BMSFLAPAB));
		break;
	default :	break;
	}
	pRecip->real.rtd.stat.a = c_rtdRef.state.stat.a;
	pRecip->real.rtd.serv.a = c_rtdRef.state.serv.a;
	GETARCHIVE(pArch);
	pArch->Devt2Dbft(&pRecip->real.rtd.dbft, c_rtdRef.state.devt);

	pRecip->real.cPoleLine = c_cLineState;
	pRecip->real.cPoleBlock = c_cLineBlocks;

	pRecip->real.cSvcCtrlCode = c_pisInfo.cCurCtrlCode;

	// 200218
	//pRecip->real.wPoleRed[PDEVRED_ATO] = c_wRed[PDEVID_ATO];
	//pRecip->real.wPoleRed[PDEVRED_RTD] = c_wRed[PDEVID_RTD];
	//pRecip->real.wPoleRed[PDEVRED_PAU] = c_wRed[PDEVID_PAU];
	//pRecip->real.wPoleRed[PDEVRED_PIS] = c_wRed[PDEVID_PIS];
	//pRecip->real.wPoleRed[PDEVRED_TRS] = c_wRed[PDEVID_TRS];
	pRecip->real.polef.cDevID = c_cPoleRedID;
	pRecip->real.polef.wCnt = c_ritem.wRed[c_cPoleRedID];
	if (++ c_cPoleRedID >= PDEVID_MAX)	c_cPoleRedID = 0;

	// 171129
	// PIS CONTROL CODE #7 :
	pRecip->real.motiv.lead.b.pisaim = c_pisInfo.bAim;
	pRecip->real.motiv.lead.b.pisuse = c_pisInfo.bUser;

	c_mtx.Unlock();
}

PVOID CVerse::Decorate(int pdi)
{
	if (pdi != PDEVID_ATO)	Backup(pdi);

	GETPAPER(pPaper);
	PVOID pSdr = GetInfo(false, pdi, LI_CURR);

	switch (pdi) {
	case PDEVID_ATO : {
			PATORINFO pAtor = (PATORINFO)pSdr;
			DecorAto(pAtor);
			Backup(PDEVID_ATO);
		}
		break;
	case PDEVID_PAU : {
			PPAURINFO pPaur = (PPAURINFO)pSdr;
			pPaur->wAddr = LADD_PAU;
			pPaur->cCtrl = DEVCHAR_CTL;
			DecorAis(&pPaur->t.s.ais);
		}
		break;
	case PDEVID_PIS : {
			PPISRINFO pPisr = (PPISRINFO)pSdr;
			pPisr->wAddr = LADD_PIS;
			pPisr->cCtrl = DEVCHAR_CTL;
			DecorAis(&pPisr->t.s.ais);

			PDOZEN pDoz = pPaper->GetDozen();
			GETSCHEME(pSch);
			for (int n = 0; n < pPaper->GetLength(); n ++) {
				if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin && pPaper->GetRouteState(n)) {		// 180911, 통신이 안되면 값은 0이 나온다. 확인
					BYTE cf = pSch->C2F(pPaper->GetTenor(), n);
					// 200218
					//WORD w = pSch->GetProportion((WORD)pDoz->recip[cf].real.ecu.a.t.s.cLw, (WORD)0, (WORD)255, (WORD)200, (WORD)750);	// 0~0xff -> 20.0~75.0ton
					WORD w = 0;
					if (pPaper->GetDeviceExFromRecip())
						w = (WORD)((double)pDoz->recip[cf].real.ecu.ct.a.t.s.cLw * 3.f + 200.f);
					else	w = pSch->GetProportion((WORD)pDoz->recip[cf].real.ecu.co.a.t.s.cLw, (WORD)0, (WORD)255, (WORD)200, (WORD)750);	// 0~0xff -> 20.0~75.0ton
					pPisr->t.s.cLw[n] = (BYTE)pSch->CalcLoad(n, w);
				}
				else	pPisr->t.s.cLw[n] = 0;
			}
		}
		break;
	case PDEVID_TRS : {
			PTRSRINFO pTrsr = (PTRSRINFO)pSdr;
			pTrsr->wAddr = LADD_TRS;
			pTrsr->cCtrl = DEVCHAR_CTL;
			pTrsr->t.s.cFlow = DEVFLOW_REQ;
			BYTE oblige = pPaper->GetOblige();
			//if ((c_psv.trscf.wDefectMap & (1 << FID_HEAD)) && (c_psv.trscf.cLines[id] & (1 << DEVSTATE_BLINDUNIT)))
			// 17/06/14, ignore backup
			//if ((c_psv.trscf.wDefectMap & (1 << FID_HEAD)) || (pDoz->recip[FID_HEAD].real.cPoleBlock & (1 << PDEVID_TRS)))
			//	pTrsr->t.s.cCode = (oblige & (1 << OBLIGE_HEAD)) ? SVCCODE_NEUTRAL : SVCCODE_CHANGE;
			//else
			pTrsr->t.s.cCode = (oblige & (1 << OBLIGE_HEAD)) ? SVCCODEH_HEAD : SVCCODEH_TAIL;
			// 171019
			//pTrsr->t.s.no.a = pDoz->recip[FID_HEAD].real.ato.a.t.s.notr.a;
			// 180704
			//if (c_pisInfo.mode.bManual)	pTrsr->t.s.no.a = XWORD(c_pisInfo.ref.wOtrn);
			//else	pTrsr->t.s.no.a = XWORD(c_wCurOtrn);
			// PIS CONTROL CODE #A
			// 180928
			//int vi = GetValidPisSide();
			//if (c_psv.piscf.fl[vi].a.t.s.ref.b.mma)	pTrsr->t.s.no.a = c_psv.piscf.fl[vi].a.t.s.notr.a;
			//else	pTrsr->t.s.no.a = pDoz->recip[FID_HEAD].real.ato.a.t.s.notr.a;
			pTrsr->t.s.no.a = XWORD(c_wCurOtrn);
		}
		break;
	case PDEVID_RTD : {
			PRTDBINFO pRtd = (PRTDBINFO)GetInfo(true, PDEVID_RTD, LI_CURR);	// sda clear
			memset(pRtd, 0, SIZE_RTDSB);
			if (!c_rtdRef.bSdrHold) {
				pRtd = (PRTDBINFO)pSdr;
				GETARCHIVE(pArch);
				pArch->ClosureBridge(pRtd);
			}
			else {
				PVOID pb = GetInfo(false, PDEVID_RTD, LI_BKUP);
				memcpy(pSdr, pb, c_wFlapLength[PDEVID_RTD][LL_REQ]);
			}
		}
		break;
	case PDEVID_FDU : {
			PFDURINFO pFdur = (PFDURINFO)pSdr;
			pFdur->wAddr = LADD_FDU;
			pFdur->cCtrl = DEVCHAR_CTL;
			pFdur->t.s.cFlow = DEVFLOW_REQ;

			PDOZEN pDoz = pPaper->GetDozen();
			GETARCHIVE(pArch);
			pArch->Dbft2BcdDevt(&pFdur->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
			pFdur->t.s.wSeq = c_seqCnts.wFdu ++;
			pFdur->t.s.no.a = XWORD(c_wCurOtrn);

			int vi = GetValidPisSide();
			if (c_psv.piscf.fl[vi].a.t.s.ref.b.mma) {
				pFdur->t.s.cCurrSt = c_psv.piscf.fl[vi].a.t.s.cCurrSt;
				pFdur->t.s.cNextSt = c_psv.piscf.fl[vi].a.t.s.cNextSt;
				pFdur->t.s.cDestSt = c_psv.piscf.fl[vi].a.t.s.cDestSt;
			}
			else {
				pFdur->t.s.cNextSt = pDoz->recip[FID_HEAD].real.ato.a.t.s.cNextSt;
				pFdur->t.s.cDestSt = pDoz->recip[FID_HEAD].real.ato.a.t.s.cDestSt;
			}
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			// !!!FIRE DETECTION CAUTION!!! check only own fdu
			bool drr = false;
			int vfi = pPaper->GetCarID() == 0 ? 0 : 1;
			if (pLsv->fire.wReleaseTimer != 0)	drr = true;
			else {
				if (pLsv->fire.wRecoverMap != 0 && c_psv.fducf.fl[vfi].a.t.s.stat.b.frr) {
					// FD STEP 8: fdu recover
					drr = true;
					pLsv->fire.wRecoverMap = 0;
					pProse->TriggerFireDetectionRelease();
					TRACK("FDS8: recovery at %d\n", vfi);
				}
			}
			pFdur->t.s.req.b.drr = drr;
			pFdur->t.s.fd.a = (BYTE)(pLsv->fire.wDefectMap & 0xff);

			pFdur->t.s.req.b.rpb =
					(!(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_PARKING)) &&
					c_psv.fducf.fl[vfi].a.t.s.stat.b.pms) ? true : false;
		}
		break;
	case PDEVID_BMS : {
			PBMSRINFO pBmsr = (PBMSRINFO)pSdr;
			pBmsr->wAddr = LADD_BMS;
			pBmsr->cCtrl = DEVCHAR_CTL;
			pBmsr->t.s.cFlow = DEVFLOW_REQ;

			PDOZEN pDoz = pPaper->GetDozen();
			GETARCHIVE(pArch);
			pArch->Dbft2BcdDevt(&pBmsr->t.s.devt, pDoz->recip[FID_HEAD].real.prol.dbft);
			pBmsr->t.s.wSeq = c_seqCnts.wBms ++;
			pBmsr->t.s.no.a = XWORD(c_wCurOtrn);
		}
		break;
	default :	break;
	}
	return pSdr;
}

void CVerse::DecorAto(PATORINFO pAtor)
{
	pAtor->t.s.valid.a = 0;
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	if (pLsv->ecucf.wSlideBitmap != 0)	pAtor->t.s.valid.b.slip = true;
	pAtor->t.s.proper.b.u10 = c_cPermNo;
	GETPAPER(pPaper);
	pAtor->t.s.proper.b.u32 = 0x20 | CAR_PSELF();
	// 17/06/21, modify
	WORD wLwSum = 0;
	for (int n = 0; n < (int)pPaper->GetLength(); n ++)
		wLwSum += (WORD)pLsv->cLw[n];
	if (wLwSum > ATO_WEIGHTOFFSET)	wLwSum -= ATO_WEIGHTOFFSET;
	else	wLwSum = 0;
	wLwSum /= 2;
	if (wLwSum > 255)	wLwSum = 255;
	pAtor->t.s.cWeight = (BYTE)(wLwSum & 0xff);
	pAtor->t.s.valid.b.weight = c_psv.vwbd.bStb;
	pAtor->t.s.fdev.a = 0;
	if (pLsv->sivcf.wDefectMap != 0)	pAtor->t.s.fdev.b.siv = true;
	if (pLsv->v3fcf.wDefectMap != 0)	pAtor->t.s.fdev.b.v3f = true;
	if (pLsv->ecucf.wDefectMap != 0)	pAtor->t.s.fdev.b.ecu = true;
	if (c_psv.trscf.wDefectMap != 0)	pAtor->t.s.fdev.b.trs = true;
	//pAtor->t.s.fdev.b.roml;
	// 170920
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.prol.cProperID & 1)	pAtor->t.s.fdev.b.bkup = true;
	if (pLsv->wLoadrSum > 2000)	pAtor->t.s.fdev.b.psov = true;							// 200% * 10
	GETTOOL(pTool);
	WORD wFailedV3fs = pTool->GetBitNumbers(pLsv->v3fcf.wDefectMap, true);
	WORD wFailedEcus = pTool->GetBitNumbers(pLsv->ecucf.wDefectMap, true);
	if (wFailedEcus >= 2 || wFailedV3fs >= 2 || pLsv->wFailedCCs >= 2 || pLsv->wNrbds > 0 ||
		(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) || pDoz->recip[FID_HEAD].real.drift.cEbrk != 0)
		pAtor->t.s.fdev.b.inhibit = true;
	//if (!GETTI(FID_HEAD, TUDIB_MRPS))	pAtor->t.s.fdev.b.mrps = true;
	//bool bDcuf = false;
	//for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
	//	if (pLsv->dcucf.wDefectMap[n] != 0) {
	//		bDcuf = true;
	//		break;
	//	}
	//}
	//pAtor->t.s.fdev.b.dcu = bDcuf;
	//if (pDoz->recip[FID_HEAD].real.motiv.cNotch != NOTCHID_C && !GETTI(FID_HEAD, TUDIB_DEADMAN))
	//	pAtor->t.s.fdev.b.dman = true;
	pAtor->t.s.cReserve1 = 0;
	pAtor->t.s.ts.b.leng = pPaper->GetLength();
	pAtor->t.s.cReserve2[0] = pAtor->t.s.cReserve2[1] = 0;

	PATORINFO pb = (PATORINFO)GetInfo(false, PDEVID_ATO, LI_BKUP);
	// 210720
	//if (memcmp(&pAtor->t.s.valid.a, &pb->t.s.valid.a, sizeof(SRATO) - 2))	++ pAtor->t.s.cChgCnt;
	if (memcmp(&pAtor->t.s.valid.a, &pb->t.s.valid.a, sizeof(SRATO) - 2))	pAtor->t.s.cChgCnt = ++ c_cAtoChgNo;
	//++ pAtor->t.s.cSeqCnt;
	pAtor->t.s.cSeqCnt = ++ c_cAtoSeqNo;
}

void CVerse::DecorAis(PSRAIS pAisr)
{
	pAisr->cFlow = DEVFLOW_REQ;
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pPaper->IsHead())	pAisr->cCode = (pDoz->recip[FID_HEAD].real.cSvcCtrlCode & 0xf) + 0x31;
	else	pAisr->cCode = ((pDoz->recip[FID_HEAD].real.cSvcCtrlCode >> 4) & 0xf) + 0x31;

	// 180704
	// PIS CONTROL CODE #8
	if (pDoz->recip[FID_HEAD].real.motiv.lead.b.pisaim) {
		bool pos = (CAR_FSELF() == FID_HEAD) ? false : true;
		if (pDoz->recip[FID_HEAD].real.motiv.lead.b.pisuse == pos)	pAisr->hd.b.mmr = true;
	}
	// PIS CONTROL CODE #A
	int vi = GetValidPisSide();
	if (c_psv.piscf.fl[vi].a.t.s.ref.b.mma) {
		pAisr->cCurrSt = c_psv.piscf.fl[vi].a.t.s.cCurrSt;
		pAisr->cNextSt = c_psv.piscf.fl[vi].a.t.s.cNextSt;
		pAisr->cDestSt = c_psv.piscf.fl[vi].a.t.s.cDestSt;
		if (GETTI(FID_HEAD, TUDIB_DCS))	pAisr->ref.b.dcw = true;
	}
	else {
		// 171115
		pAisr->cNextSt = pDoz->recip[FID_HEAD].real.ato.a.t.s.cNextSt;
		pAisr->cDestSt = pDoz->recip[FID_HEAD].real.ato.a.t.s.cDestSt;
		//pPaur->t.s.ref.b.ddns
		if (pDoz->recip[FID_HEAD].real.ato.a.t.s.valid.b.ndo) {
			pAisr->ref.b.nrd = pDoz->recip[FID_HEAD].real.ato.a.t.s.door.b.rn;
			pAisr->ref.b.nld = pDoz->recip[FID_HEAD].real.ato.a.t.s.door.b.ln;
		}
		if (pDoz->recip[FID_HEAD].real.ato.a.t.s.valid.b.dcw)
			pAisr->ref.b.dcw = pDoz->recip[FID_HEAD].real.ato.a.t.s.door.b.dcw;
		// mma후에 mmr을 지우고 싶으면 LINE A부분을 여기로 둔다.
	}
	// 171115
	if (c_wCurOtrn & 1)	pAisr->ref.b.dnf = true;			// 외선
	else	pAisr->ref.b.upf = true;						// 내선
	pAisr->no.a = XWORD(c_wCurOtrn);

	if (c_psv.trscf.cLines[0] & (1 << DEVSTATE_BLINDUNIT))	pAisr->hd.b.trs0 = true;
	if (c_psv.trscf.cLines[1] & (1 << DEVSTATE_BLINDUNIT))	pAisr->hd.b.trs9 = true;
	// 210630
	if (pPaper->GetDeviceExFromRecip()) {
		if (pDoz->recip[FID_HEAD].real.sign.scmdEx.b.fireMbc ||
			(pPaper->GetRouteState(FID_TAIL) &&
			pDoz->recip[FID_TAIL].real.sign.scmdEx.b.fireMbc))
			pAisr->hd.b.mbc = true;
		else	pAisr->hd.b.mbc = false;
	}

	if (GETTI(CAR_FSELF(), TUDIB_HCR))	pAisr->hd.b.hcr = true;
	if (GETTI(CAR_FSELF(), TUDIB_TCR))	pAisr->hd.b.tcr = true;
	pAisr->hd.b.fire = pDoz->recip[FID_HEAD].real.sign.ecmd.b.fire;
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	pAisr->fd.a = pLsv->fire.wDefectMap;
	pAisr->pa.a = pLsv->wPaMap;

	GETARCHIVE(pArch);
	WORD wDist = (WORD)pArch->GetSegment();
	pAisr->wDist = XWORD(wDist);
	// 170801
	//pArch->GetBcdDevTime(&pPaur->t.s.dt);
	pArch->Dbft2BcdDevt(&pAisr->devt, pDoz->recip[FID_HEAD].real.prol.dbft);
}

void CVerse::ReleaseServiceBlock()
{
	c_wDebs[PDEVID_PAU] = c_wDebs[PDEVID_PIS] = c_wBlocks[PDEVID_PAU] = c_wBlocks[PDEVID_PIS] = 0;
	c_cLineBlocks &= ~((1 << PDEVID_PAU) | (1 << PDEVID_PIS));
	//c_pisRef.cCtrlCode = c_pisRef.cRefCode;
	//c_pisRef.cCtrlCode = ((SVCCODEB_TAIL << 4) | SVCCODEB_HEAD);
	GETPAPER(pPaper);
	int tc1 = (int)pPaper->GetLength() - 1;
	GETARCHIVE(pArch);
	pArch->Cut(0, 721);
	// 210629
	//pArch->Cut(9, 721);
	pArch->Cut(tc1, 721);
	pArch->Cut(0, 838);
	//pArch->Cut(9, 838);
	pArch->Cut(tc1, 838);
}

// 171130
void CVerse::SetServiceControlCode()
{
	GETPAPER(pPaper);
	if (!pPaper->GetRouteState(FID_TAIL)) {		// 180911, 통신이 안되는 조건이니, 확인
		c_pisInfo.cNorCtrlCode = c_pisInfo.cCurCtrlCode = ((SVCCODEB_TAIL << 4) | SVCCODEB_HEAD);			// tail:0x32, head:0x31
		return;
	}
	PDOZEN pDoz = pPaper->GetDozen();
	//GETARCHIVE(pArch);

	// PIS CONTROL CODE #A
	if (c_psv.piscf.fl[0].a.t.s.ref.b.mma || c_psv.piscf.fl[1].a.t.s.ref.b.mma) {
		int hi = pPaper->GetTenor() ? 1 : 0;
		if ((c_psv.piscf.fl[hi].a.t.s.ref.b.mma && (c_pisInfo.cCurCtrlCode & 0xf) == SVCCODEB_TAIL) ||
			(c_psv.piscf.fl[hi ? 0 : 1].a.t.s.ref.b.mma && ((c_pisInfo.cCurCtrlCode & 0xf0) >> 4) == SVCCODEB_TAIL)) {
			TurnPisCode();
			c_pisInfo.cCurCtrlCode = c_pisInfo.cNorCtrlCode;
			if (pPaper->GetLabFromRecip())	TRACK("pis mma turn code.\r\n");
		}
	}
	else {
		if (pDoz->recip[FID_HEAD].real.cPoleBlock & ((1 << PDEVID_PAU) | (1 << PDEVID_PIS)))		// head fault
			c_pisInfo.cCurCtrlCode = ((SVCCODEB_XCHANGE << 4) | SVCCODEB_NEUTRAL);			// tail:0x34, head:0x33
		else if (pDoz->recip[FID_TAIL].real.cPoleBlock & ((1 << PDEVID_PAU) | (1 << PDEVID_PIS)))	// tail fault
			c_pisInfo.cCurCtrlCode = ((SVCCODEB_NEUTRAL << 4) | SVCCODEB_XCHANGE);			// tail:0x33, head:0x34
		// 180704
		//else	c_pisInfo.mode.cCurCtrlCode = ((SVCCODEB_TAIL << 4) | SVCCODEB_HEAD);				// tail:0x32, head:0x31
		else	c_pisInfo.cCurCtrlCode = c_pisInfo.cNorCtrlCode;
	}
}

void CVerse::TurnPisCode()
{
	c_pisInfo.cNorCtrlCode = ((c_pisInfo.cNorCtrlCode & 0xf0) >> 4) | ((c_pisInfo.cNorCtrlCode & 0xf) << 4);
}

void CVerse::SetPisMode(bool bUser, bool bAim)
{
	// 요구가 있을 때만 실행된다.
	ReleaseServiceBlock();
	c_pisInfo.bAim = bAim;		// 선두나 후미에서 최근에 조작한 방식을 저장한다.
	c_pisInfo.bUser = bUser;
}

// 180917
void CVerse::SetCurOtrn(WORD wOtrn)
{
	if (wOtrn != 0)	c_wCurOtrn = wOtrn;
	else	c_wCurOtrn = DEFAULT_OTRN;
}

// 200218
//WORD CVerse::GetServiceDebTime()
//{
//	WORD w = 0;
//	for (int n = PDEVID_PAU; n < PDEVID_UKD; n ++)
//		w += c_wFlapLength[n][LL_DEB];
//	return w * (100 / TPERIOD_BASE);	// debounce cycle * cycle time
//}
//
// 200218
void CVerse::ClearAllRed()
{
	c_mtxRed.Lock();
	memset(&c_ritem, 0, sizeof(REDITEM));
	c_mtxRed.Unlock();
}

void CVerse::ClearSheaf()
{
	memset(&c_sheaf, 0, sizeof(_SHEAF));
}

void CVerse::CopyBundle(PVOID pSrc, WORD wLength)
{
	PHDURINFO pHdur = (PHDURINFO)GetInfo(false, PDEVID_HDU, LI_CURR);
	memcpy(&pHdur->t, pSrc, wLength);
}

void CVerse::Arteriam()
{
#if 1	//---hroy 20211223
	if (c_psv.atocf.wWarmUpTimer[0] > 0)	-- c_psv.atocf.wWarmUpTimer[0];
	if (c_psv.atocf.wWarmUpTimer[1] > 0)	-- c_psv.atocf.wWarmUpTimer[1];
	if (c_psv.rtdcf.wWarmUpTimer > 0)	-- c_psv.rtdcf.wWarmUpTimer;
	if (c_psv.paucf.wWarmUpTimer > 0)	-- c_psv.paucf.wWarmUpTimer;
	if (c_psv.piscf.wWarmUpTimer > 0)	-- c_psv.piscf.wWarmUpTimer;
	if (c_psv.trscf.wWarmUpTimer > 0)	-- c_psv.trscf.wWarmUpTimer;
	if (c_psv.fducf.wWarmUpTimer > 0)	-- c_psv.fducf.wWarmUpTimer;
	if (c_psv.bmscf.wWarmUpTimer > 0)	-- c_psv.bmscf.wWarmUpTimer;
#else
	// 210803
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		if (c_psv.rtdcf.wWarmUpTimer == 0xffff) {
			c_psv.rtdcf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
									TIME_RTDWARMUPC : TIME_RTDWARMUPA;
			c_psv.paucf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
									TIME_PAUWARMUPB : TIME_PAUWARMUPA;
			c_psv.piscf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
									TIME_PISWARMUPB : TIME_PISWARMUPA;
			c_psv.trscf.wWarmUpTimer = pPaper->GetLabFromRecip() ?
									TIME_TRSWARMUPB : TIME_TRSWARMUPA;
		}
	}
	if (c_psv.rtdcf.wWarmUpTimer < 0xffff) {	// 210803
		if (c_psv.atocf.wWarmUpTimer[0] > 0)	-- c_psv.atocf.wWarmUpTimer[0];
		if (c_psv.atocf.wWarmUpTimer[1] > 0)	-- c_psv.atocf.wWarmUpTimer[1];
		if (c_psv.rtdcf.wWarmUpTimer > 0)	-- c_psv.rtdcf.wWarmUpTimer;
		if (c_psv.paucf.wWarmUpTimer > 0)	-- c_psv.paucf.wWarmUpTimer;
		if (c_psv.piscf.wWarmUpTimer > 0)	-- c_psv.piscf.wWarmUpTimer;
		if (c_psv.trscf.wWarmUpTimer > 0)	-- c_psv.trscf.wWarmUpTimer;
		// 200218
		if (c_psv.fducf.wWarmUpTimer > 0)	-- c_psv.fducf.wWarmUpTimer;
		if (c_psv.bmscf.wWarmUpTimer > 0)	-- c_psv.bmscf.wWarmUpTimer;
	}
#endif
	if (++ c_ritem.cMs >= 100) {
		c_ritem.cMs = 0;
		c_mtxRed.Lock();
		if (++ c_ritem.cSec > 59) {
			c_ritem.cSec = 0;
			if (++ c_ritem.cMin > 59) {
				c_ritem.cMin = 0;
				++ c_ritem.wHour;
			}
		}
		c_mtxRed.Unlock();
	}
}

int CVerse::BaleHduRef(bool bType, BYTE* pMesh)
{
	int leng = 0;
	if (bType) {
		//int sent = sizeof(ELISTSENTENCE);
		//int head = offsetof(ELISTSENTENCE, wReal);
		//int bott = sent - (head + 24);				// wLow[16], wHigh[16], wReal[16]
		//sprintf((char*)pMesh, "$DUR,%03d,(SHE,%d,real,32,ref,%d,BUN,%d,real,32,ref,%d,),", sent * 2, head, bott, head, bott);
		//sprintf((char*)pMesh, "$DUR,%03d,(she,%d,bun,%d,),", sizeof(ELISTSTCTAIL) * 2, sizeof(ELISTSTCTAIL), sizeof(ELISTSTCTAIL));
		//while (*pMesh != '\0') {
		//	++ pMesh;
		//	++ leng;
		//}
		sprintf((char*)pMesh, "$DUR, 512, (HEAD, 6, LIST, 100, leng, 2,),");
		while (*pMesh != '\0') {
			++ pMesh;
			++ leng;
		}
	}
	else {
		//memcpy(pMesh, &c_sheaf.tli, sizeof(ELISTSENTENCE));
		//pMesh += sizeof(ELISTSENTENCE);
		//leng += sizeof(ELISTSENTENCE);
		//memcpy(pMesh, &c_bund.tli, sizeof(ELISTSENTENCE));
		//pMesh += sizeof(ELISTSENTENCE);
		//leng += sizeof(ELISTSENTENCE);
		memcpy(pMesh, &c_cDump[0], 512);
		pMesh += 512;
		leng += 512;
	}
	return leng;
}

int CVerse::BaleSvcRef(bool bType, BYTE* pMesh)
{
	int leng = 0;
	if (bType) {
		sprintf((char*)pMesh, "$SVC, %03d, (ato,%d,pau,%d,pis,%d,trs,%d,ATO,%d,PAU,%d,PIS,%d,TRS,%d,),",
				(sizeof(ATOFLAP) + sizeof(PAUFLAP) + sizeof(PISFLAP) + sizeof(TRSFLAP)) * 2,
				sizeof(ATOFLAP), sizeof(PAUFLAP), sizeof(PISFLAP), sizeof(TRSFLAP), sizeof(ATOFLAP), sizeof(PAUFLAP), sizeof(PISFLAP), sizeof(TRSFLAP));
		while (*pMesh != '\0') {
			++ pMesh;
			++ leng;
		}
	}
	else {
		memcpy(pMesh, &c_ato[LI_CURR], sizeof(ATOFLAP));
		pMesh += sizeof(ATOFLAP);
		leng += sizeof(ATOFLAP);
		memcpy(pMesh, &c_pau[LI_CURR], sizeof(PAUFLAP));
		pMesh += sizeof(PAUFLAP);
		leng += sizeof(PAUFLAP);
		memcpy(pMesh, &c_pis[LI_CURR], sizeof(PISFLAP));
		pMesh += sizeof(PISFLAP);
		leng += sizeof(PISFLAP);
		memcpy(pMesh, &c_trs[LI_CURR], sizeof(TRSFLAP));
		pMesh += sizeof(TRSFLAP);
		leng += sizeof(TRSFLAP);

		memcpy(pMesh, &c_ato[LI_WORK], sizeof(ATOFLAP));
		pMesh += sizeof(ATOFLAP);
		leng += sizeof(ATOFLAP);
		memcpy(pMesh, &c_pau[LI_WORK], sizeof(PAUFLAP));
		pMesh += sizeof(PAUFLAP);
		leng += sizeof(PAUFLAP);
		memcpy(pMesh, &c_pis[LI_WORK], sizeof(PISFLAP));
		pMesh += sizeof(PISFLAP);
		leng += sizeof(PISFLAP);
		memcpy(pMesh, &c_trs[LI_WORK], sizeof(TRSFLAP));
		pMesh += sizeof(TRSFLAP);
		leng += sizeof(TRSFLAP);
	}
	return leng;
}

ENTRY_CONTAINER(CVerse)
	SCOOP(&c_psv.atocf.cLines[0],		sizeof(BYTE) * 2,	"Atov")
	SCOOP(&c_psv.atocf.wWarmUpTimer[0],	sizeof(WORD),		"")
	SCOOP(&c_psv.atocf.wWarmUpTimer[1],	sizeof(WORD),		"")
	SCOOP(&c_psv.paucf.cLines[0],		sizeof(BYTE) * 2,	"pauv")
	//SCOOP(&c_psv.paucf.wDefectMap,		sizeof(WORD),		"")
	SCOOP(&c_psv.piscf.cLines[0],		sizeof(BYTE) * 2,	"pisv")
	//SCOOP(&c_psv.piscf.wDefectMap,		sizeof(WORD),		"")
	SCOOP(&c_pisInfo.bAim,				sizeof(BYTE),		"")
	SCOOP(&c_pisInfo.bUser,				sizeof(BYTE),		"")
	SCOOP(&c_psv.trscf.cLines[0],		sizeof(BYTE) * 2,	"trsv")
	SCOOP(&c_psv.trscf.wDefectMap,		sizeof(WORD),		"")
	SCOOP(&c_psv.fducf.cLines[0],		sizeof(BYTE) * 2,	"fduv")
	SCOOP(&c_psv.fducf.wDefectMap,		sizeof(WORD),		"")
	SCOOP(&c_psv.bmscf.cLines[0],		sizeof(BYTE) * 2,	"bmsv")
	SCOOP(&c_psv.bmscf.wDefectMap,		sizeof(WORD),		"")
	SCOOP(&c_rtdRef.cResult,			sizeof(BYTE),		"rtdv")
	SCOOP(&c_rtdRef.wAddr,				sizeof(WORD),		"")
	SCOOP(&c_rtdRef.state,				sizeof(RTDSTATE),	"rtda")
	SCOOP(&c_cLineState,				sizeof(BYTE),		"linv")

	//SCOOP(&c_psv.trscf.fl[0].a,			sizeof(TRSAINFO),	"trs")
EXIT_CONTAINER()
