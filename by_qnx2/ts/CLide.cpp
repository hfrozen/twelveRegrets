/*
 * CLide.cpp
 *
 *  Created on: 2016. 9. 9.
 *      Author: Che
 */
#include <math.h>
#include "Track.h"
#include "CPaper.h"
#include "CVerse.h"
#include "CProse.h"
#include "CSch.h"
#include "CArch.h"
#include "CReview.h"
#include "../Component2/CBand.h"
#include "CLand.h"

#include "CLide.h"

CLide::CLide()
{
	c_pParent = NULL;
	c_rtdtf.wWaitTimer = TIME_WAITFETCHBYRTD;
	c_rtdtf.cSecond = 0xff;
	c_rtdtf.bFetch = true;	// time set by rtd only once after power on
	Initial();
}

CLide::~CLide()
{
}

void CLide::Initial()
{
	c_bAclUnknown = c_bDclUnknown = true;
	//c_pisReq.bRequest = false;
	//c_pisReq.bInit = true;
	//c_pisReq.bCurrent[0] = c_pisReq.bCurrent[1] = false;
	//c_pisReq.bStatus = false;
}

void CLide::PromptAto(PATOAINFO pAtoa, BYTE ci)
{
	GETARCHIVE(pArch);
	// 171221
	GETVERSE(pVerse);
	if (pVerse->GetWarmup(PDEVID_ATO) == 0) {
		pArch->SdaPrompt(pAtoa->t.s.fail.b.mmilk, ci, 101);
		pArch->SdaPrompt(pAtoa->t.s.fail.b.atplk, ci, 102);
		pArch->SdaPrompt(pAtoa->t.s.fail.b.ovsp, ci,  103);
		pArch->SdaPrompt(pAtoa->t.s.fail.b.eb, ci,    104);
	}
	pArch->SdaPrompt(pAtoa->t.s.note.b.hb, ci,    105);
}

// si:0/1,	fid:FID,	ci:0/3,5,7,9
void CLide::CheckAto(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (GETTI(FID_HEAD, TUDIB_HCR) && ((!bTenor && !si) || (bTenor && si))) {
		pPsv->atocf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
		if (pDoz->recip[FID_HEAD].real.motiv.cMode != MOTIVMODE_EMERGENCY) {
			if (pDoz->recip[fid].real.ato.a.t.s.cReserve1 == DEVCHAR_CTL) {
				// "정상"
				pPsv->atocf.cLines[si] &= ~((1 << DEVSTATE_BLINDUNIT) |
											(1 << DEVSTATE_BYPASS) |
											(1 << DEVSTATE_FAIL));
				pArch->Cut(ci, 100);
				PromptAto(&pDoz->recip[fid].real.ato.a, ci);
				int vi = pVerse->GetValidPisSide();
				// PIS CONTROL CODE #A
				if (!pPsv->piscf.fl[vi].a.t.s.ref.b.mma) {
					WORD w = XWORD(pDoz->recip[fid].real.ato.a.t.s.notr.a);
					pVerse->SetCurOtrn(w);
				}
				// 201016
				if (pPaper->GetDeviceExFromRecip()) {
					double dba =
						(double)pDoz->recip[FID_HEAD].real.ato.a.t.s.cSpeed;
						// Km/h
					double dbb =
						(double)pDoz->recip[FID_HEAD].real.drift.dwSpeed /
						1000.f;	// Km/h
					if (dba >= COMPARE_SPEED || dbb >= COMPARE_SPEED) {
						// 21/01/04
						//if (dba > dbb) {
						//	double db = dba;
						//	dba = dbb;
						//	dbb = db;
						//}
						//if (dba < dbb * 1.05f && dbb - dba > 5.0f)
						//	pArch->Shot(ci, 109);
						if (fabs(dba - dbb) < 10.f)	pArch->Cut(ci, 109);
						else	pArch->Shot(ci, 109);
						//if ((dba >= (dbb * 0.95f)) && dba <= (dbb * 1.05f))
						//	pArch->Cut(ci, 109);
						//else	pArch->Shot(ci, 109);
					}
					else	pArch->Cut(ci, 109);
				}
			}
			else {
				if (pVerse->GetWarmup(PDEVID_ATO, si) == 0) {
					pPsv->atocf.cLines[si] = (1 << DEVSTATE_BLINDUNIT);
					// "통신"
					pArch->Shot(ci, 100);
				}
				else {
					pPsv->atocf.cLines[si] = (1 << DEVSTATE_BYPASS);
					// "대기"
				}
				// 201016
				if (pPaper->GetDeviceExFromRecip())	pArch->Cut(ci, 109);
			}
		}
		else {
			pPsv->atocf.cLines[si] = (1 << DEVSTATE_FAIL);
			// "개방"
			// 201016
			if (pPaper->GetDeviceExFromRecip())	pArch->Cut(ci, 109);
		}
	}
	else {
		pPsv->atocf.cLines[si] = (1 << DEVSTATE_BLINDDTB);
		// "BLOCK"
		pVerse->InitAtoWarmup(si);		// 180511
		// 201016
		if (pPaper->GetDeviceExFromRecip())	pArch->Cut(ci, 109);
	}
}

