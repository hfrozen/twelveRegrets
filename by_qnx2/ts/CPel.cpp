/*
 * CPel.cpp
 *
 *  Created on: 2016. 9. 9.
 *      Author: SWCho
 */
#include "Track.h"
#include "CPaper.h"
#include "CVerse.h"
#include "CProse.h"
#include "CSch.h"
#include "CArch.h"
#include "CReview.h"
#include "CTool.h"
#include "CFio.h"
#include "CLand.h"

#include "CPel.h"

CPel::CPel()
{
	c_pParent = NULL;
	c_bDevEx = false;
	c_bCoolup = true;
	c_bEsk = false;
	c_sivs.wStabilizeTime = TIME_SIVSTABILIZE;
	//c_wSivStabilizeTime = TIME_SIVSTABILIZE;
	//c_wPanBitmap = 0;
	c_pans.wMap = 0;
	memset(&c_cms, 0, sizeof(CMSTATE));
	//c_dwCmonTimer = 0;

	for (int n = 0; n < CID_MAX; n ++)
		//c_pan[n].SetDebounceTime(DEB_PANTO);
		c_pans.m[n].st.SetDebounceTime(DEB_PANTO);
	Initial();
}

CPel::~CPel()
{
}

void CPel::Initial()
{
	//memset(&c_wPanWait[0], 0, sizeof(WORD) * CID_MAX);
	//memset(&c_sivfm[0], 0, sizeof(SIVFMON) * CID_MAX);
	//memset(&c_cmkm[0], 0, sizeof(CMKMON) * CID_MAX);
	//memset(&c_eskm[0], 0, sizeof(ESKMON) * CID_MAX);
	for (int n = 0; n < CID_MAX; n ++) {
		c_pans.m[n].wWait = c_sivs.m[n].wWaitKOff = 0;
		c_sivs.m[n].bCur = false;
	}
	memset(&c_cms, 0, sizeof(CMSTATE));
	memset(&c_esks, 0, sizeof(ESKSTATE));
	//c_bPerfectLoop = false;
	//c_wLoopOffTimer = 0;
	c_dtbs.bPerfect = false;
	c_dtbs.wOffTimer = 0;
	//c_wEskState = 0;
	//c_wLoadOffTimer = 0;
}

void CPel::CheckPantograph()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETARCHIVE(pArch);
	for (int n = 0; n < (int)pConf->pan.cLength; n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), pConf->pan.cPos[n]);
		BYTE state = 0;
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			// 200903
			//if (!GETCI(fid, CUDIB_PANPS1)) {
			if ((pPaper->GetDeviceExFromRecip() &&
				GETCI(fid, CUDIB_PANPS1)) ||
				(!pPaper->GetDeviceExFromRecip() &&
				!GETCI(fid, CUDIB_PANPS1))) {
				state |= 1;
				pArch->Enumerate((int)pConf->pan.cPos[n], 443, 445);
			}
			else	pArch->Enumerate((int)pConf->pan.cPos[n], 445, 443);
			//if (!GETCI(fid, CUDIB_PANPS2)) {
			if ((pPaper->GetDeviceExFromRecip() &&
				GETCI(fid, CUDIB_PANPS2)) ||
				(!pPaper->GetDeviceExFromRecip() &&
				!GETCI(fid, CUDIB_PANPS2))) {
				state |= 2;
				pArch->Enumerate((int)pConf->pan.cPos[n], 444, 446);
			}
			else	pArch->Enumerate((int)pConf->pan.cPos[n], 446, 444);
			// 201016
			if (pPaper->GetDeviceExFromRecip()) {
				if (GETCI(fid, CUDIB_PANCOC1))
					pArch->Shot((int)pConf->pan.cPos[n], 447);
				else	pArch->Cut((int)pConf->pan.cPos[n], 447);
				if (GETCI(fid, CUDIB_PANCOC2))
					pArch->Shot((int)pConf->pan.cPos[n], 448);
				else	pArch->Cut((int)pConf->pan.cPos[n], 448);
			}
		}

		//if (c_pan[n].Stabilize(state)) {
		//	if (c_pan[n].cur & 3)
		//	c_wPanBitmap |= (1 << pConf->pan.cPos[n]);
		//	else	c_wPanBitmap &= ~(1 << pConf->pan.cPos[n]);
		//	c_mtx.Lock();
		//	c_wPanWait[n] = WAIT_PANTO;
		//	c_mtx.Unlock();
		//}
		if (c_pans.m[n].st.Stabilize(state)) {
			if (c_pans.m[n].st.cur & 3)
				c_pans.wMap |= (1 << pConf->pan.cPos[n]);
			else	c_pans.wMap &= ~(1 << pConf->pan.cPos[n]);
			c_mtx.Lock();
			c_pans.m[n].wWait = WAIT_PANTO;
			c_mtx.Unlock();
		}
	}
}

