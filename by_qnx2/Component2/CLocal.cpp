/*
 * CLocal.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>

#include "Track.h"
#include "../Inform2/DevInfo2/DevInfo.h"
#include "Mm.h"

#include "CBand.h"
#include "CFsc.h"
#include "CPaper.h"
#include "CProse.h"
#include "CArch.h"
#include "CLocal.h"

const LOCALLAYERA CLocal::c_layer[] = {		// this order must be the same as DEVINDEX
/* SIV */	{	PSCCHB_SVF,   LADD_SIV,  38400, 0, (PSZ)"SIV"  },		// 50ms
/* V3F */	{	PSCCHB_SVF,   LADD_V3F,  38400, 0, (PSZ)"V3F"  },		// 50ms
/* ECU */	{	PSCCHB_ECU,   LADD_ECU,  38400, 0, (PSZ)"ECU"  },		// 50ms
// 200218
/* HVAC*/	//{	PSCCHB_HVAC,  LADD_HVAC, 38400, 0, (PSZ)"HVAC" },		// 100ms(HVAC & CMSB alternately)	// 200ms
/* HVAC*/	{	PSCCHB_HVAC,  0xffff,    38400, 0, (PSZ)"HCM" },		// 100ms
/* CMSB*/	{	PSCCHB_HVAC,  LADD_CMSB, 38400, 0, (PSZ)"CMSB" },		// this item is not used. it just sorted according to DEVID_ order!!!!!
/* DCUL*/	{	PSCCHB_DCUL,  0xffff,    38400, 0, (PSZ)"DCUL" },		//{	PSCCHB_DCUL,  LADD_DCUL, 38400, 0, (PSZ)"DCUL" },		// 100ms
/* DCUR*/	{	PSCCHB_DCUR,  0xffff,    38400, 0, (PSZ)"DCUR" }		//{	PSCCHB_DCUR,  LADD_DCUR, 38400, 0, (PSZ)"DCUR" }		// 100ms
};

const WORD CLocal::c_wDcuAddr[2][4] = {
	{	0x0170,	0x0270,	0x0370,	0x0470	},
	{	0x1170,	0x1270,	0x1370,	0x1470	}
};

CLocal::CLocal(PVOID pParent, QHND hReservoir)
	: CPump(pParent, hReservoir, (PSZ)"LOCAL")
{
	c_wConfig = 0;
	c_wAllotWait = 0;
	c_cDcuID[0] = c_cDcuID[1] = 0xff;
	c_bHcbAlter = false;
	c_bHvacAlter = false;
	for (int n = 0; n < DEVID_LENG; n ++) {
		c_pDev[n]= NULL;
		c_bRecvMon[n] = false;
	}
	//c_bEcuBlock = false;
	//c_wEcuTxSeq = c_wEcuRxSeq = 0;

	Initial();
}

CLocal::CLocal(PVOID pParent, QHND hReservoir, PSZ pszOwnerName)
	: CPump(pParent, hReservoir, pszOwnerName)
{
	c_wConfig = 0;
	c_wAllotWait = 0;
	c_cDcuID[0] = c_cDcuID[1] = 0xff;
	c_bHcbAlter = false;
	c_bHvacAlter = false;
	//c_bEcuAlter = false;	// 210803
	for (int n = 0; n < DEVID_LENG; n ++) {
		c_pDev[n]= NULL;
		c_bRecvMon[n] = false;
	}
	//c_bEcuBlock = false;
	//c_wEcuTxSeq = c_wEcuRxSeq = 0;

	Initial();
}

CLocal::~CLocal()
{
	Destroy();
}

void CLocal::Initial()
{
	// 200218
	//memset(&c_wRed, 0, sizeof(WORD) * DEVIDDE_MAX * LCF_MAX);
	memset(&c_wRed, 0, sizeof(WORD) * REALDEV_MAX * LCF_MAX);
}

void CLocal::Destroy()
{
	for (int n = 0; n < DEVID_LENG; n ++) {
		KILLC(c_pDev[n]);
	}
}