void CLide::PromptPau(PPAUAINFO pPaua, BYTE ci, BYTE cFlow)
{
	GETARCHIVE(pArch);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.msobl, ci, 701);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.msobr, ci, 702);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.mcob,  ci, 703);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.mpau,  ci, 704);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.ssobl, ci, 705);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.ssobr, ci, 706);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.scob,  ci, 707);
	pArch->SdaPrompt(pPaua->t.s.alarm.b.spau,  ci, 708);
	pArch->SdaPrompt(pPaua->t.s.ampf.b.c0,     0,  709);
	pArch->SdaPrompt(pPaua->t.s.ampf.b.c1,     1,  709);
	pArch->SdaPrompt(pPaua->t.s.ampf.b.c2,     2,  709);
	pArch->SdaPrompt(pPaua->t.s.ampf.b.c3,     3,  709);

	// 210217
	//pArch->SdaPrompt(pPaua->t.s.ampf.b.c4,     4,  709);
	//pArch->SdaPrompt(pPaua->t.s.ampf.b.c5,     5,  709);
	//pArch->SdaPrompt(pPaua->t.s.ampf.b.c6,     6,  709);
	//pArch->SdaPrompt(pPaua->t.s.ampf.b.c7,     7,  709);
	//pArch->SdaPrompt(pPaua->t.s.ampf.b.c8,     8,  709);
	//pArch->SdaPrompt(pPaua->t.s.ampf.b.c9,     9,  709);

	GETPAPER(pPaper);
	int length = pPaper->GetLength();
	if (length > 4) {
		pArch->SdaPrompt(pPaua->t.s.ampf.b.c4,     4,  709);
		pArch->SdaPrompt(pPaua->t.s.ampf.b.c5,     5,  709);
		if (length > 6) {
			pArch->SdaPrompt(pPaua->t.s.ampf.b.c6,     6,  709);
			pArch->SdaPrompt(pPaua->t.s.ampf.b.c7,     7,  709);
			if (length > 8) {
				pArch->SdaPrompt(pPaua->t.s.ampf.b.c8,     8,  709);
				pArch->SdaPrompt(pPaua->t.s.ampf.b.c9,     9,  709);
			}
		}
	}
	pArch->SdaPrompt(pPaua->t.s.fail.b.pa,     ci, 719);
}

// si:0/1,	fid:FID,	ci:0/3,5,7,9
void CLide::CheckPau(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	pPsv->paucf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
	if (pDoz->recip[fid].real.svc.u.a.wAddr == LADD_PAU &&
		pDoz->recip[fid].real.svc.u.a.cCtrl == DEVCHAR_CTL) {
		pPsv->paucf.cLines[si] &= ~(1 << DEVSTATE_BLINDUNIT);
		//pPsv->paucf.wDefectMap &= ~(1 << fid);
		pArch->Cut(ci, 700);
		if (pDoz->recip[fid].real.svc.u.r.wAddr == LADD_PAU &&
			(pDoz->recip[fid].real.svc.u.r.t.s.ais.cCode == SVCCODEH_TAIL ||
			// 												SVCCODEH_HEAD ||
			pDoz->recip[fid].real.svc.u.r.t.s.ais.cCode == SVCCODEH_XCHANGE)) {
			if (pVerse->GetPauWarmUpTimer() == 0) {
				PromptPau(&pDoz->recip[fid].real.svc.u.a, ci,
							pDoz->recip[fid].real.svc.u.r.t.s.ais.cFlow);
			}
		}
	}
	else {
		pPsv->paucf.cLines[si] |= (1 << DEVSTATE_BLINDUNIT);
		if (pVerse->GetWarmup(PDEVID_PAU) == 0) {
			pArch->Shot(ci, 700);		// ??????????
			if (pDoz->recip[fid].real.cPoleBlock & (1 << PDEVID_PAU))
				pArch->Shot(ci, 721);

			GETREVIEW(pRev);
			if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
				pRev->GetInspectStep() == INSPSTEP_POLECHECK) {
				CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
				++ pLfc->wPau[si];
			}
		}
	}
}

void CLide::PromptPis(PPISAINFO pPisa, BYTE ci, BYTE cFlow)
{
	GETARCHIVE(pArch);
	GETPAPER(pPaper);

	pArch->Prompt(pPisa->t.s.inf.b.update,  ci, 836);
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		if (n == 0 || n == (int)(pPaper->GetLength() - 1)) {
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.tnsu, n, 802);
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.dl,   n, 803);
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.dr,   n, 804);
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.dsuf, n, 805);
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.dt,   n, 806);
		}
		else {
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.dh, n, 805);
			pArch->SdaPrompt(pPisa->t.s.dsu[n].b.dt, n, 806);
		}
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l1, n, 828);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l2, n, 829);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l3, n, 830);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l4, n, 831);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l5, n, 832);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l6, n, 833);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l7, n, 834);
		pArch->SdaPrompt(pPisa->t.s.lcdf[n].b.l8, n, 835);
	}
}