void CPel::CheckSiv(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETPROSE(pProse);
	GETARCHIVE(pArch);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	WORD wEs = 0;
	WORD wRisingPanto = 0;
	bool bTrace = false;
	int nDecent = 0;
	for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[n]);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			pLsv->sivcf.cLines[n] &= ~(1 << DEVSTATE_BLINDDTB);
			// 200218
			//if (pDoz->recip[fid].real.svf.s.a.wAddr == LADD_SIV &&
			//	pDoz->recip[fid].real.svf.s.a.cCtrl == DEVCHAR_CTL) {
			if ((!pPaper->GetDeviceExFromRecip() &&
				pDoz->recip[fid].real.svf.s.a.wAddr == LADD_SIV &&
				pDoz->recip[fid].real.svf.s.a.cCtrl == DEVCHAR_CTL) ||
				(pPaper->GetDeviceExFromRecip() &&
				pDoz->recip[fid].real.svf.s2.a.wAddr == LADD_SIV &&
				pDoz->recip[fid].real.svf.s2.a.cCtrl == DEVCHAR_CTL)) {
				pLsv->sivcf.cLines[n] &= ~(1 << DEVSTATE_BLINDUNIT);
				pArch->Cut(pConf->siv.cPos[n], 400);

				if (pPaper->GetDeviceExFromRecip()) {
					if ((c_pans.m[n].st.cur & 3) && c_pans.m[n].wWait == 0) {
						wEs += (WORD)pDoz->recip[fid].real.svf.s2.a.t.s.cEs;
						++ wRisingPanto;
					}
					if (pDoz->recip[fid].real.svf.s2.a.t.s.trc.b.leng > 0)
						bTrace = true;

		// ===== UNTAP 3 =====================================================
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.sta.b.cttf,
								pConf->siv.cPos[n], 450, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.sta.b.inov,
								pConf->siv.cPos[n], 451, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.sta.b.inoc,
								pConf->siv.cPos[n], 452, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.sta.b.chpfov,
								pConf->siv.cPos[n], 453, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.sta.b.chpfuv,
								pConf->siv.cPos[n], 454, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.chpov,
								pConf->siv.cPos[n], 455, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.chpoc,
								pConf->siv.cPos[n], 456, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.invcov,
								pConf->siv.cPos[n], 457, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.dcoc,
								pConf->siv.cPos[n], 458, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.invoc,
								pConf->siv.cPos[n], 459, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.acov,
								pConf->siv.cPos[n], 460, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.acuv,
								pConf->siv.cPos[n], 461, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stc.b.lovd,
								pConf->siv.cPos[n], 462, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.std.b.acub,
								pConf->siv.cPos[n], 463, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.std.b.acoc,
								pConf->siv.cPos[n], 464, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.std.b.acol,
								pConf->siv.cPos[n], 465, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.std.b.bcnoc,
								pConf->siv.cPos[n], 466, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.std.b.bcoc,
								pConf->siv.cPos[n], 467, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.std.b.bcov,
								pConf->siv.cPos[n], 468, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.ste.b.chkf,
								pConf->siv.cPos[n], 469, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.ste.b.lkf,
								pConf->siv.cPos[n], 470, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.ste.b.sivkf,
								pConf->siv.cPos[n], 471, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.ste.b.stf,
								pConf->siv.cPos[n], 472, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.ste.b.igdf,
								pConf->siv.cPos[n], 473, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stf.b.gdfc1,
								pConf->siv.cPos[n], 474, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stf.b.gdfc2,
								pConf->siv.cPos[n], 475, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stf.b.gdfru,
								pConf->siv.cPos[n], 476, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stf.b.gdfrv,
								pConf->siv.cPos[n], 477, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stf.b.gdfbu,
								pConf->siv.cPos[n], 478, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stf.b.gdfbv,
								pConf->siv.cPos[n], 479, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stg.b.gdfu,
								pConf->siv.cPos[n], 480, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stg.b.gdfv,
								pConf->siv.cPos[n], 481, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stg.b.gdfw,
								pConf->siv.cPos[n], 482, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stg.b.thf,
								pConf->siv.cPos[n], 483, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s2.a.t.s.stg.b.cplv,
								pConf->siv.cPos[n], 484, DEB_SIVSDBITFAIL);
		// ===== TAP 3 =======================================================
					double pwr =
							((double)pDoz->recip[fid].real.svf.s2.a.t.s.cVs *
							3000.f / 250.f) *
							((double)pDoz->recip[fid].real.svf.s2.a.t.s.cIs *
							500.f / 250.f) * 0.1f / 3600.f;
					pArch->AddCareers(CRRITEM_SIVPOWER, n, pwr);

					// 180312
					if (!pDoz->recip[fid].real.svf.s2.a.t.s.stb.b.start)
						pLsv->sivcf.cLines[n] |= (1 << DEVSTATE_BYPASS);
					else	pLsv->sivcf.cLines[n] &= ~(1 << DEVSTATE_BYPASS);
				}
				else {
					//if ((c_pan[n].cur & 3) && c_wPanWait[n] == 0) {
					if ((c_pans.m[n].st.cur & 3) && c_pans.m[n].wWait == 0) {
						wEs += (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cEs;
						++ wRisingPanto;
					}
					if (pDoz->recip[fid].real.svf.s.a.t.s.trc.b.leng > 0)
						bTrace = true;

		// ===== UNTAP 3 =====================================================
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.dcov,
								pConf->siv.cPos[n], 407, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.fcov,
								pConf->siv.cPos[n], 408, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.mmocd,
								pConf->siv.cPos[n], 409, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.acov,
								pConf->siv.cPos[n], 410, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.acuv,
								pConf->siv.cPos[n], 411, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.bcov,
								pConf->siv.cPos[n], 412, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.ovld,
								pConf->siv.cPos[n], 413, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.std.b.mjf,
								pConf->siv.cPos[n], 414, DEB_SIVSDBITFAIL);

		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.ivoc,
								pConf->siv.cPos[n], 415, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.acoc,
								pConf->siv.cPos[n], 416, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.psft,
								pConf->siv.cPos[n], 417, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.bcoc,
								pConf->siv.cPos[n], 418, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.gfux1,
								pConf->siv.cPos[n], 419, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.gfvy1,
								pConf->siv.cPos[n], 420, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.gfwz1,
								pConf->siv.cPos[n], 421, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stc.b.thf,
								pConf->siv.cPos[n], 422, DEB_SIVSDBITFAIL);

		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.stb.b.pcd,
								pConf->siv.cPos[n], 423, DEB_SIVSDBITFAIL);

		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.sta.b.gfux2,
								pConf->siv.cPos[n], 424, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.sta.b.gfvy2,
								pConf->siv.cPos[n], 425, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.sta.b.gfwz2,
								pConf->siv.cPos[n], 426, DEB_SIVSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.ste.b.igdf,
								pConf->siv.cPos[n], 429, DEB_SIVSDBITFAIL);

		// 171114
		pArch->SdaPrompt(pDoz->recip[fid].real.svf.s.a.t.s.ste.b.llvf,
								pConf->siv.cPos[n], 402, DEB_SIVSDBITFAIL);
		// ===== TAP 3 =======================================================
					//if (pDoz->recip[fid].real.svf.s.a.t.s.cBatV <
					//	LOW_BATVOLTAGE) {
					//	if (++ c_wLowBatDeb[n] >= DEB_LOWBAT) {
					//		c_wLowBatDeb[n] = DEB_LOWBAT;
					//		pArch->Shot(pConf->siv.cPos[n], 403);
					//	}
					//}
					//else	pArch->Cut(pConf->siv.cPos[n], 403);

					// 170809
					double pwr =
							((double)pDoz->recip[fid].real.svf.s.a.t.s.cVs *
							3000.f / 250.f) *
							((double)pDoz->recip[fid].real.svf.s.a.t.s.cIs *
							//500.f / 250.f) * 0.05f / 3600.f;
							500.f / 250.f) * 0.1f / 3600.f;
					pArch->AddCareers(CRRITEM_SIVPOWER, n, pwr);

					// 180312
					if (!pDoz->recip[fid].real.svf.s.a.t.s.stb.b.start)
						pLsv->sivcf.cLines[n] |= (1 << DEVSTATE_BYPASS);
					else	pLsv->sivcf.cLines[n] &= ~(1 << DEVSTATE_BYPASS);
				}

				++ nDecent;
			}
			else {
				pLsv->sivcf.cLines[n] |= (1 << DEVSTATE_BLINDUNIT);
				if (pProse->GetWarmup(DEVID_SIV) == 0) {
					pArch->Shot(pConf->siv.cPos[n], 400);

					GETREVIEW(pRev);
					if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
							pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
						CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
						++ pLfc->wSiv[n];
					}
				}
			}
			if (pPaper->GetDeviceExFromRecip())
				memcpy(&pLsv->sivcf.fl2[n], &pDoz->recip[fid].real.svf.s2,
														sizeof(SIVFLAP2));
			else
				memcpy(&pLsv->sivcf.fl[n], &pDoz->recip[fid].real.svf.s,
														sizeof(SIVFLAPEX));
			// 200218
			//pLsv->wRed[DEVID_SIV][n] =
			//						pDoz->recip[fid].real.wLocRed[DEVID_SIV];

			if (GETCI(fid, CUDIB_SIVF)) {
				if (!c_sivs.m[n].bCur) {
					c_sivs.m[n].bCur = true;
					c_sivs.m[n].wWaitKOff = WAIT_SIVKOFF;
					// SIVK가 떨어지기를 기다리는 시간.
					pLsv->sivcf.wDefectMap |= (1 << pConf->siv.cPos[n]);
					pLsv->sivcf.cLines[n] |= (1 << DEVSTATE_FAIL);
					pArch->Shot(pConf->siv.cPos[n], 401);
				}
			}
			else {
				if (c_sivs.m[n].bCur) {
					c_sivs.m[n].bCur = false;
					c_sivs.m[n].wWaitKOff = 0;
					pLsv->sivcf.wDefectMap &= ~(1 << pConf->siv.cPos[n]);
					pLsv->sivcf.cLines[n] &= ~(1 << DEVSTATE_FAIL);
					pArch->Cut(pConf->siv.cPos[n], 401);
					pArch->Cut(pConf->siv.cPos[n], 428);
				}
			}

			if (GETCI(fid, CUDIB_SIVK)) {
				pLsv->sivcf.wCorrectMap |= (1 << pConf->siv.cPos[n]);
				if (c_sivs.m[n].bCur && c_sivs.m[n].wWaitKOff > 0 &&
					-- c_sivs.m[n].wWaitKOff == 0)
					pArch->Shot(pConf->siv.cPos[n], 428);
			}
			else {
				pLsv->sivcf.wCorrectMap &= ~(1 << pConf->siv.cPos[n]);
				if (c_sivs.m[n].bCur)	pArch->Cut(pConf->siv.cPos[n], 428);
			}

			// 200218 note:need DEVBF_CMSB
			if (GETCI(fid, CUDIB_CMG))
				pLsv->wCmgBitmap |= (1 << pConf->siv.cPos[n]);
			else	pLsv->wCmgBitmap &= ~(1 << pConf->siv.cPos[n]);

			if (GETCI(fid, CUDIB_CMF))
				pArch->Shot(pConf->siv.cPos[n], 200);
			else	pArch->Cut(pConf->siv.cPos[n], 200);
			if (GETCI(fid, CUDIB_CMBLK))
				pArch->Shot(pConf->siv.cPos[n], 201);
			else	pArch->Cut(pConf->siv.cPos[n], 201);

			if (GETUI(fid, UDOB_CMK)) {
				//if (!c_cmkm[n].bCur) {
				//	c_cmkm[n].bCur = true;
				//	c_cmkm[n].wWait = WAIT_CMKON;
				if (!c_cms.m[n].bCur) {
					c_cms.m[n].bCur = true;
					// 210218
					//c_cms.m[n].wWait = WAIT_CMKON;
					if (pPaper->GetDeviceExFromRecip())
						c_cms.m[n].wWait = WAIT_CMKONA;
					else	c_cms.m[n].wWait = WAIT_CMKON;
					pArch->Shot(pConf->siv.cPos[n], 202);
				}
				else {
					if (GETCI(fid, CUDIB_CMK)) {
						pLsv->wCmkBitmap |= (1 << pConf->siv.cPos[n]);
						//c_cmkm[n].wWait = 0;
						c_cms.m[n].wWait = 0;
						pArch->Cut(pConf->siv.cPos[n], 204);
					}
					else {
						//if (c_cmkm[n].wWait > 0 && -- c_cmkm[n].wWait == 0)
						//	pArch->Shot(pConf->siv.cPos[n], 204);
						if (c_cms.m[n].wWait > 0 && -- c_cms.m[n].wWait == 0)
							pArch->Shot(pConf->siv.cPos[n], 204);
					}
				}
			}
			else {
				//if (c_cmkm[n].bCur) {
				//	c_cmkm[n].bCur = false;
				//	c_cmkm[n].wWait = WAIT_CMKON;
				if (c_cms.m[n].bCur) {
					c_cms.m[n].bCur = false;
					// 201016
					//c_cms.m[n].wWait = WAIT_CMKON;
					c_cms.m[n].wWait = WAIT_CMKOFF;
					// 170929
					//pArch->Shot(pConf->siv.cPos[n], 203);
					pArch->Cut(pConf->siv.cPos[n], 202);
				}
				else {
					if (!GETCI(fid, CUDIB_CMK)) {
						pLsv->wCmkBitmap &= ~(1 << pConf->siv.cPos[n]);
						//c_cmkm[n].wWait = 0;
						c_cms.m[n].wWait = 0;
						pArch->Cut(pConf->siv.cPos[n], 205);
					}
					else {
						//if (c_cmkm[n].wWait > 0 && -- c_cmkm[n].wWait == 0)
						//	pArch->Shot(pConf->siv.cPos[n], 205);
						if (c_cms.m[n].wWait > 0 && -- c_cms.m[n].wWait == 0)
							pArch->Shot(pConf->siv.cPos[n], 205);
					}
				}
			}

			if (GETCI(fid, CUDIB_BVR))
				pArch->Shot(pConf->siv.cPos[n], 404);
			else	pArch->Cut(pConf->siv.cPos[n], 404);
			// 171108
			if (fid == FID_HEAD || fid == FID_TAIL) {
				if (GETCI(fid, CUDIB_UOPS))
					pArch->Shot(pConf->siv.cPos[n], 207);
				else	pArch->Cut(pConf->siv.cPos[n], 207);
				if (GETCI(fid, CUDIB_MRCS))
					pArch->Shot(pConf->siv.cPos[n], 208);
				else	pArch->Cut(pConf->siv.cPos[n], 208);
				if (GETCI(fid, CUDIB_UOS))
					pArch->Shot(pConf->siv.cPos[n], 209);
				else	pArch->Cut(pConf->siv.cPos[n], 209);
			}
		}
		else {
			pLsv->sivcf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			if (pPaper->GetDeviceExFromRecip())
				memset(&pLsv->sivcf.fl2, 0, sizeof(SIVFLAP2));
			else	memset(&pLsv->sivcf.fl, 0, sizeof(SIVFLAPEX));
			//c_bPerfectLoop = false;			// siv를 가진 객차에서 fault 발생
			c_dtbs.bPerfect = false;			// siv를 가진 객차에서 fault 발생
		}	// if (pPaper->GetRouteState(fid))

	}
	// 17/07/10
	if (nDecent == (int)pConf->siv.cLength)	pLsv->sivcf.bTrace = bTrace;

	pRecip->real.sign.scmd.a &= 0xf8;
	if (pLsv->bManualCmg) {
		if (pLsv->wManualCmgCounter > 0)	-- pLsv->wManualCmgCounter;
		if (pLsv->wManualCmgCounter != 0)	pRecip->real.sign.scmd.a &= ~7;
		else	pLsv->bManualCmg = false;
		// wManualCmgCounter가 0이 아니라는 것은 얼마(COUNT_MANUALCMG/1초)전에
		// bManualCmg가 1이 되었다는 것. 즉 1초내로 두번 눌러야 한다.
		// scmd.b.cmX를 모두 off로 하는 것은 CU의 c_bCmonf를 지우고(CMK출력 제거)
		// c_dwCmonTimer를 리셋하여 다음번 TU의 scmd.b.cmX명령을 듣도록 한다.
	}
	else if (pLsv->sivcf.wCorrectMap != 0 && pLsv->wCmgBitmap != 0 &&
		!c_bCoolup) {
		BYTE cmo = 0;
		int s0, s5, s9, e2, e6;
		s0 = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[0]);
		s5 = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[1]);
		e2 = pSch->C2F(pPaper->GetTenor(), pConf->esk.cPos[0]);
		if (pConf->siv.cLength >= 3) {
			s9 = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[2]);
			e6 = pSch->C2F(pPaper->GetTenor(), pConf->esk.cPos[1]);
		}
		else	s9 = e6 = 0;

		for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
			if (pLsv->sivcf.wCorrectMap & (1 << pConf->siv.cPos[n]))
				cmo |= (1 << n);
			// 171030
			else {
				if (pConf->siv.cLength >= 3) {
					if (n == 0 &&
						((GETCI(e2, CUDIB_ESK) && GETCI(s5, CUDIB_SIVK)) ||
						(GETCI(e2, CUDIB_ESK) && GETCI(e6, CUDIB_ESK) &&
						GETCI(s9, CUDIB_SIVK))))
						cmo |= (1 << n);

					else if (n == 1 &&
						((GETCI(e2, CUDIB_ESK) && GETCI(s0, CUDIB_SIVK)) ||
						(GETCI(e6, CUDIB_ESK) && GETCI(s9, CUDIB_SIVK))))
						cmo |= (1 << n);

					else if (n == 2 &&
						((GETCI(e6, CUDIB_ESK) && GETCI(s5, CUDIB_SIVK)) ||
						(GETCI(e6, CUDIB_ESK) && GETCI(e2, CUDIB_ESK) &&
						GETCI(s0, CUDIB_SIVK))))
						cmo |= (1 << n);
				}
				else {
					if (GETCI(e2, CUDIB_ESK) &&
						GETCI(n == 0 ? s5 : s0, CUDIB_SIVK))
						cmo |= (1 << n);
				}
			}
		}
		pRecip->real.sign.scmd.a |= (cmo & 7);
	}

	if (wRisingPanto > 0) {
		wEs /= wRisingPanto;
		wEs *= 12;	//= w * 3000 / 250
		if (wEs < LOW_LINEVOLTAGE) {
			pLsv->bAlbo = true;
			pLsv->bEnManualAlbo = pLsv->bManualAlbo = false;
			pArch->Shot(CID_BOW, 427);
		}
		else {
			pLsv->bAlbo = false;
			pLsv->bEnManualAlbo = true;
			pArch->Cut(CID_BOW, 427);
		}
		pRecip->real.sign.ecmd.b.albo =
						(pLsv->bAlbo || pLsv->bManualAlbo) ? true : false;
	}
	else	pLsv->bAlbo = true;		// 171030
}

