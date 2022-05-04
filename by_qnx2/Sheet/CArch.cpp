/*
 * CArch.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <fcntl.h>

#include "Track.h"
#include "../Component2/CTool.h"
#include "../Component2/CFio.h"
#include "../Component2/CBand.h"
#include "CPaper.h"
#include "CVerse.h"
#include "CProse.h"
#include "CSch.h"
#include "Insp.h"

#include "CArch.h"

#if	defined(_AT_TS_)
#include "../ts/CLand.h"
#endif

#include "../Inform2/TroubleType.h"

CArch::CArch()
{
	c_pCarrier = NULL;
	memset(&c_dtsTime, 0, sizeof(DTSTIME));
	memset(&c_devTime, 0, sizeof(DEVTIME));
	memset(&c_mmm, 0, sizeof(MMMONITOR));
	c_dwMakeLogbookTime = 0;
	c_auxi.SetParent(this);
	Initial(INTROFROM_KEY);
}

CArch::~CArch()
{
}

void CArch::Initial(int iFrom)
{
	if (iFrom != INTROFROM_OTRN) {
		c_wLastPeriodCmd = 0;
		c_wClosureInsideSeq = 0;
		c_wWarmupTimer = TPERIOD_WARMUP;
		c_mmm.bUsbScan = false;
		c_mmm.wUsbScanTime = 0;
		c_film[TFILMID_MM].ri = c_film[TFILMID_MM].wi = c_film[TFILMID_MM].stack = 0;
		c_film[TFILMID_RTD].ri = c_film[TFILMID_RTD].wi = c_film[TFILMID_RTD].stack = 0;
		c_rec.ri = c_rec.wi = c_rec.stack = 0;
		memset(&c_rec.cur, 0, sizeof(LOGBOOK));
		memset(&c_fspk, 0, sizeof(SPARKLE) * SPARKLE_MAX * 2);
		memset(&c_sdafd, 0, sizeof(SDAFAILDEB));
	}
	if (iFrom == INTROFROM_KEY) {
		for (int n = 0; n < SIZE_TROUBLELIST; n ++) {
			memset(&c_tbc[n], 0, sizeof(TROUBLEBYCAR));
			c_tbc[n].wMin = c_tbc[n].wSlash = c_tbc[n].wCur = MAX_OLDTROUBLE;
		}
		c_wOldtMap = 0;
	}
	InitialTable();
	c_auxi.Initial(iFrom);
}

void CArch::InitialTable()
{
	c_bLogArea = c_bChangeEnvirons = c_bDelayPrevious = false;
	c_wFutile = 0;
	c_wShotCnt = c_wCutCnt = 0;
	memset(&c_wAbacus, 0, sizeof(WORD) * MAX_TROUBLE);

	//c_troubleRef.dwIndex = TROUBLEBUFINDEX_BEGIN;	// 1;
	//c_troubleRef.dwReduce = TROUBLEBUFINDEX_BEGIN;	// 1;
	//c_troubleRef.dwSevere = c_troubleRef.dwCrash = c_troubleRef.dwState = c_troubleRef.dwOther = 0;

	c_troubleTray[TTRAY_TROUBLE].dwIndex = c_troubleTray[TTRAY_TROUBLE].dwReduce = TROUBLEBUFINDEX_BEGIN;
	c_troubleTray[TTRAY_TROUBLE].dwHeavy = 0;
	c_troubleTray[TTRAY_STATUS].dwIndex = c_troubleTray[TTRAY_STATUS].dwReduce = TROUBLEBUFINDEX_BEGIN;
	c_troubleTray[TTRAY_STATUS].dwHeavy = 0;
	c_troubleTray[TTRAY_OTHER].dwIndex = c_troubleTray[TTRAY_OTHER].dwReduce = TROUBLEBUFINDEX_BEGIN;
	c_troubleTray[TTRAY_OTHER].dwHeavy = 0;

	for (int n = 0; n < SIZE_TROUBLELIST; n ++)
		c_tbc[n].wSlash = c_tbc[n].wCur;
	c_bInsprInside = c_bInsprBridge = false;
	// 181001
	memset(&c_rtdtc, 0, sizeof(RTDTROUBLECHECK));
	// 190523
	c_bFstrBridge = false;
	c_cFullSdaStack = c_cFullSdaFlow = 0;
}

int CArch::TakeOrderBit(WORD wbf)
{
	int n;
	for (n = 0; n < 15; n ++) {
		if (wbf & (1 << n))	return n;
	}
	return n;
}

WORD CArch::BitfFromCID(int iCID)
{
	GETSCHEME(pSch);
	if (iCID == CID_ALL)	return pSch->GetCarBitmap();
	else if (iCID == CID_BOW || iCID == CID_STERN) {
		GETPAPER(pPaper);
		if (pPaper->GetTenor()) {		// true:오른쪽 방향
			if (iCID == CID_BOW)	return (WORD)(1 << (pPaper->GetLength() - 1));
			else	return (WORD)(1 << CID_CAR0);
		}
		else {							// false:왼쪽 방향
			if (iCID == CID_BOW)	return (WORD)(1 << CID_CAR0);
			else	return (WORD)(1 << (pPaper->GetLength() - 1));
		}
	}
	return (WORD)(1 << iCID);
}

void CArch::SortAdd(int iCID, WORD wCode)
{
	GETPAPER(pPaper);
	if (ISCU())	return;
	PTROUBLETRAY pTray;
	BYTE cType = pPaper->GetDeviceExFromRecip() ? c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode];
	switch (cType) {
	case TROUBLE_EMERGENCY :
	case TROUBLE_SEVERE :
	case TROUBLE_LIGHT :
	case TROUBLE_WARNING :	pTray = &c_troubleTray[TTRAY_TROUBLE];	break;
	case TROUBLE_STATUS :	pTray = &c_troubleTray[TTRAY_STATUS];	break;
	default :				pTray = &c_troubleTray[TTRAY_OTHER];	break;
	}
	if (pTray->dwIndex >= SIZE_TROUBLEINFO)	return;

	memset(&pTray->cellar[pTray->dwIndex],  0, sizeof(TROUBLEINFO));
	pTray->cellar[pTray->dwIndex].a = 0;
	pTray->cellar[pTray->dwIndex].b.cid = (BYTE)iCID;
	pTray->cellar[pTray->dwIndex].b.code = wCode;
	pTray->cellar[pTray->dwIndex].b.type = cType;
	c_mtx.Lock();
	pTray->cellar[pTray->dwIndex].b.mon = c_devTime.mon;
	pTray->cellar[pTray->dwIndex].b.day = c_devTime.day;
	pTray->cellar[pTray->dwIndex].b.uhour = c_devTime.hour;
	pTray->cellar[pTray->dwIndex].b.umin = c_devTime.min;
	pTray->cellar[pTray->dwIndex].b.usec = c_devTime.sec;
	SetStillOnFilm(&pTray->cellar[pTray->dwIndex]);
	if (pTray->dwIndex < SIZE_TROUBLEINFO) {
		if (IsTroubleType(cType)) {
			pTray->cellar[pTray->dwIndex].b.saw = false;
			if (IsSevereType(cType))	++ pTray->dwHeavy;		// 고장 조치 화면에 중고장 숫자를 표시하기위해...
		}
		++ pTray->dwIndex;
	}
	c_mtx.Unlock();
	if (IsListedType(cType))	AddEach(iCID, wCode);
}

void CArch::AddEach(int iCID, WORD wCode)
{
	GETPAPER(pPaper);
	c_mtx.Lock();
	++ iCID;
	if (IsTuCode(wCode)) {
		if (iCID == 1)	iCID = 0;
		else	iCID = pPaper->GetLength() + 1;
	}
	if (c_tbc[iCID].wCur >= MAX_TROUBLELIST) {
		// 190923, if statement bellow is not executable.
		// at first, this was necessary because MAX_OLDTROUBLE was variable.
		// cancel now because it is a fixed value
		//WORD offset = MAX_TROUBLELIST - MAX_OLDTROUBLE;
		//if (offset < MAX_OLDTROUBLE) {
		//	memcpy(&c_tbc[iCID].ts[0], &c_tbc[iCID].ts[offset], MAX_OLDTROUBLE);
		//	c_tbc[iCID].wCur -= offset;
		//	if (c_tbc[iCID].wSlash > offset)	c_tbc[iCID].wSlash -= offset;
		//	else	c_tbc[iCID].wSlash = 0;
		//	if (c_tbc[iCID].wMin > offset)	c_tbc[iCID].wMin -= offset;
		//	else	c_tbc[iCID].wMin = 0;
		//	if (pPaper->GetLabFromRecip())	TRACK("ARCH:over each trouble list %d!\n", iCID);
		//}
		//else {
		//	memcpy(&c_tbc[iCID].ts[0], &c_tbc[iCID].ts[MAX_OLDTROUBLE], offset);
			memcpy(&c_tbc[iCID].ts[0], &c_tbc[iCID].ts[MAX_OLDTROUBLE], MAX_TROUBLELIST - MAX_OLDTROUBLE);
			c_tbc[iCID].wCur -= MAX_OLDTROUBLE;
			if (c_tbc[iCID].wSlash > MAX_OLDTROUBLE)	c_tbc[iCID].wSlash -= MAX_OLDTROUBLE;
			else	c_tbc[iCID].wSlash = 0;
			if (c_tbc[iCID].wMin > MAX_OLDTROUBLE)	c_tbc[iCID].wMin -= MAX_OLDTROUBLE;
			else	c_tbc[iCID].wMin = 0;
		//}
	}
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.cid = (BYTE)iCID;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.code = wCode;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.type =
			pPaper->GetDeviceExFromRecip() ? c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode];
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.mon = c_devTime.mon;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.day = c_devTime.day;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.uhour = c_devTime.hour;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.umin = c_devTime.min;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.usec = c_devTime.sec;
	c_tbc[iCID].ts[c_tbc[iCID].wCur].b.down = false;
	++ c_tbc[iCID].wCur;
	c_mtx.Unlock();
}

//void CArch::Add(int iCID, WORD wCode)
//{
//	GETPAPER(pPaper);
//	if (ISCU() || c_troubleRef.dwIndex >= SIZE_TROUBLEINFO)	return;
//
//	//if (wCode >= 10 && wCode <= 12)	TRACK("ARCH:ADD1 %d %d.\n", wCode, iCID);
//	c_troubleRef.cellar[c_troubleRef.dwIndex].a = 0;
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.cid = (BYTE)iCID;
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.code = wCode;
//	BYTE cType = c_cTroubleTypes[wCode];
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.type = cType;
//	c_mtx.Lock();
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.mon = c_devTime.mon;
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.day = c_devTime.day;
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.uhour = c_devTime.hour;
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.umin = c_devTime.min;
//	c_troubleRef.cellar[c_troubleRef.dwIndex].b.usec = c_devTime.sec;
//	SetStillOnFilm(&c_troubleRef.cellar[c_troubleRef.dwIndex]);
//	if (c_troubleRef.dwIndex < SIZE_TROUBLEINFO) {
//		if (IsTroubleType(cType)) {
//			c_troubleRef.cellar[c_troubleRef.dwIndex].b.saw = false;
//			++ c_troubleRef.dwCrash;
//			if (IsSevereType(cType))	++ c_troubleRef.dwSevere;	// 고장 조치 화면에 중고장 숫자를 표시하기 위해...
//		}
//		else if (IsStatusType(cType))	++ c_troubleRef.dwState;
//		else	++ c_troubleRef.dwOther;
//		++ c_troubleRef.dwIndex;
//		//c_troubleRef.wRecentCode = wCode;
//		//c_troubleRef.wRecentCid = (WORD)iCID;
//	}
//
//	if (IsListedType(cType)) {
//		++ iCID;
//		if (IsTuCode(wCode)) {
//			if (iCID == 1)	iCID = 0;
//			else	iCID = pPaper->GetLength() + 1;
//		}
//		if (c_tbc[iCID].wCur >= MAX_TROUBLELIST) {
//			WORD offset = MAX_TROUBLELIST - MAX_OLDTROUBLE;
//			if (offset < MAX_OLDTROUBLE) {
//				memcpy(&c_tbc[iCID].ts[0], &c_tbc[iCID].ts[offset], MAX_OLDTROUBLE);
//				c_tbc[iCID].wCur -= offset;
//				if (c_tbc[iCID].wSlash > offset)	c_tbc[iCID].wSlash -= offset;
//				else	c_tbc[iCID].wSlash = 0;
//				if (c_tbc[iCID].wMin > offset)	c_tbc[iCID].wMin -= offset;
//				else	c_tbc[iCID].wMin = 0;
//				if (pPaper->GetLabFromRecip())	TRACK("ARCH:over each trouble list %d!\n", iCID);
//			}
//			else {
//				memcpy(&c_tbc[iCID].ts[0], &c_tbc[iCID].ts[MAX_OLDTROUBLE], offset);
//				c_tbc[iCID].wCur -= MAX_OLDTROUBLE;
//				if (c_tbc[iCID].wSlash > MAX_OLDTROUBLE)	c_tbc[iCID].wSlash -= MAX_OLDTROUBLE;
//				else	c_tbc[iCID].wSlash = 0;
//				if (c_tbc[iCID].wMin > MAX_OLDTROUBLE)	c_tbc[iCID].wMin -= MAX_OLDTROUBLE;
//				else	c_tbc[iCID].wMin = 0;
//			}
//		}
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.cid = (BYTE)iCID;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.code = wCode;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.type = cType;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.mon = c_devTime.mon;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.day = c_devTime.day;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.uhour = c_devTime.hour;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.umin = c_devTime.min;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.usec = c_devTime.sec;
//		c_tbc[iCID].ts[c_tbc[iCID].wCur].b.down = false;
//		++ c_tbc[iCID].wCur;
//	}
//	c_mtx.Unlock();
//	//if (wCode == 100)	TRACK("ARCH:ADD 100.\n");
//	//if (wCode >= 10 && wCode <= 12)	TRACK("ARCH:ADD2 %d %d.\n", wCode, iCID);
//}
//
void CArch::SortRevoke(int iCID, WORD wCode)
{
	GETPAPER(pPaper);
	if (ISCU())	return;
	PTROUBLETRAY pTray;
	BYTE cType = pPaper->GetDeviceExFromRecip() ? c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode];
	switch (cType) {
	case TROUBLE_EMERGENCY :
	case TROUBLE_SEVERE :
	case TROUBLE_LIGHT :
	case TROUBLE_WARNING :	pTray = &c_troubleTray[TTRAY_TROUBLE];	break;
	case TROUBLE_STATUS :	pTray = &c_troubleTray[TTRAY_STATUS];	break;
	default :				pTray = &c_troubleTray[TTRAY_OTHER];	break;
	}

	for (DWORD dw = pTray->dwIndex - 1; dw >= pTray->dwReduce; dw --) {
		if (!pTray->cellar[dw].b.down &&
			pTray->cellar[dw].b.cid == (BYTE)iCID &&
			pTray->cellar[dw].b.code == wCode) {
			c_mtx.Lock();
			pTray->cellar[dw].b.dhour = c_devTime.hour;
			pTray->cellar[dw].b.dmin = c_devTime.min;
			pTray->cellar[dw].b.dsec = c_devTime.sec;
			c_mtx.Unlock();
			pTray->cellar[dw].b.down = true;
			SetStillOnFilm(&pTray->cellar[dw]);
			if (IsSevereType(pPaper->GetDeviceExFromRecip() ?
				c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode])
				&& pTray->dwHeavy > 0)
				-- pTray->dwHeavy;
			if (dw == pTray->dwReduce)	++ pTray->dwReduce;
			RevokeEach(iCID, wCode);
			break;
		}
	}
}

//void CArch::Revoke(int iCID, WORD wCode)
//{
//	GETPAPER(pPaper);
//	if (ISCU() || c_troubleRef.dwIndex >= SIZE_TROUBLEINFO)	return;
//
//	for (DWORD dw = c_troubleRef.dwIndex - 1; dw >= c_troubleRef.dwReduce; dw --) {
//		if (!c_troubleRef.cellar[dw].b.down &&
//			c_troubleRef.cellar[dw].b.cid == (BYTE)iCID &&
//			c_troubleRef.cellar[dw].b.code == wCode) {
//			c_mtx.Lock();
//			c_troubleRef.cellar[dw].b.dhour = c_devTime.hour;
//			c_troubleRef.cellar[dw].b.dmin = c_devTime.min;
//			c_troubleRef.cellar[dw].b.dsec = c_devTime.sec;
//			c_mtx.Unlock();
//			c_troubleRef.cellar[dw].b.down = true;
//			SetStillOnFilm(&c_troubleRef.cellar[dw]);
//			if (IsSevereType(c_cTroubleTypes[wCode]) && c_troubleRef.dwSevere > 0)	-- c_troubleRef.dwSevere;
//			if (dw == c_troubleRef.dwReduce)	++ c_troubleRef.dwReduce;
//			RevokeEach(iCID, wCode);	// 180522
//			break;
//		}
//	}
//}
//
// 180522
void CArch::RevokeEach(int iCID, WORD wCode)
{
	GETPAPER(pPaper);
	if (IsListedType(pPaper->GetDeviceExFromRecip() ? c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode])) {
		++ iCID;
		if (IsTuCode(wCode)) {
			GETPAPER(pPaper);
			if (iCID == 1)	iCID = 0;
			else	iCID = pPaper->GetLength() + 1;
		}
		// 180522
		//for (WORD w = c_tbc[iCID].wCur - 1; w >= c_tbc[iCID].wSlash; w --) {
		for (int n = (int)(c_tbc[iCID].wCur - 1); n >= (int)c_tbc[iCID].wSlash; n --) {
			if (!c_tbc[iCID].ts[n].b.down && c_tbc[iCID].ts[n].b.code == wCode) {
				c_mtx.Lock();
				c_tbc[iCID].ts[n].b.dhour = c_devTime.hour;
				c_tbc[iCID].ts[n].b.dmin = c_devTime.min;
				c_tbc[iCID].ts[n].b.dsec = c_devTime.sec;
				c_mtx.Unlock();
				c_tbc[iCID].ts[n].b.down = true;
				return;
			}
		}
	}
}

void CArch::SortHide(int iCID, WORD wCode)
{
	GETPAPER(pPaper);
	if (ISCU())	return;
	PTROUBLETRAY pTray;
	BYTE cType = pPaper->GetDeviceExFromRecip() ? c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode];
	switch (cType) {
	case TROUBLE_EMERGENCY :
	case TROUBLE_SEVERE :
	case TROUBLE_LIGHT :
	case TROUBLE_WARNING :	pTray = &c_troubleTray[TTRAY_TROUBLE];	break;
	case TROUBLE_STATUS :	pTray = &c_troubleTray[TTRAY_STATUS];	break;
	default :				pTray = &c_troubleTray[TTRAY_OTHER];	break;
	}

	for (DWORD dw = pTray->dwIndex - 1; dw >= pTray->dwReduce; dw --) {
		if (!pTray->cellar[dw].b.down && !pTray->cellar[dw].b.saw &&
			pTray->cellar[dw].b.cid == (BYTE)iCID && pTray->cellar[dw].b.code == wCode) {
			c_mtx.Lock();
			pTray->cellar[dw].b.saw = true;
			c_mtx.Unlock();
			return;
		}
	}
}

//void CArch::Hide(int iCID, WORD wCode)
//{
//	GETPAPER(pPaper);
//	if (ISCU() || c_troubleRef.dwIndex >= SIZE_TROUBLEINFO)	return;
//
//	for (DWORD dw = c_troubleRef.dwIndex - 1; dw >= c_troubleRef.dwReduce; dw --) {
//		if (!c_troubleRef.cellar[dw].b.down && !c_troubleRef.cellar[dw].b.saw &&
//			c_troubleRef.cellar[dw].b.cid == (BYTE)iCID && c_troubleRef.cellar[dw].b.code == wCode) {
//			c_mtx.Lock();
//			c_troubleRef.cellar[dw].b.saw = true;
//			c_mtx.Unlock();
//			return;
//		}
//	}
//}
//
void CArch::AddRtdEdge()
{
	TROUBLEINFO tbi;
	tbi.a = 0;
	tbi.b.cid = 1;
	tbi.b.mon = c_devTime.mon;
	tbi.b.day = c_devTime.day;
	tbi.b.uhour = c_devTime.hour;
	tbi.b.umin = c_devTime.min;
	tbi.b.usec = c_devTime.sec;
	SetStillOnFilm(&tbi, TFILMID_RTD);
}

void CArch::Shot(int iCID, WORD wCode)
{
	++ c_wShotCnt;
	// 171101
	if (c_wWarmupTimer != 0)	return;

	GETPAPER(pPaper);
	if (ISCU() || wCode >= MAX_TROUBLE || iCID >= CID_ALL)	return;		// !!! CAUTION : CID_ALL을 가진 Shot은 할 수 없다.

	WORD bbf = BitfFromCID(iCID);
	BYTE cType = pPaper->GetDeviceExFromRecip() ? c_cTroubleTypes2[wCode] : c_cTroubleTypes1[wCode];
	if (cType != TROUBLE_EVENT && (c_wAbacus[wCode] & bbf))	return;	//  이미 등록된 코드이므로...
	c_mty.Lock();
	if (cType != TROUBLE_EVENT)	c_wAbacus[wCode] |= bbf;
	SortAdd(TakeOrderBit(bbf), wCode);
	c_mty.Unlock();
	//if (pPaper->GetLabFromRecip() && wCode == 100)	TRACK("ARCH:shot %d 100.\n", iCID);
	//if (wCode >= 10 && wCode <= 12)	TRACK("ARCH:SHOT %d %d %04X %d.\n", wCode, iCID, bbf, TakeOrderBit(bbf));
}

void CArch::Cut(int iCID, WORD wBegin, WORD wEnd, WORD wFlesh)
{
	++ c_wCutCnt;
	// 171101
	if (c_wWarmupTimer != 0)	return;

	GETPAPER(pPaper);
	if (ISCU() || wBegin >= MAX_TROUBLE || (wEnd != 0 && wEnd >= MAX_TROUBLE) ||
		(wFlesh == 0 && wFlesh >= MAX_TROUBLE) || iCID > CID_ALL)	return;	// Edit는 CID_ALL을 할 수 있으나

	WORD bbf = BitfFromCID(iCID);
	c_mty.Lock();
	do {
		WORD bbfa = bbf;
		if (c_wAbacus[wBegin] & bbfa) {
			while (c_wAbacus[wBegin] & bbfa) {
				BYTE bi = (BYTE)TakeOrderBit(bbfa);
				c_wAbacus[wBegin] &= ~(1 << bi);
				bbfa &= ~(1 << bi);
				SortRevoke(bi, wBegin);
			}
		}
	} while (++ wBegin <= wEnd);

	if (iCID < CID_ALL && wFlesh != 0)	Shot(iCID, wFlesh);	// Shot은 CID_ALL이 없다.
	c_mty.Unlock();
	//if (wBegin == 100)	TRACK("ARCH:CUT 100.\n");
}

// wCutBegin에서 wCutEnd까지 지우지만 wShot을 지우면 로그가 다시 생기므로 주의할 것.
void CArch::Closeup(int iCID, WORD wShot, WORD wCutBegin, WORD wCutEnd)
{
	// 171101
	if (c_wWarmupTimer != 0)	return;

	if (wCutBegin >= wCutEnd || wShot < wCutBegin || wShot > wCutEnd) {
		TRACK("ARCH>ERR:%s() invalid parameter!\n", __FUNCTION__);
		return;
	}

	WORD w = wCutBegin;
	int leng = 0;
	for ( ; w < wShot; w ++, leng ++);
	if (leng > 0)	Cut(iCID, wCutBegin, leng > 1 ? w - 1 : 0);

	w = wShot;
	leng = 0;
	for ( ; w < wCutEnd; w ++, leng ++);
	if (leng > 0) {
		if (leng == 1)	Cut(iCID, wCutEnd);
		else	Cut(iCID, wShot + 1, wCutEnd);
	}

	// Shot은 코드가 이미 있으면 하지않고 없으면 등록하는 함수이므로 코드를 지우게되면 다시 등록되는 문제가 발생한다.
	Shot(iCID, wShot);
}

void CArch::Enumerate(int iCID, WORD wOccur, WORD wErase)
{
	// 171101
	if (c_wWarmupTimer != 0)	return;

	Shot(iCID, wOccur);
	if (wErase != 0) {
		WORD bbf = BitfFromCID(iCID);
		c_mty.Lock();
		if (c_wAbacus[wErase] & bbf)	c_wAbacus[wErase] &= ~bbf;
		c_mty.Unlock();
	}
}

void CArch::Conceal(int iCID, WORD wErase)
{
	WORD bbf = BitfFromCID(iCID);
	c_mty.Lock();
	if (c_wAbacus[wErase] & bbf)	c_wAbacus[wErase] &= ~bbf;
	c_mty.Unlock();
}

void CArch::ConcealAtoBcf()
{
	for (int n = 0; n < 6; n ++)
		Conceal(CID_ALL, 100 + n);
	Conceal(CID_ALL, 325);
	TRACK("ARCH:CONCEAL.\n");
}

BYTE CArch::GetTroubleType(WORD wCode)
{
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip())
		return c_cTroubleTypes2[wCode];
	return c_cTroubleTypes1[wCode];
}

void CArch::Prompt(bool bSign, int iCID, WORD wCode)
{
	if (bSign)	Shot(iCID, wCode);
	else	Cut(iCID, wCode);
}

// 171110, wTcid = 0 ~ 11
PTROUBLEBYCAR CArch::GetTroubleListForEachCar(WORD wTcid)
{
	PBULKPACK pBulk = c_auxi.GetBulk();
	if (!(c_wOldtMap & (1 << wTcid)) && (pBulk->wState & (1 << BULKSTATE_OLDTROUBLE))) {
		char path[256];
		GETPAPER(pPaper);
		sprintf(path, "%s%02d", OLDTROUBLE_PATHTMP, wTcid > pPaper->GetLength() ? 11 : wTcid);	// 11, 9, 7, 5는 mm에서 11로 저장된다.
		int size = (int)c_auxi.GetFileSize(path);
		if (size >= (int)sizeof(TROUBLEINFO)) {
			int ofs = MAX_OLDTROUBLE - size / sizeof(TROUBLEINFO);
			FILE* fp;
			if ((fp = fopen(path, "r")) != NULL) {
				fread(&c_tbc[wTcid].ts[ofs], 1, size, fp);
				c_tbc[wTcid].wMin = (WORD)ofs;
				c_wOldtMap |= (1 << wTcid);
			}
		}
		else {
			c_wOldtMap |= (1 << wTcid);
			TRACK("ARCH>ERR:tcid %d old trouble empty!\n", wTcid);
		}
	}
	return &c_tbc[wTcid];
}

void CArch::ShiftCount(WORD* pwShot, WORD* pwCut)
{
	*pwShot = c_wShotCnt;
	*pwCut = c_wCutCnt;
	c_wShotCnt = 0;
	c_wCutCnt = 0;
}

void CArch::SetStillOnRecord()
{
	memcpy(&c_rec.cellar[c_rec.wi], &c_rec.cur, sizeof(LOGBOOK));
	if (++ c_rec.wi >= SIZE_RECORD)	c_rec.wi = 0;
	c_mtz.Lock();
	++ c_rec.stack;
	c_mtz.Unlock();
	if (c_rec.stack >= SIZE_RECORD) {
		c_rec.stack = SIZE_RECORD;
		c_fspk[SPARKLE_LOGBOOK][0].bFlash = true;
		c_fspk[SPARKLE_LOGBOOK][0].bCounter = false;
	}
}

bool CArch::GetStillOnRecord(PLOGBOOK pLogbook)
{
	if (c_rec.wi == c_rec.ri)	return false;
	memcpy(pLogbook, &c_rec.cellar[c_rec.ri], sizeof(LOGBOOK));
	if (++ c_rec.ri >= SIZE_RECORD)	c_rec.ri = 0;
	if (c_rec.stack == 0) {
		c_fspk[SPARKLE_LOGBOOK][1].bFlash = true;
		c_fspk[SPARKLE_LOGBOOK][1].bCounter = false;
	}
	else {
		c_mtz.Lock();
		-- c_rec.stack;
		c_mtz.Unlock();
	}
	return true;
}

void CArch::SetStillOnFilm(PTROUBLEINFO pTroub)
{
	for (int n = TFILMID_MM; n < TFILMID_MAX; n ++) {
		memcpy(&c_film[n].cellar[c_film[n].wi], pTroub, sizeof(TROUBLEINFO));
		if (++ c_film[n].wi >= SIZE_FILM)	c_film[n].wi = 0;
		c_mtz.Lock();
		++ c_film[n].stack;
		c_mtz.Unlock();
		if (c_film[n].stack >= SIZE_FILM) {
			c_film[n].stack = SIZE_FILM;
			c_fspk[n][0].bFlash = true;
			c_fspk[n][0].bCounter = false;
			//TRACK("ARCH>ERR:set still stack overflow!\n");
		}
	}
	++ c_rtdtc.dwReserve;
	//if (pTroub->b.code >= 10 && pTroub->b.code <= 12)	TRACK("ARCH:SET %d %d.\n", pTroub->b.code, pTroub->b.cid);
}

void CArch::SetStillOnFilm(PTROUBLEINFO pTroub, int to)
{
	if (to > TFILMID_RTD)	return;
	memcpy(&c_film[to].cellar[c_film[to].wi], pTroub, sizeof(TROUBLEINFO));
	if (++ c_film[to].wi >= SIZE_FILM)	c_film[to].wi = 0;
	c_mtz.Lock();
	++ c_film[to].stack;
	c_mtz.Unlock();
	if (c_film[to].stack >= SIZE_FILM) {
		c_film[to].stack = SIZE_FILM;
		c_fspk[to][0].bFlash = true;
		c_fspk[to][0].bCounter = false;
	}
}

bool CArch::GetStillOnFilm(PTROUBLEINFO pTroub, int from)
{
	if (from > TFILMID_RTD)	return false;
	if (c_film[from].wi == c_film[from].ri)	return false;
	memcpy(pTroub, &c_film[from].cellar[c_film[from].ri], sizeof(TROUBLEINFO));
	if (++ c_film[from].ri >= SIZE_FILM)	c_film[from].ri = 0;
	if (c_film[from].stack == 0) {
		c_fspk[from][1].bFlash = true;
		c_fspk[from][1].bCounter = false;
		//TRACK("ARCH>ERR:get still stack underflow!\n");
	}
	else {
		c_mtz.Lock();
		-- c_film[from].stack;
		c_mtz.Unlock();
	}
	return true;
}

bool CArch::GetSparkle(int id, bool bCh)
{
	if (id >= SPARKLE_MAX)	return false;

	int ch = bCh ? 1 : 0;		// 0:over, 1:under
	if (c_fspk[id][ch].bFlash) {
		if (!c_fspk[id][ch].bCounter) {
			c_fspk[id][ch].bCounter = true;
			c_fspk[id][ch].wCounter = TPERIOD_SPARKLE;
		}
	}
	return c_fspk[id][ch].bFlash;
}

TROUBLECODE CArch::GetRecentTrouble()
{
	TROUBLECODE tc;
	tc.a = 0;
	GETPAPER(pPaper);
	if (ISCU())	return tc;

	c_mty.Lock();
	for (DWORD dw = c_troubleTray[TTRAY_TROUBLE].dwIndex - 1; dw >= c_troubleTray[TTRAY_TROUBLE].dwReduce; dw --) {
		if (!c_troubleTray[TTRAY_TROUBLE].cellar[dw].b.down && !c_troubleTray[TTRAY_TROUBLE].cellar[dw].b.saw) {
			tc.b.code = c_troubleTray[TTRAY_TROUBLE].cellar[dw].b.code;
			tc.b.cid = c_troubleTray[TTRAY_TROUBLE].cellar[dw].b.cid;
			c_mty.Unlock();
			return tc;
		}
	}
	c_mty.Unlock();
	return tc;
}

//	if (c_troubleRef.dwIndex == TROUBLEBUFINDEX_BEGIN)	return tc;
//	c_mty.Lock();
//	for (DWORD dw = c_troubleRef.dwIndex - 1; dw >= c_troubleRef.dwReduce; dw --) {
//		if (!c_troubleRef.cellar[dw].b.down &&
//			!c_troubleRef.cellar[dw].b.saw &&
//			(c_troubleRef.cellar[dw].b.type & TROUBLETYPE_ONLY) >= TROUBLE_LIGHT) {
//			tc.b.code = c_troubleRef.cellar[dw].b.code;
//			tc.b.cid = c_troubleRef.cellar[dw].b.cid;
//			c_mty.Unlock();
//			return tc;
//		}
//	}
//	c_mty.Unlock();
//	return tc;
//}
//
void CArch::ReformT(PRECIP pRecip)
{
	PENVIRONS pEnv = c_auxi.GetEnvirons();
	// 180626
	//for (int n = 0; n < LENGTH_WHEELDIA; n ++)
	//	pRecip->real.cDiai[n] = pEnv->real.cWheelDiai[n];
	//for (int n = 0; n < CID_MAX; n ++)
	//	pRecip->real.cTempSv[n] = pEnv->real.cTempSv[n];
	pRecip->real.sign.hcmdEx.b.hts = true;	//c_auxi.GetTain();
	switch (pEnv->real.cVentTime) {
	case 2 :	pRecip->real.sign.hcmdEx.b.vnt2 = true;		pRecip->real.sign.hcmdEx.b.vnt1 = false;	break;
	case 1 :	pRecip->real.sign.hcmdEx.b.vnt2 = false;	pRecip->real.sign.hcmdEx.b.vnt1 = true;		break;
	default :	pRecip->real.sign.hcmdEx.b.vnt2 = false;	pRecip->real.sign.hcmdEx.b.vnt1 = false;	break;
	}
}

void CArch::ReformC(PRECIP pRecip)
{
	// 170801
	GetSysTimeToDbf(&pRecip->real.prol.dbft);
	pRecip->real.cMmState = c_auxi.GetMmState();
}

void CArch::BuildLogbook()
{
#if	defined(_AT_TS_)
	_TWATCHCLICK();
	memset(&c_rec.cur, 0, sizeof(LOGBOOK));

#define	LOGR	c_rec.cur.real

	LOGR.r.wSeq = c_rec.wSeq ++;
	GetSysTimeToDbf(&LOGR.r.dbft);

	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();

	for (int n = 0; n < LENGTH_TU; n ++) {
		// 17/06/20
		// GetRouteState()를 하지않으면 0을 겹쳐쓰는 경우가 생길 수 있다!!!
		if (pPaper->GetRouteState(n)) {			// 180911, 통신이 안되는대로, 확인
			int tid = (int)pDoz->recip[n].real.prol.cProperID;
			//if (n >= 4)	TRACK("ARCH>ERR:wrong tu id!(%d)\n", tid);
			if (tid >= 4)	TRACK("ARCH>ERR:wrong tu id!(%d)\n", tid);
			else {
				// 17/06/20, sign log 확장
				LOGR.r.tsc[tid].gcmd = pDoz->recip[n].real.sign.gcmd.a;
				LOGR.r.tsc[tid].ecmd = pDoz->recip[n].real.sign.ecmd.a;
				LOGR.r.tsc[tid].scmd = pDoz->recip[n].real.sign.scmd.a;
				LOGR.r.tsc[tid].hcmd = pDoz->recip[n].real.sign.hcmd.a;
				LOGR.r.tsc[tid].hcmdEx = pDoz->recip[n].real.sign.hcmdEx.a;
				//LOGR.r.cGcmd[tid] = pDoz->recip[n].real.sign.gcmd.a;
				//LOGR.r.cScmd[tid] = pDoz->recip[n].real.sign.scmd.a;
				LOGR.r.tui[tid][0].qw = pDoz->recip[n].real.inp[0].qw;
				LOGR.r.tui[tid][1].qw = pDoz->recip[n].real.inp[1].qw;
				LOGR.r.tuo[tid] = pDoz->recip[n].real.outp.c[0];
				memcpy(&LOGR.r.avs[tid], &pDoz->recip[n].real.avs, sizeof(_ANALOGV));
			}
		}
	}
	for (int n = 0; n < LENGTH_CU; n ++) {
		LOGR.r.cui[n].qw = pDoz->recip[n + LENGTH_TU].real.inp[0].qw;
		LOGR.r.cuo[n] = pDoz->recip[n + LENGTH_TU].real.outp.c[0];
	}
	memcpy(&LOGR.r.motiv, &pDoz->recip[0].real.motiv, sizeof(MOTIVE));
	memcpy(&LOGR.r.drift, &pDoz->recip[0].real.drift, sizeof(DRIFT));
	memcpy(&LOGR.r.ato, &pDoz->recip[0].real.ato.a, sizeof(SAATO));

	// 171129
	LOGR.r.oppoLead.a = pDoz->recip[FID_TAIL].real.motiv.lead.a;	// 후미차 pis 정보

	GETVERSE(pVerse);
	//CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	// 171019
	//LOGR.r.wPisOtr = pPsv->wPisOtr;
	LOGR.r.wOtrn = pVerse->GetCurOtrn();
	// 180828
	PPISAINFO pPisa = (PPISAINFO)pVerse->GetInfo(true, PDEVID_PIS, LI_CURR);
	LOGR.r.pis.cRef = pPisa->t.s.ref.a;
	LOGR.r.pis.cCurrSt = pPisa->t.s.cCurrSt;
	LOGR.r.pis.cNextSt = pPisa->t.s.cNextSt;
	LOGR.r.pis.wNotr = pPisa->t.s.notr.a;

	CLand* pLand = (CLand*)c_pCarrier;
	ASSERTP(pLand);
	pLand->GetSteerAvrLog(&c_rec.cur);	// wNotchPwm, dwTachoSpeed, dwAvrSpeed
	// 17/06/20, modify
	//LOGR.dwDistance = (DWORD)c_auxi.GetCareers(CRRITEM_DISTANCE, 0);	// m
	LOGR.r.qwDistance = (QWORD)c_auxi.GetCareers(CRRITEM_DISTANCE, 0);	// m
	LOGR.r.wSegment = (WORD)c_auxi.GetSegment();

	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();

	GETSCHEME(pSch);
	PDEVCONF pConf = pSch->GetDevConf();
	for (int n = 0; n < pConf->siv.cLength; n ++) {
		if (!IsFreeze(pLsv->sivcf.cLines[n])) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->siv.cPos[n]);
			// 200218
			//LOGR.r.siv[n].cEs = pDoz->recip[fid].real.svf.s.a.t.s.cEs;
			LOGR.r.siv[n].cEs = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.cEs : pDoz->recip[fid].real.svf.s2.a.t.s.cEs;
			//LOGR.r.siv[n].cIs = pDoz->recip[fid].real.svf.s.a.t.s.cIs;
			//LOGR.r.siv[n].cVo = pDoz->recip[fid].real.svf.s.a.t.s.cVo;
			LOGR.r.siv[n].cVo = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.cVo : pDoz->recip[fid].real.svf.s2.a.t.s.cVo;
			// 171114
			//LOGR.r.siv[n].cIo = pDoz->recip[fid].real.svf.s.a.t.s.cIo;
			//LOGR.r.siv[n].wIo = (WORD)pDoz->recip[fid].real.svf.s.a.t.s.ste.b.iom << 8 | (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cIo;
			LOGR.r.siv[n].wIo = !pPaper->GetDeviceExFromRecip() ?
								(WORD)pDoz->recip[fid].real.svf.s.a.t.s.ste.b.iom << 8 | (WORD)pDoz->recip[fid].real.svf.s.a.t.s.cIo :
								XWORD(pDoz->recip[fid].real.svf.s2.a.t.s.wIo);
			//LOGR.r.siv[n].cFo = pDoz->recip[fid].real.svf.s.a.t.s.cFo;
			LOGR.r.siv[n].cFo = !pPaper->GetDeviceExFromRecip() ? pDoz->recip[fid].real.svf.s.a.t.s.cFo : pDoz->recip[fid].real.svf.s2.a.t.s.cFo;
			//LOGR.r.siv[n].cVs = pDoz->recip[fid].real.svf.s.a.t.s.cVs;
		}
	}

	for (int n = 0; n < pConf->v3f.cLength; n ++) {
		if (!IsFreeze(pLsv->v3fcf.cLines[n])) {
			int fid = pSch->C2F(pPaper->GetTenor(), pConf->v3f.cPos[n]);
			// 171123
			LOGR.r.v3fst[n].b.regp = pDoz->recip[fid].real.svf.v.a.t.s.ref.b.regp;
			LOGR.r.v3fst[n].b.pwrp = pDoz->recip[fid].real.svf.v.a.t.s.ref.b.pwrp;
			LOGR.r.v3fst[n].b.ccos = pDoz->recip[fid].real.svf.v.a.t.s.ref.b.ccos;
			LOGR.r.v3fst[n].b.act = pDoz->recip[fid].real.svf.v.a.t.s.ref.b.act;
			LOGR.r.v3fst[n].b.cdron = pDoz->recip[fid].real.svf.v.a.t.s.rsp.b.cdron;
			LOGR.r.v3fst[n].b.fout = pDoz->recip[fid].real.svf.v.a.t.s.rsp.b.fout;
			LOGR.r.v3fst[n].b.rgoff = pDoz->recip[fid].real.svf.v.a.t.s.rsp.b.rgoff;
			LOGR.r.v3fst[n].b.poff = pDoz->recip[fid].real.svf.v.a.t.s.rsp.b.poff;

			LOGR.r.v3f[n].cIq = pDoz->recip[fid].real.svf.v.a.t.s.cIq;
			LOGR.r.v3f[n].cLv = pDoz->recip[fid].real.svf.v.a.t.s.cLv;

			// 210805
			if (!pPaper->GetDeviceExFromRecip()) {
				LOGR.r.v3f[n].cTbe = pDoz->recip[fid].real.svf.v.a.t.s.cTbe;
			}
			//LOGR.r.v3f[n].cTbe = pDoz->recip[fid].real.svf.v.a.t.s.cTbe;
			// 180828
			//LOGR.r.v3f[n].cBed = pDoz->recip[fid].real.svf.v.a.t.s.cBed;
			WORD w = XWORD(pDoz->recip[fid].real.svf.v.a.t.s.wApbr);
			LOGR.r.v3f[n].iApbr = (SHORT)w;
			//LOGR.v3f[n].wEs = pDoz->recip[fid].real.svf.v.a.t.s.wEs;
			//LOGR.v3f[n].iIdc = (SHORT)pDoz->recip[fid].real.svf.v.a.t.s.wIdc;
			LOGR.r.v3f[n].wFc = XWORD(pDoz->recip[fid].real.svf.v.a.t.s.wFc);
		}
	}

	// 210805
	for (int n = 0; n < 5; n ++)
		LOGR.r.v3f[n].cTbe = pLand->GetAtoHcrs(n);

	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		if (!IsFreeze(pLsv->ecucf.cLines[n])) {
			int fid = pSch->C2F(pPaper->GetTenor(), n);
			// 200218
			//LOGR.r.ecu[n].cBe = pDoz->recip[fid].real.ecu.a.t.s.cBe;
			//LOGR.r.ecu[n].cAsp = pDoz->recip[fid].real.ecu.a.t.s.cAsp;
			//LOGR.r.ecu[n].cLw = pDoz->recip[fid].real.ecu.a.t.s.cLw;
			//LOGR.r.ecu[n].cBed = pDoz->recip[fid].real.ecu.a.t.s.cBed;
			//LOGR.r.ecu[n].cBea = pDoz->recip[fid].real.ecu.a.t.s.cBea;
			// 171114
			//LOGR.r.ecu[n].cEp = pDoz->recip[fid].real.ecu.a.t.s.cEp;
			//LOGR.r.ecu[n].cBcp = pDoz->recip[fid].real.ecu.a.t.s.cBcp;
			//LOGR.r.cEcuRef[n] = pDoz->recip[fid].real.ecu.a.t.s.ref.a;
			//LOGR.r.ecu[n].cRef = pDoz->recip[fid].real.ecu.a.t.s.ref.a;
			if (pPaper->GetDeviceExFromRecip()) {
				LOGR.r.ecu[n].cBe = pDoz->recip[fid].real.ecu.ct.a.t.s.cBe;
				LOGR.r.ecu[n].cAsp = (pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[0] +
										pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
										pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
										pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1]) / 4;
				LOGR.r.ecu[n].cLw = pDoz->recip[fid].real.ecu.ct.a.t.s.cLw;
				LOGR.r.ecu[n].cBcp = pDoz->recip[fid].real.ecu.ct.a.t.s.cBcp;
				LOGR.r.ecu[n].cRef = pDoz->recip[fid].real.ecu.ct.a.t.s.ref.a;
			}
			else {
				LOGR.r.ecu[n].cBe = pDoz->recip[fid].real.ecu.co.a.t.s.cBe;
				LOGR.r.ecu[n].cAsp = pDoz->recip[fid].real.ecu.co.a.t.s.cAsp;
				LOGR.r.ecu[n].cLw = pDoz->recip[fid].real.ecu.co.a.t.s.cLw;
				LOGR.r.ecu[n].cBcp = pDoz->recip[fid].real.ecu.co.a.t.s.cBcp;
				LOGR.r.ecu[n].cRef = pDoz->recip[fid].real.ecu.co.a.t.s.ref.a;
			}
		}
	}

	for (int n = 0; n < SIV_MAX; n ++)
		LOGR.r.dps.qwSivPower[n] = (QWORD)c_auxi.GetCareers(CRRITEM_SIVPOWER, n);

	for (int n = 0; n < V3F_MAX; n ++) {
		LOGR.r.dps.qwV3fPower[n] = (QWORD)c_auxi.GetCareers(CRRITEM_V3FPOWER, n);
		LOGR.r.dps.qwV3fReviv[n] = (QWORD)c_auxi.GetCareers(CRRITEM_V3FREVIV, n);
	}
	// 180511
	//LOGR.r.wCtrlSide = pProse->GetControlSide();
	// 16/06/26
	LOGR.r.dwRoute = pPaper->GetRoute();
	LOGR.r.cSvcCtrlCode[0] = pDoz->recip[FID_HEAD].real.cSvcCtrlCode;
	LOGR.r.cSvcCtrlCode[1] = pDoz->recip[FID_TAIL].real.cSvcCtrlCode;
	LOGR.r.cPoleLine[0] = pDoz->recip[FID_HEAD].real.cPoleLine;
	LOGR.r.cPoleLine[1] = pDoz->recip[FID_TAIL].real.cPoleLine;
	//LOGR.r.cPoleBlock[0] = pDoz->recip[FID_HEAD].real.cPoleBlock;
	//LOGR.r.cPoleBlock[1] = pDoz->recip[FID_TAIL].real.cPoleBlock;

	PCMJOGB pCmj = c_auxi.GetCmjog();
	memcpy(&LOGR.r.cjt, &pCmj->real.cjt, sizeof(CMJOGTIMES));

	// 171207
	LOGR.r.cProperID = pLand->GetProperID();		// for RTD storage

	SYSTEMCHECK sysChk;
	pLand->CopySysChk(&sysChk);
	if (sysChk.wIntroTimer == 0) {
		BYTE cReason = (sysChk.cVmes & 0x7) | ((sysChk.cPairCtrl << 4) & 0x70);
		LOGR.r.cReason = cReason;
		if (sysChk.cPast != cReason)	pLand->LeaveOnly(cReason);
	}
	else	LOGR.r.cReason = sysChk.cPast;

	// 170911
	WORD sum = 0;
	for (int n = 0; n < (int)(sizeof(LOGINFO) - sizeof(WORD)); n ++)
		sum += (WORD)LOGR.c[n];

	LOGR.r.wCSum = sum;

#undef	LOGR
	_TWATCHMEASURED();
	c_dwMakeLogbookTime = (DWORD)(__sec_ * 1e6);
#endif
}

// 190523
void CArch::BuildFullSda()
{
	// 200218
	//for (int n = 0; n < CID_MAX; n ++) {	// CAUSION!!! not pPaper->GetLength()
	//	if (n == 0 || n == 5 || n == 9) {
	//		if (n != 5)	BuildAvsSda(n);
	//		BuildSivSda(n);
	//	}
	//	else if (n == 1 || n == 2 || n == 4 || n == 7 || n == 8)	BuildV3fSda(n);
	//	BuildUnivSda(n);
	//}
	memset(c_fst, 0, sizeof(FSDATONE));

	GETSCHEME(pSch);
	GETPAPER(pPaper);
	for (int n = 0; n < CID_MAX; n ++) {	// CAUSION!!! not pPaper->GetLength()
		if (n == 0)	BuildAvsSda(n);
		else if (n == (pPaper->GetLength() - 1))	BuildAvsSda(n);

		WORD wDevMap = pSch->GetLDeviceMap(n);
		if (wDevMap & DEVBF_SIV)	BuildSivSda(n);
		if (wDevMap & DEVBF_V3F)	BuildV3fSda(n);
		BuildUnivSda(n);
	}
	if (++ c_cFullSdaStack >= CYCLE_FULLSDASTACK) {
		c_cFullSdaStack = 0;
		memcpy(&c_fst[FSTID_REPORT], &c_fst[FSTID_WRITE], sizeof(FSDATONE));
		c_bFstrBridge = true;
		c_cFullSdaFlow = 0;
	}
}

void CArch::BuildAvsSda(int cid)
{
	GETPAPER(pPaper);
	int leng = (int)pPaper->GetLength();
	PFSDAAVS p;
	switch (leng) {
	case 4 :
		p = cid != 0 ? &c_fst[FSTID_WRITE].fss.iv.tc3.h : &c_fst[FSTID_WRITE].fss.iv.tc0.h;
		break;
	case 6 :
		p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi.tc5.h : &c_fst[FSTID_WRITE].fss.vi.tc0.h;
		break;
	case 8 :
		p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi3.tc7.h : &c_fst[FSTID_WRITE].fss.vi3.tc0.h;
		break;
	default :
		p = cid != 0 ? &c_fst[FSTID_WRITE].fss.x.tc9.h : &c_fst[FSTID_WRITE].fss.x.tc0.h;
		break;
	}
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	int fid = pSch->C2F(pPaper->GetTenor(), cid);
	WORD wAddr = pDoz->recip[fid].real.svc.t.r.wAddr;
	if (wAddr == LADD_TRS)	memcpy(&p->trs, &pDoz->recip[fid].real.svc.t.a.t.s.no.a, SIZE_FSDATRS);
	else if (wAddr == LADD_PIS)	memcpy(&p->pis, &pDoz->recip[fid].real.svc.i.a.t.s.inf.a, SIZE_FSDAPIS);
	else	memcpy(&p->pau, &pDoz->recip[fid].real.svc.u.a.t.s.alarm.a, SIZE_FSDAPAU);
}

//void CArch::BuildSivSda(int cid)
//{
//	GETPAPER(pPaper);
//	int leng = pPaper->GetLength();
//	PFSDASIV p;
//	switch (leng) {
//	case 4 :
//		if (!pPaper->GetDeviceExFromRecip())
//			p = cid != 0 ? &c_fst[FSTID_WRITE].fss.iv.tc3.s.siv : &c_fst[FSTID_WRITE].fss.iv.tc0.s.siv;
//		else	p = cid != 0 ? &c_fst[FSTID_WRITE].fss.iv.tc3.s.siv2 : &c_fst[FSTID_WRITE].fss.iv.tc0.s.siv2;
//		break;
//	case 6 :
//		if (!pPaper->GetDeviceExFromRecip())
//			p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi.tc5.s.siv : &c_fst[FSTID_WRITE].fss.vi.tc0.s.siv;
//		else	p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi.tc5.s.siv2 : &c_fst[FSTID_WRITE].fss.vi.tc0.s.siv2;
//		break;
//	case 8 :
//		if (!pPaper->GetDeviceExFromRecip())
//			p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi3.tc7.s.siv : &c_fst[FSTID_WRITE].fss.vi3.tc0.s.siv;
//		else	p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi3.tc7.s.siv2 : &c_fst[FSTID_WRITE].fss.vi3.tc0.s.siv2;
//		break;
//	default :
//		switch (cid) {
//		case 0 :	p = !pPaper->GetDeviceExFromRecip() ? &c_fst[FSTID_WRITE].fss.x.tc0.s.siv : &c_fst[FSTID_WRITE].fss.x.tc0.s.siv2;	break;
//		case 5 :	p = !pPaper->GetDeviceExFromRecip() ? &c_fst[FSTID_WRITE].fss.x.cc5.s.siv : &c_fst[FSTID_WRITE].fss.x.cc5.s.siv2;	break;
//		default :	p = !pPaper->GetDeviceExFromRecip() ? &c_fst[FSTID_WRITE].fss.x.tc9.s.siv : &c_fst[FSTID_WRITE].fss.x.tc9.s.siv2;	break;
//		}
//		break;
//	}
//	PDOZEN pDoz = pPaper->GetDozen();
//	GETSCHEME(pSch);
//	int fid = pSch->C2F(pPaper->GetTenor(), cid);
//	// 200218
//	//memcpy(p, &pDoz->recip[fid].real.svf.s.a.t.s.tst, SIZE_FSDASIV);
//	if (!pPaper->GetDeviceExFromRecip())
//		memcpy(p, &pDoz->recip[fid].real.svf.s.a.t.s.tst, SIZE_FSDASIV);
//	else	memcpy(p, &pDoz->recip[fid].real.svf.s2.a.t.s.tst, SIZE_FSDASIV2);
//}
//
void CArch::BuildSivSda(int cid)
{
	GETPAPER(pPaper);
	int leng = pPaper->GetLength();
	PFSDASIV p;
	switch (leng) {
	case 4 :
		p = cid != 0 ? &c_fst[FSTID_WRITE].fss.iv.tc3.s : &c_fst[FSTID_WRITE].fss.iv.tc0.s;
		break;
	case 6 :
		p = cid != 0 ? (PFSDASIV)&c_fst[FSTID_WRITE].fss.vi.tc5.s : (PFSDASIV)&c_fst[FSTID_WRITE].fss.vi.tc0.s;
		break;
	case 8 :
		p = cid != 0 ? &c_fst[FSTID_WRITE].fss.vi3.tc7.s : &c_fst[FSTID_WRITE].fss.vi3.tc0.s;
		break;
	default :
		switch (cid) {
		case 0 :	p = &c_fst[FSTID_WRITE].fss.x.tc0.s;	break;
		case 5 :	p = &c_fst[FSTID_WRITE].fss.x.cc5.s;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.x.tc9.s;	break;
		}
		break;
	}
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	int fid = pSch->C2F(pPaper->GetTenor(), cid);
	// 200218
	//memcpy(p, &pDoz->recip[fid].real.svf.s.a.t.s.tst, SIZE_FSDASIV);
	if (leng != 6)
		memcpy(p, &pDoz->recip[fid].real.svf.s.a.t.s.tst, SIZE_FSDASIV);
	else	memcpy(p, &pDoz->recip[fid].real.svf.s2.a.t.s.tst, SIZE_FSDASIV2);
}

void CArch::BuildV3fSda(int cid)
{
	GETPAPER(pPaper);
	int leng = pPaper->GetLength();
	PFSDAV3F p;
	switch (leng) {
	case 4 :
		p = cid != 1 ? &c_fst[FSTID_WRITE].fss.iv.cc2.v : &c_fst[FSTID_WRITE].fss.iv.cc1.v;
		break;
	case 6 :
		switch (cid) {
		case 1 :	p = &c_fst[FSTID_WRITE].fss.vi.cc1.v;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.vi.cc2.v;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.vi.cc4.v;	break;
		}
		break;
	case 8 :
		switch (cid) {
		case 1 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc1.v;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc2.v;	break;
		case 5 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc5.v;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.vi3.cc6.v;	break;
		}
		break;
	default :
		switch (cid) {
		case 1 :	p = &c_fst[FSTID_WRITE].fss.x.cc1.v;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.x.cc2.v;	break;
		case 4 :	p = &c_fst[FSTID_WRITE].fss.x.cc4.v;	break;
		case 7 :	p = &c_fst[FSTID_WRITE].fss.x.cc7.v;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.x.cc8.v;	break;
		}
		break;
	}
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	int fid = pSch->C2F(pPaper->GetTenor(), cid);
	memcpy(p, &pDoz->recip[fid].real.svf.v.a.t.s.ref, SIZE_FSDAV3F);
}

void CArch::BuildUnivSda(int cid)
{
	GETPAPER(pPaper);
	int leng = pPaper->GetLength();
	PFSDAUNIV p;
	switch (leng) {
	case 4 :
		switch (cid) {
		case 0 :	p = &c_fst[FSTID_WRITE].fss.iv.tc0.u;	break;
		case 1 :	p = &c_fst[FSTID_WRITE].fss.iv.cc1.u;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.iv.cc2.u;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.iv.tc3.u;	break;
		}
		break;
	case 6 :
		switch (cid) {
		case 0 :	p = &c_fst[FSTID_WRITE].fss.vi.tc0.u;	break;
		case 1 :	p = &c_fst[FSTID_WRITE].fss.vi.cc1.u;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.vi.cc2.u;	break;
		case 3 :	p = &c_fst[FSTID_WRITE].fss.vi.cc3.u;	break;
		case 4 :	p = &c_fst[FSTID_WRITE].fss.vi.cc4.u;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.vi.tc5.u;	break;
		}
		break;
	case 8 :
		switch (cid) {
		case 0 :	p = &c_fst[FSTID_WRITE].fss.vi3.tc0.u;	break;
		case 1 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc1.u;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc2.u;	break;
		case 3 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc3.u;	break;
		case 4 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc4.u;	break;
		case 5 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc5.u;	break;
		case 6 :	p = &c_fst[FSTID_WRITE].fss.vi3.cc6.u;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.vi3.tc7.u;	break;
		}
		break;
	default :
		switch (cid) {
		case 0 :	p = &c_fst[FSTID_WRITE].fss.x.tc0.u;	break;
		case 1 :	p = &c_fst[FSTID_WRITE].fss.x.cc1.u;	break;
		case 2 :	p = &c_fst[FSTID_WRITE].fss.x.cc2.u;	break;
		case 3 :	p = &c_fst[FSTID_WRITE].fss.x.cc3.u;	break;
		case 4 :	p = &c_fst[FSTID_WRITE].fss.x.cc4.u;	break;
		case 5 :	p = &c_fst[FSTID_WRITE].fss.x.cc5.u;	break;
		case 6 :	p = &c_fst[FSTID_WRITE].fss.x.cc6.u;	break;
		case 7 :	p = &c_fst[FSTID_WRITE].fss.x.cc7.u;	break;
		case 8 :	p = &c_fst[FSTID_WRITE].fss.x.cc8.u;	break;
		default :	p = &c_fst[FSTID_WRITE].fss.x.tc9.u;	break;
		}
		break;
	}
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	int fid = pSch->C2F(pPaper->GetTenor(), cid);
	for (int n = 0; n < 4; n ++) {
		int did = (int)(pDoz->recip[fid].real.dcul.a.wAddr & 0xf);
		if (did > 0 && did < 5)
			memcpy(p->dcu[did - 1], &pDoz->recip[fid].real.dcul.a.t.s.ref, SIZE_FSDADCU);
	}
	for (int n = 0; n < 4; n ++) {
		int did = (int)(pDoz->recip[fid].real.dcur.a.wAddr & 0xf);
		if (did > 0 && did < 5)
			memcpy(p->dcu[did - 1 + 4], &pDoz->recip[fid].real.dcur.a.t.s.ref, SIZE_FSDADCU);
	}
	// 200218
	//memcpy(p->ecu, &pDoz->recip[fid].real.ecu.a.t.s.ref, SIZE_FSDAECU);
	if (pPaper->GetDeviceExFromRecip()) {
		p->ecu[0] = pDoz->recip[fid].real.ecu.ct.a.t.s.ref.a;
		p->ecu[1] = pDoz->recip[fid].real.ecu.ct.a.t.s.cBe;
		p->ecu[2] = (pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[0] +
						pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
						pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1] +
						pDoz->recip[fid].real.ecu.ct.a.t.s.cAsp[1]) / 4;

		memcpy(&p->ecu[3], &pDoz->recip[fid].real.ecu.ct.a.t.s.cLw, SIZE_FSDAECU - 3);
	}
	else	memcpy(p->ecu, &pDoz->recip[fid].real.ecu.co.a.t.s.ref, SIZE_FSDAECU);
	// 200218
	//memcpy(p->hvac, &pDoz->recip[fid].real.hvac.a.t.s.cam, SIZE_FSDAHVAC);
	GETPROSE(pProse);
	CProse::PLSHARE pLsv = pProse->GetShareVars();
	memcpy(p->hvac, &pLsv->hvaccf.fl[cid].a.t.s.cam, SIZE_FSDAHVAC);
}

void CArch::Arteriam()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pt = localtime(&tv.tv_sec);

	c_mtt.Lock();
	c_dtsTime.year = pt->tm_year + 1900;
	c_devTime.year = (BYTE)(c_dtsTime.year % 100);
	c_dtsTime.mon = c_devTime.mon = (BYTE)(pt->tm_mon + 1);
	c_dtsTime.day = c_devTime.day = (BYTE)pt->tm_mday;
	c_dtsTime.hour = c_devTime.hour = (BYTE)pt->tm_hour;
	c_dtsTime.min = c_devTime.min = (BYTE)pt->tm_min;
	c_dtsTime.sec = c_devTime.sec = (BYTE)pt->tm_sec;
	c_dtsTime.msec = (WORD)(tv.tv_usec / 1000);
	c_mtt.Unlock();

	for (int n = 0; n < SPARKLE_MAX; n ++) {
		for (int m = 0; m < 2; m ++) {
			if (c_fspk[n][m].bFlash && c_fspk[n][m].bCounter && -- c_fspk[n][m].wCounter == 0)
				c_fspk[n][m].bFlash = c_fspk[n][m].bCounter = false;
		}
	}
	PBULKPACK pBulk = c_auxi.GetBulk();
	c_mmm.wState = pBulk->wState;
	c_mmm.wSeq = pBulk->m[BPID_PERIOD].wSeq;

	// 171101
	if (c_wWarmupTimer > 0)	-- c_wWarmupTimer;

	c_auxi.Arteriam();
}

bool CArch::SetSysTime(struct tm st, bool byRtd)
{
	if (st.tm_year < MIN_DTLYEAR4 || st.tm_year > MAX_DTLYEAR4) {
		TRACK("ARCH>ERR:set time - year!(%d)\n", st.tm_year);
		return false;
	}
	if (st.tm_mon < 1 || st.tm_mon > 12) {
		TRACK("ARCH>ERR:set time - month!(%d)\n", st.tm_mon);
		return false;
	}
	if (st.tm_mday < 1 || st.tm_mday > 31) {
		TRACK("ARCH>ERR:set time - day!(%d)\n", st.tm_mday);
		return false;
	}
	if (st.tm_hour < 0 || st.tm_hour > 23) {
		TRACK("ARCH>ERR:set time - hour!(%d)\n", st.tm_hour);
		return false;
	}
	if (st.tm_min < 0 || st.tm_min > 59) {
		TRACK("ARCH>ERR:set time - minute!(%d)\n", st.tm_min);
		return false;
	}
	if (st.tm_sec < 0 || st.tm_sec > 59) {
		TRACK("ARCH>ERR:set time - second!(%d)\n", st.tm_sec);
		return false;
	}

	st.tm_year -= 1900;
	st.tm_mon -= 1;
	time_t t = mktime(&st);
	if (t == (time_t)-1) {
		TRACK("ARCH>ERR:set time at mktime()!\n");
		return false;
	}
	struct timespec ts;
	ts.tv_sec = t;
	ts.tv_nsec = 0;
	int res = clock_settime(CLOCK_REALTIME, &ts);
	if (!Validity(res)) {
		TRACK("ARCH>ERR:set time!(%s)\n", strerror(errno));
		return false;
	}
	//c_auxi.AidCmd(AIDCMD_TIMESET);
	c_auxi.AidCmd(byRtd ? AIDCMD_TIMESETBYRTD : AIDCMD_TIMESETBYUSER);

	return true;
}

bool CArch::SetSysTimeByBcdDts(DTSTIME dtst)
{
	GETTOOL(pTool);
	struct tm st;
	st.tm_year = pTool->ToHex(dtst.year);
	st.tm_mon = pTool->ToHex(dtst.mon);
	st.tm_mday = pTool->ToHex(dtst.day);
	st.tm_hour = pTool->ToHex(dtst.hour);
	st.tm_min = pTool->ToHex(dtst.min);
	st.tm_sec = pTool->ToHex(dtst.sec);
	st.tm_isdst = 0;
	return SetSysTime(st, false);
}

void CArch::GetSysTimeToBcdDts(PDTSTIME pDtst)
{
	GETTOOL(pTool);
	c_mtt.Lock();
	pDtst->year = pTool->ToBcd(c_dtsTime.year);
	pDtst->mon = pTool->ToBcd(c_dtsTime.mon);
	pDtst->day = pTool->ToBcd(c_dtsTime.day);
	pDtst->hour = pTool->ToBcd(c_dtsTime.hour);
	pDtst->min = pTool->ToBcd(c_dtsTime.min);
	pDtst->sec = pTool->ToBcd(c_dtsTime.sec);
	pDtst->msec = pTool->ToBcd(c_dtsTime.msec);
	c_mtt.Unlock();
}

void CArch::GetSysTimeToDbf(PDBFTIME pDbft)
{
	DEVTIME devt;
	c_mtt.Lock();
	memcpy(&devt, &c_devTime, sizeof(DEVTIME));
	c_mtt.Unlock();

	Devt2Dbft(pDbft, devt);
}

bool CArch::SetSysTimeByDbf(DBFTIME dbft, bool byRtd)
{
	struct tm st;
	st.tm_year = (int)dbft.t.year + 2010;
	st.tm_mon = dbft.t.mon;
	st.tm_mday = dbft.t.day;
	st.tm_hour = dbft.t.hour;
	st.tm_min = dbft.t.min;
	st.tm_sec = dbft.t.sec;
	st.tm_isdst = 0;
	return SetSysTime(st, byRtd);
}

void CArch::Devt2Dbft(PDBFTIME pDbft, DEVTIME devt)
{
	if (devt.year > 10)	pDbft->t.year = devt.year - 10;
	else	pDbft->t.year = 0;
	pDbft->t.mon = devt.mon;
	pDbft->t.day = devt.day;
	pDbft->t.hour = devt.hour;
	pDbft->t.min = devt.min;
	pDbft->t.sec = devt.sec;
}

void CArch::Dbft2Devt(PDEVTIME pDevt, DBFTIME dbft)
{
	pDevt->year = dbft.t.year + 10;
	pDevt->mon = dbft.t.mon;
	pDevt->day = dbft.t.day;
	pDevt->hour = dbft.t.hour;
	pDevt->min = dbft.t.min;
	pDevt->sec = dbft.t.sec;
}

void CArch::Dbft2BcdDevt(PDEVTIME pDevt, DBFTIME dbft)
{
	DEVTIME devt;
	Dbft2Devt(&devt, dbft);

	GETTOOL(pTool);
	pDevt->year = pTool->ToBcd(devt.year);
	pDevt->mon = pTool->ToBcd(devt.mon);
	pDevt->day = pTool->ToBcd(devt.day);
	pDevt->hour = pTool->ToBcd(devt.hour);
	pDevt->min = pTool->ToBcd(devt.min);
	pDevt->sec = pTool->ToBcd(devt.sec);
}

bool CArch::CheckDbft(DBFTIME dbft)
{
	if (dbft.t.mon < 1 || dbft.t.mon > 12) {
		TRACK("ARCH:ERR mon %d\n", dbft.t.mon);
		return false;
	}
	if (dbft.t.day < 1 || dbft.t.day > 31) {
		TRACK("ARCH:ERR day %d\n", dbft.t.day);
		return false;
	}
	if (dbft.t.hour > 23) {
		TRACK("ARCH:ERR hour %d\n", dbft.t.hour);
		return false;
	}
	if (dbft.t.min > 59) {
		TRACK("ARCH:ERR min %d\n", dbft.t.min);
		return false;
	}
	if (dbft.t.sec > 59) {
		TRACK("ARCH:ERR sec %d\n", dbft.t.sec);
		return false;
	}
	return true;
}

bool CArch::CheckDbft(DBFTIME dbfta, DBFTIME dbftb)
{
	if (dbfta.t.year != dbftb.t.year)	return false;
	if (dbfta.t.mon != dbftb.t.mon)		return false;
	if (dbfta.t.day != dbftb.t.day)		return false;
	DWORD dwa = (DWORD)dbfta.t.hour * 3600l + (DWORD)dbfta.t.min * 60l + (DWORD)dbfta.t.sec;
	DWORD dwb = (DWORD)dbftb.t.hour * 3600l + (DWORD)dbftb.t.min * 60l + (DWORD)dbftb.t.sec;
	if (dwa == dwb)	return true;	// same
	else if (dwa < dwb) {
		DWORD dw = dwa;
		dwa = dwb;					// dwa is large
		dwb = dw;
	}
	if (dwa < 2l) {
		dwa += 2l;
		dwb += 2l;
	}
	if ((dwa - 2) <= dwb && dwa >= dwb)	return true;
	return false;
}

bool CArch::CloseCurrent()
{
	if (c_auxi.CloseCurrent()) {
		c_bLogArea = true;
		return true;
	}
	return false;
}

bool CArch::NewSheet()
{
	if (c_auxi.NewSheet()) {
		c_bLogArea = true;
		return true;
	}
	return false;
}

bool CArch::Evacuate()
{
	FILE* fs = fopen(LOG_PATHTMP, "r");
	if (fs == NULL) {
		printf("ARCH>ERR:%s() can not open log file!\n", __FUNCTION__);
		return false;
	}
	char sbuf[256];
	sprintf(sbuf, "%s%02d%02d%02d%02d%02d%02d", LOG_NAME,
			c_dtsTime.year % 100, c_dtsTime.mon, c_dtsTime.day, c_dtsTime.hour, c_dtsTime.min, c_dtsTime.sec);
	FILE* fd = fopen(sbuf, "w");
	if (fd == NULL) {
		fclose(fs);
		printf("ARCH>ERR:%s() can not create evacuation file!\n", __FUNCTION__);
		return false;
	}
	int ret = 0;
	int sum = 0;
	char buf[4096];
	do {
		ret = fread(buf, 1, 4096, fs);
		if (ret < 0) {
			fclose(fs);
			fclose(fd);
			printf("ARCH>ERR:%s() can not read!\n", __FUNCTION__);
			return false;
		}
		sum += ret;
		int res = fwrite(buf, 1, ret, fd);
		if (res < 0) {
			fclose(fs);
			fclose(fd);
			printf("ARCH>ERR:%s() can not write!\n", __FUNCTION__);
			return false;
		}
	} while (ret >= 4096);
	fclose(fs);
	fclose(fd);
	remove(LOG_PATHTMP);
	printf("ARCH:success evacuation.%s %d\n", sbuf, sum);
	return true;
}

// MM에 대한 TU 마감...
bool CArch::ClosureInside(bool bLogbook)
{
#if	defined(_AT_TS_)
	PBULKPACK pBulk = c_auxi.GetBulk();
	// 170731, PERIOD에서는 bBegin과 bEnd를 쓰지 않아야 한다.
	// 170804, 다시 쓴다
	//GETBAND(pBand);
	//pBand->Region(5);
	if (pBulk->m[BPID_PERIOD].bBegin) {
		if (++ c_mmm.wSkip >= MMMONITOR_DEBOUNCE2) {
			CLand* pLand = (CLand*)c_pCarrier;
			ASSERTP(pLand);
			CFio* pFio = pLand->GetFio();
			pFio->SetText(4, (PSZ)"m2dn");
			pLand->LeaveReason(1);
			TRACK("ARCH>ERR:mm down2!\n");
			// 171219
			//char buf[256];
			//sprintf(buf, "%s%02d%02d%02d%02d%02d%02d", LOG_PATHTMP,
			//		c_dtsTime.year % 100, c_dtsTime.mon, c_dtsTime.day, c_dtsTime.hour, c_dtsTime.min, c_dtsTime.sec);
			//rename(LOG_PATHTMP, buf);
			Evacuate();
			//pFio->DisableWatchdog();
			c_wClosureInsideSeq = 0;
			exit(0);
		}
		else {
			// 170817 TRACK을 쓰면 pBulk->m[BPID_PERIOD].bBegin로 인해 시간 지연이 300ms정도 생긴다.
			// TRACK에서 파일 쓰기를 하지않는다..
			c_bDelayPrevious = true;
			c_wClosureInsideSeq = 1;
			return false;		// BULK가 아직 추가 중이다 끝날 때까지...
		}
	}
	else {
		if (c_bDelayPrevious) {
			c_bDelayPrevious = false;
			TRACK("ARCH>MON:work %d required %d!!!\n", c_wLastPeriodCmd, c_mmm.wSkip);
		}
		c_mmm.wSkip = 0;
	}


	if (bLogbook) {
		if (c_bLogArea) {
			BuildLogbook();
			// 170831
			PV3FBRKCNTF pVbf = GetV3fBreaker();
			for (int n = 0; n < V3F_MAX; n ++) {
				pBulk->bcu.v.wHbCnt[n] = pVbf->u.v.wHbCnt[n];
				pBulk->bcu.v.wLbCnt[n] = pVbf->u.v.wLbCnt[n];
			}
			pBulk->bcu.v.wAptness = BCAPTNESS_WORD;
			WORD sum = 0;
			for (int n = 0; n < (int)(sizeof(V3FBRKCNTS) - sizeof(WORD)); n ++)
				sum += (WORD)pBulk->bcu.c[n];
			pBulk->bcu.v.wCSum = sum;

			c_auxi.AidCmdPeriod(AIDCMD_APPENDLOGBOOK, (BYTE*)&c_rec.cur, sizeof(LOGBOOK));
			SetStillOnRecord();
			c_wLastPeriodCmd = AIDCMD_APPENDLOGBOOK;
			c_wClosureInsideSeq = 3;
			//if (!(pBulk->wState & ((1 << BULKSTATE_MOUNTING) | (1 << BULKSTATE_TIMESETTING))))	c_mmm.bCmp = true;
		}
		else {
			if (c_wFutile < 0xffff)	++ c_wFutile;
			c_wClosureInsideSeq = 5;
		}
	}
	else {
		if (!(pBulk->wState & (1 << BULKSTATE_TIMESETTING))) {
			c_mmm.bCmp = false;
			c_mmm.wSeq = pBulk->m[BPID_PERIOD].wSeq;
			if (c_mmm.wSeq != c_rec.wSeq) {			// 171212 c_rec.wSeq는 건들지 말아야한다.
				TRACK("AUX:mm invalid %d %d!\n", c_rec.wSeq, c_mmm.wSeq);
				if (++ c_mmm.wDeb >= MMMONITOR_DEBOUNCE) {
					CLand* pLand = (CLand*)c_pCarrier;
					ASSERTP(pLand);
					CFio* pFio = pLand->GetFio();
					pFio->SetText(4, (PSZ)"m-dn");
					pLand->LeaveReason(2);
					TRACK("ARCH>ERR:mm down!\n");
					// 171219
					//char buf[256];
					//sprintf(buf, "%s%02d%02d%02d%02d%02d%02d", LOG_PATHTMP,
					//		c_dtsTime.year % 100, c_dtsTime.mon, c_dtsTime.day, c_dtsTime.hour, c_dtsTime.min, c_dtsTime.sec);
					//rename(LOG_PATHTMP, buf);
					Evacuate();
					//pFio->DisableWatchdog();
					c_wClosureInsideSeq = 7;
					exit(0);
				}
			}
			else	c_mmm.wDeb = 0;
		}
		if (c_bChangeEnvirons) {
			c_bChangeEnvirons = false;
			c_auxi.AidCmd(AIDCMD_SAVEENVIRONS);			// period cmd
			c_wLastPeriodCmd = AIDCMD_SAVEENVIRONS;
			c_wClosureInsideSeq = 9;
		}
		else if (c_bInsprInside) {
			c_bInsprInside = false;
			c_auxi.SaveInspect((BYTE*)&c_inspr);
			c_wLastPeriodCmd = AIDCMD_SAVEINSPECT;
			c_wClosureInsideSeq = 11;
		}
		else if (c_auxi.GetEcuTraceDemand() && !(pBulk->wState & (1 << BULKSTATE_ARRANGEECUTRACE))) {
			c_auxi.ClearEcuTraceDemand();
			c_auxi.AidCmd(AIDCMD_SAVEECUTRACE);
			c_wLastPeriodCmd = AIDCMD_SAVEECUTRACE;
			c_wClosureInsideSeq = 13;
		}
		else if (c_auxi.GetSivDetectDemand()) {
			c_auxi.ClearSivDetectDemand();
			c_auxi.AidCmd(AIDCMD_SAVESIVDETECT);
			c_wLastPeriodCmd = AIDCMD_SAVESIVDETECT;
			c_wClosureInsideSeq = 15;
		}
		else if (c_auxi.GetTraceSphereDemand(DEVID_SIV)) {
			c_auxi.ClearTraceSphereDemand(DEVID_SIV);
			c_auxi.AidCmd(AIDCMD_SAVESIVTRACESPHERE);
			c_wLastPeriodCmd = AIDCMD_SAVESIVTRACESPHERE;
			c_wClosureInsideSeq = 17;
		}
		else if (c_auxi.GetTraceSphereDemand(DEVID_V3F)) {
			c_auxi.ClearTraceSphereDemand(DEVID_V3F);
			c_auxi.AidCmd(AIDCMD_SAVEV3FTRACESPHERE);
			c_wLastPeriodCmd = AIDCMD_SAVEV3FTRACESPHERE;
			c_wClosureInsideSeq = 19;
		}
		else if (c_auxi.GetTraceSphereDemand(DEVID_ECU)) {
			c_auxi.ClearTraceSphereDemand(DEVID_ECU);
			c_auxi.AidCmd(AIDCMD_SAVEECUTRACESPHERE);
			c_wLastPeriodCmd = AIDCMD_SAVEECUTRACESPHERE;
			c_wClosureInsideSeq = 21;
		}
		else {
			if (c_bLogArea) {
				int bi = 0;
				BYTE tbulk[SIZE_BULKTINE];
				while (GetStillOnFilm((PTROUBLEINFO)&tbulk[bi], TFILMID_MM)) {	// SPARKLE_TROUBLEMM)) {
					bi += sizeof(TROUBLEINFO);
					if (((int)SIZE_BULKTINE - bi) < (int)sizeof(TROUBLEINFO))	break;
				}
				if (bi > 0) {
					c_auxi.AidCmdPeriod(AIDCMD_APPENDTROUBLE, tbulk, (WORD)bi);
					c_wLastPeriodCmd = AIDCMD_APPENDTROUBLE;
					c_wClosureInsideSeq = 23;
				}
				else {
					if (c_mmm.bUsbScan != GetUsbState()) {
						if (++ c_mmm.wUsbScanTime >= MMMONITOR_USBSCANTIME) {
							c_mmm.wUsbScanTime = 0;
							c_auxi.AidCmd(AIDCMD_USBSCAN);
							c_wLastPeriodCmd = AIDCMD_USBSCAN;
						}
						c_wClosureInsideSeq = 25;
					}
					else	c_wClosureInsideSeq = 27;
				}
			}
			else {
				if (c_wFutile < 0xffff)	++ c_wFutile;
				c_wClosureInsideSeq = 29;
			}
		}
	}
	// 190523
	BuildFullSda();

	return true;
#else
	return false;
#endif
}

// MM에 대한 CU 마감...
void CArch::ClosureInside()
{
	if (c_auxi.GetEcuTraceDemand()) {
		c_auxi.ClearEcuTraceDemand();
		c_auxi.AidCmd(AIDCMD_SAVEECUTRACE);
	}
	else if (c_auxi.GetSivDetectDemand()) {
		c_auxi.ClearSivDetectDemand();
		c_auxi.AidCmd(AIDCMD_SAVESIVDETECT);
	}
}

// RTD에 대한... SendRtd() -> Decorate() -> ClosureBridge()
void CArch::ClosureBridge(PRTDBINFO pRtdr)
{
	memset(pRtdr, 0, SIZE_RTDSB);
	c_rtdtc.dwSection = 0;
	pRtdr->wAddr = LADD_RTD;
	pRtdr->cCtrl = DEVCHAR_CTL;
	pRtdr->t.s.cFlow = DEVFLOW_REQ;
	GETPAPER(pPaper);
	BYTE uOblige = pPaper->GetOblige();
	pRtdr->t.s.pack.ob.b.main = (uOblige & (1 << OBLIGE_MAIN)) ? true : false;
	pRtdr->t.s.pack.ob.b.head = (uOblige & (1 << OBLIGE_HEAD)) ? true : false;
	pRtdr->t.s.pack.ob.b.active = (uOblige & (1 << OBLIGE_ACTIVE)) ? true : false;
	if (c_auxi.GetRtdNewSheetState()) {
		PBULKPACK pBulk = c_auxi.GetBulk();
		memcpy(&pRtdr->t.s.pack.pad.tp.tin.c[0], &pBulk->lbHead, sizeof(LOGBOOKHEAD));
		pRtdr->t.s.pack.pad.tp.wLength = sizeof(LOGBOOKHEAD);
		pRtdr->t.s.pack.cItem = RTDITEM_CLOSECURRENT;
		// TRACK("AUX:rtd new sheet!\n");			// 171206
		// ClearLogbookheadState()는 응답을 받고 실행한다.
	}
	else if (GetStillOnRecord((PLOGBOOK)&pRtdr->t.s.pack.pad.tp.tin.c[0])) {		//.f.b.c[0])) {
		pRtdr->t.s.pack.pad.tp.wLength = sizeof(LOGBOOK);
		pRtdr->t.s.pack.cItem = RTDITEM_SAVELOGBOOK;
	}
	else {
		// 190523
		if (c_bFstrBridge || c_cFullSdaFlow != 0) {
			c_bFstrBridge = false;
			memset(pRtdr->t.s.pack.pad.tp.tin.c, 0, sizeof(RTDTINE));
			memcpy(&pRtdr->t.s.pack.pad.tp.tin.c[0], FULLSDA_TEXT, SIZE_FULLSDATEXT);
			pRtdr->t.s.pack.pad.tp.tin.c[19] = c_cFullSdaFlow + '0';
			memcpy(&pRtdr->t.s.pack.pad.tp.tin.c[SIZE_FULLSDATEXT], c_fst[FSTID_REPORT].cbs[c_cFullSdaFlow], SIZE_FULLSDABLOCK);
			pRtdr->t.s.pack.pad.tp.wLength = SIZE_FULLSDATEXT + SIZE_FULLSDABLOCK;
			pRtdr->t.s.pack.cItem = RTDITEM_SAVEFULLSDASET;
			if (++ c_cFullSdaFlow >= 2)	c_cFullSdaFlow = 0;
		}
		else if (c_bInsprBridge) {
			c_bInsprBridge = false;
			memcpy(&pRtdr->t.s.pack.pad.tp.tin.c[0], &c_inspr, sizeof(INSPREPORT));
			pRtdr->t.s.pack.pad.tp.wLength = sizeof(INSPREPORT);
			pRtdr->t.s.pack.cItem = RTDITEM_SAVEINSPECT;
		}
		else {
			int bi = 0;
			while (GetStillOnFilm((PTROUBLEINFO)&pRtdr->t.s.pack.pad.tp.tin.c[bi], TFILMID_RTD)) {	// SPARKLE_TROUBLERTD)) {
				bi += sizeof(TROUBLEINFO);
				++ c_rtdtc.dwSection;
				if ((sizeof(RTDTINE) - bi) < sizeof(TROUBLEINFO))	break;
			}
			if (bi > 0) {
				pRtdr->t.s.pack.pad.tp.wLength = (WORD)bi;
				pRtdr->t.s.pack.cItem = RTDITEM_SAVETROUBLE;
				c_rtdtc.dwSend += c_rtdtc.dwSection;
			}
			else {
				c_auxi.BuildCdmaReport();
				memcpy(&pRtdr->t.s.pack.pad.tp.tin.c[0], c_auxi.GetCdmaReport(), sizeof(CDMAREPORT));
				pRtdr->t.s.pack.pad.tp.wLength = sizeof(CDMAREPORT);
				pRtdr->t.s.pack.cItem = RTDITEM_DESKREPORT;
			}
		}
	}
}

void CArch::PreInspect(WORD wItem)
{
	memset(&c_inspr, 0, sizeof(INSPREPORT));
	c_inspr.wItem = wItem;
	GetSysTimeToDbf(&c_inspr.dbft);
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_TCL))) {
		c_inspr.ics.wTcRes = XWORD(0xddd);	// 0000 1101 1101 1101 "시험 진행 중"
		c_inspr.ics.wTul = XWORD(0xff);		// "시험"
		c_inspr.ics.wCul = 0xffff;
		c_inspr.ics.wPole = 0xffff;
		c_inspr.ics.wSvf = 0xffff;
		c_inspr.ics.wEcu = 0xffff;
		c_inspr.ics.wHvac = 0xffff;
		c_inspr.ics.wEhv = XWORD(0xff);
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_V3F))) {
		c_inspr.ics.wV3fRes = XWORD(0xd);
		c_inspr.ics.wHb = 0;
		c_inspr.ics.wLb = 0;
		c_inspr.ics.wCpb = XWORD(0x155);
		//for (int n = 0; n < V3F_MAX; n ++)
		//	c_inspr.ics.wPwmv[n] = c_inspr.ics.wFc[n] = 0;
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_SIV))) {
		c_inspr.ics.wSivRes = XWORD(0xd);
		//for (int n = 0; n < SIV_MAX; n ++)
		//	c_inspr.ics.wEo[n] = c_inspr.ics.wFreq[n] = 0;
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_ECU))) {
		c_inspr.ics.ecu.wEcuRes = XWORD(0xd);
		c_inspr.ics.ecu.wEcuL = 0xffff;
		c_inspr.ics.ecu.wEcuH = XWORD(0xf);
		//for (int n = 0; n < 10; n ++)
		//	c_inspr.ics.ecu.wBcp[n] = c_inspr.ics.ecu.wPwme[n] = 0;
		for (int n = 0; n < 4; n ++) {
			c_inspr.ecu[n].wEcuRes = XWORD(0xd);
			c_inspr.ecu[n].wEcuL = 0xffff;
			c_inspr.ecu[n].wEcuH = XWORD(0xf);
			//for (int m = 0; m < 10; n ++)
			//	c_inspr.ecu[n].wBcp[m] = c_inspr.ecu[n].wPwme[m] = 0;
		}
		c_inspr.wEcui = 0;
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_COOL))) {
		c_inspr.ics.wCoolRes = XWORD(0xd);
		c_inspr.ics.wCool1 = c_inspr.ics.wCool2 = 0xffff;
		c_inspr.ics.wCool12 = XWORD(0xff);
	}
	if ((wItem & (1 << INSPITEM_ALL)) || (wItem & (1 << INSPITEM_HEAT))) {
		c_inspr.ics.wHeatRes = XWORD(0xd);
		c_inspr.ics.wHeatL = 0xffff;
		c_inspr.ics.wHeatH = XWORD(0xf);
	}
	//GETPAPER(pPaper);
	//c_inspr.wProperNo = pPaper->GetProperNo();
}

void CArch::SuffInspect(WORD wRequired)
{
	c_inspr.wRequired = wRequired;
	c_bInsprInside = c_bInsprBridge = true;
	TRACK("ARCH:suff insp.\n");
}

int CArch::Bale(bool bType, BYTE* pMesh)
{
	int leng = 0;
	if (bType) {
		sprintf((char*)pMesh, "$LOG, %d,", sizeof(LOGBOOK));
		while (*pMesh != '\0') {
			++ pMesh;
			++ leng;
		}
	}
	else {
		memcpy(pMesh, &c_rec.cur, sizeof(LOGBOOK));
		pMesh += sizeof(LOGBOOK);
		leng += sizeof(LOGBOOK);
	}
	leng += c_auxi.Bale(bType, pMesh);
	return leng;
}

void CArch::SdaPrompt(bool bSign, int iCID, WORD wCode, WORD wDeb)
{
	if (iCID >= CID_MAX || wCode >= MAX_TROUBLE) {
		TRACK("ARCH:wrong sda fault code cid %d code %d!\n", iCID, wCode);
		return;
	}

	if (bSign) {
		if (++ c_sdafd.wDeb[iCID][wCode] >= wDeb) {
			c_sdafd.wDeb[iCID][wCode] = wDeb;
			Prompt(true, iCID, wCode);
		}
	}
	else {
		if (c_sdafd.wDeb[iCID][wCode] >= wDeb)	Prompt(false, iCID, wCode);
		c_sdafd.wDeb[iCID][wCode] = 0;
	}
}
void CArch::UsbScaning(bool bScan)
{
	c_mmm.bUsbScan = bScan;
	c_mmm.wUsbScanTime = 0;
}

bool CArch::GetUsbState()
{
	PBULKPACK pBulk = c_auxi.GetBulk();
	if (pBulk->wState & (1 << BULKSTATE_SEIZEUSB))	return true;
	return false;
}

ENTRY_CONTAINER(CArch)
	//SCOOP(&c_troubleRef.dwIndex,				sizeof(DWORD),	"Arch")
	//SCOOP(&c_troubleRef.dwReduce,				sizeof(DWORD),	"")
	//SCOOP(&c_troubleRef.dwSevere,				sizeof(DWORD),	"")
	//SCOOP(&c_troubleRef.dwCrash,				sizeof(DWORD),	"")
	//SCOOP(&c_troubleRef.dwState,				sizeof(DWORD),	"")
	//SCOOP(&c_troubleRef.dwOther,				sizeof(DWORD),	"")
	//SCOOP(&c_troubleRef.wRecentCode,			sizeof(WORD),	"")
	//SCOOP(&c_troubleRef.wRecentCid,				sizeof(WORD),	"")
	SCOOP(&c_troubleTray[TTRAY_TROUBLE].dwIndex,	sizeof(DWORD),	"Arch")
	SCOOP(&c_troubleTray[TTRAY_TROUBLE].dwReduce,	sizeof(DWORD),	"")
	SCOOP(&c_troubleTray[TTRAY_TROUBLE].dwHeavy,	sizeof(DWORD),	"")
	SCOOP(&c_troubleTray[TTRAY_STATUS].dwIndex,		sizeof(DWORD),	"")
	SCOOP(&c_troubleTray[TTRAY_STATUS].dwReduce,	sizeof(DWORD),	"")
	SCOOP(&c_troubleTray[TTRAY_OTHER].dwIndex,	sizeof(DWORD),	"")
	SCOOP(&c_troubleTray[TTRAY_OTHER].dwReduce,	sizeof(DWORD),	"")
	SCOOP(&c_rtdtc.dwReserve,					sizeof(DWORD),	"rtda")
	SCOOP(&c_rtdtc.dwSend,						sizeof(DWORD),	"")
	SCOOP(&c_rtdtc.dwConfirm,					sizeof(DWORD),	"")
	//SCOOP(&c_film[TFILMID_MM].ri,				sizeof(WORD),	"fm")
	//SCOOP(&c_film[TFILMID_MM].wi,				sizeof(WORD),	"")
	//SCOOP(&c_film[TFILMID_MM].stack,			sizeof(WORD),	"")
	//SCOOP(&c_film[TFILMID_RTD].ri,				sizeof(WORD),	"fr")
	//SCOOP(&c_film[TFILMID_RTD].wi,				sizeof(WORD),	"")
	//SCOOP(&c_film[TFILMID_RTD].stack,			sizeof(WORD),	"")
	//SCOOP(&c_rec.ri,							sizeof(WORD),	"rec")
	//SCOOP(&c_rec.wi,							sizeof(WORD),	"")
	//SCOOP(&c_rec.stack,							sizeof(WORD),	"")
	//SCOOP(&c_rec.wSeq,							sizeof(WORD),	"")
	//SCOOP(&c_mmm.wState,						sizeof(WORD),	"bulk")
	//SCOOP(&c_mmm.wSeq,							sizeof(WORD),	"")
	//SCOOP(&c_mmm.wDeb,							sizeof(WORD),	"")
	//SCOOP(&c_mmm.bUsbScan,						sizeof(BYTE),	"")
	//SCOOP(&c_wFutile,							sizeof(WORD),	"fut")
	//SCOOP(&c_wClosureInsideSeq,					sizeof(WORD),	"")
	//SCOOP(&c_sfd.wDeb[0][7],					sizeof(WORD),	"")
	//SCOOP(&c_ufd.wDeb[0][1],					sizeof(WORD),	"")
	//SCOOP(&c_ifd.wDeb[0][2],					sizeof(WORD),	"")
	//SCOOP(&c_dwMakeLogbookTime,					sizeof(DWORD),	"ml")
	SCOOP(&c_inspr.ics,							sizeof(WORD) * 14, "Insp")
	ISCOOP(c_auxi)
EXIT_CONTAINER()
