/*
 * CTurf.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include "Track.h"
#include "CPaper.h"
#include "CSch.h"
#include "CArch.h"
#include "CLand.h"

#include "CTurf.h"

CTurf::CTurf()
{
	c_pParent = NULL;
	Initial();
}

CTurf::~CTurf()
{
}

void CTurf::Initial()
{
	memset(&c_match, 0, sizeof(MATCHSHAKE) * MID_MAX);
}

int CTurf::ScanRequest(int mid, BYTE reqf, BYTE cstID)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	// TIMESET 3 : DTB에서 시간 설정 요구 비트가 있으면 배너의 accept와 agree를 1로 하고 리턴...-ALL
	int rid = -1;
	// 200707
	BYTE coll = reqf | (reqf << 1);
	for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
		// 200218
		//if ((pDoz->recip[fid].real.sign.gcmd.a & reqf) && (cstID < 1 || pDoz->recip[fid].real.cst.cID == cstID)) {
		BYTE* p = mid < MID_CLFC ? &pDoz->recip[fid].real.sign.gcmd.a : &pDoz->recip[fid].real.sign.scmdEx.a;
		if (((*p & coll) == reqf) && (cstID < 1 || pDoz->recip[fid].real.cst.cID == cstID)) {
			// request item same to consult context
			rid = fid;
			break;
		}
	}
	if (rid > -1) {
		c_match[mid].banner.b.accept = true;
		if (!c_match[mid].banner.b.agree) {
			c_match[mid].banner.b.agree = true;
		}
		else	rid = -1;
	}
	return rid;
}

bool CTurf::ScanAgree(int mid)
{
	GETPAPER(pPaper);
	// ONLYTU...

	BYTE reqf, agrf;
	WORD mcode, acode;
	int leng;
	// 200218
	//if (mid == MID_TIME) {
	//	reqf = UNIF_TIMERF;
	//	agrf = UNIF_TIMEAF;
	//	mcode = 19;
	//	acode = 20;
	//	leng = (int)pPaper->GetLength() + 2;
	//}
	if (mid == MID_TIME || mid == MID_CLFC) {
		if (mid == MID_TIME) {
			reqf = UNIF_TIMERF;
			agrf = UNIF_TIMEAF;
			mcode = 19;
			acode = 20;
		}
		else {
			reqf = UNIF_CLFCRF;
			agrf = UNIF_CLFCAF;
			mcode = 94;
			acode = 95;
		}
		leng = (int)pPaper->GetLength() + 2;
	}
	else {
		if (mid == MID_ENV) {
			reqf = UNIF_ENVRF;
			agrf = UNIF_ENVAF;
			mcode = 90;
			acode = 91;
		}
		else {
			reqf = UNIF_CRRRF;
			agrf = UNIF_CRRAF;
			mcode = 92;
			acode = 93;
		}
		leng = FID_PAS;
	}

	PDOZEN pDoz = pPaper->GetDozen();
	int rid = -1;
	for (int fid = FID_HEAD; fid < FID_PAS; fid ++) {
		// 200218
		//if (pDoz->recip[fid].real.sign.gcmd.a & reqf) {
		BYTE* p = mid < MID_CLFC ? &pDoz->recip[fid].real.sign.gcmd.a : &pDoz->recip[fid].real.sign.scmdEx.a;
		if (*p & reqf) {
			rid = fid;
			break;
		}
	}

	GETARCHIVE(pArch);
	GETSCHEME(pSch);
	WORD wAttendMap = (mid == MID_TIME || mid == MID_CLFC) ? pSch->GetFlowBitmap() : 0xf;
	WORD wAgreeMap = 0;
	if (rid > -1) {
		for (int fid = FID_HEAD; fid < leng; fid ++) {
			// 180911
			//if (pPaper->GetRouteState(fid) && (pDoz->recip[fid].real.sign.gcmd.a & agrf))
			// 200218
			//if (pPaper->GetRouteState(fid) && VALIDRECIP(fid) &&
			//	(pDoz->recip[fid].real.sign.gcmd.a & agrf))
			BYTE* p = mid < MID_CLFC ? &pDoz->recip[fid].real.sign.gcmd.a : &pDoz->recip[fid].real.sign.scmdEx.a;
			if (pPaper->GetRouteState(fid) && VALIDRECIP(fid) && (*p & agrf))
				wAgreeMap |= (1 << fid);
		}
		// TIMESET 5 : 모든 차량에서 agree가 나오면 배너의 request와 accept를 지운다.-TU
		if (wAttendMap == wAgreeMap) {
			c_match[mid].banner.b.request = c_match[mid].banner.b.accept = false;
			if (c_match[mid].banner.b.agree)	c_match[mid].wAgreeTimer = TPERIOD_AGREE;
			c_match[mid].wRequestTimer = 0;
			pArch->Cut(CID_ALL, mcode);
			pArch->Cut(CID_BOW, acode);
			pArch->Cut(CID_STERN, acode);
			c_match[mid].wAgreeMap = wAgreeMap;
			c_match[mid].wAttendMap = wAttendMap;
			return true;
		}
	}
	else if (c_match[mid].banner.b.accept) {
		//c_match[mid].banner.b.accept = false;
		c_match[mid].wAgreeTimer = TPERIOD_AGREE;
		for (int fid = FID_HEAD; fid < leng; fid ++) {
			// 180911
			//if (pPaper->GetRouteState(fid) && !(pDoz->recip[fid].real.sign.gcmd.a & agrf)) {
			// 200218
			//if (pPaper->GetRouteState(fid) && VALIDRECIP(fid) &&
			//	!(pDoz->recip[fid].real.sign.gcmd.a & agrf)) {
			BYTE* p = mid < MID_CLFC ? &pDoz->recip[fid].real.sign.gcmd.a : &pDoz->recip[fid].real.sign.scmdEx.a;
			if (pPaper->GetRouteState(fid) && VALIDRECIP(fid) && (*p & agrf)) {
				int cid = pSch->F2C(pPaper->GetTenor(), fid);
				pArch->Shot(cid, (fid == FID_HEADBK || fid == FID_TAILBK) ? acode : mcode);
			}
		}
		c_match[mid].banner.b.accept = false;		// ?????
	}
	c_match[mid].wAgreeMap = wAgreeMap;
	c_match[mid].wAttendMap = wAttendMap;
	return false;
}

void CTurf::Control(PRECIP pRecip)
{
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETARCHIVE(pArch);

	int fid;
	// 시간 설정은 CU에서도 할 수 있어야 하고...
	// TIMESET 4 : 시간 설정-ALL
	if ((fid = ScanRequest(MID_TIME, UNIF_TIMERF, CSTID_NON)) > -1) {
		if (fid != CAR_FSELF()) {
			pArch->SetSysTimeByDbf(pDoz->recip[fid].real.prol.dbft,
					(int)CAR_FSELF() < LENGTH_TU ? pDoz->recip[fid].real.sign.pcmd.b.rtdtr : false);
			// 181023
			if (pDoz->recip[fid].real.sign.pcmd.b.rtdtr) {
				CLand* pLand = (CLand*)c_pParent;
				ASSERTP(pLand);
				pLand->ClearRtdTimeFetch();
			}
			pArch->Shot(CAR_PSELF(), 176);
			TRACK("TURF:time set%d %d\n", pDoz->recip[fid].real.sign.pcmd.b.rtdtr ? 1 : 0, fid);
		}
	}
	// 200218
	if ((fid = ScanRequest(MID_CLFC, UNIF_CLFCRF, CSTID_NON)) > -1) {
		CLand* pLand = (CLand*)c_pParent;
		ASSERTP(pLand);
		pLand->ClearAllRed();
	}
	if (ISTU()) {
		// 환경이나 거리 등은 TU에서만 한다.
		// 181012
		if ((fid = ScanRequest(MID_ENV, UNIF_ENVRF, CSTID_ENVIRONS)) > -1) {
			pArch->AccordEnvirons(fid);
			pArch->Shot(CAR_PSELF(), 178);
			pArch->Shot(CAR_PSELF(), 179);
		}
		if ((fid = ScanRequest(MID_CRR, UNIF_CRRRF, CSTID_CAREERS)) > -1) {
			pArch->AccordCareers(fid);
			pArch->Shot(CAR_PSELF(), 180);
		}
	}

	if (c_match[MID_TIME].wAgreeTimer > 0 && -- c_match[MID_TIME].wAgreeTimer == 0)
		c_match[MID_TIME].banner.b.agree = false;
	pRecip->real.sign.gcmd.b.unifTa = c_match[MID_TIME].banner.b.agree;

	// 200218
	if (c_match[MID_CLFC].wAgreeTimer > 0 && -- c_match[MID_CLFC].wAgreeTimer == 0)
		c_match[MID_CLFC].banner.b.agree = false;
	pRecip->real.sign.scmdEx.b.unifLfca = c_match[MID_CLFC].banner.b.agree;

	if (ISTU()) {
		for (int n = 0; n < MID_MAX; n ++) {
			if (c_match[n].banner.b.request && -- c_match[n].wRequestTimer == 0)
				c_match[n].banner.b.request = false;
		}
		// TIMESET 2 : 배너에서 시간 설정 요구가 있으면 DTB로 전송-TU
		pRecip->real.sign.gcmd.b.unifTr = c_match[MID_TIME].banner.b.request;
		// 181019
		if (!pRecip->real.sign.gcmd.b.unifTr) {
			pRecip->real.sign.pcmd.b.rtdtr = false;
			c_match[MID_TIME].banner.b.aid = false;
		}
		else	pRecip->real.sign.pcmd.b.rtdtr = c_match[MID_TIME].banner.b.aid;

		// 200218
		pRecip->real.sign.scmdEx.b.unifLfcr = c_match[MID_CLFC].banner.b.request;

		if (c_match[MID_ENV].wAgreeTimer > 0 && -- c_match[MID_ENV].wAgreeTimer == 0)
			c_match[MID_ENV].banner.b.agree = false;
		pRecip->real.sign.gcmd.b.unifEa = c_match[MID_ENV].banner.b.agree;
		// 181003
		//pRecip->real.sign.gcmd.b.unifEr = c_match[MID_ENV].banner.b.request;
		pRecip->real.sign.gcmd.b.unifEr = !c_match[MID_CRR].banner.b.request ? c_match[MID_ENV].banner.b.request : false;

		if (c_match[MID_CRR].wAgreeTimer > 0 && -- c_match[MID_CRR].wAgreeTimer == 0)
			c_match[MID_CRR].banner.b.agree = false;
		pRecip->real.sign.gcmd.b.unifCa = c_match[MID_CRR].banner.b.agree;
		// 181003
		//pRecip->real.sign.gcmd.b.unifCr = c_match[MID_CRR].banner.b.request;
		pRecip->real.sign.gcmd.b.unifCr = !c_match[MID_ENV].banner.b.request ? c_match[MID_CRR].banner.b.request : false;
	}
}

void CTurf::Monitor(PRECIP pRecip)
{
	GETPAPER(pPaper);
	//CUCUTOFF();

	if (ISTU()) {
		ScanAgree(MID_TIME);
		ScanAgree(MID_ENV);
		ScanAgree(MID_CRR);
		ScanAgree(MID_CLFC);
	}
	else {
		PDOZEN pDoz = pPaper->GetDozen();
		if (!pDoz->recip[FID_HEAD].real.sign.gcmd.b.unifTr) {
			pRecip->real.sign.gcmd.b.unifTa = false;
			c_match[MID_TIME].banner.a = 0;
			c_match[MID_TIME].wRequestTimer = c_match[MID_TIME].wAgreeTimer = 0;
		}
		if (!pDoz->recip[FID_HEAD].real.sign.scmdEx.b.unifLfcr) {
			pRecip->real.sign.scmdEx.b.unifLfca = false;
			c_match[MID_CLFC].banner.a = 0;
			c_match[MID_CLFC].wRequestTimer = c_match[MID_CLFC].wAgreeTimer = 0;
		}
	}
	/*if (ScanAgree(MID_TIME)) {
		TRACK("TURF:unify time.\n");
	}
	if (ScanAgree(MID_ENV)) {
		TRACK("TURF:unify env.\n");
	}
	if (ScanAgree(MID_CRR)) {
		TRACK("TURF:unify crrs.\n");
	}*/
}

