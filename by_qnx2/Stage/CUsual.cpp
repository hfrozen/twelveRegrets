/*
 * CUsual.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <math.h>
#include "CSch.h"
#include "CArch.h"
#include "CVerse.h"
#include "CProse.h"
#include "CPaper.h"
#include "CReview.h"
#include "../Component2/CTool.h"
#include "../ts/CLand.h"
#include "../ts/CStage.h"

#include "CUsual.h"

#define	MAKEWORD44(w, n1, n0, n3, n2)	w = (WORD)n1 << 12 | (WORD)n0 << 8 | (WORD)n3 << 4 | (WORD)n2
#define	MAKEWORD24(w, n1, n0)			w = (WORD)n1 << 12 | (WORD)n0 << 8
#define	MAKEWORD14(w, n0)				w = (WORD)n0 << 8

#define MAKEWORD82(w, n3, n2, n1, n0, n7, n6, n5, n4)\
	w = (WORD)n3 << 14 | (WORD)n2 << 12 | (WORD)n1 << 10 | (WORD)n0 << 8 | (WORD)n7 << 6 | (WORD)n6 << 4 | (WORD)n5 << 2 | (WORD)n4
#define	MAKEWORD22(w, n1, n0)			w = (WORD)n1 << 10 | (WORD)n0 << 8

#define	MAKEWORD104(w, v)\
	do {\
		MAKEWORD44(w[2], v[1], v[0], v[3], v[2]);\
		MAKEWORD44(w[1], v[5], v[4], v[7], v[6]);\
		MAKEWORD24(w[0], v[9], v[8]);\
	} while (0)

#define	MAKEWORD54(w, v)\
	do {\
		MAKEWORD44(w[1], v[1], v[0], v[3], v[2]);\
		MAKEWORD14(w[0], v[4]);\
	} while (0)

#define	MAKEWORD102(w, v)\
	do {\
		MAKEWORD82(w[1], v[3], v[2], v[1], v[0], v[7], v[6], v[5], v[4]);\
		MAKEWORD22(w[0], v[9], v[8]);\
	} while (0)

const WORD CUsual::c_wPatterns[4][CID_MAX] = {
		{	0,	3,	4,	2,	4,	2,	2,	3,	4,	1	},
		{	0,	0,	4,	3,	2,	2,	4,	3,	1,	1	},
		{	0,	0,	0,	3,	4,	2,	4,	1,	1,	1	},
		{	0,	0,	0,	0,	4,	3,	1,	1,	1,	1	}
};

CUsual::CUsual()
{
	c_pParent = NULL;
	c_pProp = NULL;
	//c_wSivbf = c_wCmgbf = 0;		// 19/12/05
	Initial();
}

CUsual::~CUsual()
{
}

void CUsual::Initial()
{
	c_bFireDialog = false;
}

void CUsual::CheckTriple(PBUNDLES pBund)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETVERSE(pVerse);
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	BYTE each[10];
	memset(&each, 0, sizeof(BYTE) * CID_MAX);
	//c_wSivbf = c_wCmgbf = 0;			// 19/12/05
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
			if (!(pLsv->sivcf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pLsv->sivcf.cLines[n] & (1 << DEVSTATE_FAIL))	each[n] = LPS_FAIL;
				else if (pLsv->sivcf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n] = LPS_LERR;
				// 180312
				else if (pLsv->sivcf.cLines[n] & (1 << DEVSTATE_BYPASS))	each[n] = LPS_SIVREADY;
				else	each[n] = LPS_NORMAL;
			}
			else	each[n] = LPS_BLOCK;		// 171101	LPS_EMPTY;

			//c_wSivbf |= (1 << n);			// 19/12/05
			BYTE fid = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[n]);
			if (pPaper->GetRouteState(fid)) {
				// 200218
				if (!pPaper->GetDeviceExFromRecip()) {
					if (GETCI(fid, CUDIB_CMBLK))	each[n + 4] = LPS_BYPASS;		// 6
					else if (GETCI(fid, CUDIB_CMK))	each[n + 4] = LPS_RUN;			// 4
					else	each[n + 4] = LPS_STOP;									// 3
					//if (!GETCI(fid, CUDIB_CMG))	c_wCmgbf |= (1 << n);			// 19/12/05
				}
				else {
					if (GETCI(fid, CUDIB_CMBLK) || pLsv->cmsbcf.fl[n].a.t.s.stat.b.bypass)	each[n + 4] = CMS_BYPASSF;
					else if (GETCI(fid, CUDIB_CMK) || pLsv->cmsbcf.fl[n].a.t.s.stat.b.invon)	each[n + 4] = CMS_RUNF;
					else	each[n + 4] = CMS_STOPF;
					if (!(pLsv->cmsbcf.cLines[n] & (1 << DEVSTATE_BLINDUNIT)))	++ each[n + 4];
				}
			}
			else	each[n + 4] = LPS_BLOCK;	// 171101	LPS_EMPTY;

			// 200218
			//WORD w = pSch->GetProportion((WORD)pDoz->recip[fid].real.svf.s.a.t.s.cVo, (WORD)0, (WORD)0xfa, (WORD)0, (WORD)750);
			WORD w = pSch->GetProportion(!pPaper->GetDeviceExFromRecip() ?
										(WORD)pDoz->recip[fid].real.svf.s.a.t.s.cVo :
										(WORD)pDoz->recip[fid].real.svf.s2.a.t.s.cVo,
										(WORD)0, (WORD)0xfa, (WORD)0, (WORD)750);
			unab.wSivV[n] = XWORD(w);
			// 171114
			//w = pSch->GetProportion((WORD)pDoz->recip[fid].real.svf.s.a.t.s.cIo, (WORD)0, (WORD)0xfa, (WORD)0, (WORD)750);
			if (!pPaper->GetDeviceExFromRecip()) {
				w = (WORD)pDoz->recip[fid].real.svf.s.a.t.s.ste.b.iom << 8;
				w |= (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cIo;
			}
			else	w = XWORD(pDoz->recip[fid].real.svf.s2.a.t.s.wIo);
			unab.wSivI[n] = XWORD(w);
			//w = pSch->GetProportion((WORD)pDoz->recip[fid].real.svf.s.a.t.s.cFo, (WORD)0, (WORD)0xfa, (WORD)0, (WORD)125);
			w = pSch->GetProportion(!pPaper->GetDeviceExFromRecip() ?
									(WORD)pDoz->recip[fid].real.svf.s.a.t.s.cFo :
									(WORD)pDoz->recip[fid].real.svf.s2.a.t.s.cFo,
									(WORD)0, (WORD)0xfa, (WORD)0, (WORD)125);
			unab.wSivF[n] = XWORD(w);
			// 200218
			if (!pPaper->GetDeviceExFromRecip()) {
				// 170731
				w = pSch->GetProportion((WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_BATTERY], (WORD)0, (WORD)4095, (WORD)0, (WORD)MAX_BATTERY);
				w += OFFSETV_BATTERY;
				//if (pConf->siv.cLength > 2 && n == 1)	w = (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cBatV;
				if (pConf->siv.cLength > 2 && n == 1)
					w = (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cBatV;
				unab.wBatV[n] = XWORD(w);
			}
			else {
				w = (WORD)pPsv->bmscf.fl[n].a.t.s.cCurV;		// 0~255V
				unab.wBatV[n] = XWORD(w);
			}
			//w = pSch->GetProportion((WORD)pDoz->recip[fid].real.svf.s.a.t.s.cEs, (WORD)0, (WORD)0xfa, (WORD)0, (WORD)3000);
			w = pSch->GetProportion(!pPaper->GetDeviceExFromRecip() ?
									(WORD)pDoz->recip[fid].real.svf.s.a.t.s.cEs :
									(WORD)pDoz->recip[fid].real.svf.s2.a.t.s.cEs,
									(WORD)0, (WORD)0xfa, (WORD)0, (WORD)3000);
			unab.wSivEV[n] = XWORD(w);

			// 180626
			//w = pDoz->recip[fid].real.svf.s.a.t.s.wVer;
			w = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.wVer : pDoz->recip[fid].real.svf.s2.a.t.s.wVer;
			w = ((w & 0xff) * 100) + (((w >> 8) & 0xff) % 100);
			unab.wSivVer[n] = XWORD(w);
		}
	}
	unab.wSiv = (WORD)each[1] << 12 | (WORD)each[0] << 8 | (WORD)each[2];
	unab.wCmsb = (WORD)each[5] << 12 | (WORD)each[4] << 8 | (WORD)each[6];

	// 200218
	if (pPaper->GetDeviceExFromRecip() && pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
			if (!(pPsv->bmscf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pLsv->sivcf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n] = LPS_LERR;
				else	each[n] = LPS_NORMAL;
			}
			else	each[n] = LPS_BLOCK;
		}
		unab.wBms = (WORD)each[1] << 12 | (WORD)each[0] << 8 | (WORD)each[2];
	}

	// esk
	memset(&each, 0, sizeof(BYTE) * CID_MAX);
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < (int)pConf->esk.cLength; n ++) {
			BYTE fid = pSch->C2F(pPaper->GetTenor(), pConf->esk.cPos[n]);
			if (pPaper->GetRouteState(fid) && GETCI(fid, CUDIB_ESK))	each[n] = 1;
		}
	}
	MAKEWORD24(unab.wEsk, each[1], each[0]);
}

void CUsual::CheckQuintuplet(PBUNDLES pBund)
{
	GETPAPER(pPaper);
	GETSCHEME(pSch);
	GETPROSE(pProse);
	//CLand* pLand = (CLand*)c_pProp;
	//ASSERTP(pLand);

	CProse::PLSHARE pLsv = pProse->GetShareVars();
	PDEVCONF pConf = pSch->GetDevConf();
	PDOZEN pDoz = pPaper->GetDozen();

	BYTE each[CID_MAX];
	WORD wHb, wLb;
	wHb = wLb = 0;
	WORD wLin = 0;
	memset(&each, 0, sizeof(BYTE) * CID_MAX);
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < (int)pConf->v3f.cLength; n ++) {
			if (!(pLsv->v3fcf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pLsv->v3fcf.cLines[n] & (1 << DEVSTATE_FAIL))	each[n] = LPS_FAIL;
				else if (pLsv->v3fcf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n] = LPS_LERR;
				else	each[n] = LPS_NORMAL;
			}
			else	each[n] = LPS_BLOCK;	// 171101	LPS_EMPTY;

			BYTE cf = pSch->C2F(pPaper->GetTenor(), pConf->v3f.cPos[n]);
			if (pPaper->GetRouteState(cf)) {
				if (pDoz->recip[cf].real.svf.v.a.t.s.det.b.hb)	wHb |= (LPS_ON << (n << 1));
				else	wHb |= (LPS_OFF << (n << 1));
				if (pDoz->recip[cf].real.svf.v.a.t.s.det.b.lb1 ||
					pDoz->recip[cf].real.svf.v.a.t.s.det.b.lb2)	wLb |= (LPS_ON << (n << 1));
				else	wLb |= (LPS_OFF << (n << 1));
			}
			else {
				wHb |= (LPS_EMPTY << (n << 1));
				wLb |= (LPS_EMPTY << (n << 1));
			}

			WORD w = pSch->GetProportion((WORD)pDoz->recip[cf].real.svf.v.a.t.s.cIq, (WORD)0, (WORD)0xff, (WORD)0, (WORD)2000);
			unab.wV3fI[n] = unab.wImv[n] = XWORD(w);
			// 170817
			//unab.wV3fV[n] = pDoz->recip[cf].real.svf.v.a.t.s.wFc;	// 원래가 bin-endian 값이다.
			w = pDoz->recip[cf].real.svf.v.a.t.s.wFc;	// 원래가 bin-endian 값이다.
			w = XWORD(w);
			w /= 10;
			unab.wV3fV[n] = XWORD(w);
			unab.wV3fP[n] = XWORD((WORD)pDoz->recip[cf].real.svf.v.a.t.s.cTbe);
			unab.wV3fEs[n] = pDoz->recip[cf].real.svf.v.a.t.s.wEs;
			unab.wV3fIdc[n] = pDoz->recip[cf].real.svf.v.a.t.s.wIdc;

			w = XWORD(pDoz->recip[cf].real.svf.v.a.t.s.wApbr);
			SHORT v = (SHORT)w;
			if (v >= 0)	w = 0;
			else	w = (WORD)abs(v);
			unab.wV3fA[n] = XWORD(w);

			w = 0;
			if (pDoz->recip[cf].real.svf.v.a.t.s.cmd.b.p)	w = 2;
			else if (pDoz->recip[cf].real.svf.v.a.t.s.cmd.b.b)	w = 3;
			wLin |= (w << (n << 1));

			// 180626
			w = pDoz->recip[cf].real.svf.v.a.t.s.wVer;
			w = ((w & 0xff) * 100) + (((w >> 8) & 0xff) % 100);
			unab.wV3fVer[n] = XWORD(w);
		}
	}
	MAKEWORD54(unab.wV3f, each);
	unab.wHb = XWORD(wHb);
	unab.wLb = XWORD(wLb);
	unab.wV3fD = XWORD(wLin);
}

void CUsual::CheckOctet(PBUNDLES pBund)
{
	GETPAPER(pPaper);
	GETSCHEME(pSch);
	GETPROSE(pProse);
	//CLand* pLand = (CLand*)c_pProp;
	//ASSERTP(pLand);

	CProse::PLSHARE pLsv = pProse->GetShareVars();
	PDOZEN pDoz = pPaper->GetDozen();
	BYTE each[10];
	_QUARTET qa, qb;
	qa.dw = qb.dw = 0;
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		int hofs = (int)pPaper->GetHeadCarOffset();
		for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
			BYTE fid = pSch->C2F(pPaper->GetTenor(), n);
			bool bInv = (pDoz->recip[fid].real.prol.cProperID == PID_RIGHT || pDoz->recip[fid].real.prol.cProperID == PID_RIGHTBK) ? true : false;
			memset(&each, 0, sizeof(BYTE) * CID_MAX);
			for (int m = 0; m < 4; m ++) {
				if (pLsv->dcucf.cLines[n][m] & (1 << DOORSTATE_OPEN))		each[m] = 8;		// 4;
				if (!(pLsv->dcucf.cLines[n][m] & (1 << DEVSTATE_BLINDDTB))) {
					if (pLsv->dcucf.cLines[n][m] & (1 << DEVSTATE_BLINDUNIT))				each[m] |= 1;
					else if (pLsv->dcucf.cLines[n][m] & (1 << DOORSTATE_EMERGENCYHANDLE))	each[m] |= 2;
					else if (pLsv->dcucf.cLines[n][m] & (1 << DEVSTATE_BYPASS))				each[m] |= 3;
					else if (pLsv->dcucf.cLines[n][m] & (1 << DOORSTATE_OBSTACLE))			each[m] |= 4;
					else if (pLsv->dcucf.cLines[n][m] & (1 << DOORSTATE_MAJF))				each[m] |= 5;
					else if (pLsv->dcucf.cLines[n][m] & (1 << DEVSTATE_FAIL))				each[m] |= 6;
				}
				else	each[m] |= 1;			// 171101
			}
			if (bInv)	MAKEWORD44(unab.wDoor[n + hofs][1], each[2], each[3], each[0], each[1]);
			else	MAKEWORD44(unab.wDoor[n + hofs][0], each[1], each[0], each[3], each[2]);

			memset(&each, 0, sizeof(BYTE) * CID_MAX);
			for (int m = 0; m < 4; m ++) {
				if (pLsv->dcucf.cLines[n][m + 4] & (1 << DOORSTATE_OPEN))	each[m] = 8;		// 4;
				if (!(pLsv->dcucf.cLines[n][m + 4] & (1 << DEVSTATE_BLINDDTB))) {
					if (pLsv->dcucf.cLines[n][m + 4] & (1 << DEVSTATE_BLINDUNIT))				each[m] |= 1;
					else if (pLsv->dcucf.cLines[n][m + 4] & (1 << DOORSTATE_EMERGENCYHANDLE))	each[m] |= 2;
					else if (pLsv->dcucf.cLines[n][m + 4] & (1 << DEVSTATE_BYPASS))				each[m] |= 3;
					else if (pLsv->dcucf.cLines[n][m + 4] & (1 << DOORSTATE_OBSTACLE))			each[m] |= 4;
					else if (pLsv->dcucf.cLines[n][m + 4] & (1 << DOORSTATE_MAJF))				each[m] |= 5;
					else if (pLsv->dcucf.cLines[n][m + 4] & (1 << DEVSTATE_FAIL))				each[m] |= 6;
				}
				else	each[m] |= 1;		// 171101
			}
			if (bInv)	MAKEWORD44(unab.wDoor[n + hofs][0], each[2], each[3], each[0], each[1]);
			else	MAKEWORD44(unab.wDoor[n + hofs][1], each[1], each[0], each[3], each[2]);

			// 171019
			if (pPaper->GetRouteState(fid)) {
				// 180118
				//if (!GETCI(fid, CUDIB_DIS))	qb.dw |= (DWORD)(2 << ((n + hofs) << 1));
				//if (GETCI(fid, CUDIB_EED) || GETCI(fid, CUDIB_EAD))	qa.dw |= (DWORD)(3 << ((n + hofs) << 1));
				if (!GETCI(fid, CUDIB_DIS))	qb.dw |= (DWORD)(3 << ((n + hofs) << 1));
				if (GETCI(fid, CUDIB_EED) || GETCI(fid, CUDIB_EAD))	qa.dw |= (DWORD)(2 << ((n + hofs) << 1));
			}
		}
		unab.wBypass[1] = XWORD(qb.w[0]);
		unab.wBypass[0] = XWORD(qb.w[1]);
		unab.wEmhand[1] = XWORD(qa.w[0]);
		unab.wEmhand[0] = XWORD(qa.w[1]);
	}
}

void CUsual::HeadLine()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&hdlb, 0, sizeof(HDLSTCTAIL));

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	hdlb.wLineNo = XWORD(2);
	BYTE c = pPaper->GetLength();
	hdlb.wLength = XWORD(c);
	//c = GETCIB(FID_HEAD, CUDIB_CT01);
	c = pVerse->GetPermNo();
	GETTOOL(pTool);
	c = pTool->ToHex(c);
	hdlb.wProperNo = XWORD(c);
	// 171019
	//hdlb.wOtrNo = pDoz->recip[FID_HEAD].real.ato.a.t.s.notr.a;
	//CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	//hdlb.wOtrNo = XWORD(pPsv->wOtrNo);
	WORD w = pVerse->GetCurOtrn();
	hdlb.wOtrn = XWORD(w);

	DTSTIME bcdt;
	GETARCHIVE(pArch);
	pArch->GetSysTimeToBcdDts(&bcdt);
	hdlb.wYear = XWORD(bcdt.year);
	hdlb.wDate = (WORD)bcdt.day << 8 | bcdt.mon;
	hdlb.wTime = (WORD)bcdt.min << 8 | bcdt.hour;
	hdlb.wSecond = XWORD(bcdt.sec);

	GETSCHEME(pSch);
	BYTE cf = pSch->GetCarFormID();
	PDEVCONF pConf = pSch->GetDevConf();
	// 171101
	//for (int n = CID_CAR0; n < CID_MAX; n ++)
	//	hdlb.wPatt[n] = XWORD(c_wPatterns[cf][n]);
	int hofs = (int)pPaper->GetHeadCarOffset();
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		// 190813
		//hdlb.wPatt[n + hofs] = XWORD(c_wPatterns[cf][n]);
		hdlb.wPatt[n + hofs] = XWORD(c_wPatterns[cf][n + hofs]);
		int fid = pSch->C2F(pPaper->GetTenor(), n);
		if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin && !pPaper->GetRouteState(fid))
			hdlb.wPatt[n + hofs] |= 0x800;	// XWORD(8), 0 |= 8 -> 8
	}

	CLand* pLand = (CLand*)c_pProp;
	ASSERTP(pLand);
	for (int n = 0; n < (int)pConf->pan.cLength; n ++) {
		BYTE pan = pLand->GetPantoState(n);
		// 190813
		//if (pan & 1)	hdlb.wPatt[pConf->pan.cPos[n]] |= 0x100;	// XWORD(1), 4 |= 1 -> 5
		//if (pan & 2)	hdlb.wPatt[pConf->pan.cPos[n]] |= 0x200;	// XWORD(2), 4 |= 2 -> 6
		if (pan & 1)	hdlb.wPatt[pConf->pan.cPos[n] + hofs] |= 0x100;	// XWORD(1), 4 |= 1 -> 5
		if (pan & 2)	hdlb.wPatt[pConf->pan.cPos[n] + hofs] |= 0x200;	// XWORD(2), 4 |= 2 -> 6
	}

	w = (WORD)pDoz->recip[FID_HEAD].real.motiv.cMode;
	hdlb.wMode = XWORD(w);
	w = (WORD)pLand->GetSteerLogicPwm();		// c_steer.GetLogicPwm()	{ return c_mam.cLogicPwm; }
	if (pDoz->recip[FID_HEAD].real.motiv.cEffType == EFFORT_POWERING)	hdlb.wPowerRate = XWORD(w);
	else if (pDoz->recip[FID_HEAD].real.motiv.cEffType == EFFORT_BRAKING)	hdlb.wBrakeRate = XWORD(w);

	//hdlb.wAtcCode
	hdlb.wSpeed = XWORD(pDoz->recip[FID_HEAD].real.drift.dwSpeed / 1000);
	double dbDistance = pArch->GetCareers(CRRITEM_DISTANCE, 0);
	_QUARTET quar;
	quar.dw = (DWORD)(dbDistance / 1000.f);		// km
	hdlb.dwOdoMeter = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
	quar.dw = (DWORD)dbDistance;
	hdlb.dwDistance = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
	// 181023
	w = (WORD)fmod(dbDistance, 1000.f);
	hdlb.wOdoMeter = XWORD(w);

	// PIS CONTROL CODE #A
	int vi = pVerse->GetValidPisSide();
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	if (pPsv->piscf.fl[vi].a.t.s.ref.b.mma) {
		w = (WORD)pPsv->piscf.fl[vi].a.t.s.cNextSt;
		hdlb.wNextStation = XWORD(w);
		w = (WORD)pPsv->piscf.fl[vi].a.t.s.cDestSt;
		hdlb.wDestStation = XWORD(w);
	}
	else {
		w = (WORD)pDoz->recip[FID_HEAD].real.ato.a.t.s.cNextSt;
		hdlb.wNextStation = XWORD(w);
		w = (WORD)pDoz->recip[FID_HEAD].real.ato.a.t.s.cDestSt;
		hdlb.wDestStation = XWORD(w);
	}
	// 180910
	if (pPsv->piscf.fl[vi].a.t.s.ref.b.mma && pPsv->piscf.fl[vi].a.t.s.ref.b.sim)	hdlb.ref.b.psim = true;

	if (pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT) {
		hdlb.cabEx.b.lm = true;
		hdlb.cabEx.b.ldl = pDoz->recip[FID_HEAD].real.motiv.lead.b.f;
		hdlb.cabEx.b.rdl = pDoz->recip[FID_HEAD].real.motiv.lead.b.r;
		hdlb.cabEx.b.rdr = (pPaper->GetRouteState(FID_TAIL) && GETTI(FID_TAIL, TUDIB_FORWARD)) ? true : false;
		hdlb.cabEx.b.ldr = (pPaper->GetRouteState(FID_TAIL) && GETTI(FID_TAIL, TUDIB_REVERSE)) ? true : false;
	}
	else {
		hdlb.cabEx.b.rm = true;
		hdlb.cabEx.b.rdr = pDoz->recip[FID_HEAD].real.motiv.lead.b.f;
		hdlb.cabEx.b.ldr = pDoz->recip[FID_HEAD].real.motiv.lead.b.r;
		hdlb.cabEx.b.ldl = (pPaper->GetRouteState(FID_TAIL) && GETTI(FID_TAIL, TUDIB_FORWARD)) ? true : false;
		hdlb.cabEx.b.rdl = (pPaper->GetRouteState(FID_TAIL) && GETTI(FID_TAIL, TUDIB_REVERSE)) ? true : false;
	}
	if (CAR_PSELF() == 0)	hdlb.cabEx.b.ls = true;
	else	hdlb.cabEx.b.rs = true;
	if (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED))	hdlb.cabEx.b.zs = true;
	if (pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_EB)	hdlb.cabEx.b.eb = true;
	if (pDoz->recip[FID_HEAD].real.motiv.cMode != MOTIVMODE_EMERGENCY)	hdlb.cabEx.b.otrv = true;

	if (GETTI(FID_HEAD, TUDIB_HCR))	hdlb.cabEx.b.hcr = true;
	//if (pScore->bReal)	hdlb.cab.b.initiative = true;

	// 180704
	//if (!pVerse->GetPisMode() &&
	if (!pPsv->piscf.fl[vi].a.t.s.ref.b.mma &&
		(((pDoz->recip[FID_HEAD].real.cSvcCtrlCode & 0xf0) >> 4) == SVCCODEB_XCHANGE ||
		(pDoz->recip[FID_HEAD].real.cSvcCtrlCode & 0xf) == SVCCODEB_XCHANGE))
		hdlb.cabEx.b.releaseblk = true;

	// 200218
	// 170808
	//hdlb.cabEx.b.lab = pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab;
	hdlb.cabEx.b.lab = pPaper->GetLabFromRecip();

	hdlb.lampEx.a = (WORD)(pDoz->recip[FID_HEAD].real.drift.cNbrk & 7);
	if (pDoz->recip[FID_HEAD].real.drift.cEbrk != 0)	hdlb.lampEx.b.eb = true;
	if (GETTI(FID_HEAD, TUDIB_ADBS))	hdlb.lampEx.b.adbs = true;
	// 171206
	//if (!GETTI(FID_HEAD, TUDIB_DIAPDC))	hdlb.lamp.b.psdc = true;
	if (pPaper->GetRouteState(FID_TAIL) && !GETTI(FID_TAIL, TUDIB_DIAPDC))	hdlb.lampEx.b.psdc = true;

	// 200218
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();

	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		hdlb.lampEx.b.domin = true;
		// 200218
		if (!pPaper->GetDeviceExFromRecip()) {
			for (int n = 0; n < pPaper->GetLength(); n ++) {
				BYTE fid = pSch->C2F(pPaper->GetTenor(), n);
				if (pPaper->GetRouteState(fid) && GETCI(fid, CUDIB_FDUF) && GETCI(fid, CUDIB_FD)) {
					hdlb.lampEx.b.fire = true;
					break;
				}
			}
		}
		else {
			// FD STEP 3: display fire dialog to DU
			if (pLsv->fire.wDefectSync != 0) {
				if (!c_bFireDialog && pPaper->GetLabFromRecip()) {
					c_bFireDialog = true;
					TRACK("FDS3: dialog\n");
				}
				hdlb.lampEx.b.fire = true;
			}
			else	c_bFireDialog = false;
		}
	}
	// 171107
	PBULKPACK pBulk = pArch->GetBulk();
	if (pBulk->wState & (1 << BULKSTATE_OLDTROUBLE))	hdlb.lampEx.b.oldt = true;
	// 171110
	//hdlb.lamp.b.oldt = true;
	// 171129
	// 171115
	//hdlb.lamp.b.mindi = pVerse->GetPisManual();

	if (pArch->GetUsbScanState())	hdlb.cabEx.b.uscan = true;
	CStage* pStage = (CStage*)c_pParent;
	CStage::PDOWNLOADINFO pDownloadInfo = pStage->GetDownloadInfo();
	CLand::PTRACEINFO pTraceInfo = pLand->GetTraceInfo();
	if ((pDownloadInfo->wItem >= CStage::DOWNLOADITEM_LOGBOOKENTRY && pDownloadInfo->wItem <= CStage::DOWNLOADITEM_PUTINSPECTTEXT) ||
		pTraceInfo->wItem != 0) {
		if (pArch->GetUsbState())	hdlb.cabEx.b.usb = true;
		hdlb.wMsg = XWORD(pDownloadInfo->wMsg);
		if (pDownloadInfo->iEntryIndex >= 0 || pTraceInfo->wItem != 0) {	// downloading
			hdlb.cabEx.b.usb = false;
			hdlb.lampEx.b.dling = true;
			hdlb.wMsg = XWORD(44);		// wMsg = 44;
			WORD wProgress = 0;
			if (pDownloadInfo->iEntryIndex >= 0) {
				if (pDownloadInfo->size.total > 0) {
					double db = (double)pDownloadInfo->size.cur / (double)pDownloadInfo->size.total;
					db *= 1000.f;
					wProgress = (WORD)db;
				}
				else	wProgress = 0;
			}
			else {
				double db = ((double)pTraceInfo->wChapterID * (double)pTraceInfo->wPageMax + (double)pTraceInfo->wPageID) /
															((double)pTraceInfo->wChapterMax * (double)pTraceInfo->wPageMax);
				db *= 1000.f;
				wProgress = (WORD)db;
			}
			hdlb.wProgress = XWORD(wProgress);
		}
	}
	else {
		if (pDownloadInfo->bFailur)	hdlb.wMsg = XWORD(49);	// 171219
	}

	// 200218
	//GETPROSE(pProse);
	//CProse::PLSHARE pLsv = pProse->GetShareVars();
	hdlb.lampEx.b.mcm = (pLsv->sivcf.wCorrectMap != 0 && pLsv->wCmgBitmap != 0 && pLsv->wCmkBitmap == 0) ? true : false;
	hdlb.lampEx.b.mav = pLsv->bEnManualAlbo;
	hdlb.lampEx.b.mas = pLsv->bManualAlbo;

	// 200218
	hdlb.lampEx.b.devEx = pPaper->GetDeviceExFromRecip();

	GETREVIEW(pRev);
	// insp
	hdlb.insp.b.stop = (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NDETECT_ZEROSPEED)) ? true : false;
	hdlb.insp.b.pb = (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_PARKING)) ? true : false;
	// 19/12/05
	//hdlb.insp.b.cmi = c_wSivbf == c_wCmgbf ? true : false;
	hdlb.insp.b.cmi = VALIDMRPS() ? true : false;
	hdlb.insp.b.em = pDoz->recip[FID_HEAD].real.motiv.cMode == MOTIVMODE_EMERGENCY ? true : false;
	hdlb.insp.b.adc = GETTI(FID_HEAD, TUDIB_DIR) ? true : false;
	hdlb.insp.b.ebr = pDoz->recip[FID_HEAD].real.drift.cEbrk == 0 ? true : false;
	hdlb.insp.b.mb7 = pDoz->recip[FID_HEAD].real.motiv.cNotch == NOTCHID_B7 ? true : false;
	hdlb.insp.b.sb = (pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) ? true : false;
	hdlb.insp.b.ros = pRev->GetInspectPause();

	// 210630
	if (pPaper->GetDeviceExFromRecip() &&
		(pDoz->recip[FID_HEAD].real.sign.scmdEx.b.fireMbc ||
		(pPaper->GetRouteState(FID_TAIL) &&
		pDoz->recip[FID_TAIL].real.sign.scmdEx.b.fireMbc)))
		hdlb.wFireBuzz = 0x100;
	else	hdlb.wFireBuzz = 0;

	//w = pStage->GetInitiativeMsg();
	//hdlb.wMsgInitiative = XWORD(w);

	//hdlb.wFailCar
	TROUBLECODE tc = pArch->GetRecentTrouble();
	if (tc.b.code != 0) {
		WORD code = tc.b.code;
		hdlb.wTroubleCode = XWORD(code);
		w = tc.b.cid;
		hdlb.wTroubleID = XWORD(w);
		hdlb.lampEx.b.trb = true;			// 위 오른쪽 "고장"버튼 표시
		w = (1 << (w + hofs)) & 0x3ff;
		hdlb.ctnp.a = XWORD(w);
		//if (pPaper->GetOblige() & (1 << OBLIGE_HEAD)) {
		if (pPaper->IsHead()) {
			if (IsSevereType(pArch->GetTroubleType(code))) {
				hdlb.lampEx.b.severe = true;	// 고장 조치 화면
				// 180511
				pArch->SortHide((WORD)tc.b.cid, (int)tc.b.code);
				if (pPaper->GetLabFromRecip())	TRACK("USL:add severe %d %d.\n", tc.b.cid, tc.b.code);
			}
			//hdlb.lamp.b.trb = true;			// 위 오른쪽 "고장"버튼 표시
			//w = 1 << w;
			//hdlb.carmLp.a = XWORD(w);
		}
	}
	// 180704
	//int vi = pVerse->GetValidPisSide();
	if (pPsv->piscf.fl[vi].r.t.s.ais.cCode == SVCCODEH_XCHANGE)	{
		if (vi != 0)	hdlb.ctnp.b.pisxr = true;
		else	hdlb.ctnp.b.pisxl = true;
	}
	else {
		if (vi != 0)	hdlb.ctnp.b.pisnr = true;
		else	hdlb.ctnp.b.pisnl = true;
	}
	if (pPsv->piscf.fl[vi].a.t.s.ref.b.mma)	hdlb.ctnp.b.pismm = true;

	WORD seq = 0;
	WORD agree = 0;
	WORD cancel = 0;
	//if ((pPaper->GetOblige() & (1 << OBLIGE_HEAD)) && pRev->GetInspectStep() == INSPSTEP_NON &&
	if (pPaper->IsHead() && pRev->GetInspectStep() == INSPSTEP_NON &&
		pStage->GetDownloadItem() == CStage::DOWNLOADITEM_NON && pLand->GetTraceInfoSeq() == CLand::TRACESEQ_NON) {
		if (!pPaper->GetDeviceExFromRecip()) {
			// 170727, 차상 시험이나 다운로드 중일 때는 화재 다이얼로그가 나타나지 않도록 하였지만 화재 시퀀스에 래치되는 것은 그대로두어 시험이나 다운로드가 끝나면
			// 다이얼로그가 나타난다. 이를 나타나지 않도록 할 수 있지만 안나타나는 게 나은건지 나타나는게 나은건지 확인할 수 없어 그대로 둔다.
			// 180626
			//for (int n = 0; n < pPaper->GetLength(); n ++) {
			//	switch (pLsv->fire.wSeq[n]) {
			//	case CProse::FS_WAITUSERCOMMAND :
			//		agree |= (1 << n);				// "확인" 단추 보이기
			//		cancel |= (1 << n);				// "취소" 단추 보이기
			//		break;
			//	case CProse::FS_ACTIVE :
			//		seq |= (1 << n);				// 눌러진 "확인"
			//		agree |= (1 << n);				// "확인" 단추 보이기
			//		break;
			//	default :
			//		break;
			//	}
			//}
			switch (pLsv->fire.wSeq) {
			case CProse::FS_WAITUSERCOMMAND :
				agree |= 1;				// "확인" 단추 보이기
				cancel |= 1;			// "취소" 단추 보이기
				break;
			case CProse::FS_ACTIVE :
				seq |= 1;				// 취소 -> 해제
				cancel |= 1;			// "해제(취소)" 단추 보이기
				break;
			default :
				break;
			}

			hdlb.wFireDlgSeq = XWORD(seq);
			hdlb.wFireDlgAgree = XWORD(agree);
			hdlb.wFireDlgCancel = XWORD(cancel);
			//c_bFireDialog = (agree != 0) ? true : false;
			//hdlb.lamp.b.fdlg = c_bFireDialog;
			hdlb.lampEx.b.fdlg = (cancel != 0) ? true : false;
		}
		else {
			hdlb.lampEx.b.fdlg = pLsv->fire.wSeq != CProse::FS_OFF ? true : false;
		}
	}
	//hdlb.wListupWait = pStage->GetListupWait();
	hdlb.wOldLoadTime = XWORD(pBulk->wOldLoadTime);
	w = pArch->GetSevereLength();
	hdlb.wSevereLength = XWORD(w);
	hdlb.ref.b.aux = (pLand->GetProperID() & 1) ? true : false;
	int fid = (CAR_FSELF() < FID_TAIL) ? FID_HEADBK : FID_TAILBK;
	// 180911
	//if (pPaper->GetRouteState(fid))	hdlb.ref.b.waux = true;
	if (pPaper->GetRouteState(fid) && VALIDRECIP(fid))	hdlb.ref.b.waux = true;
	// 180910
	//GetTrack(&hdlb.wText[0]);
	//BYTE* pTrack = (BYTE*)GetTrack();
	//if (pTrack != NULL) {
	//	if (c_wScreenOut < 60000)	++ c_wScreenOut;
	//	_DUET du;
	//	for (int n = 0; n < 40; n ++) {
	//		du.c[1] = *pTrack ++;
	//		du.c[0] = *pTrack ++;
	//		hdlb.wText[n] = du.w;
	//	}
	//	hdlb.wTextPresent = 0xffff;
	//}

	// 180405
	MOMENTUMINFO moment;
	pLand->CopyMomentum(&moment);
	w = (WORD)pTool->Deviround10(DWORD(moment.power.time.dbCurrent * 100.f));		// 1.254sec -> 125 -> 13 -> 1.3sec - du에서 소수점 한자리로 표시된다.
	hdlb.wPowerTime = XWORD(w);
	w = (WORD)moment.power.distance.dbCurrent;
	hdlb.wPowerDistance = XWORD(w);
	w = (WORD)pTool->Deviround10((DWORD)(moment.brake.time.dbCurrent * 100.f));
	hdlb.wBrakeTime = XWORD(w);
	w = (WORD)moment.brake.distance.dbCurrent;
	hdlb.wBrakeDistance = XWORD(w);
	hdlb.wMomentum = XWORD(moment.vMomentum);

	pVerse->CopyBundle(&hdlb, sizeof(HEADLINESENTENCE));
}

void CUsual::ServiceA()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&unab, 0, sizeof(UNASTCTAIL));

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	WORD w = 0;
	if (GETTI(FID_HEAD, TUDIB_DMMM))	w = 1;
	else if (GETTI(FID_HEAD, TUDIB_DMAM))	w = 2;
	else if (GETTI(FID_HEAD, TUDIB_DMAA))	w = 3;
	unab.wDoorMode[0] = XWORD(w);

	//unab.curLog
	CheckOctet(pBund);	// door
	CheckTriple(pBund);	// siv
	// carmLp
	CheckQuintuplet(pBund);	// v3f

	// tu
	w = (LPS_LERR << 12) | (LPS_LERR << 8) | (LPS_LERR << 4) | LPS_LERR;
	for (int n = 0; n < LENGTH_TU; n ++) {
		if (pPaper->GetRouteState(n)) {
			w &= ~(0xf << (pDoz->recip[n].real.prol.cProperID * 4));		// GetTuPID()보다는 간결해보인다...
			w |= (LPS_NORMAL << (pDoz->recip[n].real.prol.cProperID * 4));
		}
	}
	unab.wTu = XWORD(w);

	_QUARTET qa;
	qa.dw = 0;
	int hofs = pPaper->GetHeadCarOffset();
	for (int n = 0; n < (int)pPaper->GetLength() - 2; n ++) {
		if (pPaper->GetRouteState(n + LENGTH_TU))	qa.dw |= (DWORD)(LPS_NORMAL << ((n + hofs) * 4));
		else	qa.dw |= (DWORD)(LPS_LERR << ((n + hofs) * 4));
	}
	unab.dwCu = XDWORD(qa.dw);

	// pole dev.
	int vi = pVerse->GetValidPisSide();
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	BYTE each[12];
	memset(&each, 0, sizeof(BYTE) * 12);
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < 2; n ++) {
			if (!(pPsv->trscf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pPsv->trscf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n] = LPS_LERR;
				else	each[n] = LPS_NORMAL;
			}
			else	each[n] = LPS_BLOCK;		// 171101	LPS_EMPTY;
			// 171204
			if (!(pPsv->rtdcf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pPsv->rtdcf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n + 2] = LPS_LERR;
				else	each[n + 2] = LPS_NORMAL;
			}
			else	each[n + 2] = LPS_BLOCK;

			//if (pDoz->recip[cf].real.cPoleState & (1 << PDEVID_RTD))	each[n + 2] = LPS_NORMAL;
			//else	each[n + 2] = LPS_LERR;

			// 17/07/07
			int m = n;
			if (pDoz->recip[FID_HEAD].real.prol.cProperID >= PID_RIGHT)	m ^= 1;
			// 180704
			//if (!pVerse->GetPisMode() &&
			if (!pPsv->piscf.fl[vi].a.t.s.ref.b.mma &&
				((pDoz->recip[FID_HEAD].real.cSvcCtrlCode >> (m * 4)) & 0xf) == SVCCODEB_NEUTRAL)
				each[n + 4] = each[n + 6] = LPS_FAIL;
			else {
				if (!(pPsv->piscf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
					if (pPsv->piscf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n + 4] = LPS_LERR;
					else	each[n + 4] = LPS_NORMAL;
				}
				else	each[n + 4] = LPS_BLOCK;		// 171101
				if (!(pPsv->paucf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
					if (pPsv->paucf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n + 6] = LPS_LERR;
					else	each[n + 6] = LPS_NORMAL;
				}
				else	each[n + 6] = LPS_BLOCK;		// 171101
			}

			// 180123
			//if (!(pPsv->atocf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
			//	if (pPsv->atocf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n + 8] = LPS_LERR;
			//	else	each[n + 8] = LPS_NORMAL;
			//}
			//else	each[n + 8] = LPS_EMPTY;	// 이게 기본이다.
			if (!(pPsv->atocf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pPsv->atocf.cLines[n] & (1 << DEVSTATE_FAIL))			each[n + 8] = LPS_ATOOPEN;		// "개방"
				else if (pPsv->atocf.cLines[n] & (1 << DEVSTATE_BYPASS))	each[n + 8] = LPS_ATOWAIT;		// "대기"
				else if (pPsv->atocf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n + 8] = LPS_LERR;			// "통신"
				else	each[n + 8] = LPS_NORMAL;
			}
			//else	each[n + 8] = LPS_BLOCK;
			// 200218
			if (!(pPsv->bmscf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pPsv->bmscf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[n + 10] = LPS_LERR;
				else	each[n + 10] = LPS_NORMAL;
			}
			else	each[n + 10] = LPS_BLOCK;
		}
	}
	//if (pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT) {
	//	if (pDoz->recip[FID_HEAD].real.cPoleLine & (1 << PDEVID_RTD))	each[2] = LPS_NORMAL;
	//	else	each[2] = LPS_LERR;
	//	if (pDoz->recip[FID_TAIL].real.cPoleLine & (1 << PDEVID_RTD))	each[3] = LPS_NORMAL;
	//	else	each[3] = LPS_LERR;
	//}
	//else {
	//	if (pDoz->recip[FID_HEAD].real.cPoleLine & (1 << PDEVID_RTD))	each[3] = LPS_NORMAL;
	//	else	each[3] = LPS_LERR;
	//	if (pDoz->recip[FID_TAIL].real.cPoleLine & (1 << PDEVID_RTD))	each[2] = LPS_NORMAL;
	//	else	each[2] = LPS_LERR;
	//}

	MAKEWORD44(unab.wTrsRtd, each[1], each[0], each[3], each[2]);
	MAKEWORD44(unab.wPisPau, each[5], each[4], each[7], each[6]);
	MAKEWORD24(unab.wAto, each[9], each[8]);
	MAKEWORD24(unab.wBms, each[11], each[10]);

	GETARCHIVE(pArch);
	PCMJOGB pCmj = pArch->GetCmjog();
	double rt = (double)pCmj->real.cjt.srt.b.day * (double)SECONDONEDAY + (double)pCmj->real.cjt.srt.b.sec;

	for (int n = 0; n < CM_MAX; n ++) {
		double cmt = (double)pCmj->real.cjt.cm[n].b.day * (double)SECONDONEDAY + (double)pCmj->real.cjt.cm[n].b.sec;
		w = (WORD)(cmt * 1000.f / rt);
		unab.wCmRunrate[n] = XWORD(w);
	}

	PV3FBRKCNTF pVbf = pArch->GetV3fBreaker();
	for (int n = 0; n < V3F_MAX; n ++) {
		unab.wHbCnt[n] = XWORD(pVbf->u.v.wHbCnt[n]);
		unab.wLbCnt[n] = XWORD(pVbf->u.v.wLbCnt[n]);
	}
	// 180806
	w = (WORD)(pDoz->recip[FID_HEAD].real.ato.a.t.s.cVer & 0xff);
	unab.wAtoVer = XWORD(w);

	pVerse->CopyBundle(&unab, sizeof(UNITAPSENTENCE));
}

void CUsual::ServiceB()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&unbb, 0, sizeof(UNBSTCTAIL));
	// 200218
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	//GETARCHIVE(pArch);
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	//PDEVCONF pConf = pSch->GetDevConf();
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();

	BYTE each[8][CID_MAX];
	memset(&each, 0, sizeof(BYTE) * 8 * CID_MAX);
	// 181019
	BYTE bcsp = 0;
	for (int n = 0; n < LENGTH_TU; n ++) {
		if (pDoz->recip[n].real.avs.wCurve[ADCONVCH_PRESSURE] > 500)	++ bcsp;	// valid sensor
	}

	// 180219
	WORD brcs = 0;
	int hofs = (int)pPaper->GetHeadCarOffset();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
			if (!(pLsv->ecucf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pLsv->ecucf.cLines[n] & (1 << DEVSTATE_FAIL))	each[0][n + hofs] = LPS_FAIL;
				else if (pLsv->ecucf.cLines[n] & (1 << DEVSTATE_BLINDUNIT))	each[0][n + hofs] = LPS_LERR;
				else	each[0][n + hofs] = LPS_NORMAL;
			}
			else	each[0][n + hofs] = LPS_BLOCK;		// 171101	LPS_EMPTY;

			BYTE fid = pSch->C2F(pPaper->GetTenor(), n);
			if (pPaper->GetRouteState(fid)) {
				if (GETCI(fid, CUDIB_BCCS))	each[1][n + hofs] = LPS_NORMAL;	// 170717, 신호 반전
				else	each[1][n + hofs] = LPS_CYLCUT;

				if (GETCI(fid, CUDIB_ASCS))	each[2][n + hofs] = LPS_NORMAL;
				else	each[2][n + hofs] = LPS_CYLCUT;

				// 200218
				if (!pPaper->GetDeviceExFromRecip()) {
					if (!GETCI(fid, CUDIB_FDUF))		each[3][n + hofs] = LPS_FAIL;
					else if (GETCI(fid, CUDIB_FD))	each[3][n + hofs] = LPS_FIREF;
					else	each[3][n + hofs] = LPS_NORMAL;
				}
				else {
					if (pLsv->fire.wUnitJamMap & (1 << n))	each[3][n + hofs] = LPS_FAIL;
					else if (pLsv->fire.wDefectMap & (1 << n))	each[3][n + hofs] = LPS_FIREF;
					// 200218
					else if ((pPsv->fducf.fl[0].a.t.s.wRecover & (1 << n)) ||
						(pPsv->fducf.fl[1].a.t.s.wRecover & (1 << n)))
						each[3][n + hofs] = LPS_BYPASS;	// "준비"
					else {
						if (fid == FID_HEAD || fid == FID_TAIL) {
							if (pPsv->fducf.cLines[n != 0 ? 1 : 0] & (1 << DEVSTATE_BLINDUNIT))
								each[3][n + hofs] = LPS_LERR;
							else	each[3][n + hofs] = LPS_NORMAL;
						}
						else	each[3][n + hofs] = LPS_NORMAL;
					}
				}

				if (GETCI(fid, CUDIB_PA1) || GETCI(fid, CUDIB_PA2))	each[4][n + hofs] = LPS_CALL;
				else	each[4][n + hofs] = LPS_NORMAL;

				if (!GETUI(fid, UDOB_DCL))	each[5][n + hofs] = LPS_ON;
				else	each[5][n + hofs] = LPS_OFF;

				if (GETCI(fid, CUDIB_LK1))	each[6][n + hofs] = LPS_ON;
				else	each[6][n + hofs] = LPS_OFF;

				if (GETCI(fid, CUDIB_LK2))	each[7][n + hofs] = LPS_ON;
				else	each[7][n + hofs] = LPS_OFF;

				if (!GETCI(fid, CUDIB_BRCS))	brcs |= (1 << (n + hofs));		// 180219
			}
			else	each[1][n + hofs] = each[2][n + hofs] = each[3][n + hofs] =
						each[4][n + hofs] = each[5][n + hofs] = each[6][n + hofs] = each[7][n + hofs] = LPS_BLOCK;		//171101	LPS_EMPTY;

			// 200218
			//double dbAsp = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.a.t.s.cAsp);			// 102
			double dbAsp = 0.f;
			if (pPaper->GetDeviceExFromRecip())
				dbAsp = pSch->KPaToKgcm2((WORD)((pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[0] +
										pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
										pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[2] +
										pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[3]) / 4 * 3));
			else	dbAsp = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.co.a.t.s.cAsp);
			WORD w = (WORD)(dbAsp * 10.f);
			unbb.wAsp[n + hofs] = XWORD(w);
			// 200629
			// 181019
			//double dbBcp;
			//if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) && bcsp > 0) {
			//	brcs |= (1 << (n + hofs));
			//	dbBcp = pSch->GetProportion((double)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE], 819.f, 4096.f, 0.f, 101.972f);
			//}
			//else	dbBcp = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.a.t.s.cBcp) * 10.f;	// 소수점 한자리까지... 102
			if ((pDoz->recip[FID_HEAD].real.drift.cNbrk & (1 << NBRAKE_SECURITY)) && bcsp > 0)
				brcs |= (1 << (n + hofs));
			// 200218
			//double dbBcp = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.a.t.s.cBcp) * 10.f;	// 소수점 한자리까지... 102
			//double dbBcp = pSch->BarToKgcm2(pPaper->GetDeviceExFromRecip() ?
			//							pDoz->recip[fid].real.ecu.ct.a.t.s.cBcp : pDoz->recip[fid].real.ecu.co.a.t.s.cBcp) * 10.f;
			double dbBcp;
			if (pPaper->GetDeviceExFromRecip())
				dbBcp = pSch->KPaToKgcm2((WORD)pDoz->recip[fid].real.ecu.ct.a.t.s.cBcp * 4);
			else	dbBcp = pSch->BarToKgcm2(pDoz->recip[fid].real.ecu.co.a.t.s.cBcp);
			w = (WORD)(dbBcp * 10.f);
			unbb.wBcv[n + hofs] = XWORD(w);				// value
			unbb.wBcp[n + hofs] = XWORD(w);				// graph
			// 200629
			w = pSch->GetProportion((WORD)pDoz->recip[fid].real.avs.wCurve[ADCONVCH_PRESSURE], (WORD)819, (WORD)4095, (WORD)0, (WORD)102);
			unbb.wBccs[n + hofs] = XWORD(w);

			// 200218
			//w = pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.a.t.s.cLw, (WORD)0, (WORD)255, (WORD)200, (WORD)750);	// 0~0xff -> 20.0~75.0ton
			if (pPaper->GetDeviceExFromRecip())
				w = (WORD)((double)pDoz->recip[fid].real.ecu.ct.a.t.s.cLw * 3.f + 200.f);
			else	w = pSch->GetProportion((WORD)pDoz->recip[fid].real.ecu.co.a.t.s.cLw, (WORD)0, (WORD)255, (WORD)200, (WORD)750);	// 0~0xff -> 20.0~75.0ton
			unbb.wCalcLw[n + hofs] = XWORD(w);
			w = pSch->CalcLoad(n, w);	// ton
			unbb.wLoadr[n + hofs] = XWORD(w);
		}
	}
	MAKEWORD104(unbb.wEcu, each[0]);
	MAKEWORD104(unbb.wBcs, each[1]);
	MAKEWORD104(unbb.wAss, each[2]);
	MAKEWORD104(unbb.wFire, each[3]);
	MAKEWORD104(unbb.wPhone, each[4]);
	MAKEWORD104(unbb.wLampDc, each[5]);
	MAKEWORD104(unbb.wLampAc1, each[6]);
	MAKEWORD104(unbb.wLampAc2, each[7]);
	unbb.wBrcs = XWORD(brcs);
	if (pPaper->GetDeviceExFromRecip()) {
		brcs = 0;
		if (pPsv->fducf.fl[0].a.t.s.stat.b.mt1)	brcs |= 1;
		if (pPsv->fducf.fl[0].a.t.s.stat.b.mt2)	brcs |= 2;
		if (pPsv->fducf.fl[1].a.t.s.stat.b.mt1)	brcs |= 4;
		if (pPsv->fducf.fl[1].a.t.s.stat.b.mt2)	brcs |= 8;
		unbb.wFireCtrl = XWORD(brcs);
	}

	memset(&each, 0, sizeof(BYTE) * 8 * CID_MAX);
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
			int cf = pSch->C2F(pPaper->GetTenor(), n);
			if (!(pLsv->hvaccf.cLines[n] & (1 << DEVSTATE_BLINDDTB))) {
				if (pLsv->hvaccf.cLines[n] & (1 << DEVSTATE_BLINDUNIT)) {
					each[6][n + hofs] = LPS_LERR;
					each[0][n + hofs] = each[1][n + hofs] = each[2][n + hofs] = 4;
					each[3][n + hofs] = each[4][n + hofs] = each[5][n + hofs] = 2;
				}
				else {
					int cc = pSch->F2C(pPaper->GetTenor(), cf);
					each[6][n + hofs] = LPS_NORMAL;
					// 200218
					//if (pDoz->recip[cf].real.hvac.a.t.s.sta.b.h3)	each[0][n + hofs] = 3;
					//else if (pDoz->recip[cf].real.hvac.a.t.s.sta.b.h2)	each[0][n + hofs] = 2;
					//else if (pDoz->recip[cf].real.hvac.a.t.s.sta.b.h1)	each[0][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.sta.b.h3)	each[0][n + hofs] = 3;
					else if (pLsv->hvaccf.fl[cc].a.t.s.sta.b.h2)	each[0][n + hofs] = 2;
					else if (pLsv->hvaccf.fl[cc].a.t.s.sta.b.h1)	each[0][n + hofs] = 1;
					else	each[0][n + hofs] = 0;

					//if (pDoz->recip[cf].real.hvac.a.t.s.stb.b.u1fc)	each[1][n + hofs] = 3;
					//else if (pDoz->recip[cf].real.hvac.a.t.s.sta.b.u1hc)	each[1][n + hofs] = 2;
					//else if (pDoz->recip[cf].real.hvac.a.t.s.sta.b.u1v)	each[1][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.stb.b.u1fc)	each[1][n + hofs] = 3;
					else if (pLsv->hvaccf.fl[cc].a.t.s.sta.b.u1hc)	each[1][n + hofs] = 2;
					else if (pLsv->hvaccf.fl[cc].a.t.s.sta.b.u1v)	each[1][n + hofs] = 1;
					else	each[1][n + hofs] = 0;

					//if (pDoz->recip[cf].real.hvac.a.t.s.stb.b.u2fc)	each[2][n + hofs] = 3;
					//else if (pDoz->recip[cf].real.hvac.a.t.s.stb.b.u2hc)	each[2][n + hofs] = 2;
					//else if (pDoz->recip[cf].real.hvac.a.t.s.sta.b.u2v)	each[2][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.stb.b.u2fc)	each[2][n + hofs] = 3;
					else if (pLsv->hvaccf.fl[cc].a.t.s.stb.b.u2hc)	each[2][n + hofs] = 2;
					else if (pLsv->hvaccf.fl[cc].a.t.s.sta.b.u2v)	each[2][n + hofs] = 1;
					else	each[2][n + hofs] = 0;

					//if (pDoz->recip[cf].real.hvac.a.t.s.stc.b.lffk1)	each[3][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.stc.b.lffk1)	each[3][n + hofs] = 1;
					else	each[3][n + hofs] = 0;

					//if (pDoz->recip[cf].real.hvac.a.t.s.stc.b.lffk2)	each[7][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.stc.b.lffk2)	each[7][n + hofs] = 1;
					else	each[7][n + hofs] = 0;

					//if (pDoz->recip[cf].real.hvac.a.t.s.stc.b.vf)		each[4][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.stc.b.vf)	each[4][n + hofs] = 1;
					else	each[4][n + hofs] = 0;

					//if (pDoz->recip[cf].real.hvac.a.t.s.stc.b.apdk)	each[5][n + hofs] = 1;
					if (pLsv->hvaccf.fl[cc].a.t.s.stc.b.apdk)	each[5][n + hofs] = 1;
					else	each[5][n + hofs] = 0;

					// 171221
					//SHORT v = (SHORT)pDoz->recip[cf].real.hvac.a.t.s.cTemp;
					//BYTE c = pDoz->recip[cf].real.hvac.a.t.s.cTemp;
					BYTE c = pLsv->hvaccf.fl[cc].a.t.s.cTemp;
					WORD sv = (c & 0x80) ? (WORD)c | 0xff00 : (WORD)c & 0xff;
					unbb.wIntTemp[n + hofs] = XWORD(sv);
				}
				if (n == 0 || n == (int)pPaper->GetLength() - 1) {
					WORD w = pSch->GetProportion((int)pDoz->recip[cf].real.avs.wCurve[ADCONVCH_OUTTEMP], 1023, 4095, -50, 100);
					unbb.wExtTemp[n != 0 ? 1 : 0] = XWORD(w);
				}
			}
			else {
				each[6][n + hofs] = LPS_BLOCK;		// 171101	LPS_EMPTY;
				each[0][n + hofs] = each[1][n + hofs] = each[2][n + hofs] = 4;
				each[3][n + hofs] = each[4][n + hofs] = each[5][n + hofs] = each[7][n + hofs] = 2;
			}
		}
	}
	MAKEWORD104(unbb.wHeat, each[0]);
	MAKEWORD104(unbb.wCool1, each[1]);
	MAKEWORD104(unbb.wCool2, each[2]);
	MAKEWORD102(unbb.wFan1, each[3]);
	MAKEWORD102(unbb.wVent, each[4]);
	MAKEWORD102(unbb.wClean, each[5]);
	MAKEWORD104(unbb.wHvac, each[6]);
	MAKEWORD102(unbb.wFan2, each[7]);

	// 171027
	WORD w = pProse->GetControlSide();
	unbb.wCtrlCab = XWORD(w);
	// 201016
	if (pPaper->GetDeviceExFromRecip()) {
		w = pProse->GetControlExSide();
		unbb.wCtrlExCab = XWORD(w);
	}

	pVerse->CopyBundle(&unbb, sizeof(UNITBPSENTENCE));
}