int CLocal::GetLIDfromCh(int ch)
{
	switch (ch) {
	case PSCCHB_SVF :
		if (c_wConfig & (1 << DEVID_SIV))	return DEVID_SIV;
		else	return DEVID_V3F;
		break;
	case PSCCHB_ECU :	return DEVID_ECU;	break;
	// 200218
	//case PSCCHB_HVAC :	return DEVID_HVAC;	break;
	case PSCCHB_HVAC :
		{
			GETPAPER(pPaper);
			if (pPaper->GetDeviceExFromRecip() && (c_wConfig & (1 << DEVID_SIV)) && c_bHcbAlter)
				return DEVID_CMSB;
			else	return DEVID_HVAC;
		}
		break;
	case PSCCHB_DCUL :	return DEVID_DCUL;	break;
	case PSCCHB_DCUR :	return DEVID_DCUR;	break;
	default :	break;
	}
	return 0xff;
}

int CLocal::GetSerialID(int id)
{
	if (id < DEVID_DCUL)		return id;
	else if (id < DEVID_DCUR)	return id + (int)c_cDcuID[0];
	else if (id < DEVID_CM)		return id + (int)c_cDcuID[1] + 3;
	else	return id + 6;
}

int CLocal::GetSubID(int id)
{
	int iSubi;
	if (id == DEVID_DCUL)	iSubi = (int)c_cDcuID[0];
	else if (id == DEVID_DCUR)	iSubi = (int)c_cDcuID[1];
	else	iSubi = 0;
	return iSubi;
}

void CLocal::IncreaseRed(int id, int errf)
{
	INCWORD(c_wRed[id][errf]);
	if (errf > LCF_ALL)	INCWORD(c_wRed[id][LCF_ALL]);
}

WORD CLocal::CaptureRed(WORD* pBuf)
{
	// 200218
	//memcpy(pBuf, &c_wRed, sizeof(WORD) * DEVIDDE_MAX * LCF_MAX);
	//return sizeof(WORD) * DEVIDDE_MAX * LCF_MAX;
	memcpy(pBuf, &c_wRed, sizeof(WORD) * REALDEV_MAX * LCF_MAX);
	return sizeof(WORD) * REALDEV_MAX * LCF_MAX;
}

void CLocal::Timeout(int id)
{
	//if (id == DEVID_ECU) {
	//	if (!c_bEcuBlock) {
	//		GETFSC(pFsc);
	//		WORD w = pFsc->GetRxSeq(PSCCHB_ECU);
	//		TRACK("ECU RX WARN B: %d %d %d\n", c_wEcuTxSeq, c_wEcuRxSeq, w);
	//		c_wEcuTxSeq = c_wEcuRxSeq = w;
	//		c_bEcuBlock = true;
	//	}
	//}

	c_bRecvMon[id] = false;
	GETPROSE(pProse);
	BYTE cSubi = GetSubID(id);
	if (!pProse->Repair(id, cSubi, LCF_RTIMEOUT))	IncreaseRed(GetSerialID(id), LCF_RTIMEOUT);
}

bool CLocal::SendSiv()
{
	if (c_bRecvMon[DEVID_SIV])	Timeout(DEVID_SIV);

	GETPROSE(pProse);
	PSIVRINFO pSivr = (PSIVRINFO)pProse->Decorate(DEVID_SIV, 0);
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())		// CU or main
		bRes = c_pDev[DEVID_SIV]->Send((BYTE*)pSivr, SIZE_SIVSR);
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_SIV), LCF_SEND);
		TRACK("%s>SIV>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[DEVID_SIV] = true;

	return bRes;
}

bool CLocal::SendV3f()
{
	if (c_bRecvMon[DEVID_V3F])	Timeout(DEVID_V3F);

	GETPROSE(pProse);
	PV3FRINFO pV3fr = (PV3FRINFO)pProse->Decorate(DEVID_V3F, 0);
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())		// CU or main
		bRes = c_pDev[DEVID_V3F]->Send((BYTE*)pV3fr, SIZE_V3FSR);
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_V3F), LCF_SEND);
		TRACK("%s>V3F>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[DEVID_V3F] = true;

	return bRes;
}