// 200218
void CPel::CheckCmsb()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETPROSE(pProse);
	GETARCHIVE(pArch);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[n]);
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			pLsv->sivcf.cLines[n] &= ~(1 << DEVSTATE_BLINDDTB);
			if (pDoz->recip[fid].real.hcb.cb.wSign == SIGN_CMSBFLAP) {
				if (pDoz->recip[fid].real.hcb.cb.c.a.wAddr == LADD_CMSB &&
					pDoz->recip[fid].real.hcb.cb.c.a.cCtrl == DEVCHAR_CTL) {
					pLsv->cmsbcf.cLines[n] &= ~(1 << DEVSTATE_BLINDUNIT);
					pArch->Cut(pConf->siv.cPos[n], 240);
		// ===== UNTAP 3 =====================================================
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.stat.b.bypass,
								pConf->siv.cPos[n], 241, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.ot,
								pConf->siv.cPos[n], 242, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.lv,
								pConf->siv.cPos[n], 243, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.ov,
								pConf->siv.cPos[n], 244, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.hot,
								pConf->siv.cPos[n], 245, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.ooc,
								pConf->siv.cPos[n], 246, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.ool,
								pConf->siv.cPos[n], 247, DEB_CMSBSDBITFAIL);
		pArch->SdaPrompt(pDoz->recip[fid].real.hcb.cb.c.a.t.s.f.b.os,
								pConf->siv.cPos[n], 248, DEB_CMSBSDBITFAIL);
		// ===== TAP 3 =======================================================
					if (pDoz->recip[fid].real.hcb.cb.c.a.t.s.stat.b.bypass)
						pLsv->cmsbcf.cLines[n] |= (1 << DEVSTATE_BYPASS);
					else	pLsv->cmsbcf.cLines[n] &= ~(1 << DEVSTATE_BYPASS);
				}
				else {
					pLsv->cmsbcf.cLines[n] |= (1 << DEVSTATE_BLINDUNIT);
					if (pProse->GetWarmup(DEVID_CMSB) == 0)
						pArch->Shot(pConf->siv.cPos[n], 240);
					GETREVIEW(pRev);
					if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) &&
							pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
						CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
						++ pLfc->wCmsb[n];
					}
				}
				memcpy(&pLsv->cmsbcf.fl[n], &pDoz->recip[fid].real.hcb.cb.c,
														sizeof(CMSBFLAP));

				// 210720
				if (GETCI(fid, CUDIB_CMSBF))
					pArch->Shot(pConf->siv.cPos[n], 206);
				else	pArch->Cut(pConf->siv.cPos[n], 206);
			}
			//else {
			//	is not cmsb frame
			//}
		}
		else {
			pLsv->cmsbcf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			memset(&pLsv->cmsbcf.fl, 0, sizeof(CMSBFLAP));
		}
	}	// if (pPaper->GetRouteState(fid))
}

