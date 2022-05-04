/*
 * CUniv.cpp
 *
 *
 *  Created on: 2015. 11. 25.
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
#include "CBand.h"
#include "CFio.h"
#include "CLand.h"

#include "CUniv.h"

// 210803
// 열차에서 실제로 측정한 B1에서의 BC값 평균 = 2480.344 = 5.173bar
// 제동 불 완해를 판단하는 값은 B1보다 좀 크게해서..
// ~ 5.5bar = 5.60846Kg/cm2 = C2586 = 21.58mA
#define	NRBDJUDGE_BCV		2590	// (2586)

CUniv::CUniv()
{
	c_pParent = NULL;
	Initial();
}

CUniv::~CUniv()
{
}

void CUniv::Initial()
{
	// 180917
	//for (int n = 0; n < ECU_MAX; n ++)	c_wNrbdTime[n] = 0;

	c_dm.mode = DOORMODE_NON;
	c_dcPrev = 0;
	memset(&c_dm, 0, sizeof(DOORMODE));
	c_dm.d.SetDebounceTime(DEB_DOORMODE);
}

void CUniv::CheckV3f()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETARCHIVE(pArch);
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	bool bTrace = false;
	int nDecent = 0;
	for (int n = 0; n < (int)pConf->v3f.cLength; n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), pConf->v3f.cPos[n]);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			pLsv->v3fcf.cLines[n] &= ~(1 << DEVSTATE_BLINDDTB);
			if (pDoz->recip[fid].real.svf.v.a.wAddr == LADD_V3F && pDoz->recip[fid].real.svf.v.a.cCtrl == DEVCHAR_CTL) {
				pLsv->v3fcf.cLines[n] &= ~(1 << DEVSTATE_BLINDUNIT);
				pArch->Cut(pConf->v3f.cPos[n], 500);

				WORD wEs = XWORD(pDoz->recip[fid].real.svf.v.a.t.s.wEs);
				WORD wIdc = XWORD(pDoz->recip[fid].real.svf.v.a.t.s.wIdc);
				SHORT idc = (SHORT)wIdc;
				// 170809
				//double pwr = ((double)wEs / 10.f) * ((double)idc / 10.f) * 0.05f / 3600.f;
				double pwr = ((double)wEs / 10.f) * ((double)idc / 10.f) * 0.1f / 3600.f;
				if (pwr >= 0.f)	pArch->AddCareers(CRRITEM_V3FPOWER, n, pwr);
				else	pArch->AddCareers(CRRITEM_V3FREVIV, n, fabs(pwr));
				if (pDoz->recip[fid].real.svf.v.a.t.s.trc.b.leng > 0)	bTrace = true;

				// 170831
				PV3FBRKCNTF pVbf = pArch->GetV3fBreaker();
				if (pDoz->recip[fid].real.svf.v.a.t.s.det.b.hb) {
					if (!pVbf->bHb[n]) {
						pVbf->bHb[n] = true;
						if (pVbf->u.v.wHbCnt[n] < 0xffff)	++ pVbf->u.v.wHbCnt[n];
					}
				}
				else	pVbf->bHb[n] = false;

				if (pDoz->recip[fid].real.svf.v.a.t.s.det.b.lb1) {
					if (!pVbf->bLb[n]) {
						pVbf->bLb[n] = true;
						if (pVbf->u.v.wLbCnt[n] < 0xffff)	++ pVbf->u.v.wLbCnt[n];
					}
				}
				else	pVbf->bLb[n] = false;

				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.ref.b.ccos, pConf->v3f.cPos[n], 502);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.bpsf,  pConf->v3f.cPos[n], 503);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.spsf,  pConf->v3f.cPos[n], 504);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.apsf,  pConf->v3f.cPos[n], 505);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.u,    pConf->v3f.cPos[n], 506);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.x,    pConf->v3f.cPos[n], 507);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.v,    pConf->v3f.cPos[n], 508);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.y,    pConf->v3f.cPos[n], 509);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.w,    pConf->v3f.cPos[n], 510);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.z,    pConf->v3f.cPos[n], 511);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.gdf.b.b,    pConf->v3f.cPos[n], 512);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.iocd,  pConf->v3f.cPos[n], 513);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.mocd,  pConf->v3f.cPos[n], 514);
				//pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.ref.b.rstr, pConf->v3f.cPos[n], 515);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fb.b.hbtf,  pConf->v3f.cPos[n], 516);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fb.b.lgd,   pConf->v3f.cPos[n], 517);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.bocd,  pConf->v3f.cPos[n], 518);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fa.b.pud,   pConf->v3f.cPos[n], 519);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fc.b.frf,   pConf->v3f.cPos[n], 520);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fc.b.pbf,   pConf->v3f.cPos[n], 521);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fb.b.vllvd, pConf->v3f.cPos[n], 522);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fb.b.fclvd, pConf->v3f.cPos[n], 523);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fb.b.fcovd, pConf->v3f.cPos[n], 524);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fb.b.pgd,   pConf->v3f.cPos[n], 525);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.otd.b.u,    pConf->v3f.cPos[n], 526);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.otd.b.v,    pConf->v3f.cPos[n], 527);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.otd.b.w,    pConf->v3f.cPos[n], 528);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.cpt.b.hbo,  pConf->v3f.cPos[n], 529);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.cpt.b.hbc,  pConf->v3f.cPos[n], 530);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.cpt.b.lb2o, pConf->v3f.cPos[n], 531);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.cpt.b.lb2c, pConf->v3f.cPos[n], 532);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.cpt.b.lb1o, pConf->v3f.cPos[n], 533);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.cpt.b.lb1c, pConf->v3f.cPos[n], 534);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fc.b.fcd,   pConf->v3f.cPos[n], 535);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fc.b.bsd,   pConf->v3f.cPos[n], 536);
				pArch->SdaPrompt(pDoz->recip[fid].real.svf.v.a.t.s.fc.b.comf,  pConf->v3f.cPos[n], 537);

				++ nDecent;
			}
			else {
				pLsv->v3fcf.cLines[n] |= (1 << DEVSTATE_BLINDUNIT);
				if (pProse->GetWarmup(DEVID_V3F) == 0) {
					pArch->Shot(pConf->v3f.cPos[n], 500);

					GETREVIEW(pRev);
					if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) && pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
						CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
						++ pLfc->wV3f[n];
					}
				}
			}
			memcpy(&pLsv->v3fcf.fl[n], &pDoz->recip[fid].real.svf.v, sizeof(V3FFLAP));
			// 200218
			//pLsv->wRed[DEVID_V3F][n] = pDoz->recip[fid].real.wLocRed[DEVID_V3F];
		}
		else {
			pLsv->v3fcf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			memset(&pLsv->v3fcf.fl[n], 0, sizeof(V3FFLAP));
		}	// if (pPaper->GetRouteState(fid))

		if (!(pLsv->v3fcf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {		// 16/06/26, Modify
			if (GETCI(fid, CUDIB_VFF)) {
				pLsv->v3fcf.cLines[n] |= (1 << DEVSTATE_FAIL);
				pLsv->v3fcf.wDefectMap |= (1 << pConf->v3f.cPos[n]);
				pArch->Shot(pConf->v3f.cPos[n], 501);
			}
			else {
				pLsv->v3fcf.cLines[n] &= ~(1 << DEVSTATE_FAIL);
				pLsv->v3fcf.wDefectMap &= ~(1 << pConf->v3f.cPos[n]);
				pArch->Cut(pConf->v3f.cPos[n], 501);
			}

			if (!GETCI(fid, CUDIB_MDS))	pArch->Shot(pConf->v3f.cPos[n], 538);
			else	pArch->Cut(pConf->v3f.cPos[n], 538);
		}
	}
	// 17/07/10
	if (nDecent == (int)pConf->v3f.cLength)	pLsv->v3fcf.bTrace = bTrace;
}

void CUniv::CheckEcu(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	WORD wSlide = 0;
	GETSCHEME(pSch);
	GETARCHIVE(pArch);
	GETPROSE(pProse);
	bool bTrace = false;
	int nDecent = 0;
	WORD wNrbd = 0;
	WORD wLoadrSum = 0;
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	CLand* pLand = (CLand*)c_pParent;
	ASSERTP(pLand);
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			pLsv->ecucf.cLines[n] &= ~(1 << DEVSTATE_BLINDDTB);
			if (pDoz->recip[fid].real.ecu.co.a.wAddr == LADD_ECU && pDoz->recip[fid].real.ecu.co.a.cCtrl == DEVCHAR_CTL) {
				pLsv->ecucf.cLines[n] &= ~(1 << DEVSTATE_BLINDUNIT);
				pArch->Cut(n, 300);

				if (pDoz->recip[fid].real.ecu.co.a.t.s.ref.b.slide)	wSlide |= (1 << n);
				if (pDoz->recip[fid].real.ecu.co.a.t.s.trc.b.leng > 0)	bTrace = true;
				// 170720
				//pLsv->cLw[n] = (BYTE)pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.a.t.s.cLw, (WORD)0, (WORD)255, (WORD)20, (WORD)75);	// 0~0xff -> 20~75ton
				WORD w = pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.co.a.t.s.cLw, (WORD)0, (WORD)255, (WORD)200, (WORD)750);	// 0~0xff -> 20.0~75.0ton
				pLsv->cLw[n] = (BYTE)(w / 10);
				wLoadrSum += pSch->CalcLoad(n, w);

				// 171221
				//pArch->SdaPrompt(pDoz->recip[fid].real.ecu.a.t.s.fa.b.lwf,   n, 310);
				//pArch->SdaPrompt(pDoz->recip[fid].real.ecu.a.t.s.fa.b.rbedf, n, 311);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fa.b.lwf,   n, 312);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fa.b.emvf,  n, 313);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fa.b.dvff,  n, 314);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fa.b.dvfr,  n, 315);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fb.b.cpsf,  n, 317);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fb.b.tpsf,  n, 318);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fb.b.a1as,  n, 319);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fb.b.a2as,  n, 320);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fb.b.a3as,  n, 321);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fb.b.a4as,  n, 322);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fc.b.ebrf,  n, 324);
				if (GETTI(FID_HEAD, TUDIB_HCR) && pLand->GetEcuBcfPrevent() == 0)
					pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fc.b.bcf,   n, 325);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fc.b.tllf,  n, 326);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fc.b.rbeaf, n, 327);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.r2hf,  n, 329);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.r1hf,  n, 330);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.cb07f, n, 332);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.eb03f, n, 333);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.eb01f, n, 334);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.mb04f, n, 335);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fd.b.alwf,  n, 336);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.lrpd,  n, 339);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.cif,   n, 340);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.rpsf,  n, 341);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.cvpsf, n, 342);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.vmvf,  n, 343);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.cmvf,  n, 344);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.r6hf,  n, 345);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.co.a.t.s.fe.b.r5hf,  n, 346);

				// 170717
				if (GETTI(FID_HEAD, TUDIB_POWERING) && pDoz->recip[fid].real.ecu.co.a.t.s.ref.b.hb) {
					if (++ pLsv->ecucf.wHbReleaseWatch[n] >= WATCH_HBRELEASE) {
						pLsv->ecucf.wHbReleaseWatch[n] = WATCH_HBRELEASE;
						pArch->Shot(n, 108);
					}
				}
				else {
					pLsv->ecucf.wHbReleaseWatch[n] = 0;
					pArch->Cut(n, 108);
				}

				// 171107
				//WORD wDev = pSch->GetLDeviceMap(n);
				//w = pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.a.t.s.cBcp, (WORD)0, (WORD)255, (WORD)0, (WORD)1000);	// 0~0xff -> 0.00 ~ 10.00 bar
				//if (GETCI(fid, CUDIB_NRBD) && (((wDev & DEVBF_V3F) && w >= 49) || (!(wDev & DEVBF_V3F) && w >= 62))) {
				//	pArch->Shot(n, 304);
				//	++ wNrbd;				// 170920
				//}
				//else	pArch->Cut(n, 304);
				// 171113
				if (GETCI(fid, CUDIB_NRBD))	pArch->Shot(n, 304);
				else	pArch->Cut(n, 304);
				// 200918
				// 200629
				//WORD wDev = pSch->GetLDeviceMap(n);
				//w = 0;
				//if (pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE] < 0xff0)
				//	w = (WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE];
				//w = pSch->GetProportion(w, (WORD)819, (WORD)4095, (WORD)0, (WORD)1019);
				//if (((wDev & DEVBF_V3F) && w >= 98) || (!(wDev & DEVBF_V3F) && w >= 124))	pArch->Shot(n, 307);
				//else	pArch->Cut(n, 307);

				// 180320
				//if (!(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) && GETTI(FID_HEAD, TUDIB_POWERING) && GETTI(FID_HEAD, TUDIB_DIR)) {	// door close state
				// 180917
				//if (!(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) &&
				//	GETTI(FID_HEAD, TUDIB_POWERING) && GETTI(FID_HEAD, TUDIB_DIR) && GETCI(fid, CUDIB_ECUMF)) {	// door close state & ecu normal
				//	WORD wDev = pSch->GetLDeviceMap(n);
				//	w = pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.a.t.s.cBcp, (WORD)0, (WORD)255, (WORD)0, (WORD)1000);	// 0~0xff -> 0.00 ~ 10.00 bar
				//	if (((wDev & DEVBF_V3F) && w >= 47) || (!(wDev & DEVBF_V3F) && w >= 67)) {
				//		if (++ c_wNrbdTime[n] >= TIME_NRBDETECT) {
				//			c_wNrbdTime[n] = TIME_NRBDETECT;
				//			pArch->Shot(n, 303);
				//		}
				//	}
				//	else {
				//		c_wNrbdTime[n] = 0;
				//		pArch->Cut(n, 303);
				//	}
				//}
				//else {
				//	c_wNrbdTime[n] = 0;
				//	pArch->Cut(n, 303);
				//}
				++ nDecent;
			}	// if (pDoz->recip[fid].real.ecu.a.wAddr == LADD_ECU && pDoz->recip[fid].real.ecu.a.cCtrl == DEVCHAR_CTL) {
			else {
				pLsv->ecucf.cLines[n] |= (1 << DEVSTATE_BLINDUNIT);
				if (pProse->GetWarmup(DEVID_ECU) == 0) {
					pArch->Shot(n, 300);

					GETREVIEW(pRev);
					if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) && pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
						CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
						++ pLfc->wEcu[n];
					}
				}
			}
			memcpy(&pLsv->ecucf.fl[n], &pDoz->recip[fid].real.ecu, sizeof(ECUFLAP));
			// 200218
			//pLsv->wRed[DEVID_ECU][n] = pDoz->recip[fid].real.wLocRed[DEVID_ECU];
		}	// if (pPaper->GetRouteState(fid)) {
		else {
			pLsv->ecucf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			memset(&pLsv->ecucf.fl[n], 0, sizeof(ECUFLAP));
		}	// if (pPaper->GetRouteState(fid))

		if (!(pLsv->ecucf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {	// 17/06/26, Modify
			if (!GETCI(fid, CUDIB_ECUMF)) {
				pLsv->ecucf.cLines[n] |= (1 << DEVSTATE_FAIL);
				pLsv->ecucf.wDefectMap |= (1 << n);
				pArch->Shot(n, 302);
			}
			else {
				pLsv->ecucf.cLines[n] &= ~(1 << DEVSTATE_FAIL);
				pLsv->ecucf.wDefectMap &= ~(1 << n);
				pArch->Cut(n, 302);
			}

			if (GETCI(fid, CUDIB_INSBR))	pArch->Shot(n, 301);
			else	pArch->Cut(n, 301);

			// 171107
			//if (GETCI(fid, CUDIB_NRBD)) {
			//	pArch->Shot(n, 304);
			//	++ wNrbd;				// 170920
			//}
			//else	pArch->Cut(n, 304);
			if (!GETCI(fid, CUDIB_BRCS))	pArch->Shot(n, 306);
			else	pArch->Cut(n, 306);

			if (!GETCI(fid, CUDIB_BCCS))	pArch->Shot(n, 212);
			else	pArch->Cut(n, 212);
			if (!GETCI(fid, CUDIB_ASCS))	pArch->Shot(n, 213);
			else	pArch->Cut(n, 213);
		}
	}	// for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
	pLsv->ecucf.wSlideBitmap = wSlide;
	// 17/07/10
	if (nDecent == (int)pPaper->GetLength())	pLsv->ecucf.bTrace = bTrace;
	// 170920
	pLsv->wLoadrSum = wLoadrSum;
	pLsv->wNrbds = wNrbd;
}

void CUniv::CheckEcu2(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	WORD wSlide = 0;
	GETSCHEME(pSch);
	GETARCHIVE(pArch);
	GETPROSE(pProse);
	bool bTrace = false;
	int nDecent = 0;
	WORD wNrbd = 0;
	WORD wLoadrSum = 0;
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	CLand* pLand = (CLand*)c_pParent;
	ASSERTP(pLand);
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			pLsv->ecucf.cLines[n] &= ~(1 << DEVSTATE_BLINDDTB);
			if (pDoz->recip[fid].real.ecu.ct.a.wAddr == LADD_ECU && pDoz->recip[fid].real.ecu.ct.a.cCtrl == DEVCHAR_CTL) {
				pLsv->ecucf.cLines[n] &= ~(1 << DEVSTATE_BLINDUNIT);
				pArch->Cut(n, 300);

				if (pDoz->recip[fid].real.ecu.ct.a.t.s.ref.b.slide)	wSlide |= (1 << n);
				if (pDoz->recip[fid].real.ecu.ct.a.t.s.trc.b.leng > 0)	bTrace = true;
				WORD w = (WORD)((double)pDoz->recip[fid].real.ecu.ct.a.t.s.cLw * 3.f + 200.f);
				pLsv->cLw[n] = (BYTE)(w / 10);
				wLoadrSum += pSch->CalcLoad(n, w);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.majf, n, 349);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.minf, n, 348);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fa.b.lwf,   n, 353);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fa.b.emvf,  n, 352);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fa.b.dvff,  n, 351);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fa.b.dvfr,  n, 350);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fb.b.bcpsf, n, 358);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fb.b.a1as,  n, 357);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fb.b.a2as,  n, 356);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fb.b.a3as,  n, 355);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fb.b.a4as,  n, 354);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fc.b.ebrf,  n, 362);
				if (GETTI(FID_HEAD, TUDIB_HCR) && pLand->GetEcuBcfPrevent() == 0)
					pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fc.b.bcf,   n, 361);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fc.b.tllf,  n, 360);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fc.b.rbeaf, n, 359);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fd.b.as1psf, n, 367);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fd.b.as2psf, n, 366);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fd.b.as3psf, n, 365);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fd.b.as4psf, n, 364);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fd.b.alwf,   n, 363);

				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fe.b.acpsf, n, 370);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fe.b.r6hf,  n, 369);
				pArch->SdaPrompt(pDoz->recip[fid].real.ecu.ct.a.t.s.fe.b.r5hf,  n, 368);

				// 170717
				if (GETTI(FID_HEAD, TUDIB_POWERING) && pDoz->recip[fid].real.ecu.ct.a.t.s.ref.b.hb) {
					if (++ pLsv->ecucf.wHbReleaseWatch[n] >= WATCH_HBRELEASE) {
						pLsv->ecucf.wHbReleaseWatch[n] = WATCH_HBRELEASE;
						pArch->Shot(n, 108);
					}
				}
				else {
					pLsv->ecucf.wHbReleaseWatch[n] = 0;
					pArch->Cut(n, 108);
				}

				if (GETCI(fid, CUDIB_NRBD))	pArch->Shot(n, 304);
				else	pArch->Cut(n, 304);

				++ nDecent;
			}	// if (pDoz->recip[fid].real.ecu.a.wAddr == LADD_ECU && pDoz->recip[fid].real.ecu.a.cCtrl == DEVCHAR_CTL) {
			else {
				pLsv->ecucf.cLines[n] |= (1 << DEVSTATE_BLINDUNIT);
				if (pProse->GetWarmup(DEVID_ECU) == 0) {
					pArch->Shot(n, 300);

					GETREVIEW(pRev);
					if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) && pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
						CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
						++ pLfc->wEcu[n];
					}
				}
			}
			memcpy(&pLsv->ecucf.fl[n], &pDoz->recip[fid].real.ecu, sizeof(ECUFLAP));
		}	// if (pPaper->GetRouteState(fid)) {
		else {
			pLsv->ecucf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			memset(&pLsv->ecucf.fl[n], 0, sizeof(ECUFLAP));
		}	// if (pPaper->GetRouteState(fid))

		if (!(pLsv->ecucf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {	// 17/06/26, Modify
			if (!GETCI(fid, CUDIB_ECUMF)) {
				pLsv->ecucf.cLines[n] |= (1 << DEVSTATE_FAIL);
				pLsv->ecucf.wDefectMap |= (1 << n);
				pArch->Shot(n, 302);
			}
			else {
				pLsv->ecucf.cLines[n] &= ~(1 << DEVSTATE_FAIL);
				pLsv->ecucf.wDefectMap &= ~(1 << n);
				pArch->Cut(n, 302);
			}

			if (GETCI(fid, CUDIB_INSBR))	pArch->Shot(n, 301);
			else	pArch->Cut(n, 301);

			if (!GETCI(fid, CUDIB_BRCS))	pArch->Shot(n, 306);
			else	pArch->Cut(n, 306);

			if (!GETCI(fid, CUDIB_BCCS))	pArch->Shot(n, 212);
			else	pArch->Cut(n, 212);
			if (!GETCI(fid, CUDIB_ASCS))	pArch->Shot(n, 213);
			else	pArch->Cut(n, 213);
		}

		if (GETTI(FID_HEAD, TUDIB_POWERING) &&
			pDoz->recip[FID_HEAD].real.drift.cEbrk == 0 &&
			pDoz->recip[FID_HEAD].real.drift.cNbrk == 0) {
			// 200918
			// 40mA:	C4095
			// 10mA:	C1024
			//  8mA:	 C819
			// 10.19Kg/cm2		= C3276 => + C819 = C4095 = 40mA
			// 0.6Kg/cm2		=  C193 => + C819 = C1012 = 9.8mA
			// 0.98Kg/cm2		=  C315 => + C819 = C1134 = 11.07mA
			// 1.24Kg/cm2		=  C398 => + C819 = C1217 = 11.8mA
			// 210803
			// 열차에서 실제로 측정한 B1에서의 BC값 평균 = 2480.344 = 5.173bar
			// 제동 불 완해를 판단하는 값은 B1보다 좀 크게해서..
			// ~ 5.5bar = 5.60846Kg/cm2 = C2586 = 21.58mA
			if (pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE] >= NRBDJUDGE_BCV) {
				if (!(pLsv->ecucf.wNrbdDetectedMap & (1 << n))) {
					if (pLsv->ecucf.wNrbdDetectMap & (1 << n)) {
						if (pLsv->ecucf.wNrbdDetectTimer[n] == 0) {
							pArch->Shot(n, 307);
							pLsv->ecucf.wNrbdDetectedMap |= (1 << n);
						}
					}
					else {
						pLsv->ecucf.wNrbdDetectMap |= (1 << n);
						pLsv->ecucf.wNrbdDetectTimer[n] = TIME_NRBDDETECT;
					}
				}
			}
			else {
				pLsv->ecucf.wNrbdDetectMap &= ~(1 << n);
				pLsv->ecucf.wNrbdDetectedMap &= ~(1 << n);
				pLsv->ecucf.wNrbdDetectTimer[n] = 0;
				pArch->Cut(n, 307);
			}
		}
		else	pArch->Cut(n, 307);

	}	// for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
	pLsv->ecucf.wSlideBitmap = wSlide;
	if (nDecent == (int)pPaper->GetLength())	pLsv->ecucf.bTrace = bTrace;
	pLsv->wLoadrSum = wLoadrSum;
	pLsv->wNrbds = wNrbd;
}

void CUniv::CheckHvac()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	GETARCHIVE(pArch);
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			if (pDoz->recip[fid].real.hcb.cb.wSign != SIGN_CMSBFLAP) {		//&&
				//pDoz->recip[fid].real.hcb.hv.wSeq != pLsv->hvaccf.fl[n].wSeq) {
				pLsv->hvaccf.cLines[n] &= ~(1 << DEVSTATE_BLINDDTB);
				if (pDoz->recip[fid].real.hcb.hv.a.wAddr == LADD_HVAC && pDoz->recip[fid].real.hcb.hv.a.cCtrl == DEVCHAR_CTL) {
					pLsv->hvaccf.cLines[n] &= ~(1 << DEVSTATE_BLINDUNIT);
					pArch->Cut(n, 600);

					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u1c1ol, n, 601);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u1c2ol, n, 602);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u2c1ol, n, 603);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u2c2ol, n, 604);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u1eol,  n, 605);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u2eol,  n, 606);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u1d1lp, n, 607);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fa.b.u1d1hp, n, 608);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.u1d2lp, n, 609);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.u1d2hp, n, 610);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.u2d1lp, n, 611);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.u2d1hp, n, 612);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.u2d2lp, n, 613);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.u2d2hp, n, 614);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fb.b.vfk,    n, 615);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.ts1,    n, 616);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.ts2,    n, 617);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.ts3,    n, 618);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.ts4,    n, 619);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.cmk1,   n, 620);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.cmk2,   n, 621);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.efk1,   n, 622);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fc.b.cmk3,   n, 623);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fd.b.cmk4,   n, 624);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fd.b.efk2,   n, 625);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fd.b.rhek1,  n, 626);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fd.b.rhek2,  n, 627);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fd.b.lffk1,  n, 628);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fd.b.lffk2,  n, 629);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fe.b.apdk,   n, 630);
					// 171114
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fe.b.apdkf,	n, 631);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fe.b.vdp,    n, 632);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fe.b.p24,    n, 633);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fe.b.co2m1,  n, 634);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.fe.b.co2m2,  n, 635);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.ff.b.vf1do,  n, 636);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.ff.b.vf1dc,  n, 637);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.ff.b.vf2do,  n, 638);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.ff.b.vf2dc,  n, 639);
					pArch->SdaPrompt(pDoz->recip[fid].real.hcb.hv.a.t.s.ff.b.hvac,   n, 640);
				}
				else {
					pLsv->hvaccf.cLines[n] |= (1 << DEVSTATE_BLINDUNIT);
					if (pProse->GetWarmup(DEVID_HVAC) == 0) {
						pArch->Shot(n, 600);

						GETREVIEW(pRev);
						if ((pRev->GetInspectItem() & (1 << INSPITEM_TCL)) && pRev->GetInspectStep() == INSPSTEP_LOCALCHECK) {
							CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
							++ pLfc->wHvac[n];
						}
					}
				}
				memcpy(&pLsv->hvaccf.fl[n], &pDoz->recip[fid].real.hcb.hv, sizeof(HVACFLAP));
				// 200218
				//pLsv->wRed[DEVID_HVAC][n] = pDoz->recip[fid].real.wLocRed[DEVID_HVAC];
			}
			//else {
			//	is not hvac frame
			//}
		}
		else {
			pLsv->hvaccf.cLines[n] |= (1 << DEVSTATE_BLINDDTB);
			memset(&pLsv->hvaccf.fl[n], 0, sizeof(HVACFLAP));
		}	// if (pPaper->GetRouteState(fid))
	}
}

bool CUniv::CheckFire()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETSCHEME(pSch);
	GETARCHIVE(pArch);

	bool bFire = false;
	WORD fd, fduf;
	fd = fduf = 0;
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			if (!GETCI(fid, CUDIB_FDUF)) {
				fduf |= (1 << n);
				pArch->Shot(n, 217);
			}
			else {
				fduf &= ~(1 << n);
				pArch->Cut(n, 217);
			}
			if (GETCI(fid, CUDIB_FD)) {
				fd |= (1 << n);
				pArch->Shot(n, 218);
			}
			else {
				fd &= ~(1 << n);
				pArch->Cut(n, 218);
			}
		}
	}
	pLsv->fire.wUnitJamMap = fduf;
	pLsv->fire.wDefectMap = fd;

	switch (pLsv->fire.wSeq) {
	case CProse::FS_OFF :
		if (pLsv->fire.wDefectMap != 0) {
			pLsv->fire.wSeq = CProse::FS_WAITUSERCOMMAND;
			pLsv->fire.wUserCmd = 0;
		}
		break;
	case CProse::FS_WAITUSERCOMMAND :
		switch (pLsv->fire.wUserCmd) {
		case CProse::UCF_AGREE :
			pLsv->fire.wSeq = CProse::FS_ACTIVE;
			pLsv->fire.wUserCmd = 0;
			break;
		case CProse::UCF_IGNORE :
			pLsv->fire.wSeq = CProse::FS_IGNORE;
			pLsv->fire.wUserCmd = 0;
			break;
		default :
			break;
		}
		break;
	case CProse::FS_ACTIVE :
		bFire = true;
		if (pLsv->fire.wUserCmd == CProse::UCF_IGNORE) {
			pLsv->fire.wSeq = CProse::FS_IGNORE;
			pLsv->fire.wUserCmd = 0;
			bFire = false;
		}
		break;
	case CProse::FS_IGNORE :
		if (pLsv->fire.wDefectMap == 0)
			pLsv->fire.wSeq = CProse::FS_OFF;
		break;
	}
	return bFire;
}

void CUniv::CheckFire2()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	GETSCHEME(pSch);
	GETARCHIVE(pArch);

	bool bTenor = pPaper->GetTenor();
	BYTE mif = bTenor ? 2 : 1;
	// !!!FIRE DETECTION CAUTION!!! whichever, only find detection signal
	int vfi = (pPsv->fducf.fl[(int)bTenor].a.t.s.stat.a & mif) != 0 ? (int)bTenor : (int)(bTenor ^ true);	// valid fdu ?
	WORD wDetect = pPsv->fducf.fl[vfi].a.t.s.det.a & 0x3f;
	BYTE cFault = pPsv->fducf.fl[vfi].a.t.s.uf.a;

	WORD fd, fduf;
	fd = fduf = 0;
	for (int n = 0; n < 2; n ++) {
		int cid = n > 0 ? pPaper->GetLength() - 1 : 0;
		int fid = pSch->C2F(pPaper->GetTenor(), cid);
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			if (!GETCI(fid, CUDIB_FDUF) || (cFault & mif) != 0) {
				fduf |= (1 << cid);
				pArch->Shot(cid, 217);
			}
			else {
				fduf &= ~(1 << cid);
				pArch->Cut(cid, 217);
			}
			if (GETCI(fid, CUDIB_FD))	fd |= (1 << cid);
			else	fd &= ~(1 << cid);
		}
		mif = mif == 1 ? 2 : 1;
	}
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		if (wDetect & (1 << n)) {
			// find at sda
			if (fd != 0) {
				pArch->Shot(n, 218);
			}
			else {
				// but not di
				wDetect &= ~(1 << n);
				pArch->Cut(n, 218);
			}
		}
		else {
			pArch->Cut(n, 218);
		}
	}
	// FD STEP 1: find fire car
	if (wDetect != 0)
		TRACK("FDS1: find fire car %#x from tc%d\n", wDetect, vfi);
	pLsv->fire.wUnitJamMap = fduf;
	pLsv->fire.wDefectSync = fd;
	pLsv->fire.wDefectMap = wDetect;

	switch (pLsv->fire.wSeq) {
	case CProse::FS_OFF :
		if (pLsv->fire.wDefectSync != 0) {
			// FD STEP 2: find fire detection
			pLsv->fire.wSeq = CProse::FS_WAITUSERCOMMAND;
			pLsv->fire.wUserCmd = 0;
			pLsv->fire.wRecoverMap = pLsv->fire.wDefectMap;
			pLsv->fire.bAck = false;
			pLsv->fire.bMbCancel = false;
			TRACK("FDS2:detect %#x %#x\n", pLsv->fire.wDefectMap, pLsv->fire.wDefectSync);
		}
		break;
	case CProse::FS_WAITUSERCOMMAND :
		break;
	case CProse::FS_CLOSE :
		pLsv->fire.wDefectMap = 0;
		pLsv->fire.bAck = pLsv->fire.bMbCancel = false;
		pLsv->fire.wSeq = CProse::FS_OFF;
		break;
	}
}

void CUniv::CheckPassenger()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETSCHEME(pSch);
	GETARCHIVE(pArch);

	WORD pa1, pa2;
	pa1 = pa2 = 0;
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			if (GETCI(fid, CUDIB_PA1)) {
				pa1 |= (1 << n);
				pArch->Shot(n, 219);
			}
			else {
				pa1 &= ~(1 << n);
				pArch->Cut(n, 219);
			}
			if (GETCI(fid, CUDIB_PA2)) {
				pa2 |= (1 << n);
				pArch->Shot(n, 220);
			}
			else {
				pa2 &= ~(1 << n);
				pArch->Cut(n, 220);
			}
		}
	}
	pLsv->wPaMap = pa1 | pa2;
}

void CUniv::CheckDoorLine(int cid, PDCUFLAP pDcu)
{
	GETARCHIVE(pArch);
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	BYTE ha = HIGHBYTE(pDcu->r.wAddr);
	BYTE la = LOWBYTE(pDcu->r.wAddr);
	if (la == 0x70 && ((ha >= 1 && ha <= 4) || (ha >= 0x11 && ha <= 0x14))) {
		BYTE m;
		if (ha < 0x10)	m = (BYTE)(ha - 1);		// 0~3:left		// 4~7:left		// 0~3:left
		else	m = (BYTE)(ha - 0x11 + 4);		// 4~7:right	// 0~3:right	// 4~7:right
		memcpy(&pLsv->dcucf.fl[cid][m], pDcu, sizeof(DCUFLAP));

		ha = HIGHBYTE(pDcu->a.wAddr);
		la = LOWBYTE(pDcu->a.wAddr);
		if (la == 0x70 && ((ha >= 1 && ha <= 4) || (ha >= 0x11 && ha <= 0x14)) && pDcu->a.cCtrl == DEVCHAR_CTL) {
			if (ha < 0x10)	m = (BYTE)(ha - 1);		// 0~3:left		// 4~7:left		// 0~3:left
			else	m = (BYTE)(ha - 0x11 + 4);		// 4~7:right	// 0~3:right	// 4~7:right

			// stat.b.isol || stat.b.f || stat.b.od || stat.b.eed || stat.b.ead || stat.b.odbps ||
			// pf.b.majf || pf.b.minf || pf.b.mcf || pf.b.dlsf || pf.b.dcs1f || pf.b.dcs2f ||
			// ff.b.unl || ff.b.lock || pf.b.obsc || pf.b.obso
			if ((pDcu->a.t.s.stat.a & 0xfa) == 0 && (pDcu->a.t.s.pf.a & 0x3f) == 0 && (pDcu->a.t.s.ff.a & 0xf) == 0) {
				pLsv->dcucf.mon[cid][m].stat.b.vain = false;
				pLsv->dcucf.mon[cid][m].stat.b.cci = pDcu->a.t.s.stat.b.fullo;
			}

			pLsv->dcucf.cLines[cid][m] &= ~(1 << DEVSTATE_BLINDUNIT);
			if (pDcu->a.t.s.stat.b.isol)	pLsv->dcucf.cLines[cid][m] |= (1 << DEVSTATE_BYPASS);
			else	pLsv->dcucf.cLines[cid][m] &= ~(1 << DEVSTATE_BYPASS);
			// 171019
			//if (pDcu->a.t.s.stat.b.f || pDcu->a.t.s.pf.b.majf)	pLsv->dcucf.cLines[cid][m] |= (1 << DEVSTATE_FAIL);
			//else	pLsv->dcucf.cLines[cid][m] &= ~(1 << DEVSTATE_FAIL);
			if (pDcu->a.t.s.stat.b.f)	pLsv->dcucf.cLines[cid][m] |= (1 << DEVSTATE_FAIL);
			else	pLsv->dcucf.cLines[cid][m] &= ~(1 << DEVSTATE_FAIL);
			if (pDcu->a.t.s.stat.b.od)	pLsv->dcucf.cLines[cid][m] |= (1 << DOORSTATE_OBSTACLE);
			else	pLsv->dcucf.cLines[cid][m] &= ~(1 << DOORSTATE_OBSTACLE);
			if (pDcu->a.t.s.stat.b.eed || pDcu->a.t.s.stat.b.ead)	pLsv->dcucf.cLines[cid][m] |= (1 << DOORSTATE_EMERGENCYHANDLE);
			else	pLsv->dcucf.cLines[cid][m] &= ~(1 << DOORSTATE_EMERGENCYHANDLE);
			if (pDcu->a.t.s.pf.b.majf)	pLsv->dcucf.cLines[cid][m] |= (1 << DOORSTATE_MAJF);
			else	pLsv->dcucf.cLines[cid][m] &= ~(1 << DOORSTATE_MAJF);

			if (!pDcu->a.t.s.ref.b.dok || pDcu->a.t.s.stat.b.f || pDcu->a.t.s.pf.b.majf)	pLsv->dcucf.wDefectMap[cid] |= (1 << m);
			else	pLsv->dcucf.wDefectMap[cid] &= ~(1 << m);

			pArch->Cut(cid, 900 + m);
			pArch->Prompt(pDcu->a.t.s.stat.b.od, cid, 884 + m);
			pArch->Prompt(pDcu->a.t.s.stat.b.isol, cid, 892 + m);
			pArch->Prompt(pDcu->a.t.s.stat.b.f,  cid, 908 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.dcu2r, cid, 856 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.dcu2f, cid, 864 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.dcs2f, cid, 916 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.dcs1f, cid, 924 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.dlsf, cid, 932 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.mcf,  cid, 940 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.minf, cid, 948 + m);
			pArch->Prompt(pDcu->a.t.s.pf.b.majf, cid, 956 + m);
			pArch->Prompt(pDcu->a.t.s.ff.b.obso, cid, 964 + m);
			pArch->Prompt(pDcu->a.t.s.ff.b.obsc, cid, 972 + m);
			pArch->Prompt(pDcu->a.t.s.ff.b.lock, cid, 980 + m);
			pArch->Prompt(pDcu->a.t.s.ff.b.unl,  cid, 988 + m);
		}
		else {
			pLsv->dcucf.cLines[cid][m] |= (1 << DEVSTATE_BLINDUNIT);
			if (pProse->GetWarmup(DEVID_DCU) == 0) {
				INCWORD(pLsv->wReceivedRed[DEVID_DCUL + m][cid]);
				//GETPAPER(pPaper);
				//if (!pPaper->GetLabFromRecip())
					pArch->Shot(cid, 900 + m);		// ??????????
			}
		}
	}
	//else {
	// 출입문 통신이 없으므로 CProse에 있는 배열을 지워야하는데 주소를 모르니 어디를 지워야하는지 알 수가 없다...
	//}
}

void CUniv::CheckDoorMode(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	PDOZEN pDoz = pPaper->GetDozen();
	BYTE dm;
	if (GETTI(FID_HEAD, TUDIB_DMAA))	dm = DOORMODE_AOAC;
	else if (GETTI(FID_HEAD, TUDIB_DMAM))	dm = DOORMODE_AOMC;
	else if (GETTI(FID_HEAD, TUDIB_DMMM))	dm = DOORMODE_MOMC;
	else	dm = DOORMODE_NON;
	if (c_dm.d.Stabilize(dm))	c_dm.mode = c_dm.d.cur;

	GETARCHIVE(pArch);
	if (c_dm.mode != c_dm.verf) {
		c_dm.verf = c_dm.mode;
		pArch->Cut(CID_BOW, 872, 874);
		if (c_dm.mode == DOORMODE_AOAC)	pArch->Shot(CID_BOW, 872);
		else if (c_dm.mode == DOORMODE_AOMC)	pArch->Shot(CID_BOW, 873);
		else if (c_dm.mode == DOORMODE_MOMC)	pArch->Shot(CID_BOW, 874);
	}

	if (GETTI(FID_HEAD, TUDIB_PDS))		pArch->Shot(CID_BOW, 875);
	else	pArch->Cut(CID_BOW, 875);

	if (GETTI(FID_HEAD, TUDIB_PLDO))	pArch->Shot(CID_BOW, 876);
	else	pArch->Cut(CID_BOW, 876);

	if (GETTI(FID_HEAD, TUDIB_PRDO))	pArch->Shot(CID_BOW, 877);
	else	pArch->Cut(CID_BOW, 877);

	if (GETTI(FID_HEAD, TUDIB_DCS))		pArch->Shot(CID_BOW, 878);
	else	pArch->Cut(CID_BOW, 878);

	if (GETTI(FID_HEAD, TUDIB_ADBS))	pArch->Shot(CID_BOW, 881);
	else	pArch->Cut(CID_BOW, 881);
}

void CUniv::CheckDoorCmd()
{
}

void CUniv::CheckDoorEach()
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	GETARCHIVE(pArch);
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		//for (UCURV m = 0; m < 8; m ++)	pProse->c_dcus[n][m].r.cCtrl = 0;
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		// 180911
		//if (pPaper->GetRouteState(fid)) {
		if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
			for (int m = 0; m < 8; m ++) {
				pLsv->dcucf.cLines[n][m] &= ~(1 << DEVSTATE_BLINDDTB);
				pLsv->dcucf.mon[n][m].stat.b.vain = true;
			}

			for (int m = 0; m < 4; m ++) {
				if (GETCI(fid, (CUDIB_LD0 + m))) {
					pLsv->dcucf.cLines[n][m] |= (1 << DOORSTATE_OPEN);
					pLsv->dcucf.mon[n][m].stat.b.cdi = true;
				}
				else {
					pLsv->dcucf.cLines[n][m] &= ~(1 << DOORSTATE_OPEN);
					pLsv->dcucf.mon[n][m].stat.b.cdi = false;
				}
				if (GETCI(fid, (CUDIB_RD0 + m))) {
					pLsv->dcucf.cLines[n][m + 4] |= (1 << DOORSTATE_OPEN);
					pLsv->dcucf.mon[n][m + 4].stat.b.cdi = true;
				}
				else {
					pLsv->dcucf.cLines[n][m + 4] &= ~(1 << DOORSTATE_OPEN);
					pLsv->dcucf.mon[n][m + 4].stat.b.cdi = false;
				}
			}

			// 170922
			//bool bInv = pDoz->recip[fid].real.prol.cProperID == PID_RIGHT ? true : false;
			CheckDoorLine(n, &pDoz->recip[fid].real.dcul);
			CheckDoorLine(n, &pDoz->recip[fid].real.dcur);

			// 200218
			// 171213
			//if (pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab) {
			if (pPaper->GetLabFromRecip()) {
				if (!GETTI(FID_HEAD, TUDIB_ADBS) && !GETCI(fid, CUDIB_EED) && !GETCI(fid, CUDIB_EAD)) {
					// check di & sda, when not fault (from DI)
					for (int m = 0; m < 8; m ++) {
						if (!pLsv->dcucf.mon[n][m].stat.b.vain) {
							// check, when not fault (from SD)
							// 1st. check DI (previous & current)
							if (pLsv->dcucf.mon[n][m].stat.b.pdi != pLsv->dcucf.mon[n][m].stat.b.cdi) {
								pLsv->dcucf.mon[n][m].stat.b.pdi = pLsv->dcucf.mon[n][m].stat.b.cdi;
								pLsv->dcucf.mon[n][m].stat.b.xdi = true;
							}
							else	pLsv->dcucf.mon[n][m].stat.b.xdi = false;
							// 2nd. check SD (previous & current)
							if (pLsv->dcucf.mon[n][m].stat.b.pci != pLsv->dcucf.mon[n][m].stat.b.cci) {
								pLsv->dcucf.mon[n][m].stat.b.pci = pLsv->dcucf.mon[n][m].stat.b.cci;
								pLsv->dcucf.mon[n][m].stat.b.xci = true;
							}
							else	pLsv->dcucf.mon[n][m].stat.b.xci = false;

							if (pLsv->dcucf.mon[n][m].stat.b.xdi || pLsv->dcucf.mon[n][m].stat.b.xci) {
								if (pLsv->dcucf.mon[n][m].stat.b.cdi != pLsv->dcucf.mon[n][m].stat.b.cci) {
									pLsv->dcucf.mon[n][m].stat.b.xch = true;
									pLsv->dcucf.mon[n][m].wXchTimer = TIME_DCUXCHSKIP;
								}
								else {
									pLsv->dcucf.mon[n][m].stat.b.xch = false;
									pLsv->dcucf.mon[n][m].wXchTimer = 0;
									pArch->Cut(n, 582 + m);
									pArch->Cut(n, 590 + m);
								}
							}
							else if (pLsv->dcucf.mon[n][m].stat.b.xch) {
								if (pLsv->dcucf.mon[n][m].wXchTimer > 0 && -- pLsv->dcucf.mon[n][m].wXchTimer == 0)
									pArch->Shot(n, pLsv->dcucf.mon[n][m].stat.b.cdi ? 582 + m : 590 + m);
							}
						}
						else {
							pLsv->dcucf.mon[n][m].stat.b.xch = false;
							pLsv->dcucf.mon[n][m].wXchTimer = 0;
						}
					}
				}
			}
			if (!GETCI(fid, CUDIB_DIS))	pArch->Shot(n, 996);
			else	pArch->Cut(n, 996);
			if (GETCI(fid, CUDIB_EED))	pArch->Shot(n, 997);
			else	pArch->Cut(n, 997);
			if (GETCI(fid, CUDIB_EAD))	pArch->Shot(n, 998);
			else	pArch->Cut(n, 998);
			if (GETCI(fid, CUDIB_EDF))	pArch->Shot(n, 999);
			else	pArch->Cut(n, 999);
		}
		else {
			for (int m = 0; m < 8; m ++) {
				pLsv->dcucf.cLines[n][m] |= (1 << DEVSTATE_BLINDDTB);
				//if (pProse->c_dcus[n][m].r.cCtrl == 0)
				memset(&pLsv->dcucf.fl[n][m], 0, sizeof(DCUFLAP));
			}
		}	// if (pPaper->GetRouteState(fid))
	}
}

void CUniv::Monitor(PRECIP pRecip)
{
	GETPAPER(pPaper);
	CUCUTOFF();
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		CheckDoorMode(pRecip);
		CheckDoorEach();
		CheckV3f();
		// 200218
		//CheckEcu(pRecip);
		if (!pPaper->GetDeviceExFromRecip())	CheckEcu(pRecip);
		else	CheckEcu2(pRecip);
		CheckHvac();
		// 200218
		GETPROSE(pProse);
		CProse::PLSHARE pLsv = pProse->GetShareVars();

		if (!pPaper->GetDeviceExFromRecip())	pRecip->real.sign.ecmd.b.fire = CheckFire();	// for pau, pis
		else {
			CheckFire2();
			pRecip->real.sign.ecmd.b.fire = pLsv->fire.bAck;
		}
		CheckPassenger();
		//CheckGeneral(pRecip);
		// 201016
		GETSCHEME(pSch);
		GETARCHIVE(pArch);
		if (pPaper->GetDeviceExFromRecip()) {
			for (int n = CID_CAR1; n < CID_CAR5; n ++) {
				int fid = pSch->C2F(pPaper->GetTenor(), n);
				if (!GETCI(fid, CUDIB_UBCF))	pArch->Shot(n, 772);	//"무정전 방송 조명장치 고장"
				else	pArch->Cut(n, 772);
			}
		}
		// 200218
		for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
			int fid = pSch->C2F(pPaper->GetTenor(), n);
			if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
				if (pDoz->recip[fid].real.localf.cDevID < REALDEV_MAX)
					pLsv->wReceivedRed[pDoz->recip[fid].real.localf.cDevID][n] = pDoz->recip[fid].real.localf.wCnt;
			}
		}
	}
}

void CUniv::Control(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		CFio* pFio = pLand->GetFio();
		bool bCmd = (pDoz->recip[FID_HEAD].real.sign.ecmd.b.ac1 || pDoz->recip[FID_HEAD].real.sign.ecmd.b.ac2) ? true : false;
		pFio->SetOutput(UDOB_ACL1, bCmd);
		pFio->SetOutput(UDOB_ACL2, bCmd);
		pFio->SetOutput(UDOB_DCL, pDoz->recip[FID_HEAD].real.sign.ecmd.b.dc);
		pFio->SetOutput(UDOB_ALBO, pDoz->recip[FID_HEAD].real.sign.ecmd.b.albo);
	}

	if (ISTU() && pPaper->IsHead()) {	//(pPaper->GetOblige() & (1 << OBLIGE_HEAD))) {
		GETREVIEW(pRev);
		WORD wItem = pRev->GetInspectItem();
		WORD wStep = pRev->GetInspectStep();
		pRecip->real.sign.insp.b.v3f = pRecip->real.sign.insp.b.ecub = pRecip->real.sign.insp.b.ecua =
		pRecip->real.sign.insp.b.hvac = pRecip->real.sign.insp.b.horc = false;
		if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED)) && wItem != 0) {
			GETSCHEME(pSch);
			PDEVCONF pConf = pSch->GetDevConf();
			GETPROSE(pProse);
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
			GETARCHIVE(pArch);
			PINSPREPORT pInsp = pArch->GetInspectReport();
			int nPage = 0;
			switch (wStep) {
			case INSPSTEP_LOCALJUDGE :	case INSPSTEP_ECUB0JUDGE :	case INSPSTEP_ECUB7JUDGE :	case INSPSTEP_ECUEBJUDGE :
			case INSPSTEP_ECUASJUDGE :	case INSPSTEP_COOLJUDGE :	case INSPSTEP_HEATJUDGE :
				if (pRev->GetInspectLapse())	nPage = pRev->NextInspectStep();
				break;
			case INSPSTEP_LOCALREADY : {
					pRev->CoordDuo(&pInsp->ics.wTcRes, 1, 0);		// "시험 진행 중"
					pRev->CoordDuo(&pInsp->ics.wTcRes, 3, 2);		// ""
					//pInsp->ics.wSvf = 0xffff;						// "시험"
					//pInsp->ics.wEcu = 0xffff;
					//pInsp->ics.wHvac = 0xffff;
					//pInsp->ics.wEhv = 0xff00;
					// 200218
					//pInsp->ics.wExDev &= 0xf00;
					//pInsp->ics.wExDev |= 0xf0ff;
					switch (pPaper->GetLength()) {
					case 4 :
						pInsp->ics.wSvf = XWORD(0x3c0f);
						pInsp->ics.wEcu = XWORD(0x3fc0);
						pInsp->ics.wHvac = XWORD(0x3fc0);
						pInsp->ics.wEhv = XWORD(0);
						pInsp->ics.wExDev &= XWORD(0xf);
						pInsp->ics.wExDev |= XWORD(0x3cf0);
						break;
					case 6 :
						pInsp->ics.wSvf = XWORD(0x3c3f);
						pInsp->ics.wEcu = XWORD(0xfff0);
						pInsp->ics.wHvac = XWORD(0xfff0);
						pInsp->ics.wEhv = XWORD(0);
						pInsp->ics.wExDev &= XWORD(0xf);
						pInsp->ics.wExDev |= XWORD(0x3cf0);
						break;
					case 8 :
						pInsp->ics.wSvf = XWORD(0x3cff);
						pInsp->ics.wEcu = XWORD(0xfffc);
						pInsp->ics.wHvac = XWORD(0xfffc);
						pInsp->ics.wEhv = XWORD(0x33);
						pInsp->ics.wExDev &= XWORD(0xf);
						pInsp->ics.wExDev |= XWORD(0x3cf0);
						break;
					default :
						pInsp->ics.wSvf = XWORD(0xffff);
						pInsp->ics.wEcu = XWORD(0xffff);
						pInsp->ics.wHvac = XWORD(0xffff);
						pInsp->ics.wEhv = XWORD(0xff);
						pInsp->ics.wExDev &= XWORD(0xf);
						pInsp->ics.wExDev |= XWORD(0xfff0);
						break;
					}
					for (int n = 0; n < 2; n ++)
						pLfc->wPis[n] = pLfc->wPau[n] = pLfc->wTrs[n] = pLfc->wRtd[n] = 0;
					// 19/12/05
					pInsp->wMsg = 0;
					pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_LOCALCHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);		// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse()) {
					pRev->NextInspectStep();		// 미리 스텝을 바꿔놓으면 카운터 값이 바뀌지 않는다!!!
					// 200218
					for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
						pRev->CoordDuo(&pInsp->ics.wSvf, pLfc->wSiv[n] > INSPLINE_FCOUNT ? 2 : 1, (n + 1) * 2);	// 1:OK/2:NG
						if (pPaper->GetDeviceExFromRecip()) {
							pRev->CoordDuo(&pInsp->ics.wExDev, pLfc->wCmsb[n] > INSPLINE_FCOUNT ? 2 : 1, n < 2 ? (n + 6) * 2 : n * 2);
							pRev->CoordDuo(&pInsp->ics.wExDev, pLfc->wBms[n] > INSPLINE_FCOUNT ? 2 : 1, (n + 1) * 2);
						}
					}
					for (int n = 0; n < (int)pConf->v3f.cLength; n ++)
						pRev->CoordDuo(&pInsp->ics.wSvf, pLfc->wV3f[n] > INSPLINE_FCOUNT ? 2 : 1, (n ^ 4) * 2);	// 1:OK/2:NG
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if ((n + hofs) < 8) {
							pRev->CoordDuo(&pInsp->ics.wEcu, pLfc->wEcu[n] > INSPLINE_FCOUNT ? 2 : 1, ((n + hofs) ^ 4) * 2);	// 1:OK/2:NG
							pRev->CoordDuo(&pInsp->ics.wHvac, pLfc->wHvac[n] > INSPLINE_FCOUNT ? 2 : 1, ((n + hofs) ^ 4) * 2);	// 1:OK/2:NG
						}
						else {
							pRev->CoordDuo(&pInsp->ics.wEhv, pLfc->wEcu[n] > INSPLINE_FCOUNT ? 2 : 1, ((n + hofs) - 8) * 2 + 8);
							pRev->CoordDuo(&pInsp->ics.wEhv, pLfc->wHvac[n] > INSPLINE_FCOUNT ? 2 : 1, ((n + hofs) - 8) * 2 + 12);
						}
					}
					pRev->CoordDuo(&pInsp->ics.wTcRes, 2, 0);			// end
					// 201029
					//if (((pInsp->ics.wSvf & 0xaaaa) | (pInsp->ics.wEcu & 0xaaaa) | (pInsp->ics.wHvac & 0xaaaa) | (pInsp->ics.wEhv & 0xaaaa)) != 0) {
					if (((pInsp->ics.wSvf & 0xaaaa) | (pInsp->ics.wEcu & 0xaaaa) |
						(pInsp->ics.wHvac & 0xaaaa) | (pInsp->ics.wEhv & 0xaaaa) |
						(pInsp->ics.wExDev & 0xaaaa)) != 0) {
						pRev->CoordDuo(&pInsp->ics.wTcRes, 2, 2);
						pRev->InspectPause();
					}
					else	pRev->CoordDuo(&pInsp->ics.wTcRes, 1, 2);	// 1:OK/2:NG
				}
				break;
			case INSPSTEP_ECUB0READY :	case INSPSTEP_ECUB7READY :
			case INSPSTEP_ECUEBREADY :	case INSPSTEP_ECUASREADY : {
					switch (wStep) {
					case INSPSTEP_ECUB0READY :	pInsp->wEcui = 0;	break;
					case INSPSTEP_ECUB7READY :	pInsp->wEcui = 1;	break;
					case INSPSTEP_ECUEBREADY :	pInsp->wEcui = 2;	break;
					default :					pInsp->wEcui = 3;	break;
					}
					pRev->CoordDuo(&pInsp->ecu[pInsp->wEcui].wEcuRes, 3, 8);	// ""
					pRev->CoordDuo(&pInsp->ecu[pInsp->wEcui].wEcuRes, 3, 10);	// ""
					pInsp->ecu[pInsp->wEcui].wEcuL = 0;							// box
					pInsp->ecu[pInsp->wEcui].wEcuH = 0;

					switch (wStep) {
					case INSPSTEP_ECUB0READY :
						pInsp->wSubItem = XWORD(10);	// "B0 시험"
						pInsp->wMsg = XWORD(25);		// "주간 제어기를 중립에 두시고 보안 제동을 완해하시오."
						break;
					case INSPSTEP_ECUB7READY :
						pInsp->wSubItem = XWORD(17);	// "B7 시험"
						pInsp->wMsg = XWORD(32);		// "주간 제어기를 B7에 두시오."
						break;
					case INSPSTEP_ECUEBREADY :
						pInsp->wSubItem = XWORD(18);	// "EB 시험"
						pInsp->wMsg = XWORD(33);		// "주간 제어기를 EB에 두시오."
						break;
					default :
						pInsp->wSubItem = XWORD(19);	// "안티 스키드 시험"
						pInsp->wMsg = XWORD(34);		// "주간 제어기를 B7에 두시고 보안 제동을 체결하시오."
					}

					for (int n = 0; n < 10; n ++) {
						pInsp->ecu[pInsp->wEcui].wBcp[n] = pInsp->ecu[pInsp->wEcui].wPwme[n] = c_inspAid.wSeq[n] = 0;
						c_inspAid.bResult1[n] = c_inspAid.bResult2[n] = false;
					}
					c_inspAid.bExit = false;
					pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_ECUB0ENTRY :	case INSPSTEP_ECUB7ENTRY :
			case INSPSTEP_ECUEBENTRY :	case INSPSTEP_ECUASENTRY :
				if ((wStep == INSPSTEP_ECUB0ENTRY && pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_C && !(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY))) ||
					(wStep == INSPSTEP_ECUB7ENTRY && pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_B7 && !(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY))) ||
					(wStep == INSPSTEP_ECUEBENTRY && pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_EB &&
							(pDoz->recip[FID_HEAD].real.drift.cEbrk & (1 << EBCAUSE_MC)) && !(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY))) ||
					(wStep == INSPSTEP_ECUASENTRY && pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_B7 && (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)))) {
					pInsp->wMsg = 0;
					pRev->CoordDuo(&pInsp->ecu[pInsp->wEcui].wEcuRes, 1, 8);	// "시험 중"
					// 201029
					// c_inspAid.qu[0].dw = 0xfffff;							// "시험"
					switch (pPaper->GetLength()) {
					case 4 :	c_inspAid.qu[0].dw = 0x3fc0;	break;
					case 6 :	c_inspAid.qu[0].dw = 0xfff0;	break;
					case 8 :	c_inspAid.qu[0].dw = 0x3fffc;	break;
					default :	c_inspAid.qu[0].dw = 0xfffff;	break;
					}
					pInsp->ecu[pInsp->wEcui].wEcuL = XWORD(c_inspAid.qu[0].w[0]);
					pInsp->ecu[pInsp->wEcui].wEcuH = XWORD(c_inspAid.qu[0].w[1]);
					pRev->NextInspectStep();
					if (wStep == INSPSTEP_ECUASENTRY)	pRecip->real.sign.insp.b.ecua = true;
					else	pRecip->real.sign.insp.b.ecub = true;
				}
				break;
			case INSPSTEP_ECUB0CHECK :	case INSPSTEP_ECUB7CHECK :
			case INSPSTEP_ECUEBCHECK :	case INSPSTEP_ECUASCHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);		// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse())	c_inspAid.bExit = true;
				else {
					if (wStep == INSPSTEP_ECUASCHECK)	pRecip->real.sign.insp.b.ecua = true;
					else	pRecip->real.sign.insp.b.ecub = true;
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (!IsFreeze(pLsv->ecucf.cLines[n])) {
							int fid = pSch->C2F(pPaper->GetTenor(), n);
							// 200218
							//double db = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.a.t.s.cBcp);
							double db;
							if (pPaper->GetDeviceExFromRecip())
								db = pSch->KPaToKgcm2((WORD)pDoz->recip[fid].real.ecu.ct.a.t.s.cBcp * 4);
							else	db = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.co.a.t.s.cBcp);
							WORD wBc = (WORD)(db * 10.f);
							pInsp->ecu[pInsp->wEcui].wBcp[n + hofs] = XWORD(wBc);
							// 200218
							//WORD wBe = pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.a.t.s.cBe, (WORD)0, (WORD)255, (WORD)0, (WORD)100);
							WORD wBe;
							if (pPaper->GetDeviceExFromRecip())
								wBe = (WORD)((double)pDoz->recip[fid].real.ecu.ct.a.t.s.cBe * 0.4f);
							else	wBe = (WORD)pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.co.a.t.s.cBe,
																		(WORD)0, (WORD)255, (WORD)0, (WORD)100);
							pInsp->ecu[pInsp->wEcui].wPwme[n + hofs] = XWORD(wBe);

							// 200218
							//bool bTesting = wStep == INSPSTEP_ECUASCHECK ? pDoz->recip[fid].real.ecu.a.t.s.evt.b.ast : pDoz->recip[fid].real.ecu.a.t.s.evt.b.bt;
							bool bTesting = false;
							if (pPaper->GetDeviceExFromRecip())
								bTesting = wStep == INSPSTEP_ECUASCHECK ?
										pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.ast : pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.bt;
							else	bTesting = wStep == INSPSTEP_ECUASCHECK ?
										pDoz->recip[fid].real.ecu.co.a.t.s.evt.b.ast : pDoz->recip[fid].real.ecu.co.a.t.s.evt.b.bt;
							switch (c_inspAid.wSeq[n]) {
							case 0 :
								if (bTesting)	c_inspAid.wSeq[n] = 1;
								break;
							case 1 :
								if (!bTesting) {
									c_inspAid.wSeq[n] = 2;
									// 200218
									//c_inspAid.bResult1[n] = wStep == INSPSTEP_ECUASCHECK ? pDoz->recip[fid].real.ecu.a.t.s.evt.b.astr : pDoz->recip[fid].real.ecu.a.t.s.evt.b.btr;
									if (pPaper->GetDeviceExFromRecip())
										c_inspAid.bResult1[n] = wStep == INSPSTEP_ECUASCHECK ? pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.astr : pDoz->recip[fid].real.ecu.ct.a.t.s.evt.b.btr;
									else	c_inspAid.bResult1[n] = wStep == INSPSTEP_ECUASCHECK ? pDoz->recip[fid].real.ecu.co.a.t.s.evt.b.astr : pDoz->recip[fid].real.ecu.co.a.t.s.evt.b.btr;
									pRev->CoordDuo(&c_inspAid.qu[0].dw, c_inspAid.bResult1[n] ? (DWORD)1 : (DWORD)2, (n + hofs) << 1);
									pInsp->ecu[pInsp->wEcui].wEcuL = XWORD(c_inspAid.qu[0].w[0]);
									pInsp->ecu[pInsp->wEcui].wEcuH = XWORD(c_inspAid.qu[0].w[1]);
								}
								break;
							default :
								break;
							}
						}
					}
					bool bExit = true;
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (c_inspAid.wSeq[n] != 2)	bExit = false;
					}
					if (bExit)	c_inspAid.bExit = bExit;
				}
				if (c_inspAid.bExit) {
					pRev->CoordDuo(&pInsp->ecu[pInsp->wEcui].wEcuRes, 2, 8);	// "결과"
					bool bGood = true;
					c_inspAid.qu[0].dw = 0;
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (!c_inspAid.bResult1[n]) {
							bGood = false;
							pRev->CoordDuo(&c_inspAid.qu[0].dw, (DWORD)2, (n + hofs) << 1);		// "고장"
						}
						else	pRev->CoordDuo(&c_inspAid.qu[0].dw, (DWORD)1, (n + hofs) << 1);	// "정상"
					}
					pInsp->ecu[pInsp->wEcui].wEcuL = XWORD(c_inspAid.qu[0].w[0]);
					pInsp->ecu[pInsp->wEcui].wEcuH = XWORD(c_inspAid.qu[0].w[1]);
					pRev->CoordDuo(&pInsp->ecu[pInsp->wEcui].wEcuRes, bGood ? 1 : 2, 10);		// 1:"정상/2:"고장"
					if (bGood)	pRev->NextInspectStep();
					else	pRev->InspectPause();
				}
				break;
			case INSPSTEP_V3FREADY : {
					pRev->CoordDuo(&pInsp->ics.wV3fRes, 3, 8);	// ""
					pRev->CoordDuo(&pInsp->ics.wV3fRes, 3, 10);	// ""
					pInsp->wSubItem = 0;
					//pInsp->wMsg = XWORD(35);						// "보안 제동을 체결하고 주차 제동을 완해하시오."
					//pInsp->wMsg = XWORD(38);						// 170928, "보안 제동을 체결하시오."
					pInsp->wMsg = XWORD(36);						// "보안 제동과 주차 제동을 체결하시오."
					pInsp->ics.wHb = pInsp->ics.wLb = 0;			// box
					pInsp->ics.wCpb = 0x5501;						// ""
					pInsp->ics.wV3f = 0;
					for (int n = 0; n < V3F_MAX; n ++) {
						pInsp->ics.wPwmv[n] = pInsp->ics.wFc[n] = c_inspAid.wSeq[n] = 0;
						c_inspAid.bResult1[n] = c_inspAid.bResult2[n] = false;
					}
					c_inspAid.bExit = false;
					pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_V3FIGNITE :
				if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) &&
					(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_PARKING))) {
					pInsp->wMsg = XWORD(21);						// "주간 제어기를 P1에 두시오."
					c_wP1DelayTimer = TIME_P1DELAY;
					pRev->NextInspectStep();
				}
				break;
			// 180103
			//case INSPSTEP_V3FREADYB :
			//	if (!(pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_PARKING))) {
			//		pInsp->wMsg = XWORD(21);						// "주간 제어기를 P1에 두시오."
			//		pRev->NextInspectStep();
			//	}
			//	break;
			case INSPSTEP_V3FENTRY :
				// 190108
				//if (pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_P1) {
				if (pDoz->recip[FID_HEAD].real.motiv.cEffType != EFFORT_BRAKING &&
					pDoz->recip[FID_HEAD].real.motiv.cEffort > PWMRATE_MINP1) {
					if (-- c_wP1DelayTimer == 0) {
						pRev->CoordDuo(&pInsp->ics.wV3fRes, 1, 8);	// "시험 진행 중"
						c_inspAid.qu[0].dw = 0x3ff;					// "시험"
						pInsp->ics.wV3f = XWORD(c_inspAid.qu[0].w[0]);
						pInsp->wMsg = 0;
						pRev->NextInspectStep();
						pRecip->real.sign.insp.b.v3f = true;
					}
				}
				else	c_wP1DelayTimer = TIME_P1DELAY;
				break;
			case INSPSTEP_V3FCHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);		// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse())	c_inspAid.bExit = true;
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)	c_inspAid.bExit = true;
				else {
					pRecip->real.sign.insp.b.v3f = true;
					WORD wHb, wLb, wCpb;
					wHb = wLb = 0x155;						// "OFF"
					wCpb = 0;								//
					for (int n = 0; n < (int)pConf->v3f.cLength; n ++) {
						int fid = pSch->C2F(pPaper->GetTenor(), pConf->v3f.cPos[n]);
						if (pDoz->recip[fid].real.svf.v.a.t.s.det.b.hb)	pRev->CoordDuo(&wHb, 2, n << 1);	// "ON"
						if (pDoz->recip[fid].real.svf.v.a.t.s.det.b.lb1 ||
							pDoz->recip[fid].real.svf.v.a.t.s.det.b.lb2)	pRev->CoordDuo(&wLb, 2, n << 1);	// "ON"
						WORD w = 0;
						if (pDoz->recip[fid].real.svf.v.a.t.s.cmd.b.p)	w = 2;
						else if (pDoz->recip[fid].real.svf.v.a.t.s.cmd.b.b)	w = 3;
						pRev->CoordDuo(&wCpb, w, n << 1);
						pInsp->ics.wPwmv[n] = XWORD((WORD)pDoz->recip[fid].real.svf.v.a.t.s.cTbe);
						pInsp->ics.wFc[n] = pDoz->recip[fid].real.svf.v.a.t.s.wFc;		// big-endian 값이다.

						switch (c_inspAid.wSeq[n]) {
						case 0 :
							if (pDoz->recip[fid].real.svf.v.a.t.s.tst.b.st == 1)	c_inspAid.wSeq[n] = 1;
							break;
						case 1 :
							if (pDoz->recip[fid].real.svf.v.a.t.s.tst.b.st == 2) {
								c_inspAid.wSeq[n] = 2;
								c_inspAid.bResult1[n] = pDoz->recip[fid].real.svf.v.a.t.s.tst.b.ok;
								pRev->CoordDuo(&c_inspAid.qu[0].w[0], c_inspAid.bResult1[n] ? (WORD)1 : (WORD)2, n << 1);
								pInsp->ics.wV3f = XWORD(c_inspAid.qu[0].w[0]);
							}
							break;
						default :	break;
						}
					}
					pInsp->ics.wHb = XWORD(wHb);
					pInsp->ics.wLb = XWORD(wLb);
					pInsp->ics.wCpb = XWORD(wCpb);

					bool bExit = true;
					for (int n = 0; n < (int)pConf->v3f.cLength; n ++) {
						if (c_inspAid.wSeq[n] != 2)	bExit = false;
					}
					if (bExit)	c_inspAid.bExit = bExit;
				}
				if (c_inspAid.bExit) {
					pRecip->real.sign.insp.b.v3f = false;
					bool bGood = true;
					c_inspAid.qu[0].dw = 0;
					for (int n = 0; n < (int)pConf->v3f.cLength; n ++) {
						if (!c_inspAid.bResult1[n]) {
							bGood = false;
							pRev->CoordDuo(&c_inspAid.qu[0].w[0], (WORD)2, n << 1);		// "고장"
						}
						else	pRev->CoordDuo(&c_inspAid.qu[0].w[0], (WORD)1, n << 1);	// "정상"
					}
					pInsp->ics.wV3f = XWORD(c_inspAid.qu[0].w[0]);
					pRev->CoordDuo(&pInsp->ics.wV3fRes, 2, 8);				// "시험 완료"
					pRev->CoordDuo(&pInsp->ics.wV3fRes, bGood ? 1 : 2, 10);	// 1:"정상"/2:"고장"
					if (bGood) {
						pInsp->wMsg = XWORD(32);					// "주간 제어기를 B7에 두시오."
						pRev->NextInspectStep();
					}
					else	pRev->InspectPause();
				}
				break;
			case INSPSTEP_V3FJUDGE :
				if (pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_B7) {
					pInsp->wMsg = 0;
					if (pRev->GetInspectLapse())	nPage = pRev->NextInspectStep();
					//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)	nPage = pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_COOLREADY : {
					pRev->CoordDuo(&pInsp->ics.wCoolRes, 3, 8);	// ""
					pRev->CoordDuo(&pInsp->ics.wCoolRes, 3, 10);	// ""
					pInsp->ics.wCool1 = pInsp->ics.wCool2 = pInsp->ics.wCool12 = 0;	// box
					pInsp->wSubItem = 0;
					pInsp->wMsg = XWORD(37);					// "HVAC 선택 스위치를 '시험'으로 놓으시오."
					for (int n = 0; n < HVAC_MAX; n ++) {
						c_inspAid.wSeq[n] = 0;
						c_inspAid.bResult1[n] = c_inspAid.bResult2[n] = false;
					}
					c_inspAid.bExit = false;
					pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_COOLENTRY : {
					BYTE ci = (GETTIS(FID_HEAD, TUDIB_CHCS1) & 0xf);
					if (ci == 1) {
						pInsp->wMsg = 0;
						pRev->CoordDuo(&pInsp->ics.wCoolRes, 1, 8);	// "시험 진행 중"
						// 201029
						//c_inspAid.qu[0].dw = c_inspAid.qu[1].dw = 0xfffff;	// "시험"
						switch (pPaper->GetLength()) {
						case 4 :	c_inspAid.qu[0].dw = c_inspAid.qu[1].dw = 0x3fc0;	break;
						case 6 :	c_inspAid.qu[0].dw = c_inspAid.qu[1].dw = 0xfff0;	break;
						case 8 :	c_inspAid.qu[0].dw = c_inspAid.qu[1].dw = 0x3fffc;	break;
						default :	c_inspAid.qu[0].dw = c_inspAid.qu[1].dw = 0xfffff;	break;
						}
						pInsp->ics.wCool1 = XWORD(c_inspAid.qu[0].w[0]);
						pInsp->ics.wCool2 = XWORD(c_inspAid.qu[1].w[0]);
						WORD w = (c_inspAid.qu[0].w[1] & 0xf) | ((c_inspAid.qu[1].w[1] & 0xf) << 4);
						pInsp->ics.wCool12 = XWORD(w);
						pRev->NextInspectStep();
						pRecip->real.sign.insp.b.hvac = true;
						pRecip->real.sign.insp.b.horc = false;
					}
				}
				break;
			case INSPSTEP_COOLCHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);		// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse())	c_inspAid.bExit = true;
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)	c_inspAid.bExit = true;

				//else if (pDoz->recip[FID_HEAD].real.hcb.cb.wSign != SIGN_CMSBFLAP) {
				else {
					pRecip->real.sign.insp.b.hvac = true;
					pRecip->real.sign.insp.b.horc = false;
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						int fid = pSch->C2F(pPaper->GetTenor(), n);
						if (pDoz->recip[fid].real.hcb.cb.wSign != SIGN_CMSBFLAP) {
							switch (c_inspAid.wSeq[n]) {
							case 0 :
								if (pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq == 1)	c_inspAid.wSeq[n] = 1;
								break;
							case 1 :
								if (pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq == 2) {
									c_inspAid.wSeq[n] = 2;
									c_inspAid.bResult1[n] = pDoz->recip[fid].real.hcb.hv.a.t.s.stb.b.u1tok;
									c_inspAid.bResult2[n] = pDoz->recip[fid].real.hcb.hv.a.t.s.stb.b.u2tok;
									pRev->CoordDuo(&c_inspAid.qu[0].dw, c_inspAid.bResult1[n] ? (DWORD)1 : (DWORD)2, (n + hofs) << 1);
									pRev->CoordDuo(&c_inspAid.qu[1].dw, c_inspAid.bResult2[n] ? (DWORD)1 : (DWORD)2, (n + hofs) << 1);
									pInsp->ics.wCool1 = XWORD(c_inspAid.qu[0].w[0]);
									pInsp->ics.wCool2 = XWORD(c_inspAid.qu[1].w[0]);
									WORD w = (c_inspAid.qu[0].w[1] & 0xf) | ((c_inspAid.qu[1].w[1] & 0xf) << 4);
									pInsp->ics.wCool12 = XWORD(w);
								}
								break;
							default :	break;
							}
						}
						// 170817
						//c_inspAid.wSeq[n] = (WORD)pDoz->recip[fid].real.hcb.h.a.t.s.test.b.seq;
					}

					bool bExit = true;
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (c_inspAid.wSeq[n] != 2)	bExit = false;
					}
					if (bExit)	c_inspAid.bExit = bExit;
				}
				if (c_inspAid.bExit) {
					bool bGood = true;
					c_inspAid.qu[0].dw = c_inspAid.qu[1].dw = 0;
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (!c_inspAid.bResult1[n]) {
							bGood = false;
							pRev->CoordDuo(&c_inspAid.qu[0].dw, (DWORD)2, (n + hofs) << 1);		// "고장"
						}
						else	pRev->CoordDuo(&c_inspAid.qu[0].dw, (DWORD)1, (n + hofs) << 1);	// "정상"
						if (!c_inspAid.bResult2[n]) {
							bGood = false;
							pRev->CoordDuo(&c_inspAid.qu[1].dw, (DWORD)2, (n + hofs) << 1);		// "고장"
						}
						else	pRev->CoordDuo(&c_inspAid.qu[1].dw, (DWORD)1, (n + hofs) << 1);	// "정상"
					}
					pInsp->ics.wCool1 = XWORD(c_inspAid.qu[0].w[0]);
					pInsp->ics.wCool2 = XWORD(c_inspAid.qu[1].w[0]);
					WORD w = (c_inspAid.qu[0].w[1] & 0xf) | ((c_inspAid.qu[1].w[1] & 0xf) << 4);
					pInsp->ics.wCool12 = XWORD(w);
					pRev->CoordDuo(&pInsp->ics.wCoolRes, 2, 8);				// "시험 완료"
					pRev->CoordDuo(&pInsp->ics.wCoolRes, bGood ? 1 : 2, 10);	// 1:"정상"/2:"고장"
					if (bGood)	pRev->NextInspectStep();
					else	pRev->InspectPause();
				}
				break;
			case INSPSTEP_HEATREADY : {
					pRev->CoordDuo(&pInsp->ics.wHeatRes, 3, 8);	// ""
					pRev->CoordDuo(&pInsp->ics.wHeatRes, 3, 10);	// ""
					pInsp->ics.wHeatL = pInsp->ics.wHeatH = 0;			// box
					pInsp->wSubItem = 0;
					pInsp->wMsg = XWORD(37);					// "HVAC 선택 스위치를 '시험'으로 놓으시오."
					for (int n = 0; n < HVAC_MAX; n ++) {
						c_inspAid.wSeq[n] = 0;
						c_inspAid.bResult1[n] = c_inspAid.bResult2[n] = false;
					}
					c_inspAid.bExit = false;
					pRev->NextInspectStep();
				}
				break;
			case INSPSTEP_HEATENTRY : {
					BYTE ci = (GETTIS(FID_HEAD, TUDIB_CHCS1) & 0xf);
					if (ci == 1) {
						pInsp->wMsg = 0;
						pRev->CoordDuo(&pInsp->ics.wHeatRes, 1, 8);	// "시험 진행 중"
						// 201029
						//c_inspAid.qu[0].dw = 0xfffff;			// "시험"
						switch (pPaper->GetLength()) {
						case 4 :	c_inspAid.qu[0].dw = 0x3fc0;	break;
						case 6 :	c_inspAid.qu[0].dw = 0xfff0;	break;
						case 8 :	c_inspAid.qu[0].dw = 0x3fffc;	break;
						default :	c_inspAid.qu[0].dw = 0xfffff;	break;
						}
						pInsp->ics.wHeatL = XWORD(c_inspAid.qu[0].w[0]);
						pInsp->ics.wHeatH = XWORD(c_inspAid.qu[0].w[1]);
						pRev->NextInspectStep();
						pRecip->real.sign.insp.b.hvac = true;
						pRecip->real.sign.insp.b.horc = true;
					}
				}
				break;
			case INSPSTEP_HEATCHECK :
				// 19/12/05
				if (!VALIDMRPS()) {
					pRev->InspectPause();
					pInsp->wMsg = XWORD(52);		// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
				}
				else if (pRev->GetInspectLapse())	c_inspAid.bExit = true;
				//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)	c_inspAid.bExit = true;
				//else if (pDoz->recip[FID_HEAD].real.hcb.cb.wSign != SIGN_CMSBFLAP) {
				else {
					pRecip->real.sign.insp.b.hvac = true;
					pRecip->real.sign.insp.b.horc = true;
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						int fid = pSch->C2F(pPaper->GetTenor(), n);
						if (pDoz->recip[fid].real.hcb.cb.wSign != SIGN_CMSBFLAP) {
							switch (c_inspAid.wSeq[n]) {
							case 0 :
								if (pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq == 1)	c_inspAid.wSeq[n] = 1;
								break;
							case 1 :
								if (pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.seq == 2) {
									c_inspAid.wSeq[n] = 2;
									c_inspAid.bResult1[n] = pDoz->recip[fid].real.hcb.hv.a.t.s.test.b.ok;
									pRev->CoordDuo(&c_inspAid.qu[0].dw, c_inspAid.bResult1[n] ? (DWORD)1 : (DWORD)2, (n + hofs) << 1);
									pInsp->ics.wHeatL = XWORD(c_inspAid.qu[0].w[0]);
									pInsp->ics.wHeatH = XWORD(c_inspAid.qu[0].w[1]);
								}
								break;
							default :	break;
							}
						}
					}

					bool bExit = true;
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (c_inspAid.wSeq[n] != 2)	bExit = false;
					}
					if (bExit)	c_inspAid.bExit = bExit;
				}
				if (c_inspAid.bExit) {
					bool bGood = true;
					c_inspAid.qu[0].dw = 0;
					// 201029
					int hofs = pPaper->GetHeadCarOffset();
					for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
						if (!c_inspAid.bResult1[n]) {
							bGood = false;
							pRev->CoordDuo(&c_inspAid.qu[0].dw, (DWORD)2, (n + hofs) << 1);		// "고장"
						}
						else	pRev->CoordDuo(&c_inspAid.qu[0].dw, (DWORD)1, (n + hofs) << 1);	// "정상"
					}
					pInsp->ics.wHeatL = XWORD(c_inspAid.qu[0].w[0]);
					pInsp->ics.wHeatH = XWORD(c_inspAid.qu[0].w[1]);
					pRev->CoordDuo(&pInsp->ics.wHeatRes, 2, 8);				// "시험 완료"
					pRev->CoordDuo(&pInsp->ics.wHeatRes, bGood ? 1 : 2, 10);	// 1:"정상"/2:"고장"
					if (bGood)	pRev->NextInspectStep();
					else	pRev->InspectPause();
				}
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

void CUniv::Arteriam()
{
}

ENTRY_CONTAINER(CUniv)
	SCOOP(&c_dm.d.cur,		sizeof(BYTE),	"Univ")
	SCOOP(&c_dm.d.buf,		sizeof(BYTE),	"")
	SCOOP(&c_dm.d.deb,		sizeof(BYTE),	"")
	SCOOP(&c_dm.mode,		sizeof(BYTE),	"")
	SCOOP(&c_dm.verf,		sizeof(BYTE),	"")
	//SCOOP(&c_inspAid,		sizeof(INSPECTAID),	"ui");
EXIT_CONTAINER()