bool CLocal::SendEcu()
{
	if (c_bRecvMon[DEVID_ECU])	Timeout(DEVID_ECU);

	GETPROSE(pProse);
	PECURINFO pEcur = (PECURINFO)pProse->Decorate(DEVID_ECU, 0);
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())		// CU or main
		bRes = c_pDev[DEVID_ECU]->Send((BYTE*)pEcur, SIZE_ECUSR);
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_ECU), LCF_SEND);
		TRACK("%s>ECU>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else {
		c_bRecvMon[DEVID_ECU] = true;
		//++ c_wEcuTxSeq;
	}

	return bRes;
}

// 200218
bool CLocal::SendCmsb()
{
	if (c_bRecvMon[DEVID_CMSB])	Timeout(DEVID_CMSB);

	GETPROSE(pProse);
	PCMSBRINFO pCmsbr = (PCMSBRINFO)pProse->Decorate(DEVID_CMSB, 0);
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())
		bRes = c_pDev[DEVID_HVAC]->Send((BYTE*)pCmsbr, SIZE_CMSBSR);	// CAUSION !!!: real channel
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_CMSB), LCF_SEND);
		TRACK("%s>CMSB>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[DEVID_CMSB] = true;

	return bRes;
}

bool CLocal::SendHvac()
{
	if (c_bRecvMon[DEVID_HVAC])	Timeout(DEVID_HVAC);

	GETPROSE(pProse);
	// 200218
	PHVACRINFO pHvacr = (PHVACRINFO)pProse->Decorate(DEVID_HVAC, 0);
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())		// CU or main
		bRes = c_pDev[DEVID_HVAC]->Send((BYTE*)pHvacr, SIZE_HVACSR);
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_HVAC), LCF_SEND);
		TRACK("%s>HVAC>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[DEVID_HVAC] = true;

	return bRes;
}

bool CLocal::SendDcul()
{
	if (c_bRecvMon[DEVID_DCUL])	Timeout(DEVID_DCUL);

	GETPROSE(pProse);
	if (c_cDcuID[0] < 4)	pProse->SetFinalDculID(c_cDcuID[0]);
	if (++ c_cDcuID[0] > 3)	c_cDcuID[0] = 0;
	WORD wAddr = c_wDcuAddr[0][c_cDcuID[0]];
	// 171124
	//if (!(c_pDev[DEVID_DCUL]->ModifyAddr(wAddr, c_layer[DEVID_DCUL].wCycleTime))) {
	//	IncreaseRed(GetSerialID(DEVID_DCUL), LCF_SEND);
	//	TRACK("%s>DCUL>ERR:address modify error!(0x%04X)\n", c_szOwnerName, wAddr);
	//	return false;
	//}

	PDCURINFO pDculr = (PDCURINFO)pProse->Decorate(DEVID_DCUL, c_cDcuID[0]);
	pDculr->wAddr = wAddr;
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())		// CU or main
		bRes = c_pDev[DEVID_DCUL]->Send((BYTE*)pDculr, SIZE_DCUSR);
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_DCUL), LCF_SEND);
		TRACK("%s>DCUL>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[DEVID_DCUL] = true;

	return bRes;
}