void CPel::CheckEsk()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETARCHIVE(pArch);
	for (int n = 0; n < (int)pConf->esk.cLength; n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), pConf->esk.cPos[n]);
		if (pPaper->GetRouteState(fid)) {	// 180911, 전원 투입 후, 확인
			// 171101
			//if (GETUI(fid, UDOB_ESK) && !GETCI(fid, CUDIB_ESK))
			//	pArch->Shot(pConf->esk.cPos[n], 433);
			//else if (!GETUI(fid, UDOB_ESK) && GETCI(fid, CUDIB_ESK))
			//	pArch->Shot(pConf->esk.cPos[n], 432);
			//else	pArch->Cut(pConf->esk.cPos[n], 432, 433);
			if (GETUI(fid, UDOB_ESK)) {
				//if (!c_eskm[n].bCur) {
				//	c_eskm[n].bCur = true;
				//	c_eskm[n].wWait = WAIT_ESKON;
				//}
				if (!c_esks.m[n].bCur) {
					c_esks.m[n].bCur = true;
					c_esks.m[n].wWait = WAIT_ESKON;
				}
				else {
					if (GETCI(fid, CUDIB_ESK)) {
						//c_eskm[n].wWait = 0;
						c_esks.m[n].wWait = 0;
						pArch->Cut(pConf->esk.cPos[n], 433);
						// cut "esk 감지 없음"
					}
					else {
						//if (c_eskm[n].wWait > 0 && -- c_eskm[n].wWait == 0)
						//	pArch->Shot(pConf->esk.cPos[n], 433);
						//	// shot "esk 감지 없음"
						if (c_esks.m[n].wWait > 0 && -- c_esks.m[n].wWait == 0)
							pArch->Shot(pConf->esk.cPos[n], 433);
							// shot "esk 감지 없음"
					}
				}
			}
			else {
				//if (c_eskm[n].bCur) {
				//	c_eskm[n].bCur = false;
				//	c_eskm[n].wWait = WAIT_ESKON;
				//}
				if (c_esks.m[n].bCur) {
					c_esks.m[n].bCur = false;
					c_esks.m[n].wWait = WAIT_ESKON;
				}
				else {
					if (!GETCI(fid, CUDIB_ESK)) {
						//c_eskm[n].wWait = 0;
						c_esks.m[n].wWait = 0;
						pArch->Cut(pConf->esk.cPos[n], 432);
						// cut "esk 감지"
					}
					else {
						//if (c_eskm[n].wWait > 0 && -- c_eskm[n].wWait == 0)
						//	pArch->Shot(pConf->esk.cPos[n], 432);
						//	// shot "esk 감지"
						if (c_esks.m[n].wWait > 0 && -- c_esks.m[n].wWait == 0)
							pArch->Shot(pConf->esk.cPos[n], 432);
							// shot "esk 감지"
					}
				}
			}
		}
		//else	c_bPerfectLoop = false;	// 17/06/30, esk 객차의 DTB 상태는 유보..
	}
}

