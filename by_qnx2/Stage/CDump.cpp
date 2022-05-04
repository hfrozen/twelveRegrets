/*
 * CDump.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include "Track.h"
#include "Mm.h"
#include "CSch.h"
#include "CArch.h"
#include "CVerse.h"
#include "CProse.h"
#include "CPaper.h"
#include "../Component2/CTool.h"
#include "../Component2/CBand.h"
#include "../ts/CLand.h"
#include "../ts/CStage.h"

#include "CDump.h"

const BYTE CDump::c_cLineBar[FID_MAX] = {
		0,	1,	17,	18,	2,	4,	6,	8,	10,	12,	14,	16
};

const DWORD CDump::c_dwFillBar[2][7][2] = {
		{{ 0x75550, 0x0008 }, { 0x75540, 0x0020 }, { 0x75500, 0x0080 }, { 0x75400, 0x0200 }, { 0x75000, 0x0800 }, { 0x74000, 0x2000 }, { 0x70000, 0x8000 }},
		{{ 0x05557, 0x8000 }, { 0x01557, 0x2000 }, { 0x00557, 0x0800 }, { 0x00157, 0x0200 }, { 0x00057, 0x0080 }, { 0x00017, 0x0020 }, { 0x00007, 0x0008 }}
};

CDump::CDump()
{
	c_pParent = NULL;
	c_pProp = NULL;
	c_dwEntrySelMap = 0;
}

CDump::~CDump()
{
}

void CDump::ClearEntryList()
{
	c_dwEntrySelMap = 0;
	memset(&c_entrys, 0, sizeof(ENTRYSHAPE) * MAXLENGTH_ENTRY);
}

void CDump::PioState()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&icdb, 0, sizeof(IOCDUMPSTCTAIL));

	//CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < LENGTH_TU; n ++) {
			if (pPaper->GetRouteState(n)) {
				int tid = (int)pDoz->recip[n].real.prol.cProperID;
				if (tid >= LENGTH_TU)	continue;
				for (int m = 0; m < 8; m ++)
					icdb.wTi[tid][m] = XWORD(pDoz->recip[n].real.inp[m < 4 ? 1 : 0].w[m & 3]);	// CFio에서 바꾸었으므로 여기서 바꾼다.
				icdb.wTo[tid] = XWORD(pDoz->recip[n].real.outp.w);
				WORD w = LOWWORD(pDoz->recip[n].real.avs.dwPwm);	// period of pwm
				icdb.wAi[tid][0] = XWORD(w);
				w = HIGHWORD(pDoz->recip[n].real.avs.dwPwm);		// width of pwm
				icdb.wAi[tid][1] = XWORD(w);
				icdb.wAi[tid][2] = XWORD(pDoz->recip[n].real.avs.wTacho);
				icdb.wAi[tid][3] = XWORD(pDoz->recip[n].real.avs.wCurve[ADCONVCH_BATTERY]);
				icdb.wAi[tid][4] = XWORD(pDoz->recip[n].real.avs.wCurve[ADCONVCH_PRESSURE]);
				icdb.wAi[tid][5] = XWORD(pDoz->recip[n].real.avs.wCurve[ADCONVCH_OUTTEMP]);
			}
		}

		int hofs = (int)pPaper->GetHeadCarOffset();
		for (int n = 0; n < (int)pPaper->GetLength() - 2; n ++) {
			if (pPaper->GetRouteState(n + LENGTH_TU)) {
				for (int m = 0; m < 4; m ++)
					icdb.wCi[n + hofs][m] = XWORD(pDoz->recip[n + LENGTH_TU].real.inp[0].w[m]);
				icdb.wCo[n + hofs] = XWORD(pDoz->recip[n + LENGTH_TU].real.outp.w);
				icdb.wPi[n + hofs] = XWORD(pDoz->recip[n + LENGTH_TU].real.avs.wCurve[ADCONVCH_PRESSURE]);
			}
		}

		GETARCHIVE(pArch);
		PCAREERS pCrr = pArch->GetCareers();
		_QUARTET quar;
		quar.dw = (DWORD)(pCrr->real.dbDistance / 1000.f);	// 18/10/19
		icdb.dwDistance = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
		for (int n = 0; n < SIV_MAX; n ++) {
			quar.dw = (DWORD)(pCrr->real.dbSivPower[n] / 1000.f);
			icdb.dwSivPwr[n] = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
		}
		for (int n = 0; n < V3F_MAX; n ++) {
			quar.dw = (DWORD)(pCrr->real.dbV3fPower[n] / 1000.f);
			icdb.dwV3fPwr[n] = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
			quar.dw = (DWORD)(pCrr->real.dbV3fReviv[n] / 1000.f);
			icdb.dwV3fRev[n] = ((DWORD)quar.c[0] << 24) | ((DWORD)quar.c[1] << 16) | ((DWORD)quar.c[2] << 8) | (DWORD)quar.c[3];
		}

		//bool bTenor = pPaper->GetTenor();	// false:왼쪽 방향, true:오른쪽 방향
		GETSCHEME(pSch);
		PDEVCONF pConf = pSch->GetDevConf();
		for (int n = 0; n < (int)pConf->siv.cLength; n ++) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[n]);
			// 200218
			//if (pPaper->GetRouteState(fid))	icdb.trace.wSiv[n] = XWORD((WORD)pDoz->recip[fid].real.svf.s.a.t.s.trc.b.leng);
			if (pPaper->GetRouteState(fid))
				icdb.trace.wSiv[n] =
								!pPaper->GetDeviceExFromRecip() ?
								XWORD((WORD)pDoz->recip[fid].real.svf.s.a.t.s.trc.b.leng) :
								XWORD((WORD)pDoz->recip[fid].real.svf.s2.a.t.s.trc.b.leng);
		}
		for (int n = 0; n < (int)pConf->v3f.cLength; n ++) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->v3f.cPos[n]);
			if (pPaper->GetRouteState(fid))	icdb.trace.wV3f[n] = XWORD((WORD)pDoz->recip[fid].real.svf.v.a.t.s.trc.b.leng);
		}
		for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
			int fid = pSch->C2F(pPaper->GetTenor(), n);
			// 200218
			//if (pPaper->GetRouteState(fid))	icdb.trace.wEcu[n + hofs] = XWORD((WORD)pDoz->recip[fid].real.ecu.a.t.s.trc.b.leng);
			if (pPaper->GetRouteState(fid))
				icdb.trace.wEcu[n + hofs] =
						XWORD((WORD)(pPaper->GetDeviceExFromRecip() ?
						pDoz->recip[fid].real.ecu.ct.a.t.s.trc.b.leng : pDoz->recip[fid].real.ecu.co.a.t.s.trc.b.leng));
		}
		// 200218
		CVerse::PPSHARE pPsv = pVerse->GetShareVars();
		memcpy(icdb.wBatC, pPsv->bmscf.cvs, sizeof(BMSCVS) * 2);
		WORD wLen = (WORD)(pPaper->GetLength() - 1);
		icdb.wTcRight = XWORD(wLen);
	}
	pVerse->CopyBundle(&icdb, sizeof(IOCDUMPSENTENCE));
}

void CDump::LineText(WORD wCid, WORD dhid)
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&ledb, 0, sizeof(LKEDUMPSTCTAIL));
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETSCHEME(pSch);
	BYTE ci = (BYTE)(wCid - HK_CAR0);
	BYTE n;
	int fi;
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		switch (dhid) {
		case HK_SIDE :
			ci = ci != 0 ? 1 : 0;
			memcpy(&ledb.wLine[0],  &pPsv->atocf.fl[ci].r, SIZE_ATOSR);
			memcpy(&ledb.wLine[7],  &pPsv->atocf.fl[ci].a, SIZE_ATOSA);
			memcpy(&ledb.wLine[18], &pPsv->piscf.fl[ci].r, SIZE_PISSR);
			memcpy(&ledb.wLine[36], &pPsv->paucf.fl[ci].a, SIZE_PAUSA);
			memcpy(&ledb.wLine[41], &pPsv->piscf.fl[ci].a, SIZE_PISSA);
			memcpy(&ledb.wLine[58], &pPsv->trscf.fl[ci].r, SIZE_TRSSR);
			memcpy(&ledb.wLine[63], &pPsv->trscf.fl[ci].a, SIZE_TRSSA);
			//200218
			memcpy(&ledb.wLine[68], &pPsv->fducf.fl[ci].r, SIZE_FDUSR);
			memcpy(&ledb.wLine[80], &pPsv->fducf.fl[ci].a, SIZE_FDUSAAB);
			break;
		case HK_V3F :
			n = pSch->GetV3fID(ci);
			if (n < 0xff) {
				memcpy(&ledb.wLine[0],  &pLsv->v3fcf.fl[n].r, SIZE_V3FSR);
				// 171221
				//memcpy(&ledb.wLine[12], &pLsv->v3fcf.fl[n].a, sizeof(V3FAINFO));
				fi = pSch->C2F(pPaper->GetTenor(), ci);
				if (pPaper->GetRouteState(fi) && pLsv->v3fcf.fl[n].r.t.s.cFlow == DEVTRACE_REQ &&
					pDoz->recip[fi].real.cst.cID == CSTID_TRACE &&
					//(pDoz->recip[fi].real.cst.roll.trc.ans.wDevIDMap & ((1 << ci) | (1 << TRACEITEM_V3F))) == ((1 << ci) | (1 << TRACEITEM_V3F))) {
					pDoz->recip[fi].real.cst.roll.trc.ans.wDevIDMap == ((1 << ci) | (1 << TRACEITEM_V3F))) {
					ledb.wLine[12] = LADD_V3F;
					ledb.wLine[13] = ((WORD)DEVTRACE_ANS << 8) | (WORD)DEVCHAR_CTL;
					memcpy(&ledb.wLine[14], pDoz->recip[fi].real.cst.roll.trc.c, sizeof(V3FAINFO) - 4);
				}
				else	memcpy(&ledb.wLine[12], &pLsv->v3fcf.fl[n].a, sizeof(V3FAINFO));
			}
			break;
		case HK_ECU : {
				// 200218
				//memcpy(&ledb.wLine[0],  &pLsv->ecucf.fl[ci].r, SIZE_ECUSR);
				//memcpy(&ledb.wLine[6],  &pLsv->ecucf.fl[ci].a, SIZE_ECUSA);
				if (pPaper->GetDeviceExFromRecip()) {
					memcpy(&ledb.wLine[0],  &pLsv->ecucf.fl[ci].ct.r, SIZE_ECUSR);
					memcpy(&ledb.wLine[6],  &pLsv->ecucf.fl[ci].ct.a, SIZE_ECUSA);
				}
				else {
					memcpy(&ledb.wLine[0],  &pLsv->ecucf.fl[ci].co.r, SIZE_ECUSR);
					memcpy(&ledb.wLine[6],  &pLsv->ecucf.fl[ci].co.a, SIZE_ECUSA);
				}
			}
			memcpy(&ledb.wLine[23], &pLsv->hvaccf.fl[ci].r, SIZE_HVACSR);
			memcpy(&ledb.wLine[33], &pLsv->hvaccf.fl[ci].a, SIZE_HVACSA);
			break;
		case HK_SIV :
			n = pSch->GetSivID(ci);
			if (n < 0xff) {
				if (!pPaper->GetDeviceExFromRecip())
					memcpy(&ledb.wLine[0], &pLsv->sivcf.fl[n].r, SIZE_SIVSR);
				else	memcpy(&ledb.wLine[0], &pLsv->sivcf.fl2[n].r, SIZE_SIVSR);
				// 171221
				//memcpy(&ledb.wLine[7], &pLsv->sivcf.fl[n].a, sizeof(SIVAINFOEX));
				fi = pSch->C2F(pPaper->GetTenor(), ci);
				if (pPaper->GetRouteState(fi) && pLsv->sivcf.fl[n].r.t.s.cFlow == DEVTRACE_REQ &&
					pDoz->recip[fi].real.cst.cID == CSTID_TRACE &&
					//(pDoz->recip[fi].real.cst.roll.trc.ans.wDevIDMap & ((1 << ci) | (1 << TRACEITEM_SIV))) == ((1 << ci) | (1 << TRACEITEM_SIV))) {
					pDoz->recip[fi].real.cst.roll.trc.ans.wDevIDMap == ((1 << ci) | (1 << TRACEITEM_SIV))) {
					ledb.wLine[7] = LADD_SIV;
					ledb.wLine[8] = ((WORD)DEVTRACE_ANS << 8) | (WORD)DEVCHAR_CTL;
					memcpy(&ledb.wLine[9], pDoz->recip[fi].real.cst.roll.trc.c, sizeof(SIVAINFOEX) - 4);
				}
				else {
					if (!pPaper->GetDeviceExFromRecip())
						memcpy(&ledb.wLine[7], &pLsv->sivcf.fl[n].a, sizeof(SIVAINFOEX));
					else	memcpy(&ledb.wLine[7], &pLsv->sivcf.fl2[n].a, sizeof(SIVAINFO2));
				}
				// 200218
				if (pPaper->GetDeviceExFromRecip()) {
					memcpy(&ledb.wLine[22], &pPsv->bmscf.fl[n != 0 ? 1 : 0].r, SIZE_BMSSR);
					memcpy(&ledb.wLine[30], &pPsv->bmscf.fl[n != 0 ? 1 : 0].a, SIZE_BMSSAAB);
					memcpy(&ledb.wLine[42], &pLsv->cmsbcf.fl[n].r, SIZE_CMSBSR);
					memcpy(&ledb.wLine[49], &pLsv->cmsbcf.fl[n].a, SIZE_CMSBSA);
				}
			}
			break;
		case HK_DCU :
			memcpy(&ledb.wLine[0],  &pLsv->dcucf.fl[ci][4].r, SIZE_DCUSR);
			for (BYTE m = 0; m < 4; m ++)
				memcpy(&ledb.wLine[m * 5 + 6],  &pLsv->dcucf.fl[ci][m + 4].a, SIZE_DCUSA);
			memcpy(&ledb.wLine[26], &pLsv->dcucf.fl[ci][0].r, SIZE_DCUSR);
			for (BYTE m = 0; m < 4; m ++)
				memcpy(&ledb.wLine[m * 5 + 32], &pLsv->dcucf.fl[ci][m].a, SIZE_DCUSA);
			break;
		default :	break;
		}
		ledb.wLine[98] = XWORD(wCid);
		ledb.wLine[99] = XWORD(dhid);
	}

	GETARCHIVE(pArch);
	PENVIRONS pEnv = pArch->GetEnvirons();
	WORD w;
	for (int n = 0; n < LENGTH_WHEELDIA; n ++) {
		w = (WORD)pEnv->real.cWheelDiai[n] + MIN_WHEELDIA;
		ledb.wWheelDiam[n] = XWORD(w);
	}
	for (int n = 0; n < 10; n ++) {		// pPaper->GetHeadCarOffset()이 필요없다. DU에서 애니메이션을 사용하였다.
		w = (WORD)pEnv->real.cTempSv[n];
		ledb.wTempSv[n] = XWORD(w);
	}
	w = (WORD)pEnv->real.cVentTime;
	ledb.wVentSv = XWORD(w);
	// 171129
	//w = pVerse->GetPisMode() ? 1 : 0;
	//ledb.wPisManual = XWORD(w);
	w = pDoz->recip[FID_HEAD].real.motiv.lead.b.pisaim ? 1 : 0;
	ledb.wPisManual = XWORD(w);

	//GETTOOL(pTool);
	// 171122
	for (int n = 0; n < (CID_MAX + 2); n ++)
		ledb.wVertc[n] = ledb.wVermm[n] = ledb.wVerFpga[n] = 0;

	//BYTE tid;
	//bool bTenor = pPaper->GetTenor();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		//tid = bTenor ? 2 : 0;
		for (int n = 0; n < LENGTH_TU; n ++) {
			// 171122
			// 171019
			//w = pTool->ToHex(pDoz->recip[n].real.prol.wVersion);
			//w = (WORD)(pDoz->recip[n].real.prol.cVersion + 200);
			//ledb.wVertc[tid & 3] = XWORD(w);
			// 170809
			//w = (WORD)(pDoz->recip[n].real.cMmVer + 200);
			//ledb.wVermm[tid & 3] = XWORD(w);
			//++ tid;
			if (pPaper->GetRouteState(n)) {
				w = (WORD)(pDoz->recip[n].real.prol.cVersion + 200);
				ledb.wVertc[pDoz->recip[n].real.prol.cProperID] = XWORD(w);
				w = (WORD)(pDoz->recip[n].real.cMmVer + 200);
				ledb.wVermm[pDoz->recip[n].real.prol.cProperID] = XWORD(w);
				w = pDoz->recip[n].real.wFpgaVersion;
				ledb.wVerFpga[pDoz->recip[n].real.prol.cProperID] = XWORD(w);
			}
		}
		int hofs = pPaper->GetHeadCarOffset();
		for (int n = 0; n < (int)pPaper->GetLength() - 2; n ++) {
			// 171122
			// 171019
			//w = pTool->ToHex(pDoz->recip[n + LENGTH_TU].real.prol.wVersion);
			//w = (WORD)(pDoz->recip[n + LENGTH_TU].real.prol.cVersion + 200);
			//ledb.wVertc[n + hofs + LENGTH_TU] = XWORD(w);
			//w = (WORD)(pDoz->recip[n + LENGTH_TU].real.cMmVer + 200);
			//ledb.wVermm[n + hofs + LENGTH_TU] = XWORD(w);
			if (pPaper->GetRouteState(n + LENGTH_TU)) {
				w = (WORD)(pDoz->recip[n + LENGTH_TU].real.prol.cVersion + 200);
				ledb.wVertc[n + hofs + LENGTH_TU] = XWORD(w);
				w = (WORD)(pDoz->recip[n + LENGTH_TU].real.cMmVer + 200);
				ledb.wVermm[n + hofs + LENGTH_TU] = XWORD(w);
				w = pDoz->recip[n + LENGTH_TU].real.wFpgaVersion;
				ledb.wVerFpga[n + hofs + LENGTH_TU] = XWORD(w);
			}
		}
	}

	pVerse->CopyBundle(&ledb, sizeof(LKEDUMPSENTENCE));
}

//void CDump::CopyRedBlock(int di, int si, bool bInv)
//{
//	GETVERSE(pVerse);
//	PBUNDLES pBund = pVerse->GetBundle();
//
//	GETPROSE(pProse);
//	CProse::PLSHARE pLsv = pProse->GetShareVars();
//	XWORDF(lksb.wEcu[di], pLsv->wReceivedRed[DEVID_ECU][si]);
//	XWORDF(lksb.wHvac[di], pLsv->wReceivedRed[DEVID_HVAC][si]);
//	// 190109
//	if (!bInv) {
//		for (int n = 0; n < 8; n ++)
//			XWORDF(lksb.wDcu[di][n], pLsv->wSubjectiveRed[n + DEVID_DCUL][si]);
//	}
//	else {
//		for (int n = 0;n < 8; n ++)
//			XWORDF(lksb.wDcu[di][7 - n], pLsv->wSubjectiveRed[n + DEVID_DCUL][si]);
//	}
//}
//
void CDump::LineState()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	memset(&lksb, 0, sizeof(LKSTATESENTENCE));
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();

	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	GETPAPER(pPaper);
	GETSCHEME(pSch);
	PDOZEN pDoz = pPaper->GetDozen();
	if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		for (int n = 0; n < LENGTH_TU; n ++) {
			// 17/07/06, Modify
			//int tid = (int)pDoz->recip[n].real.prol.cProperID;
			//XWORDF(lksb.wDtb[tid], pPaper->GetRed(n));
			XWORDF(lksb.wDtb[n], pPaper->GetRed(n));
		}

		int hofs = pPaper->GetHeadCarOffset();
		for (int n = 0; n < (int)pPaper->GetLength() - 2; n ++)
			XWORDF(lksb.wDtb[n + hofs + 4], pPaper->GetRed(n + LENGTH_TU));

		DWORD route = pPaper->GetRoute();
		BYTE ti = pPaper->GetTenor() ? 1 : 0;

		_QUARTET qta, qtb;
		qta.dw = qtb.dw = 0;
		for (int n = FID_HEAD; n < (int)pPaper->GetLength() + 2; n ++) {
			if (route & (CPaper::ROUTE_MAIN << (n << 1)))	qta.dw |= (1 << c_cLineBar[pDoz->recip[n].real.prol.cProperID]);
			if (route & (CPaper::ROUTE_AUX << (n << 1)))	qtb.dw |= (1 << c_cLineBar[pDoz->recip[n].real.prol.cProperID]);
		}

		for (int n = 0; n < 7; n ++) {
			if (qta.dw & c_dwFillBar[ti][n][0])	qta.dw |= c_dwFillBar[ti][n][1];
			if (qtb.dw & c_dwFillBar[ti][n][0])	qtb.dw |= c_dwFillBar[ti][n][1];
		}
		lksb.wDtbas[0] = XWORD(qta.w[0]);
		lksb.wDtbas[1] = XWORD(qta.w[1]);
		lksb.wDtbbs[0] = XWORD(qtb.w[0]);
		lksb.wDtbbs[1] = XWORD(qtb.w[1]);

		for (int n = 0; n < 2; n ++) {
			// 200218
			//di = n ? 9 : 0;
			//si = n ? (int)pPaper->GetLength() - 1 : 0;
			//CopyRedBlock(di, si, si == (int)(pPaper->GetLength() - 1) ? true : false);
			XWORDF(lksb.wAto[n], pPsv->wReceivedRed[PDEVID_ATO][n]);	//[PDEVRED_ATO][n]);
			XWORDF(lksb.wPau[n], pPsv->wReceivedRed[PDEVID_PAU][n]);	//[PDEVRED_PAU][n]);
			XWORDF(lksb.wPis[n], pPsv->wReceivedRed[PDEVID_PIS][n]);	//[PDEVRED_PIS][n]);
			XWORDF(lksb.wTrs[n], pPsv->wReceivedRed[PDEVID_TRS][n]);	//[PDEVRED_TRS][n]);
			XWORDF(lksb.wRtd[n], pPsv->wReceivedRed[PDEVID_RTD][n]);	//PDEVRED_RTD][n]);
			// 200218
			XWORDF(lksb.wFdu[n], pPsv->wReceivedRed[PDEVID_FDU][n]);
			XWORDF(lksb.wBms[n], pPsv->wReceivedRed[PDEVID_BMS][n]);
		}

		// 200218
		//for (int n = 0; n < (int)pPaper->GetLength() - 2; n ++)
		//	CopyRedBlock(n + hofs + 1, n + 1, false);
		int si, vi;
		si = vi = 0;
		for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
			WORD wDevMap = pSch->GetLDeviceMap(n);
			if (wDevMap & DEVBF_SIV) {
				XWORDF(lksb.wSiv[si], pLsv->wReceivedRed[DEVID_SIV][n]);
				XWORDF(lksb.wCmsb[si ++], pLsv->wReceivedRed[DEVID_CMSB][n]);
			}
			if (wDevMap & DEVBF_V3F)	XWORDF(lksb.wV3f[vi ++], pLsv->wReceivedRed[DEVID_V3F][n]);
			XWORDF(lksb.wEcu[n + hofs], pLsv->wReceivedRed[DEVID_ECU][n]);
			XWORDF(lksb.wHvac[n + hofs], pLsv->wReceivedRed[DEVID_HVAC][n]);
			for (int m = 0; m < 8; m ++)
				XWORDF(lksb.wDcu[n][n < (int)(pPaper->GetLength() - 1) ? m : (7 - m)], pLsv->wReceivedRed[DEVID_DCUL + m][n]);
		}

		//PDEVCONF pConf = pSch->GetDevConf();
		//for (int n = 0; n < pConf->siv.cLength; n ++)
		//	XWORDF(lksb.wSiv[n], pLsv->wReceivedRed[DEVID_SIV][n]);
		//for (int n = 0; n < pConf->v3f.cLength; n ++)
		//	XWORDF(lksb.wV3f[n], pLsv->wReceivedRed[DEVID_V3F][n]);

		GETBAND(pBand);
		CBand::PDTBTIMES pDtm = pBand->GetDtbTimes();
		for (int n = 0; n < 13; n ++) {
			lksb.wDtbat[n] = XWORD(pDtm->wTime[0][n]);
			lksb.wDtbbt[n] = XWORD(pDtm->wTime[1][n]);
		}
		WORD w = pVerse->GetRedHour();
		lksb.wHour = XWORD(w);
		w = (WORD)pVerse->GetRedMin();
		lksb.wMin = XWORD(w);
		w = (WORD)pVerse->GetRedSec();
		lksb.wSec = XWORD(w);
		//lksb.wUpdate = 0xffff;
	}
	pVerse->CopyBundle(&lksb, sizeof(LKSTATESENTENCE));
}

WORD CDump::GetSortListByPage(WORD wPage, bool bTrouble)
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	PELISTSTCTAIL pLs = bTrouble ? &pBund->tli : &pBund->sli;
	memset(pLs, 0, sizeof(ELISTSTCTAIL));

	GETARCHIVE(pArch);
	PTROUBLETRAY pTray;
	if (bTrouble)	pTray = pArch->GetTroubleTray(TTRAY_TROUBLE);
	else	pTray = pArch->GetTroubleTray(TTRAY_STATUS);

	WORD w;
	// 181001
//	if (pTray->dwIndex > 0) {
	if (pTray->dwIndex > TROUBLEBUFINDEX_BEGIN) {
		WORD line = 0;
		GETTOOL(pTool);
		// 201124
		//for (DWORD dw = pTray->dwIndex - 1 - (wPage * TROUBLELIST_LINE); dw >= pTray->dwReduce; dw --) {
		for (DWORD dw = pTray->dwIndex - 1 - (wPage * TROUBLELIST_LINE); dw >= TROUBLEBUFINDEX_BEGIN; dw --) {
			pLs->s.date[line].b.mon = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.mon);
			pLs->s.date[line].b.day = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.day);
			pLs->s.occurrence[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.uhour);
			pLs->s.occurrence[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.umin);
			pLs->s.second[line].b.occur = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.usec);
			pLs->s.destroy[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.dhour);
			pLs->s.destroy[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.dmin);
			pLs->s.second[line].b.dest = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.dsec);
			w = pTray->cellar[dw].b.cid;
			pLs->s.wCID[line] = XWORD(w);
			w = pTray->cellar[dw].b.code;
			pLs->s.wReal[line] = XWORD(w);
			if (++line >= TROUBLELIST_LINE)	break;
		}
	}
	pLs->s.wCurPage = XWORD(wPage + 1);
	w = (WORD)(pTray->dwIndex & 0xffff);
	WORD wTotal = (WORD)(pTray->dwIndex / TROUBLELIST_LINE);
	if (pTray->dwIndex % TROUBLELIST_LINE)	++ wTotal;
	pLs->s.wTotalPage = XWORD(wTotal);
	pLs->s.wIndex = XWORD(w);
	w = (WORD)(pTray->dwReduce & 0xffff);
	pLs->s.wReduce = XWORD(w);
	w = (WORD)(pTray->dwHeavy & 0xffff);
	pLs->s.wAlarms = XWORD(w);
	pLs->s.wFence = 0;
	pVerse->CopyBundle(pLs, sizeof(ELISTSENTENCE));

	if (wTotal > 0)	-- wTotal;
	return wTotal;
}

//WORD CDump::GetSortList(WORD wPage, bool bTrouble, bool bAll)
//{
//	GETVERSE(pVerse);
//	PBUNDLES pBund = pVerse->GetBundle();
//	PELISTSTCTAIL pLs = bTrouble ? &pBund->tli : &pBund->sli;
//	memset(pLs, 0, sizeof(ELISTSTCTAIL));
//
//	GETARCHIVE(pArch);
//	PTROUBLETRAY pTray;
//	if (bTrouble)	pTray = pArch->GetTroubleTray(TTRAY_TROUBLE);
//	else	pTray = pArch->GetTroubleTray(TTRAY_STATUS);
//
//	WORD w;
//	if (pTray->dwIndex > 0) {
//		WORD line = 0;
//		WORD curp = 0;
//		bool bCharge = false;
//		GETTOOL(pTool);
//
//		for (DWORD dw = pTray->dwIndex - 1; dw >= pTray->dwReduce; dw --) {
//			if (bAll || !pTray->cellar[dw].b.down) {
//				if (curp == wPage && !bCharge) {
//					pLs->s.date[line].b.mon = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.mon);
//					pLs->s.date[line].b.day = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.day);
//					pLs->s.occurrence[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.uhour);
//					pLs->s.occurrence[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.umin);
//					pLs->s.second[line].b.occur = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.usec);
//					pLs->s.destroy[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.dhour);
//					pLs->s.destroy[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.dmin);
//					pLs->s.second[line].b.dest = (BYTE)pTool->ToBcd((BYTE)pTray->cellar[dw].b.dsec);
//					w = pTray->cellar[dw].b.cid;
//					pLs->s.wCID[line] = XWORD(w);
//					w = pTray->cellar[dw].b.code;
//					pLs->s.wReal[line] = XWORD(w);
//				}
//				if (++ line >= TROUBLELIST_LINE) {
//					line = 0;
//					if (curp == wPage)	bCharge = true;
//					else if (curp < wPage)	++ curp;
//				}
//			}
//			if (bCharge)	break;		// 12줄만 하고 그만한다. 계속하다보니 dwIndex가 너무 많아질 때 CLand::Arteriam()의 실행시간이 길어져 두달간 애먹었다.
//		}
//	}
//	pLs->s.wCurPage = XWORD(wPage + 1);
//	w = (WORD)(pTray->dwIndex & 0xffff);
//	WORD wTotal = (WORD)(pTray->dwIndex / TROUBLELIST_LINE);
//	if (pTray->dwIndex % TROUBLELIST_LINE)	++ wTotal;
//	pLs->s.wTotalPage = XWORD(wTotal);
//	pLs->s.wIndex = XWORD(w);
//	w = (WORD)(pTray->dwReduce & 0xffff);
//	pLs->s.wReduce = XWORD(w);
//	w = (WORD)(pTray->dwHeavy & 0xffff);
//	pLs->s.wAlarms = XWORD(w);
//	pLs->s.wFence = 0;
//	pVerse->CopyBundle(pLs, sizeof(ELISTSENTENCE));
//
//	if (wTotal > 0)	-- wTotal;
//	return wTotal;
//}
//
WORD CDump::GetEachListByPage(WORD wID, WORD wPage)
{
	GETPAPER(pPaper);
	if (wID > pPaper->GetLength() + 2)	wID = 0;

	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	PELISTSTCTAIL pLs = &pBund->tli;
	memset(pLs, 0, sizeof(ELISTSTCTAIL));

	GETARCHIVE(pArch);
	PTROUBLEBYCAR pTrbc = pArch->GetTroubleListForEachCar(wID);

	WORD line = 0;
	WORD fence = 0;
	WORD w;
	GETTOOL(pTool);
	int fi = 0;
	for (int n = (int)pTrbc->wCur - (wPage * TROUBLELIST_LINE); n >= (int)pTrbc->wMin; n --) {
		if (pTrbc->ts[n].b.mon > 0 && pTrbc->ts[n].b.mon < 13) {
			pLs->s.date[line].b.mon = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.mon);
			pLs->s.date[line].b.day = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.day);
			pLs->s.occurrence[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.uhour);
			pLs->s.occurrence[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.umin);
			pLs->s.second[line].b.occur = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.usec);
			pLs->s.destroy[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.dhour);
			pLs->s.destroy[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.dmin);
			pLs->s.second[line].b.dest = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.dsec);
			w = pTrbc->ts[n].b.cid;
			pLs->s.wCID[line] = XWORD(w);
			w = pTrbc->ts[n].b.code;
			pLs->s.wReal[line] = XWORD(w);
			if (n == (int)pTrbc->wSlash)	fence = (1 << fi);
			++ line;
			if (line >= TROUBLELIST_LINE)	break;
			++ fi;
		}
		if (line >= TROUBLELIST_LINE)	break;
	}
	pLs->s.wCurPage = XWORD(wPage + 1);
	w = pTrbc->wCur - pTrbc->wMin;
	WORD wTotal = w / TROUBLELIST_LINE;
	if (w % TROUBLELIST_LINE)	++ wTotal;
	pLs->s.wTotalPage = XWORD(wTotal);

	// 200218
	// 171227
	//PDOZEN pDoz = pPaper->GetDozen();
	//if (pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab)	pLs->s.wFence = XWORD(fence);
	if (pPaper->GetLabFromRecip())	pLs->s.wFence = XWORD(fence);
	else	pLs->s.wFence = 0;
	pVerse->CopyBundle(pLs, sizeof(ELISTSENTENCE));

	if (wTotal > 0)	-- wTotal;
	return wTotal;
}

//WORD CDump::GetTroubleForEachCar(WORD wID, WORD wPage)
//{
//	GETPAPER(pPaper);
//	if (wID > pPaper->GetLength() + 2)	wID = 0;
//
//	GETVERSE(pVerse);
//	PBUNDLES pBund = pVerse->GetBundle();
//
//	PELISTSTCTAIL pLs = &pBund->tli;
//	memset(pLs, 0, sizeof(ELISTSTCTAIL));
//
//	GETARCHIVE(pArch);
//	PTROUBLEBYCAR pTrbc = pArch->GetTroubleListForEachCar(wID);
//
//	WORD line = 0;
//	WORD curp = 0;
//	WORD fence = 0;
//	bool bCharge = false;
//	WORD w;
//	GETTOOL(pTool);
//	int fi = 0;
//	for (int n = (int)pTrbc->wCur; n >= (int)pTrbc->wMin; n --) {
//		if (pTrbc->ts[n].b.mon > 0 && pTrbc->ts[n].b.mon < 13) {
//			if (curp == wPage && !bCharge) {
//				pLs->s.date[line].b.mon = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.mon);
//				pLs->s.date[line].b.day = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.day);
//				pLs->s.occurrence[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.uhour);
//				pLs->s.occurrence[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.umin);
//				pLs->s.second[line].b.occur = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.usec);
//				pLs->s.destroy[line].b.hour = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.dhour);
//				pLs->s.destroy[line].b.min = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.dmin);
//				pLs->s.second[line].b.dest = (BYTE)pTool->ToBcd((BYTE)pTrbc->ts[n].b.dsec);
//				w = pTrbc->ts[n].b.cid;
//				pLs->s.wCID[line] = XWORD(w);
//				w = pTrbc->ts[n].b.code;
//				pLs->s.wReal[line] = XWORD(w);
//				if (n == (int)pTrbc->wSlash)	fence = (1 << fi);
//			}
//			if (++ line >= TROUBLELIST_LINE) {
//				line = 0;
//				if (curp == wPage)	bCharge = true;
//				else if (curp < wPage) ++ curp;
//			}
//		}
//		//if (n == (int)pTrbc->wSlash)	fence = (1 << fi);
//		if (bCharge)	break;
//		++ fi;
//	}
//	pLs->s.wCurPage = XWORD(wPage + 1);
//	w = pTrbc->wCur - pTrbc->wMin;
//	WORD wTotal = w / TROUBLELIST_LINE;
//	if (w % TROUBLELIST_LINE)	++ wTotal;
//	pLs->s.wTotalPage = XWORD(wTotal);
//
//	// 171227
//	PDOZEN pDoz = pPaper->GetDozen();
//	if (pDoz->recip[FID_HEAD].real.sign.scmdEx.b.lab)	pLs->s.wFence = XWORD(fence);
//	else	pLs->s.wFence = 0;
//
//	pVerse->CopyBundle(pLs, sizeof(ELISTSENTENCE));
//
//	if (wTotal > 0)	-- wTotal;
//	return wTotal;
//}
//
PENTRYSHAPE CDump::GetEntry(int n)
{
	if (n < MAXLENGTH_ENTRY)	return &c_entrys[n];
	return NULL;
}

void CDump::SetEntrySelMap(DWORD dwSel)
{
	if (dwSel != 0)	c_dwEntrySelMap = dwSel;
	else {
		for (int n = 0; n < MAXLENGTH_ENTRY; n ++) {
			if (c_entrys[n].szName[0] != 0)	c_dwEntrySelMap |= (1 << n);
		}
	}
}

DWORD CDump::MonoEntryList(PVOID pVoid)
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();

	GETTOOL(pTool);
	PBULKPACK pBulk = (PBULKPACK)pVoid;
	DWORD dwIndex = pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex;
	//TRACK("DUMP:entry length %d\n", pBulk->m[BPID_OCCASN].bt.rs.s.wLength);
	for (int n = 0; n < (int)pBulk->m[BPID_OCCASN].bt.pad.sp.wLength; n ++) {
		DWORD dw = (DWORD)(atol(&pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[n].szName[1]) * 100);
			// yymmddhh
		dw %= 100000000;
		//TRACK("DUMP:entry%d:%06d\n", n, dw);
		WORD w = (WORD)pTool->ToBcd((WORD)(dw / 10000));
		entb.wYm[dwIndex] = XWORD(w);
		w = (WORD)pTool->ToBcd((WORD)(dw % 10000));
		entb.wDh[dwIndex] = XWORD(w);
		entb.wMs[dwIndex] = 0;
		strncpy(c_entrys[dwIndex].szName, pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[n].szName, SIZE_ENTRYNAME);
		c_entrys[dwIndex].dwSize = pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[n].dwSize;
		if (++ dwIndex >= pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal)	break;
	}
	return dwIndex;
}

DWORD CDump::TriEntryList(PVOID pVoid, _QUARTET quTri)
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();

	GETTOOL(pTool);
	PBULKPACK pBulk = (PBULKPACK)pVoid;
	DWORD dwIndex = pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex;
	//TRACK("DUMP:entry length %d\n", pBulk->m[BPID_OCCASN].bt.rs.s.wLength);
	for (int n = 0; n < (int)pBulk->m[BPID_OCCASN].bt.pad.sp.wLength; n ++) {
		int id;
		if (dwIndex < (QWORD)quTri.c[0])	id = (int)dwIndex;
		else if ((dwIndex - quTri.c[0]) < (QWORD)quTri.c[1])	id = (int)(dwIndex - quTri.c[0] + LENGTH_PDTENTRY);
		else	id = (int)(dwIndex - (quTri.c[0] + quTri.c[1]) + LENGTH_PDTENTRY + LENGTH_DAILYENTRY);

		QWORD qw = (QWORD)atoll(&pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[n].szName[1]);
			// yymmddhhmmss
		qw %= 1000000000000ll;				//   ddhhmmss
		//TRACK("DUMP:entry%d(%d):%012llu\n", n, id, qw);
		WORD w = (WORD)pTool->ToBcd((WORD)(qw / 100000000));
		entb.wYm[id] = XWORD(w);
			// ddhhmmss
		qw %= 100000000;				//  mmss
		w = (WORD)pTool->ToBcd((WORD)(qw / 10000));
		entb.wDh[id] = XWORD(w);
		w = (WORD)pTool->ToBcd((WORD)(qw % 10000));
		entb.wMs[id] = XWORD(w);
		strncpy(c_entrys[id].szName, pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[n].szName, SIZE_ENTRYNAME);
		c_entrys[id].dwSize = pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[n].dwSize;
		if (++ dwIndex >= pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal)	break;
	}
	return dwIndex;
}

void CDump::SetItemText()
{
	GETVERSE(pVerse);
	PBUNDLES pBund = pVerse->GetBundle();
	GETARCHIVE(pArch);
	PENVIRONS pEnv = pArch->GetEnvirons();

	GETTOOL(pTool);
	WORD w = pTool->ToBcd(pEnv->real.dt.year);
	ussb.sv.year = XWORD(w);
	w = (WORD)(BYTE)pTool->ToBcd(pEnv->real.dt.mon);
	ussb.sv.month = XWORD(w);
	w = (WORD)(BYTE)pTool->ToBcd(pEnv->real.dt.day);
	ussb.sv.day = XWORD(w);
	w = (WORD)(BYTE)pTool->ToBcd(pEnv->real.dt.hour);
	ussb.sv.hour = XWORD(w);
	w = (WORD)(BYTE)pTool->ToBcd(pEnv->real.dt.min);
	ussb.sv.minute = XWORD(w);
	w = (WORD)(BYTE)pTool->ToBcd(pEnv->real.dt.sec);
	ussb.sv.second = XWORD(w);

	for (int n = 0; n < LENGTH_WHEELDIA; n ++) {
		w = (WORD)pEnv->real.cWheelDiai[n] + MIN_WHEELDIA;
		ussb.sv.wheeldiam[n] = XWORD(w);
	}
	for (int n = 0; n < 10; n ++) {
		w = (WORD)pEnv->real.cTempSv[n];
		ussb.sv.temp[n] = XWORD(w);
	}
	w = (WORD)pEnv->real.cVentTime;
	ussb.sv.venttime = XWORD(w);
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	ussb.sv.pisManual = pDoz->recip[FID_HEAD].real.motiv.lead.b.pisaim ? 1 : 0;
	//ussb.sv.pisManual = pVerse->GetPisMode() ? 1 : 0;
	//ussb.sv.initiativeMode = 0;

	pVerse->CopyBundle(&ussb, sizeof(USERSETSENTENCE));
}