bool CLocal::SendDcur()
{
	if (c_bRecvMon[DEVID_DCUR])	Timeout(DEVID_DCUR);

	GETPROSE(pProse);
	if (c_cDcuID[1] < 4)	pProse->SetFinalDcurID(c_cDcuID[1]);
	if (++ c_cDcuID[1] > 3)	c_cDcuID[1] = 0;
	WORD wAddr = c_wDcuAddr[1][c_cDcuID[1]];
	// 171124
	//if (!(c_pDev[DEVID_DCUR]->ModifyAddr(wAddr, c_layer[DEVID_DCUR].wCycleTime))) {
	//	IncreaseRed(GetSerialID(DEVID_DCUR), LCF_SEND);
	//	TRACK("%s>DCUR>ERR:address modify error!(0x%04X)\n", c_szOwnerName, wAddr);
	//	return false;
	//}

	PDCURINFO pDcurr = (PDCURINFO)pProse->Decorate(DEVID_DCUR, c_cDcuID[1]);
	pDcurr->wAddr = wAddr;
	bool bRes = true;
	GETPAPER(pPaper);
	if (!pPaper->IsCab() || pPaper->IsMain())		// CU or main
		bRes = c_pDev[DEVID_DCUR]->Send((BYTE*)pDcurr, SIZE_DCUSR);
	if (!bRes) {
		IncreaseRed(GetSerialID(DEVID_DCUR), LCF_SEND);
		TRACK("%s>DCUR>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[DEVID_DCUR] = true;

	return bRes;
}

bool CLocal::Allot(int lid)
{
	//TRACK("Allot:%d %s ch %d addr 0x%04x bps %d\n", lid, c_layer[lid].pszName,
	//								c_layer[lid].iCh, c_layer[lid].wAddr, c_layer[lid].dwBps);
	// 200410
	//if (lid >= (int)(sizeof(c_layer) / sizeof(c_layer[0]))) {
	//	TRACK("%s>ERR:Unregistered local device id!(%d)\n", lid);
	//	return false;
	//}
	KILLC(c_pDev[lid]);
	c_pDev[lid] = new CDev(this, c_hReservoir, c_layer[lid].pszName);
	if (!c_pDev[lid]->Open(c_layer[lid].iCh, c_layer[lid].wAddr, c_layer[lid].dwBps, c_layer[lid].wCycleTime)) {
		TRACK("%s>ERR:initial failure!\n", c_layer[lid].pszName);
		delete c_pDev[lid];
		c_pDev[lid] = NULL;
		return false;
	}
	if (c_layer[lid].wAddr == LADD_SIV || c_layer[lid].wAddr == LADD_V3F)	c_wAllotWait = TIME_ALLOTWAIT;
	return true;
}

bool CLocal::Setup(WORD wConfig)
{
	if (wConfig == 0)	return false;

	c_wConfig = wConfig;
	for (int n = 0; n < DEVID_LENG; n ++) {
		KILLC(c_pDev[n]);
		if (c_wConfig & (1 << n)) {
			if (!Allot(n))	return false;
		}
	}
	c_cDcuID[0] = c_cDcuID[1] = 3;
	return true;
}

bool CLocal::ReSetup(WORD wConfig)
{
	if (!wConfig)	return false;

	for (int n = 0; n < DEVID_LENG; n ++) {
		if (c_wConfig & (1 << n)) {
			if (!(wConfig & (1 << n))) {
				KILLC(c_pDev[n]);
				TRACK("%s>WARNING:remove.\n", c_layer[n].pszName);
			}
		}
		else {
			if (wConfig & (1 << n)) {
				if (!Allot(n))	return false;
			}
		}
	}
	c_wConfig = wConfig;
	return true;
}

void CLocal::Send50()
{
	// 210803
	// 200918
	SendEcu();
	// 210803
	//c_bEcuAlter ^= true;
	//if (c_bEcuAlter)	SendEcu();
	if (c_wAllotWait == 0) {
		if (c_wConfig & (1 << DEVID_V3F))	SendV3f();
		if (c_wConfig & (1 << DEVID_SIV))	SendSiv();
	}
	else	-- c_wAllotWait;
}

// 200218
void CLocal::Send100()
{
	GETPAPER(pPaper);
	if (pPaper->GetDeviceExFromRecip() && (c_wConfig & (1 << DEVID_SIV))) {
		c_bHcbAlter ^= true;
		if (!c_bHcbAlter)	SendHvac();
		else	SendCmsb();
	}
	else {
		c_bHcbAlter = false;
		c_bHvacAlter ^= true;
		if (c_bHvacAlter)	SendHvac();
	}
	GETPROSE(pProse);
	pProse->SetFinalAlter(c_bHcbAlter);		// 0: hvac, 1: cmsb
}

void CLocal::TakeReceiveDev(DWORD dw)
{
	c_mtx.Lock();
	TAGUNPAR(dw, wLeng, state, ch);
	bool bReal = wLeng >= 0x8000 ? false : true;

	BYTE ldi = GetLIDfromCh(ch);
	if (ldi >= DEVID_CM) {
		IncreaseRed(GetSerialID(DEVIDDE_UKD), LCF_RADDR);
		c_mtx.Unlock();
		TRACK("%s>ERR:unknown ch.%d!\n", c_szOwnerName, ch);
		return;
	}

	// 200218
	BYTE rdi = ldi;
	if (rdi == DEVID_CMSB)	rdi = DEVID_HVAC;		// physical channel.

	GETPAPER(pPaper);
	GETPROSE(pProse);
	c_bRecvMon[ldi] = false;

	// check timeout
	if (!bReal) {
		if (pPaper->IsCab() && !pPaper->IsMain()) {
			IncreaseRed(GetSerialID(ldi), LCF_RECHO);
			c_mtx.Unlock();
			TRACK("%s>ERR!!!!!!!!!!:%s() UNREACHABLE LINE B !!!!!!!!\n", c_szOwnerName, __FUNCTION__);	// tu aux에서는 나타날 수 없는 작업.
			return;
		}
		Timeout(ldi);
		c_mtx.Unlock();
		return;
	}

	BYTE cSubi = GetSubID(ldi);
	// check status
	if ((state & ~8) != 0) {
		if (!pProse->Repair(ldi, cSubi, LCF_RCRC))	IncreaseRed(GetSerialID(ldi), LCF_RCRC);
		c_mtx.Unlock();
#if	defined(DEBUGLOCAL_ERRORTEXT)
		TRACK("%s>%s>ERR:status!(0x%02X)\n", c_szOwnerName, c_layer[ldi].pszName, state);
#endif
		return;
	}

	// 171124
	LOCALPREFIX prefix;
	// 200218
	//c_pDev[ldi]->GetPrefix(&prefix);
	c_pDev[rdi]->GetPrefix(&prefix);

	// check control char
	if (prefix.cCtrl != DEVCHAR_CTL) {
		if (pProse->Repair(ldi, cSubi, LCF_RFRAME))	IncreaseRed(GetSerialID(ldi), LCF_RFRAME);
		c_mtx.Unlock();
#if	defined(DEBUGLOCAL_ERRORTEXT)
		TRACK("%s>%s>ERR:control char.!(0x%02X)\n", c_szOwnerName, c_layer[ldi].pszName, prefix.cCtrl);
#endif
		return;
	}

	// check address
	if ((ldi == DEVID_DCUL && prefix.wAddr != c_wDcuAddr[ldi - DEVID_DCUL][c_cDcuID[0]]) ||
		(ldi == DEVID_DCUR && prefix.wAddr != c_wDcuAddr[ldi - DEVID_DCUL][c_cDcuID[1]]) ||
		// 200218
		//(ldi < DEVID_DCUL && prefix.wAddr != c_layer[ldi].wAddr)) {
		(ldi == DEVID_HVAC && prefix.wAddr != LADD_HVAC) ||
		(ldi == DEVID_CMSB && prefix.wAddr != LADD_CMSB) ||
		(ldi < DEVID_HVAC && prefix.wAddr != c_layer[ldi].wAddr)) {
		if (!pPaper->IsMain() &&
			((ldi == DEVID_DCUL && (prefix.wAddr == c_wDcuAddr[0][0] || prefix.wAddr == c_wDcuAddr[0][1] || prefix.wAddr == c_wDcuAddr[0][2] || prefix.wAddr == c_wDcuAddr[0][3])) ||
			(ldi == DEVID_DCUR && (prefix.wAddr == c_wDcuAddr[1][0] || prefix.wAddr == c_wDcuAddr[1][1] || prefix.wAddr == c_wDcuAddr[1][2] || prefix.wAddr == c_wDcuAddr[1][3]))))
			c_cDcuID[ldi - DEVID_DCUL] = (BYTE)(((prefix.wAddr >> 8) & 0xf) - 1);
		else {
			if (pProse->Repair(ldi, cSubi, LCF_RADDR))	IncreaseRed(GetSerialID(ldi), LCF_RADDR);
			c_mtx.Unlock();
#if	defined(DEBUGLOCAL_ERRORTEXT)
			TRACK("%s>%s>ERR:address!(0x%04X)\n", c_szOwnerName, c_layer[ldi].pszName, prefix.wAddr);
#endif
			return;
		}
	}

	// check flow control
	// 171221
	//if (prefix.cFlow != DEVFLOW_ANS) {
	if (prefix.cFlow != DEVFLOW_ANS && prefix.cFlow != DEVTRACE_ANS) {
		if (!pPaper->IsMain() && prefix.cFlow == DEVFLOW_REQ) {
			if (ldi == DEVID_DCUL || ldi == DEVID_DCUR)
				c_cDcuID[ldi - DEVID_DCUL] = (BYTE)(((prefix.wAddr >> 8) & 0xf) - 1);
			c_mtx.Unlock();
			return;
		}
		if (pProse->Repair(ldi, cSubi, LCF_RFRAME))	IncreaseRed(GetSerialID(ldi), LCF_RFRAME);
		c_mtx.Unlock();
#if	defined(DEBUGLOCAL_ERRORTEXT)
		TRACK("%s>%s>ERR:flow char.!(0x%02X)\n", c_szOwnerName, c_layer[ldi].pszName, prefix.cFlow);
#endif
		return;
	}

	PLOCALAINFO p = (PLOCALAINFO)pProse->GetInfo(true, ldi, cSubi, LI_CURR);
	WORD wTakeLength = pProse->GetLength(true, ldi);	// with trace
	// 200218
	//c_pDev[ldi]->Bunch((BYTE*)p, wTakeLength);
	c_pDev[rdi]->Bunch((BYTE*)p, wTakeLength);

	// check size
	WORD wTakeLengthNor = wTakeLength;
	// 180625
	//if (ldi == DEVID_SIV || ldi == DEVID_V3F)	wTakeLengthA = ldi == DEVID_SIV ? sizeof(SIVAINFO) : sizeof(V3FAINFO);
	// 190909
	//WORD wSivType = 0;
	//if (ldi == DEVID_V3F)	wTakeLengthA = sizeof(V3FAINFO);
	//else if (ldi == DEVID_SIV) {
	//	wSivType = pProse->GetSivType();
	//	if (wSivType == 2)	wTakeLengthA = sizeof(SIVAINFOEX);
	//	else if (wSivType == 1)	wTakeLengthA = sizeof(SIVAINFO);
	//	else {
	//		if (wLeng == sizeof(SIVAINFOEX)) {
	//			pProse->SetSivType(true);
	//			wTakeLengthA = sizeof(SIVAINFOEX);
	//			wSivType = 2;
	//		}
	//		else if (wLeng == sizeof(SIVAINFO)) {
	//			pProse->SetSivType(false);
	//			wTakeLengthA = sizeof(SIVAINFO);
	//			wSivType = 1;
	//		}
	//		else wTakeLengthA = sizeof(SIVAINFOEX);
	//	}
	//}
	if (ldi == DEVID_SIV || ldi == DEVID_V3F)	wTakeLengthNor = ldi == DEVID_SIV ? sizeof(SIVAINFOEX) : sizeof(V3FAINFO);

	if (wLeng != wTakeLength && wLeng != wTakeLengthNor) {
		if (!pProse->Repair(ldi, cSubi, LCF_RSIZE))	IncreaseRed(GetSerialID(ldi), LCF_RSIZE);
		c_mtx.Unlock();
#if	defined(DEBUGLOCAL_ERRORTEXT)
		TRACK("%s>%s>ERR:length!(%d,%02X,%02X)\n", c_szOwnerName, c_layer[ldi].pszName, wLeng, p->lf.cCtrl, p->lf.cFlow);
		//Dump((BYTE*)p, 6);
#endif
		return;
	}
	//if (ldi == DEVID_SIV && wSivType == 1) {
	//	PSIVAINFOEX pSiva = (PSIVAINFOEX)p;
	//	pSiva->t.s.wVer = pSiva->t.s.wSpr[0] = pSiva->t.s.wSpr[1] = pSiva->t.s.wSpr[2] = pSiva->t.s.wSpr[3] = 0;
	//}

	pProse->Success(ldi, cSubi);
	c_mtx.Unlock();
#if	defined(DEBUGLOCAL_STATETEXT)
	else	TRACK("%s>%s:receive %d bytes.\n", c_szOwnerName, c_layer[ldi].pszName, wLeng);
#endif

	//SendTag(TAG_RECEIVELOCAL, c_pParent, ldi);	//MAKEDWORD(bReal, id));

	// 170918
	//if (ldi == DEVID_ECU)	pProse->CorralEcuTrace();
	//else if (ldi == DEVID_SIV)	pProse->CorralSivDetect();
}

BEGIN_TAGTABLE(CLocal, CPump)
	_TAGMAP(TAG_RECEIVEDEV, CLocal::TakeReceiveDev)
END_TAGTABLE();

ENTRY_CONTAINER(CLocal)
	SCOOP(&c_wConfig,			sizeof(WORD),	"Loc")
	SCOOP(&c_bHcbAlter,			sizeof(BYTE),	"")
	SCOOP(&c_cDcuID[0],			sizeof(BYTE),	"")
	SCOOP(&c_cDcuID[1],			sizeof(BYTE),	"")
	//SCOOP(&c_bRecvMon[DEVID_SIV],	sizeof(BYTE),	"")
	//SCOOP(&c_bRecvMon[DEVID_V3F],	sizeof(BYTE),	"")
	//SCOOP(&c_bRecvMon[DEVID_ECU],	sizeof(BYTE),	"")
	//SCOOP(&c_bRecvMon[DEVID_HVAC],	sizeof(BYTE),	"")
	//SCOOP(&c_bRecvMon[DEVID_DCUL],	sizeof(BYTE),	"")
	//SCOOP(&c_bRecvMon[DEVID_DCUR],	sizeof(BYTE),	"")
	//SCOOP(&c_etf.iTotalLength,	sizeof(int),	"")
	//SCOOP(&c_wRed[DEVID_SIV][LCF_ALL],		(sizeof(WORD) * LCF_MAX),		"SIVF")
	//SCOOP(&c_wRed[DEVID_V3F][LCF_ALL],		(sizeof(WORD) * LCF_MAX),		"V3FF")
	//SCOOP(&c_wRed[DEVID_ECU][LCF_ALL],		(sizeof(WORD) * LCF_MAX),		"DCUF")
	//SCOOP(&c_wRed[DEVID_HVAC][LCF_ALL],		(sizeof(WORD) * LCF_MAX),		"HVCF")
	//SCOOP(&c_wRed[DEVID_DCUL][LCF_ALL],		(sizeof(WORD) * LCF_MAX),		"DL1F")
	//SCOOP(&c_wRed[DEVID_DCUL + 1][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DL2F")
	//SCOOP(&c_wRed[DEVID_DCUL + 2][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DL3F")
	//SCOOP(&c_wRed[DEVID_DCUL + 3][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DL4F")
	//SCOOP(&c_wRed[DEVID_DCUR + 3][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DR1F")
	//SCOOP(&c_wRed[DEVID_DCUR + 4][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DR2F")
	//SCOOP(&c_wRed[DEVID_DCUR + 5][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DR3F")
	//SCOOP(&c_wRed[DEVID_DCUR + 6][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"DR4F")
	//SCOOP(&c_wRed[DEVID_DCUR + 7][LCF_ALL],	(sizeof(WORD) * LCF_MAX),		"UNKF")
EXIT_CONTAINER()