void CPel::CheckExtenstionPower(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETARCHIVE(pArch);

	if (!GETTI(FID_HEAD, TUDIB_SIVCN1)) {
		pLsv->sivcf.wManualDefectMap |=
						(1 << pConf->siv.cPos[pPaper->GetTenor() ?
						pConf->siv.cLength - 1 : 0]);
		pArch->Shot(CID_BOW, 405);
	}
	else {
		pLsv->sivcf.wManualDefectMap &=
							~(1 << pConf->siv.cPos[pPaper->GetTenor() ?
											pConf->siv.cLength - 1 : 0]);
		pArch->Cut(CID_BOW, 405);
	}

	if (pConf->siv.cLength > 2 && (!GETTI(FID_HEAD, TUDIB_SIVCN2) ||
		((pPaper->GetRouteState(FID_TAIL) &&
		!GETTI(FID_TAIL, TUDIB_SIVCN2))))) {	// 180911, 전원 투입 후, 확인
		pLsv->sivcf.wManualDefectMap |= (1 << pConf->siv.cPos[1]);
		pArch->Shot((int)pConf->siv.cPos[1], 406);
	}
	else {
		pLsv->sivcf.wManualDefectMap &= ~(1 << pConf->siv.cPos[1]);
		pArch->Cut((int)pConf->siv.cPos[1], 406);
	}

	if (pPaper->GetRouteState(FID_TAIL) && !GETTI(FID_TAIL, TUDIB_SIVCN1)) {	// 180911, 전원 투입 후, 확인
		pLsv->sivcf.wManualDefectMap |=
							(1 << pConf->siv.cPos[pPaper->GetTenor() ?
											0 : pConf->siv.cLength - 1]);
		pArch->Shot(CID_STERN, 405);
	}
	else {
		pLsv->sivcf.wManualDefectMap &=
							~(1 << pConf->siv.cPos[pPaper->GetTenor() ?
											0 : pConf->siv.cLength - 1]);
		pArch->Cut(CID_STERN, 405);
	}

	WORD esk = 0;
	bool lor = false;
	bool lrr = false;
	bool eskp = false;

	GETTOOL(pTool);
	WORD sivf = pLsv->sivcf.wManualDefectMap | pLsv->sivcf.wDefectMap;
	// SIVF | SIVCN1, 2
	sivf &= ~pLsv->sivcf.wCorrectMap;		// SIVF & ~SIVK
	//if (c_wPanBitmap != 0 && !pLsv->bAlbo && sivf != 0) {							// SIVF 감지
	if (c_pans.wMap != 0 && !pLsv->bAlbo && sivf != 0) {							// SIVF 감지
		// 170729
		if (pPaper->GetLength() < 10)	esk = (1 << pConf->esk.cPos[0]);			// 10량보다 작을 때는 esk가 하나 뿐이다.
		else {																		// 10량에선...
			BYTE flen = pTool->GetBitNumbers(sivf, true);							// SIVF 갯수
			BYTE clen = pTool->GetBitNumbers(pLsv->sivcf.wCorrectMap, true);		// SIVK 갯수
			// 171030, siv가 다 고장나면 esk를 붙일 필요가 없다.
			if (flen < 3 && clen > 0) {
				if (flen > 1)
					esk = ((1 << pConf->esk.cPos[0]) |
							(1 << pConf->esk.cPos[1]));
							// 두대 이상 고장이면 esk를 다 붙인다.
				else {																			// 한대가 고장이면 어느 쪽을 붙일까?
					if (sivf & ((1 << pConf->siv.cPos[0]) |
							(1 << pConf->siv.cPos[1])))
						esk = (1 << pConf->esk.cPos[0]);										// 0호차, 5호차 고장이면 esk2를 붙인다.
					else	esk = (1 << pConf->esk.cPos[1]);									// 9호차 고장이면 esk6을 붙인다.
				}
			}
		}

		WORD w = pLsv->sivcf.wDefectMap != 0 ?
									TIME_LOADOFFAUTO : TIME_LOADOFFMANUAL;
		//if (c_wLoadOffTimer < w) {	// 부하 차단이 되었을까? 시간을 보고...
		//	if (c_wLoadOffTimer == 0)
		//		c_wLoadOffTimer = 1;	// 부하 차단을 먼저 실행한다.
		//								// wLoadOffTimer는 증가하는 값이다.
		if (c_esks.wLoadOffTimer < w) {	// 부하 차단이 되었을까? 시간을 보고...
			if (c_esks.wLoadOffTimer == 0)
				c_esks.wLoadOffTimer = 1;	// 부하 차단을 먼저 실행한다.
											// wLoadOffTimer는 증가하는 값이다.
			lor = true;
			lrr = false;
			//esk = 0;
		}
		else {
			//if (pPaper->GetLength() < 10)
			//	esk = (1 << pConf->esk.cPos[0]);	// 10량보다 작을 때는 esk가 하나 뿐이다.
			//else {								// 10량에선...
			//	BYTE flen = pTool->GetBitNumbers(sivf, true);	// SIVF 갯수
			//	if (flen > 1)
			//		esk = ((1 << pConf->esk.cPos[0]) |
			//				(1 << pConf->esk.cPos[1]));
			//				// 두대 이상 고장이면 esk를 다 붙인다.
			//	else {					// 한대가 고장이면 어느 쪽을 붙일까?
			//		if (sivf & ((1 << pConf->siv.cPos[0]) |
			//			(1 << pConf->siv.cPos[1])))
			//			esk = (1 << pConf->esk.cPos[0]);
			//			// 0호차, 5호차 고장이면 esk2를 붙인다.
			//		else	esk = (1 << pConf->esk.cPos[1]);
			//			// 9호차 고장이면 esk6을 붙인다.
			//	}
			//}
			eskp = true;
			WORD eska = 0;
			for (int n = 0; n < (int)pConf->esk.cLength; n ++) {
				int fid = pSch->C2F(pPaper->GetTenor(), pConf->esk.cPos[n]);
				if (GETCI(fid, CUDIB_ESK))
					eska |= (1 << pConf->esk.cPos[n]);
			}
			if (esk == eska) {
				if (pConf->esk.cLength > 1 &&
					esk == ((1 << pConf->esk.cPos[0]) |
					(1 << pConf->esk.cPos[1]))) {
					lor = true;
					lrr = false;
				}
				else {
					lor = false;
					lrr = true;
				}
			}
		}
	}
	else {			// normal
		//c_wLoadOffTimer = 0;
		c_esks.wLoadOffTimer = 0;
	}
	pRecip->real.sign.scmd.b.esk0 =
							(esk & (1 << pConf->esk.cPos[0])) ? true : false;
	if (pConf->esk.cLength > 1)
		pRecip->real.sign.scmd.b.esk1 =
							(esk & (1 << pConf->esk.cPos[1])) ? true : false;
	//c_wEskState = esk;
	c_esks.wState = esk;
	pRecip->real.sign.scmd.b.eskp = eskp;
	pRecip->real.sign.scmd.b.lrr = lrr;
	pRecip->real.sign.scmd.b.lor = lor;

	if (esk & (1 << pConf->esk.cPos[0]))
		pArch->Shot(pConf->esk.cPos[0], 435);
	else	pArch->Cut(pConf->esk.cPos[0], 435);
	if (esk & (1 << pConf->esk.cPos[1]))
		pArch->Shot(pConf->esk.cPos[1], 435);
	else	pArch->Cut(pConf->esk.cPos[1], 435);
}