// si:0/1,	fid:FID,	ci:0/3,5,7,9
void CLide::CheckPis(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	pPsv->piscf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
	if (pDoz->recip[fid].real.svc.i.a.wAddr == LADD_PIS &&
		pDoz->recip[fid].real.svc.i.a.cCtrl == DEVCHAR_CTL) {
		pPsv->piscf.cLines[si] &= ~(1 << DEVSTATE_BLINDUNIT);
		//pPsv->piscf.wDefectMap &= ~(1 << fid);
		pArch->Cut(ci, 800);
		if (pDoz->recip[fid].real.svc.i.r.wAddr == LADD_PIS &&
			// 171130
			//(pDoz->recip[fid].real.svc.i.r.t.s.cCode == SVCCODEH_HEAD ||
			//pDoz->recip[fid].real.svc.i.r.t.s.cCode == SVCCODEH_TAIL ||
			//pDoz->recip[fid].real.svc.i.r.t.s.cCode == SVCCODEH_XCHANGE)) {
			pDoz->recip[fid].real.svc.i.r.t.s.ais.cCode != SVCCODEH_NEUTRAL) {
			if (pDoz->recip[fid].real.svc.i.r.t.s.ais.cCode == SVCCODEH_HEAD ||
				// 180704, 									SVCCODEH_TAIL ||
				// 180102, 									SVCCODEH_HEAD ||
				pDoz->recip[fid].real.svc.i.r.t.s.ais.cCode ==
				SVCCODEH_XCHANGE) {
				WORD w = XWORD(pDoz->recip[fid].real.svc.i.a.t.s.notr.a);
				// PIS CONTROL CODE #A
				if (pPsv->piscf.fl[si].a.t.s.ref.b.mma)
					pVerse->SetCurOtrn(w);
				// 200218
				//if (pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab)
				// 210218, 내일 입회 시험이 있다해서 막아둔 것을 푼다
				// if (pPaper->GetLabFromRecip())
				// 21-06-15
				GETVERSE(pVerse);
				if (pVerse->GetPisWarmUpTimer() == 0) {
					PromptPis(&pDoz->recip[fid].real.svc.i.a, ci,
							pDoz->recip[fid].real.svc.i.r.t.s.ais.cFlow);
				}
				// 210218, 해제->당분간은 막아둔다...by Team leader
			}
			// 180704
			bool bManual = pVerse->GetPisManual(si ? true : false);
			if (pDoz->recip[fid].real.svc.i.a.t.s.ref.b.mma != bManual) {
				if (pDoz->recip[fid].real.svc.i.a.t.s.ref.b.mma) {
					pArch->Shot(ci, 842);
					// PIS CONTROL CODE #9
					pVerse->SetPisManual(si ? true : false, true);
					if (pPaper->GetLabFromRecip())
						TRACK("LIDE:enter pis manual from %d.\r\n", ci);
				}
				else {
					pArch->Cut(ci, 842);
					pVerse->SetPisManual(si ? true : false, false);
					if (pPaper->GetLabFromRecip())
						TRACK("LIDE:exit pis manual from %d.\r\n", ci);
				}
			}
			// 180704
			// PIS master change demand
			if (pDoz->recip[fid].real.svc.i.r.t.s.ais.cCode <
				SVCCODEH_NEUTRAL &&
				pDoz->recip[fid].real.svc.i.a.t.s.ref.b.mchg) {
				if (pPsv->trscf.cDebMastChg[si] < DEB_PISMASTERCHG) {
					++ pPsv->trscf.cDebMastChg[si];
					if (pPsv->trscf.cDebMastChg[si] ==
						(DEB_PISMASTERCHG - 1)) {
						pVerse->TurnPisCode();
						pArch->Shot(ci, 845);
						if (pPaper->GetLabFromRecip())
							TRACK("LIDE:turn pis code from %s.\r\n",
									fid == FID_HEAD ? "head" : "tail");
					}
				}
			}
			else	pPsv->trscf.cDebMastChg[si] = 0;
		}
	}
	else {
		pPsv->piscf.cLines[si] |= (1 << DEVSTATE_BLINDUNIT);
		if (pVerse->GetWarmup(PDEVID_PIS) == 0) {
			pArch->Shot(ci, 800);		// ??????????
			if (pDoz->recip[fid].real.cPoleBlock & (1 << PDEVID_PIS))
				pArch->Shot(ci, 838);

			GETREVIEW(pRev);
			if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
				pRev->GetInspectStep() == INSPSTEP_POLECHECK) {
				CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
				++ pLfc->wPis[si];
			}
			pPsv->trscf.cDebMastChg[si] = 0;
		}
	}
}

void CLide::PromptTrs(PTRSAINFO pTrsa, BYTE ci, BYTE cFlow)
{
	GETARCHIVE(pArch);
	pArch->SdaPrompt(pTrsa->t.s.fail.b.tpwr, ci, 751);
	pArch->SdaPrompt(pTrsa->t.s.fail.b.tosc, ci, 752);
	pArch->SdaPrompt(pTrsa->t.s.fail.b.rpwr, ci, 753);
	pArch->SdaPrompt(pTrsa->t.s.fail.b.rosc, ci, 754);
}

// si:0/1,	fid:FID,	ci:0/9
void CLide::CheckTrs(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	pPsv->trscf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
	if (pDoz->recip[fid].real.svc.t.a.wAddr == LADD_TRS &&
		pDoz->recip[fid].real.svc.t.a.cCtrl == DEVCHAR_CTL) {
		pPsv->trscf.cLines[si] &= ~(1 << DEVSTATE_BLINDUNIT);
		pPsv->trscf.wDefectMap &= ~(1 << fid);
		pArch->Cut(ci, 750);
		// 180226
		pArch->Cut(ci, 756);
		if (pDoz->recip[fid].real.svc.t.r.t.s.cCode == SVCCODEH_HEAD ||
			pDoz->recip[fid].real.svc.t.r.t.s.cCode == SVCCODEH_XCHANGE) {
			PromptTrs(&pDoz->recip[fid].real.svc.t.a, ci,
						pDoz->recip[fid].real.svc.t.r.t.s.cFlow);
		}
	}
	else {
		pPsv->trscf.cLines[si] |= (1 << DEVSTATE_BLINDUNIT);
		if (pVerse->GetWarmup(PDEVID_TRS) == 0) {
			pPsv->trscf.wDefectMap |= (1 << fid);
			// 171204, need for ATO
			pArch->Shot(ci, 750);		// ??????????
			if (pDoz->recip[fid].real.cPoleBlock & (1 << PDEVID_TRS))
				pArch->Shot(ci, 756);

			GETREVIEW(pRev);
			if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
				pRev->GetInspectStep() == INSPSTEP_POLECHECK) {
				CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
				++ pLfc->wTrs[si];
			}
		}
	}
	// 210803
	if (pPaper->GetDeviceExFromRecip()) {
		// 201118
		// 201016
		if (!GETTI(fid, TUDIB_TRSF))	pArch->Shot(ci, 755);
		// 201116
		//if (GETTI(fid, TUDIB_TRSF))	pArch->Shot(ci, 755);
		else	pArch->Cut(ci, 755);
	}
}

// 201123
#define	DEB_SDBITFAIL2		6