// only FID_HEAD
void CTurf::StirTimeMatch(bool byRtd)
{
	// TIMESET 1 : 각 프로세스에서의 시간 설정을 배너에 담아둔다-TU.
	memset(&c_match[MID_TIME], 0, sizeof(MATCHSHAKE));
	c_match[MID_TIME].banner.b.request = true;
	c_match[MID_TIME].wRequestTimer = TPERIOD_TIMEMATCH;
	c_match[MID_TIME].banner.b.aid = byRtd;
	//TRACK("TURF:stir time match\n");
}

// only FID_HEAD or FID_TAIL's main
void CTurf::StirEnvironMatch()
{
	memset(&c_match[MID_ENV], 0, sizeof(MATCHSHAKE));
	c_match[MID_ENV].banner.b.request = true;
	c_match[MID_ENV].wRequestTimer = TPERIOD_ENVMATCH;
}

// only FID_HEAD or FID_TAIL's main
void CTurf::StirCareerMatch()
{
	memset(&c_match[MID_CRR], 0, sizeof(MATCHSHAKE));
	c_match[MID_CRR].banner.b.request = true;
	c_match[MID_CRR].wRequestTimer = TPERIOD_CRRMATCH;
}

// 200218
void CTurf::StirClearLfc()
{
	memset(&c_match[MID_CLFC], 0, sizeof(MATCHSHAKE));
	c_match[MID_CLFC].banner.b.request = true;
	c_match[MID_CLFC].wRequestTimer = TPERIOD_CLFCMATCH;
}

ENTRY_CONTAINER(CTurf)
	SCOOP(&c_match[MID_TIME].banner.a,		sizeof(BYTE),	"Turf")
	SCOOP(&c_match[MID_ENV].banner.a,		sizeof(BYTE),	"")
	SCOOP(&c_match[MID_CRR].banner.a,		sizeof(BYTE),	"")
	SCOOP(&c_match[MID_CLFC].banner.a,		sizeof(BYTE),	"")
	SCOOP(&c_match[MID_CLFC].wAttendMap,	sizeof(WORD),	"")
	SCOOP(&c_match[MID_CLFC].wAgreeMap,		sizeof(WORD),	"")
EXIT_CONTAINER()