void CPel::Monitor(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETARCHIVE(pArch);
	//c_bPerfectLoop = true;
	c_dtbs.bPerfect = true;
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		// 200218
		c_bDevEx = pDoz->recip[FID_HEAD].real.sign.pcmd.b.devEx;
		CheckPantograph();
		CheckSiv(pRecip);		// c_bPerfect는 여기서 false가 될 수 있다.
		if (pPaper->GetDeviceExFromRecip())	CheckCmsb();	// 200218
		CheckEsk();
		//if (c_bPerfectLoop) {
		if (c_dtbs.bPerfect) {
			CheckExtenstionPower(pRecip);
			//c_wLoopOffTimer = 0;
			c_dtbs.wOffTimer = 0;
			pArch->Cut(CID_BOW, 437);
		}
		else {
			//if (c_wEskState != 0) {	// DTB가 불완전한 상태에서 ESK도 투입된 상황...
			if (c_esks.wState != 0) {	// DTB가 불완전한 상태에서 ESK도 투입된 상황...
				//if (c_wLoopOffTimer < TIME_LOOPOFF) {
				if (c_dtbs.wOffTimer < TIME_LOOPOFF) {
					//if (c_wLoopOffTimer == 0)	c_wLoopOffTimer = 1;
					if (c_dtbs.wOffTimer == 0)	c_dtbs.wOffTimer = 1;
				}
				else {
					// 170729, esk0,1, lrr, lor은 그대로 둔다.
					//pRecip->real.sign.scmd.b.esk0 =
					//pRecip->real.sign.scmd.b.esk1 = false;
					pRecip->real.sign.scmd.b.eskp = false;
					pArch->Shot(CID_BOW, 437);
				}
			}
		}

		PDOZEN pDoz = pPaper->GetDozen();
		if (GETTI(FID_HEAD, TUDIB_EPANDS))	pArch->Shot(CID_BOW, 430);
		else	pArch->Cut(CID_BOW, 430);	// 비상 판토 다운을 지우는게 맞는지 좀 더 검토...
		// 170717
		if (GETTI(FID_HEAD, TUDIB_PANUP))	pArch->Shot(CID_BOW, 439);
		else	pArch->Cut(CID_BOW, 439);
		if (GETTI(FID_HEAD, TUDIB_PANDN))	pArch->Shot(CID_BOW, 440);
		else	pArch->Cut(CID_BOW, 440);

		GETSCHEME(pSch);
		PDEVCONF pConf = pSch->GetDevConf();
		for (int n = 0; n < pConf->pan.cLength; n ++) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->pan.cPos[n]);
			if (pPaper->GetRouteState(fid)) {	// 180911, 전원 투입 후... 확인
				if (GETCI(fid, CUDIB_IPDR))
					pArch->Shot(pConf->pan.cPos[n], 441);
				else	pArch->Cut(pConf->pan.cPos[n], 441);
			}
		}

		GETPROSE(pProse);
		CProse::PLSHARE pLsv = pProse->GetShareVars();

		if (c_bCoolup && pLsv->sivcf.wCorrectMap != 0 &&
			c_sivs.wStabilizeTime > 0) {
			//TRACK("PEL:for debug -- c_sivs.wStabilizeTime\n");
			if (-- c_sivs.wStabilizeTime == 0)	c_bCoolup = false;
		}
	}
}