void CLide::PromptFdu(PFDUAINFOAB pFdua, BYTE ci)
{
	_OCTET oct;
	for (int n = 0; n < 8; n ++)
		oct.c[n] = pFdua->t.s.sstate[n];

	_QUARTET qua;
	for (int n = 0; n < 4; n ++)
		qua.c[n] = pFdua->t.s.sclean[n];

	GETARCHIVE(pArch);
	GETPAPER(pPaper);
	WORD wSel = 1;
	for (int cid = 0; cid < (int)pPaper->GetLength(); cid ++) {
		// 210630, 화재 감시는 CUniv에서 하고있다.
		//pArch->SdaPrompt((pFdua->t.s.det.a & wSel) ? true : false, cid,
		//												218, DEB_SDBITFAIL2);
		wSel <<= 1;
		pArch->SdaPrompt((oct.c[0] & 0x03) == 0x02 ? true : false, cid,
														277, DEB_SDBITFAIL2);
		pArch->SdaPrompt((oct.c[0] & 0x0c) == 0x08 ? true : false, cid,
														278, DEB_SDBITFAIL2);
		pArch->SdaPrompt((oct.c[0] & 0x30) == 0x20 ? true : false, cid,
														279, DEB_SDBITFAIL2);
		pArch->SdaPrompt((qua.c[0] & 1) ? true : false, cid,
														281, DEB_SDBITFAIL2);
		pArch->SdaPrompt((qua.c[0] & 2) ? true : false, cid,
														282, DEB_SDBITFAIL2);
		pArch->SdaPrompt((qua.c[0] & 4) ? true : false, cid,
														283, DEB_SDBITFAIL2);
		oct.qw >>= 6LL;
		qua.dw >>= 3L;
	}

	//pArch->SdaPrompt(pFdua->t.s.uf.b.cl, 0, 217);
	//pArch->SdaPrompt(pFdua->t.s.uf.b.cr, pPaper->GetLength() - 1, 217);

	pArch->SdaPrompt(pFdua->t.s.fl.b.sw, ci, 285, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fl.b.d, ci, 286, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs1.b.intemp, ci, 287, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs1.b.ad, ci, 288, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs1.b.mem, ci, 289, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs1.b.cs, ci, 290, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs2.b.im, ci, 291, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs2.b.diom, ci, 292, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs2.b.adm, ci, 293, DEB_SDBITFAIL2);
	pArch->SdaPrompt(pFdua->t.s.fs2.b.cpum, ci, 294, DEB_SDBITFAIL2);
}

// si:0/1,	fid:FID,	ci:0/9
void CLide::CheckFdu(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	pPsv->fducf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
	if (pDoz->recip[fid].real.svc.f.a.wAddr == LADD_FDU &&
		pDoz->recip[fid].real.svc.f.a.cCtrl == DEVCHAR_CTL) {
		pPsv->fducf.cLines[si] &= ~(1 << DEVSTATE_BLINDUNIT);
		pPsv->fducf.wDefectMap &= ~(1 << fid);
		pArch->Cut(ci, 276);
	}
	else {
		pPsv->fducf.cLines[si] |= (1 << DEVSTATE_BLINDUNIT);
		if (pVerse->GetWarmup(PDEVID_FDU) == 0) {
			pPsv->fducf.wDefectMap |= (1 << fid);
			pArch->Shot(ci, 276);

			GETREVIEW(pRev);
			if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
				pRev->GetInspectStep() == INSPSTEP_POLECHECK) {
				CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
				++ pLfc->wFdu[si];
			}
		}
	}
}

// BMS should be in CPel like SIV,
// but in case of 24 car,
// BMS line tie to service line, so it is here
// in the future, should move to CPel
void CLide::PromptBms(PBMSAINFOAB pBmsa, BYTE ci)
{
	GETARCHIVE(pArch);
	pArch->SdaPrompt(pBmsa->t.s.stat.b.chgc ? false : true, ci, 251);
	pArch->SdaPrompt(pBmsa->t.s.stat.b.disc ? false : true, ci, 252);
	pArch->SdaPrompt(pBmsa->t.s.stat.b.chgcf, ci, 253);
	pArch->SdaPrompt(pBmsa->t.s.stat.b.discf, ci, 254);
	pArch->SdaPrompt(pBmsa->t.s.stat.b.pdcf, ci, 255);
	pArch->SdaPrompt(pBmsa->t.s.stat.b.clcf, ci, 256);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.cov, ci, 257);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.cuv, ci, 258);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.pov, ci, 259);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.puv, ci, 260);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.occ, ci, 261);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.ocd, ci, 262);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.ci, ci, 263);
	pArch->SdaPrompt(pBmsa->t.s.fc.b.scf, ci, 264);
	pArch->SdaPrompt(pBmsa->t.s.fe.b.lvf, ci, 265);
	pArch->SdaPrompt(pBmsa->t.s.fe.b.pot, ci, 266);
	pArch->SdaPrompt(pBmsa->t.s.fe.b.put, ci, 267);
	pArch->SdaPrompt(pBmsa->t.s.fe.b.tsf, ci, 268);
	pArch->SdaPrompt(pBmsa->t.s.fe.b.cof, ci, 269);
	pArch->SdaPrompt(pBmsa->t.s.fe.b.shf, ci, 270);
}

// si:0/1,	fid:FID,	ci:0/9
void CLide::CheckBms(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	pPsv->bmscf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
	if (pDoz->recip[fid].real.svc.m.a.wAddr == LADD_BMS &&
		pDoz->recip[fid].real.svc.m.a.cCtrl == DEVCHAR_CTL) {
		pPsv->bmscf.cLines[si] &= ~(1 << DEVSTATE_BLINDUNIT);
		pPsv->bmscf.wDefectMap &= ~(1 << fid);
		pArch->Cut(ci, 250);
		PromptBms(&pDoz->recip[fid].real.svc.m.a, ci);
	}
	else {
		pPsv->bmscf.cLines[si] |= (1 << DEVSTATE_BLINDUNIT);
		if (pVerse->GetWarmup(PDEVID_BMS) == 0) {
			pPsv->bmscf.wDefectMap |= (1 << fid);
			pArch->Shot(ci, 250);

			GETREVIEW(pRev);
			if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
				pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
				CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
				++ pLfc->wBms[si];
			}
		}
	}
}