// TU & CU
void CPel::Control(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		// 200218
		c_bDevEx = pDoz->recip[FID_HEAD].real.sign.pcmd.b.devEx;
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		CFio* pFio = pLand->GetFio();
		GETPAPER(pPaper);
		GETSCHEME(pSch);
		PDEVCONF pConf = pSch->GetDevConf();

		BYTE cmn = 0xff;
		for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
			if (pConf->siv.cPos[n] == CAR_PSELF()) {	// have siv ?
				cmn = n;
				break;
			}
		}
		if (cmn < 0xff) {
			BYTE cmo = pDoz->recip[FID_HEAD].real.sign.scmd.a & 7;
			if (cmo & (1 << cmn)) {
				//if (!c_bCmonf) {
				if (!c_cms.bFlag) {
					//c_bCmonf = true;
					c_cms.bFlag = true;
					c_mtx.Lock();
					//c_dwCmonTimer = TIME_CMON;		// max on time
					c_cms.dwOnTimer = TIME_CMON;		// max time
					c_mtx.Unlock();
					pFio->SetOutput(UDOB_CMK, true);
					//TRACK("PEL:cmk on.\n");
				}
			}
			else {
				//c_bCmonf = false;
				c_cms.bFlag = false;
				c_mtx.Lock();
				//c_dwCmonTimer = 0;
				c_cms.dwOnTimer = 0;
				c_mtx.Unlock();
				pFio->SetOutput(UDOB_CMK, false);
				//TRACK("PEL:cmk off.\n");
			}
		}

		BYTE eskn = 0xff;
		for (int n = 0; n < (int)pConf->esk.cLength; n ++) {
			if (pConf->esk.cPos[n] == CAR_PSELF()) {
				eskn = n;
				break;
			}
		}
		if (eskn < 0xff) {
			// 19/10/22
			//if ((pDoz->recip[FID_HEAD].real.sign.scmd.a &
			//	(1 << (eskn + 4))) &&
			//	pDoz->recip[FID_HEAD].real.sign.scmd.b.eskp)
			//	pFio->SetOutput(UDOB_ESK, true);
			//else	pFio->SetOutput(UDOB_ESK, false);
			// FOR_SAFETY, to receive everything!
			//BYTE scmd = pDoz->recip[FID_HEAD].real.sign.scmd.a;
			//if (pPaper->GetRouteState(FID_HEADBK))
			//	scmd &= pDoz->recip[FID_HEADBK].real.sign.scmd.a;
			//if (pPaper->GetRouteState(FID_TAIL))
			//	scmd &= pDoz->recip[FID_TAIL].real.sign.scmd.a;
			//if (pPaper->GetRouteState(FID_TAILBK))
			//	scmd &= pDoz->recip[FID_TAILBK].real.sign.scmd.a;
			//if ((scmd & (1 << (eskn + 4))) && (scmd & 0x8))
			//	pFio->SetOutput(UDOB_ESK, true);
			//else	pFio->SetOutput(UDOB_ESK, false);
			//pFio->SetOutput(UDOB_ESK,
			//	(pDoz->recip[FID_HEAD].real.sign.scmd.a & (1 << (eskn + 4))) ?
			//													true : false);

			int judge = 1;
			BYTE scmd = pDoz->recip[FID_HEAD].real.sign.scmd.a;
			if (pPaper->GetRouteState(FID_HEADBK)) {
				scmd &= pDoz->recip[FID_HEADBK].real.sign.scmd.a;
				++ judge;
			}
			if (pPaper->GetRouteState(FID_TAIL)) {
				scmd &= pDoz->recip[FID_TAIL].real.sign.scmd.a;
				++ judge;
			}
			if (pPaper->GetRouteState(FID_TAILBK)) {
				scmd &= pDoz->recip[FID_TAILBK].real.sign.scmd.a;
				++ judge;
			}
			if ((scmd & (1 << (eskn + 4))) && (scmd & 0x8)) {
				pFio->SetOutput(UDOB_ESK, true);
				if (!c_bEsk) {
					c_bEsk = true;
					TRACK("PEL:esk on, %d agreed.\n", judge);
				}
			}
			else {
				pFio->SetOutput(UDOB_ESK, false);
				if (c_bEsk) {
					c_bEsk = false;
					TRACK("PEL:esk off, %d agreed.\n", judge);
				}
			}
		}
	}

	if (ISTU() && pPaper->IsHead()) {
		//(pPaper->GetOblige() & (1 << OBLIGE_HEAD))) {
		GETREVIEW(pRev);
		WORD wItem = pRev->GetInspectItem();
		WORD wStep = pRev->GetInspectStep();
		if ((pDoz->recip[FID_HEAD].real.drift.cNbrk &
			(1 << NDETECT_ZEROSPEED)) &&
			(wItem & (1 << INSPITEM_SIV)) && wStep >= INSPSTEP_SIVREADY &&
			wStep <= INSPSTEP_SIVJUDGE) {
			GETSCHEME(pSch);
			PDEVCONF pConf = pSch->GetDevConf();
			GETARCHIVE(pArch);
			PINSPREPORT pInsp = pArch->GetInspectReport();
			int nPage = 0;
			pRecip->real.sign.insp.b.siv = false;
			switch (wStep) {
			case INSPSTEP_SIVREADY : {
					pRev->CoordDuo(&pInsp->ics.wSivRes, 3, 8);	// ""
					pRev->CoordDuo(&pInsp->ics.wSivRes, 3, 10);	// ""
					pInsp->wSubItem = 0;
					pInsp->wMsg = 0;
					pInsp->ics.wSiv = 0;
					for (int n = 0; n < SIV_MAX; n ++) {
						pInsp->ics.wEo[n] = pInsp->ics.wFreq[n] =
														c_inspAid.wSeq[n] = 0;
						c_inspAid.bResult[n] = false;
					}
					c_inspAid.bExit = false;
					if (pDoz->recip[FID_HEAD].real.motiv.cNotch !=
						NOTCHID_B7 ||
						!(pDoz->recip[FID_HEAD].real.drift.cNbrk &
						(1 << NBRAKE_SECURITY))) {
						pInsp->wMsg = XWORD(34);
						// "주간 제어기를 B7에 두시고 보안 제동을 체결하시오."
					}
					pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_SIVENTRY :
				if (pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_B7 ||
					(pDoz->recip[FID_HEAD].real.drift.cNbrk &
					(1 << NBRAKE_SECURITY))) {
					pInsp->wMsg = 0;
					c_inspAid.w = 0x3f;
					pInsp->ics.wSiv = XWORD(c_inspAid.w);
					pRev->CoordDuo(&pInsp->ics.wSivRes, 1, 8);	// "시험 진행 중"
					pRev->NextInspectStep();
					pRecip->real.sign.insp.b.siv = true;
					// pSivr->t.s.cmd.b.tsr = true;
				}
				break;
			case INSPSTEP_SIVCHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);
					// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse())
					c_inspAid.bExit = true;
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)
				//	c_inspAid.bExit = true;
				else {
					pRecip->real.sign.insp.b.siv = true;
					// 각 SIV마다 제어할 수 없으므로 여기서는 끝날 때 까지 1로 유지한다.
					for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
						int fid = pSch->C2F(pPaper->GetTenor(),
													pConf->siv.cPos[n]);
						WORD w, v;
						v = pPaper->GetDeviceExFromRecip() ?
								(WORD)pDoz->recip[fid].real.svf.s2.a.t.s.cVo :
								(WORD)pDoz->recip[fid].real.svf.s.a.t.s.cVo;
						w = pSch->GetProportion(v, (WORD)0, (WORD)0xff,
														(WORD)0, (WORD)750);
						pInsp->ics.wEo[n] = XWORD(w);
						v = pPaper->GetDeviceExFromRecip() ?
								(WORD)pDoz->recip[fid].real.svf.s2.a.t.s.cFo :
								(WORD)pDoz->recip[fid].real.svf.s.a.t.s.cFo;
						w = pSch->GetProportion(v, (WORD)0, (WORD)0xff,
														(WORD)0, (WORD)125);
						pInsp->ics.wFreq[n] = XWORD(w);

				// ===== UNTAP 2 =============================================
				switch (c_inspAid.wSeq[n]) {
				case 0 :
					// 200218
					//if (pDoz->recip[fid].real.svf.s.a.t.s.tst.b.st == 1)
					//	c_inspAid.wSeq[n] = 1;
					if ((!pPaper->GetDeviceExFromRecip() &&
						pDoz->recip[fid].real.svf.s.a.t.s.tst.b.st == 1) ||
						(pPaper->GetDeviceExFromRecip() &&
						pDoz->recip[fid].real.svf.s2.a.t.s.tst.b.st == 1))
						c_inspAid.wSeq[n] = 1;
					break;
				case 1 :
					//if (pDoz->recip[fid].real.svf.s.a.t.s.tst.b.st == 2) {
					if ((!pPaper->GetDeviceExFromRecip() &&
						pDoz->recip[fid].real.svf.s.a.t.s.tst.b.st == 2) ||
						(pPaper->GetDeviceExFromRecip() &&
						pDoz->recip[fid].real.svf.s2.a.t.s.tst.b.st == 2)) {
						c_inspAid.wSeq[n] = 2;
						//if (pDoz->recip[fid].real.svf.s.a.t.s.tst.b.vok &&
						//	pDoz->recip[fid].real.svf.s.a.t.s.tst.b.fok)
						if ((!pPaper->GetDeviceExFromRecip() &&
							pDoz->recip[fid].real.svf.s.a.t.s.tst.b.vok &&
							pDoz->recip[fid].real.svf.s.a.t.s.tst.b.fok) ||
							(pPaper->GetDeviceExFromRecip() &&
							pDoz->recip[fid].real.svf.s2.a.t.s.tst.b.vok
							&& pDoz->recip[fid].real.svf.s2.a.t.s.tst.b.fok))
							c_inspAid.bResult[n] = true;

						pRev->CoordDuo(&c_inspAid.w, c_inspAid.bResult[n] ?
												(WORD)1 : (WORD)2, n << 1);
						pInsp->ics.wSiv = XWORD(c_inspAid.w);
					}
					break;
				default :	break;
				}
				// ===== TAP 2 ===============================================
					}

					bool bExit = true;
					for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
						if (c_inspAid.wSeq[n] != 2)	bExit = false;
					}
					if (bExit)	c_inspAid.bExit = bExit;
				}
				if (c_inspAid.bExit) {
					pRecip->real.sign.insp.b.siv = false;
					bool bGood = true;
					c_inspAid.w = 0;
					for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
						if (!c_inspAid.bResult[n]) {
							bGood = false;
							pRev->CoordDuo(&c_inspAid.w, (WORD)2, n << 1);
						}
						else	pRev->CoordDuo(&c_inspAid.w, (WORD)1, n << 1);
					}
					pInsp->ics.wSiv = XWORD(c_inspAid.w);
					pRev->CoordDuo(&pInsp->ics.wSivRes, 2, 8);	// "시험 완료"
					pRev->CoordDuo(&pInsp->ics.wSivRes, bGood ? 1 : 2, 10);
															// 1:"정상"/2:"고장"
					if (bGood)	pRev->NextInspectStep();
					else	pRev->InspectPause();
				}
				break;
			case INSPSTEP_SIVJUDGE :
				if (pRev->GetInspectLapse())
					nPage = pRev->NextInspectStep();
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)
				//	nPage = pRev->NextInspectStep();
				break;
			default :	break;
			}
			if (nPage != 0) {
				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				pLand->RegisterDuPage((WORD)nPage);
			}
		}
	}
}