void CLide::PromptRtd(PRTDSTATEA pRtds, BYTE ci)
{
	GETARCHIVE(pArch);
	pArch->SdaPrompt(pRtds->stat.b.fail, ci, 661);
	// 201207
	//pArch->SdaPrompt(!pRtds->stat.b.modm, ci, 662);
	//pArch->SdaPrompt(!pRtds->stat.b.cpm, ci, 663);
	pArch->SdaPrompt(pRtds->stat.b.modm ? false : true, ci, 662);
	pArch->SdaPrompt(pRtds->stat.b.cpm ? false : true, ci, 663);
	pArch->SdaPrompt(pRtds->stat.b.txfw, ci, 664);
	pArch->SdaPrompt(pRtds->stat.b.txfl, ci, 665);
	pArch->SdaPrompt(pRtds->stat.b.tv, ci, 670);
	pArch->SdaPrompt(pRtds->serv.b.wifi, ci, 666);
	pArch->SdaPrompt(pRtds->serv.b.lte1, ci, 667);
	pArch->SdaPrompt(pRtds->serv.b.lte2, ci, 668);
	pArch->SdaPrompt(pRtds->serv.b.lte3, ci, 669);
}

// si:0/1,	fid:FID,	ci:0/9
void CLide::CheckRtd(bool bTenor, BYTE si, BYTE fid, BYTE ci)
{
	GETARCHIVE(pArch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	pPsv->rtdcf.cLines[si] &= ~(1 << DEVSTATE_BLINDDTB);
	if (pDoz->recip[fid].real.cPoleLine & (1 << PDEVID_RTD)) {
		pPsv->rtdcf.cLines[si] &= ~(1 << DEVSTATE_BLINDUNIT);
		//pPsv->rtdcf.wDefectMap &= ~(1 << fid);
		pArch->Cut(ci, 660);
		// 201207
		//PromptRtd(&pDoz->recip[fid].real.rtd, ci);
		if (!pPaper->GetDeviceExFromRecip())
			PromptRtd(&pDoz->recip[fid].real.rtd, ci);
		else {
			if (pVerse->GetWarmup(PDEVID_RTD) == 0)
				PromptRtd(&pDoz->recip[fid].real.rtd, ci);
		}
	}
	else {
		pPsv->rtdcf.cLines[si] |= (1 << DEVSTATE_BLINDUNIT);
		//pPsv->rtdcf.wDefectMap |= (1 << fid);
		// 201207
		//pArch->Shot(ci, 660);
		if (pVerse->GetWarmup(PDEVID_RTD) == 0)
			pArch->Shot(ci, 660);
	}

}

void CLide::Control(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	bool bInsp = false;

	if (ISTU() && pPaper->IsHead()) {
		//(pPaper->GetOblige() & (1 << OBLIGE_HEAD))) {
		GETREVIEW(pRev);
		WORD wItem = pRev->GetInspectItem();
		WORD wStep = pRev->GetInspectStep();
		if ((pDoz->recip[FID_HEAD].real.drift.cNbrk &
			(1 << NDETECT_ZEROSPEED)) &&
			wItem != 0 &&
			((wStep >= INSPSTEP_POLEREADY && wStep <= INSPSTEP_POLEJUDGE) ||
			(wStep >= INSPSTEP_PAUSE && wStep <= INSPSTEP_FINAL))) {
			bInsp = true;		// 180511
			CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
			GETARCHIVE(pArch);
			PINSPREPORT pInsp = pArch->GetInspectReport();
			int nPage = 0;
			//switch (pDoz->recip[FID_HEAD].real.cst.roll.insp.wStep) {
			switch (wStep) {
			case INSPSTEP_POLEREADY : {
					pRev->CoordDuo(&pInsp->ics.wTcRes, 1, 12);	// "시험 진행 중"
					pRev->CoordDuo(&pInsp->ics.wTcRes, 3, 14);	// ""
					pInsp->ics.wPole = 0xffff;					// "시험"
					// 200218
					pInsp->ics.wExDev &= 0xf0ff;
					pInsp->ics.wExDev |= 0xf00;
					for (int n = 0; n < 2; n ++) {
						pLfc->wPis[n] = pLfc->wPau[n] = pLfc->wTrs[n] =
						pLfc->wRtd[n] = pLfc->wFdu[n] = 0;
					}
					pRev->NextInspectStep();
					// 19/12/05
					pInsp->wMsg = 0;
				}
				break;
			case INSPSTEP_POLECHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);
					// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse()) {
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0) {
					pRev->NextInspectStep();
					// 미리 스텝을 바꿔놓으면 카운터 값이 바뀌지 않는다!!!
					for (int n = 0; n < 2; n ++) {
						pRev->CoordDuo(&pInsp->ics.wPole,
								pLfc->wTrs[n] > INSPECTNGLIMIT_POLE ? 2 : 1,
								n * 2 + 8);		// 1:OK/2:NG
						pRev->CoordDuo(&pInsp->ics.wPole,
								pLfc->wPis[n] > INSPECTNGLIMIT_POLE ? 2 : 1,
								n * 2 + 12);	// 1:OK/2:NG
						pRev->CoordDuo(&pInsp->ics.wPole,
								pLfc->wPau[n] > INSPECTNGLIMIT_POLE ? 2 : 1,
								n * 2);		// 1:OK/2:NG
						pRev->CoordDuo(&pInsp->ics.wPole,
								pLfc->wRtd[n] > INSPECTNGLIMIT_POLE ? 2 : 1,
								n * 2 + 4);	// 1:OK/2:NG
						// 200218
						pRev->CoordDuo(&pInsp->ics.wExDev,
								pLfc->wFdu[n] > INSPECTNGLIMIT_POLE ? 2 : 1,
								n * 2 + 8);
					}
					pRev->CoordDuo(&pInsp->ics.wTcRes, 2, 12);
					// inspect end
					//if ((pInsp->ics.wPole & 0xaaaa) != 0) {
					if ((pInsp->ics.wPole & 0xaaaa) != 0 ||
						(pInsp->ics.wExDev & 0x0a00) != 0) {
						pRev->CoordDuo(&pInsp->ics.wTcRes, 2, 14);
						// 1:OK/2:NG
						pRev->InspectPause();
					}
					else	pRev->CoordDuo(&pInsp->ics.wTcRes, 1, 14);
				}
				break;
			case INSPSTEP_POLEJUDGE :
				if (pRev->GetInspectLapse())
					nPage = pRev->NextInspectStep();
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)
				//	nPage = pRev->NextInspectStep();
				break;
			case INSPSTEP_PAUSE :
				break;
			case INSPSTEP_CANCELREADY :
				if (wStep >= INSPSTEP_COOLREADY &&
					wStep <= INSPSTEP_HEATJUDGE) {
					GETPROSE(pProse);
					pProse->SetInspectCancel(true);
					// 단순히 HVAC의 inspect를 취소하는 플래그를 일정 기간동안 설정한다..
				}
				pInsp->wMsg = XWORD(34);
				// "주간 제어기를 B7에 두시고 보안 제동을 체결하시오."
				pRev->SetInspectStep(INSPSTEP_ENDREADY);
				break;
			case INSPSTEP_ENDREADY :
				if (pDoz->recip[FID_HEAD].real.motiv.cNotch != NOTCHID_B7 ||
					!(pDoz->recip[FID_HEAD].real.drift.cNbrk &
					(1 << NBRAKE_SECURITY))) {
					pInsp->wMsg = XWORD(34);
					// "주간 제어기를 B7에 두시고 보안 제동을 체결하시오."
					pRev->SetInspectStep(INSPSTEP_ENDENTRY);
				}
				else	pRev->SetInspectStep(INSPSTEP_FINAL);
				break;
			case INSPSTEP_ENDENTRY :
				if (pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_B7 &&
					(pDoz->recip[FID_HEAD].real.drift.cNbrk &
					(1 << NBRAKE_SECURITY)))  {
					pInsp->wMsg = 0;
					pRev->SetInspectStep(INSPSTEP_FINAL);
				}
				break;
			case INSPSTEP_FINAL : {
					GETARCHIVE(pArch);
					pArch->SuffInspect(pRev->GetInspectTime());
					// pRev->InspectClear() 전에 실행하여야 시간이 남아있다!
					GETREVIEW(pRev);
					pRev->InspectClear();
				}
				nPage = INSPECTENTRY_PAGE;
				break;
			default :	break;
			}
			if (nPage > 0) {
				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				pLand->RegisterDuPage((WORD)nPage);
			}
		}
	}

	// 180511
	if (ISTU() && !bInsp) {
		if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
			int left =
					pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT ?
														FID_HEAD : FID_TAIL;
			int right = left == FID_HEAD ? FID_TAIL : FID_HEAD;

			GETPROSE(pProse);
			pRecip->real.sign.hcmd.b.cmd =
				pProse->c_hvacMode.Check((BYTE)(GETTIS(left, TUDIB_CHCS1) &
						0xf), (BYTE)(GETTIS(right, TUDIB_CHCS1) & 0xf));
			pRecip->real.sign.hcmd.b.lff =
				pProse->c_hlfsSw.Check(PTI2B(left, TUDIB_HLFS),
												PTI2B(right, TUDIB_HLFS));
			pRecip->real.sign.hcmd.b.elff =
				pProse->c_elfsSw.Check(PTI2B(left, TUDIB_ELFS),
												PTI2B(right, TUDIB_ELFS));
			pRecip->real.sign.hcmd.b.ef =
				pProse->c_vfsSw.Check(PTI2B(left, TUDIB_VFS),
												PTI2B(right, TUDIB_VFS));

			if (!pPaper->GetDeviceExFromRecip())
				pRecip->real.sign.hcmd.b.apdk =
					pProse->c_apdsSw.Check(PTI2B(left, TUDIB_APDS),
												PTI2B(right, TUDIB_APDS));
			else {
				pRecip->real.sign.hcmd.b.apdk =
					pProse->c_apdsSw.Check(PCI2B(left, CUDIB_APDS1),
												PCI2B(right, CUDIB_APDS1));
				pRecip->real.sign.hcmdEx.b.apdk2 =
					pProse->c_apdsSw.Check(PCI2B(left, CUDIB_APDS2),
												PCI2B(right, CUDIB_APDS2));
			}
		}
	}
}