void CPel::Alone()
{
	// 버스 통신이 없을 때도 제어가 필요하다.
	CLand* pLand = (CLand*)c_pParent;
	ASSERTP(pLand);
	CFio* pFio = pLand->GetFio();

	GETPAPER(pPaper);
	GETSCHEME(pSch);
	WORD wDevMap = pSch->GetLDeviceMap((int)CAR_PSELF());
	if (wDevMap & DEVBF_SIV) {
		INCERT inp;
		pFio->GetInput(inp);
		if (inp.oct.c[CUDIB_SIVK >> 3] & (1 << (CUDIB_SIVK & 7))) {
			if (inp.oct.c[CUDIB_CMG >> 3] & (1 << (CUDIB_CMG & 7))) {
				//if (!c_bCmonf) {
				//	c_bCmonf = true;
				if (!c_cms.bFlag) {
					c_cms.bFlag = true;
					c_mtx.Lock();
				//	c_dwCmonTimer = TIME_CMON;
					c_cms.dwOnTimer = TIME_CMON;
					c_mtx.Unlock();
					pFio->SetOutput(UDOB_CMK, true);
					//TRACK("PEL:cmk on.\n");
				}
			}
			else {
				//c_bCmonf = false;
				c_cms.bFlag = false;
				c_mtx.Lock();
				//c_dwCmonTimer = 0;
				c_cms.dwOnTimer = 0;
				c_mtx.Unlock();
				pFio->SetOutput(UDOB_CMK, false);
				//TRACK("PEL:cmk off.\n");
			}
		}
	}
	if (wDevMap & DEVBF_ESK) {
		OUTCERT outp;
		pFio->GetOutput(outp);
		if (outp.duet.c[0] & (1 << UDOB_ESK))
			pFio->SetOutput(UDOB_ESK, false);
	}
}

void CPel::Arteriam()
{
	GETPAPER(pPaper);
	if (ISTU()) {
		GETSCHEME(pSch);
		PDEVCONF pConf = pSch->GetDevConf();
		for (int n = 0; n < (int)pConf->pan.cLength; n ++) {
			//if (c_wPanWait[n] > 0) {
			if (c_pans.m[n].wWait > 0) {
				c_mtx.Lock();
				//-- c_wPanWait[n];
				-- c_pans.m[n].wWait;
				c_mtx.Unlock();
			}
		}
		//if (c_extSupp.wWait)	-- c_extSupp.wWait;
		//if (c_wLoopOffTimer > 0 && c_wLoopOffTimer < TIME_LOOPOFF)
		//	++ c_wLoopOffTimer;
		if (c_dtbs.wOffTimer > 0 && c_dtbs.wOffTimer < TIME_LOOPOFF)
			++ c_dtbs.wOffTimer;
		GETPROSE(pProse);
		CProse::PLSHARE pLsv = pProse->GetShareVars();
		WORD w = pLsv->sivcf.wDefectMap != 0 ?
									TIME_LOADOFFAUTO : TIME_LOADOFFMANUAL;
		//if (c_wLoadOffTimer > 0 && c_wLoadOffTimer < w)
		//	++ c_wLoadOffTimer;
		if (c_esks.wLoadOffTimer > 0 && c_esks.wLoadOffTimer < w)
			++ c_esks.wLoadOffTimer;
	}

	//if (c_dwCmonTimer != 0) {
	if (c_cms.dwOnTimer != 0) {
		c_mtx.Lock();
		//-- c_dwCmonTimer;
		-- c_cms.dwOnTimer;
		c_mtx.Unlock();
		//if (c_dwCmonTimer == 0) {
		if (c_cms.dwOnTimer == 0) {
			CLand* pLand = (CLand*)c_pParent;
			ASSERTP(pLand);
			CFio* pFio = pLand->GetFio();
			pFio->SetOutput(UDOB_CMK, false);
			//TRACK("PEL:cmk off.\n");
		}
	}
}

ENTRY_CONTAINER(CPel)
	//SCOOP(&c_dwCmonTimer,		sizeof(DWORD),	"Pel")
	//SCOOP(&c_wLoadOffTimer,		sizeof(WORD),	"")
	SCOOP(&c_cms.dwOnTimer,		sizeof(DWORD),	"Pel")
	SCOOP(&c_esks.wLoadOffTimer,	sizeof(WORD),	"")
	SCOOP(&c_sivs.wStabilizeTime,	sizeof(WORD),	"")
	SCOOP(&c_bCoolup,				sizeof(bool),	"")
	SCOOP(&c_pans.m[0].st.cur,		sizeof(BYTE),	"pan")
	SCOOP(&c_pans.m[1].st.cur,		sizeof(BYTE),	"")
	SCOOP(&c_pans.m[2].st.cur,		sizeof(BYTE),	"")
	//SCOOP(&c_extSupp.wSeq,		sizeof(WORD),	"")
	//SCOOP(&c_extSupp.wWait,		sizeof(WORD),	"")
	//SCOOP(&c_extSupp.wState,	sizeof(WORD),	"")
EXIT_CONTAINER()