void CLide::LampControl(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	int left =
			pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT ?
														FID_HEAD : FID_TAIL;
	int right = left == FID_HEAD ? FID_TAIL : FID_HEAD;
	GETPROSE(pProse);
	// 180717
	//if (pProse->c_acSw.IsInit()) {
	//	pRecip->real.sign.ecmd.b.ac1 = pRecip->real.sign.ecmd.b.ac2 =
	//		pProse->c_acSw.Check(PTI2B(left, TUDIB_ALCS),
	//											PTI2B(right, TUDIB_ALCS));
	//}
	//if (pProse->c_dcSw.IsInit()) {
	//	pRecip->real.sign.ecmd.b.dc =
	//		pProse->c_dcSw.Check(NTI2B(left, TUDIB_DLCS),
	//											NTI2B(right, TUDIB_DLCS));
	//}
	if (pProse->c_acSw.IsInit()) {
		bool acl = pProse->c_acSw.Check(PTI2B(left, TUDIB_ALCS),
												PTI2B(right, TUDIB_ALCS));
		if (c_bAclUnknown) {
			bool bAct = pProse->c_acSw.GetAct();
			if (bAct) {
				pRecip->real.sign.ecmd.b.ac1 =
							pRecip->real.sign.ecmd.b.ac2 = acl;
				c_bAclUnknown = false;
			}
			else	pRecip->real.sign.ecmd.b.ac1 =
								pRecip->real.sign.ecmd.b.ac2 = true;
		}
		else	pRecip->real.sign.ecmd.b.ac1 =
							pRecip->real.sign.ecmd.b.ac2 = acl;
	}
	else	pRecip->real.sign.ecmd.b.ac1 =
						pRecip->real.sign.ecmd.b.ac2 = true;

	if (pProse->c_dcSw.IsInit()) {
		bool dcl = pProse->c_dcSw.Check(NTI2B(left, TUDIB_DLCS),
												NTI2B(right, TUDIB_DLCS));
		if (c_bDclUnknown) {
			bool bAct = pProse->c_dcSw.GetAct();
			if (bAct) {
				pRecip->real.sign.ecmd.b.dc = dcl;
				c_bDclUnknown = false;
			}
			else	pRecip->real.sign.ecmd.b.dc = false;	// 180808, true;
		}
		else	pRecip->real.sign.ecmd.b.dc = dcl;
	}
	else	pRecip->real.sign.ecmd.b.dc = false;	// 180808,	true;
}

void CLide::Monitor(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	for (int n = 0; n < 2; n ++) {
		int cid = n ? pPaper->GetLength() - 1 : 0;
		int fid = pSch->C2F(pPaper->GetTenor(), cid);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			//if (pDoz->recip[FID_HEAD].real.motiv.cMode !=
			//									MOTIVMODE_EMERGENCY)
			CheckAto(pPaper->GetTenor(), n, fid, cid);
			memcpy(&pPsv->atocf.fl[n], &pDoz->recip[fid].real.ato,
														sizeof(ATOFLAP));

			WORD w = pDoz->recip[fid].real.svc.u.r.wAddr;
			// 200218
			//if (w == LADD_PAU) {
			//	CheckPau(pPaper->GetTenor(), n, fid, cid);
			//	memcpy(&pPsv->paucf.fl[n], &pDoz->recip[fid].real.svc.u,
			//											sizeof(PAUFLAP));
			//}
			//else if (w == LADD_PIS) {
			//	CheckPis(pPaper->GetTenor(), n, fid, cid);
			//	memcpy(&pPsv->piscf.fl[n], &pDoz->recip[fid].real.svc.i,
			//											sizeof(PISFLAP));
			//}
			//else if (w == LADD_TRS) {
			//	CheckTrs(pPaper->GetTenor(), n, fid, cid);
			//	memcpy(&pPsv->trscf.fl[n], &pDoz->recip[fid].real.svc.t,
			//											sizeof(TRSFLAP));
			//}
			switch (w) {
			case LADD_PAU :
				CheckPau(pPaper->GetTenor(), n, fid, cid);
				memcpy(&pPsv->paucf.fl[n], &pDoz->recip[fid].real.svc.u,
														sizeof(PAUFLAP));
				break;
			case LADD_PIS :
				CheckPis(pPaper->GetTenor(), n, fid, cid);
				memcpy(&pPsv->piscf.fl[n], &pDoz->recip[fid].real.svc.i,
														sizeof(PISFLAP));
				break;
			case LADD_TRS :
				CheckTrs(pPaper->GetTenor(), n, fid, cid);
				memcpy(&pPsv->trscf.fl[n], &pDoz->recip[fid].real.svc.t,
														sizeof(TRSFLAP));
				break;
			case LADD_FDU :
				if (pPaper->GetDeviceExFromRecip()) {
					CheckFdu(pPaper->GetTenor(), n, fid, cid);
					memcpy(&pPsv->fducf.fl[n], &pDoz->recip[fid].real.svc.f,
														sizeof(FDUFLAPAB));
				}
				break;
			case LADD_BMS :
				if (pPaper->GetDeviceExFromRecip()) {
					CheckBms(pPaper->GetTenor(), n, fid, cid);
					memcpy(&pPsv->bmscf.fl[n], &pDoz->recip[fid].real.svc.m,
														sizeof(BMSFLAPAB));
					if (pDoz->recip[fid].real.cst.cID == CSTID_BMSEX)
						memcpy(&pPsv->bmscf.cvs[n],
									&pDoz->recip[fid].real.cst.roll.bcv,
														sizeof(BMSCVS));
				}
				break;
			default :
				break;
			}

			CheckRtd(pPaper->GetTenor(), n, fid, cid);

			// 200218
			//for (int m = 0; m < PDEVRED_MAX; m ++)
			//	pPsv->wRed[m][n] = pDoz->recip[fid].real.wPoleRed[m];
			if (pDoz->recip[fid].real.polef.cDevID < PDEVID_MAX)
				pPsv->wReceivedRed[pDoz->recip[fid].real.polef.cDevID][n] =
										pDoz->recip[fid].real.polef.wCnt;
		}
		else {
			pPsv->atocf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			pPsv->paucf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			pPsv->piscf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			pPsv->trscf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			// 200218
			if (pPaper->GetDeviceExFromRecip()) {
				pPsv->fducf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
				pPsv->bmscf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			}
			pPsv->rtdcf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			memset(&pPsv->atocf.fl[n], 0, sizeof(ATOFLAP));
			memset(&pPsv->paucf.fl[n], 0, sizeof(PAUFLAP));
			memset(&pPsv->piscf.fl[n], 0, sizeof(PISFLAP));
			memset(&pPsv->trscf.fl[n], 0, sizeof(TRSFLAP));
			memset(&pPsv->fducf.fl[n], 0, sizeof(FDUFLAPAB));
			memset(&pPsv->bmscf.fl[n], 0, sizeof(BMSFLAPAB));
		}

		// 171130, move to CVerse
		// 17/07/07, 17/06/26,
		//if (((pDoz->recip[FID_HEAD].real.cSvcCtrlCode & 0xc) >> 2) !=
		//	SVCCODEB_NEUTRAL &&
		//	(pDoz->recip[FID_HEAD].real.cSvcCtrlCode & 3) !=
		//	SVCCODEB_NEUTRAL) {
		//	if (pDoz->recip[FID_HEAD].real.cPoleBlock &
		//		((1 << PDEVID_PAU) | (1 << PDEVID_PIS)))
		//		pPsv->cSvcCtrlCode =
		//					((SVCCODEB_XCHANGE << 2) | SVCCODEB_NEUTRAL);
		//						// tail:0x34, head:0x33
		//	else if (pDoz->recip[FID_TAIL].real.cPoleBlock &
		//		((1 << PDEVID_PAU) | (1 << PDEVID_PIS)))
		//		pPsv->cSvcCtrlCode =
		//					((SVCCODEB_NEUTRAL << 2) | SVCCODEB_XCHANGE);
		//						// tail:0x33, head:0x34
		//	// 171129
		//	//else	pPsv->cSvcCtrlCode =
		//					((SVCCODEB_TAIL << 2) | SVCCODEB_HEAD);
		//						// tail:0x32, head:0x31
		//	else	pPsv->cSvcCtrlCode = pPsv->cRefSvcCode;
		//}
	}
	// 200218
	if (pPaper->GetDeviceExFromRecip()) {
		if (!IsFreeze(pPsv->fducf.cLines[0]) &&
			!IsFreeze(pPsv->fducf.cLines[1])) {
			BYTE mf = pPaper->GetTenor() ? 2 : 1;
			int mid = pPaper->GetTenor() ? 1 : 0;
			int sid = pPaper->GetTenor() ? 0 : 1;
			if ((pPsv->fducf.fl[mid].a.t.s.stat.a & mf) != 0)
				PromptFdu(&pPsv->fducf.fl[mid].a,
						pPaper->GetTenor() ? pPaper->GetLength() - 1 : 0);
			else {
				mf = mf == 1 ? 2 : 1;
				if ((pPsv->fducf.fl[sid].a.t.s.stat.a & mf) != 0)
					PromptFdu(&pPsv->fducf.fl[sid].a,
						pPaper->GetTenor() ? 0 : pPaper->GetLength() - 1);
			}
		}
		else if (!IsFreeze(pPsv->fducf.cLines[0]))
			PromptFdu(&pPsv->fducf.fl[0].a, 0);
		else if (!IsFreeze(pPsv->fducf.cLines[1]))
			PromptFdu(&pPsv->fducf.fl[1].a, pPaper->GetLength() - 1);
	}

	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		if (CAR_FSELF() == FID_HEAD && c_rtdtf.wWaitTimer == 0) {
			int fid = INVALID_HANDLE;
			GETARCHIVE(pArch);
			if (c_rtdtf.bFetch) {
				fid = FID_HEAD;
				if (!pDoz->recip[fid].real.rtd.stat.b.tv ||
					!pArch->CheckDbft(pDoz->recip[fid].real.rtd.dbft)) {
					fid = FID_TAIL;
					if (!pPaper->GetRouteState(FID_TAIL) ||
						!pDoz->recip[fid].real.rtd.stat.b.tv ||
						!pArch->CheckDbft(pDoz->recip[fid].real.rtd.dbft))
						fid = INVALID_HANDLE;	// 180911, 확인
				}
			}
			if (fid > INVALID_HANDLE) {
				if (c_rtdtf.cSecond == 0xff)
					c_rtdtf.cSecond = pDoz->recip[fid].real.rtd.dbft.t.sec;
				else if (c_rtdtf.cSecond !=
					pDoz->recip[fid].real.rtd.dbft.t.sec) {
					c_rtdtf.cSecond = pDoz->recip[fid].real.rtd.dbft.t.sec;
		// ===== UNTAP 3 =====================================================
		if (c_rtdtf.cSecond == 0) {
			if (pArch->CheckDbft(pDoz->recip[fid].real.rtd.dbft)) {
				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				if (!pLand->GetTimeMatchStatus() &&
					!pArch->GetTimeSetStatus()) {
					pArch->SetSysTimeByDbf(pDoz->recip[fid].real.rtd.dbft,
																	true);
					pLand->StirTimeMatch(true);
					pArch->Shot(CAR_PSELF(), 671);
					c_rtdtf.bFetch = false;
					TRACK("LIDE:set time %02d/%02d/%02d %02d:%02d:%02d "
							"by %d\n",
							pDoz->recip[fid].real.rtd.dbft.t.year + 10,
							pDoz->recip[fid].real.rtd.dbft.t.mon,
							pDoz->recip[fid].real.rtd.dbft.t.day,
							pDoz->recip[fid].real.rtd.dbft.t.hour,
							pDoz->recip[fid].real.rtd.dbft.t.min,
							pDoz->recip[fid].real.rtd.dbft.t.sec,
							pDoz->recip[fid].real.prol.cProperID);
				}
			}
		}
		// ===== TAP 3 =====================================================
				}
			}
		}
		// 181019
		//else	c_rtdtf.bFetch = false;
	}
}

void CLide::Arteriam()
{
	if (c_rtdtf.wWaitTimer > 0)	-- c_rtdtf.wWaitTimer;
	//GETARCHIVE(pArch);
	//for (int n = 0; n < 2; n ++) {
	//	if (c_svcf[n].wSeq == 1) {
	//		c_mtx.Lock();
	//		if (-- c_svcf[n].wWatch == 0) {
	//			c_svcf[n].wSeq = 2;
	//			pArch->Shot(n ? CID_STERN : CID_BOW, 790);
	//		}
	//		c_mtx.Unlock();
	//	}
	//}
}

//ENTRY_CONTAINER(CLide)
//	SCOOP(&c_cA,						sizeof(BYTE),		"Lide")
//	SCOOP(&c_cB,						sizeof(BYTE),		"")
//	SCOOP(&c_cC,						sizeof(BYTE),		"")
//EXIT_CONTAINER()
