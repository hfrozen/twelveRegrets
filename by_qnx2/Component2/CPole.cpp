/*
 * CPole.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <string.h>

#include "Track.h"
#include "../Inform2/DevInfo2/DevInfo.h"
#include "CBand.h"
#include "CPaper.h"
#include "CVerse.h"
#include "CArch.h"
#include "CPole.h"

const LOCALLAYERA CPole::c_layer[] = {
/* PDEVID_ATO */	{	PSCCHA_ATO,	0xffff,		19200,		0,	(PSZ)"ATO"	},
/* PDEVID_HDU */	{	PSCCHA_HDU,	0xffff,		115200,		0,	(PSZ)"HDU"	},
/* PDEVID_RTD */	{	PSCCHB_RTD,	LADD_RTD,	1152000,	0,	(PSZ)"RTD"	},
/* PDEVID_PAU  */	{	PSCCHB_SVC,	LADD_PAU,	38400,		0,	(PSZ)"PAU"	},
/* PDEVID_PIS */	{	PSCCHB_SVC,	LADD_PIS,	38400,		0,	(PSZ)"PIS"	},
/* PDEVID_TRS */	{	PSCCHB_SVC,	LADD_TRS,	38400,		0,	(PSZ)"TRS"	},
// 200218
/* PDEVID_FDU */	{	PSCCHB_SVC, LADD_FDU,	38400,		0,	(PSZ)"FDU"	},
/* PDEVID_BMS */	{	PSCCHB_SVC,	LADD_BMS,	38400,		0,	(PSZ)"BMS"	}
};

//const BYTE CPole::c_cSvcSequence[] = { PDEVID_PAU, PDEVID_PIS, PDEVID_TRS };
const BYTE CPole::c_cSvcSequence[] = { PDEVID_PAU, PDEVID_TRS, PDEVID_PIS, PDEVID_FDU, PDEVID_BMS };	// 3, 5, 4, 6, 7

const WORD CPole::c_crc16Table[] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

CPole::CPole(PVOID pParent, QHND hReservoir)
	: CPump(pParent, hReservoir, (PSZ)"POLE")
{
	c_cAtoSeq = c_cAtoChg = 0;
	c_cSvcCurID = c_cSvcReceivedID = 0xff;
	for (int n = 0; n < REALPDEVCH_MAX; n ++)
		c_nd[n].pTerm = NULL;
	for (int n = 0; n < PDEVID_MAX; n ++)
		c_bRecvMon[n] = false;

	Initial();
}

CPole::CPole(PVOID pParent, QHND hReservoir, PSZ pszOwnerName)
	: CPump(pParent, hReservoir, pszOwnerName)
{
	c_cAtoSeq = c_cAtoChg = 0;
	c_cSvcCurID = c_cSvcReceivedID = 0xff;
	for (int n = 0; n < REALPDEVCH_MAX; n ++)
		c_nd[n].pTerm = NULL;
	for (int n = 0; n < PDEVID_MAX; n ++)
		c_bRecvMon[n] = false;

	Initial();
}

CPole::~CPole()
{
	Destroy();
}

void CPole::Initial()
{
	memset(&c_wRed, 0, sizeof(WORD) * PDEVID_MAX * LCF_MAX);
	c_cSvcCurID = c_cSvcReceivedID = 0xff;
}

void CPole::Destroy()
{
	for (int n = 0; n < REALPDEVCH_MAX; n ++) {
		if (n < PDEVID_RTD)	KILLC(c_nd[n].pTerm);
		else	KILLC(c_nd[n].pDev);
	}
}

WORD CPole::Crc161d0f(BYTE* p, WORD wLength)
{
	WORD crc = 0x1d0f;
	for (int n = 0; n < (int)wLength; n ++) {
		WORD tbi = ((crc >> 8) ^ *p ++) & 0xff;
		crc = (crc << 8) ^ c_crc16Table[tbi];
	}
	return crc;
}

WORD CPole::Crc16ffff(BYTE* p, WORD wLength)
{
	WORD crc = 0xffff;
	for (int n = 0; n < (int)wLength; n ++) {
		crc ^= (unsigned short)*p ++ << 8;
		for (int m = 0; m < 8; m ++)
			crc = crc << 1 ^ (crc & 0x8000 ? 0x1021 : 0);
	}
	return crc;
}

WORD CPole::DleEncoder(BYTE* pSrc, WORD wLength)
{
	BYTE* pDest = &c_cDle[0][2];
	WORD append = 0;
	for (WORD w = 0; w < wLength; w ++) {
		if (*pSrc == 0x10) {
			*pDest ++ = 0x10;
			++ append;
		}
		*pDest ++ = *pSrc ++;
	}
	c_cDle[0][0] = 0x10;
	c_cDle[0][1] = 2;
	*pDest ++ = 0x10;
	*pDest ++ = 3;
	wLength += (append + 4);
	WORD crc = Crc161d0f(c_cDle[0], wLength);
	*pDest ++ = HIGHBYTE(crc);
	*pDest ++ = LOWBYTE(crc);
	return wLength + 2;
}

int CPole::DleDecoder(BYTE* pDest, WORD wLength)
{
	BYTE* pBkup = pDest;	// for calc. crc
	bool bDle = 0;
	int bi = -1;
	WORD crc;
	for (WORD w = 0; w < wLength; w ++) {
		BYTE c = *pDest;
		if (bDle && (c == 2 || c == 3 || c == 0x10)) {
			switch (c) {
			case 2 :
				if (bi < 0)	bi = 0;
				else	return ADFAIL_STX;
				break;
			case 3 :
				if (bi != sizeof(SAATO))	return ADFAIL_SIZE;
				crc = Crc161d0f(pBkup, w + 1);
				if (HIGHBYTE(crc) != *(pDest + 1) || LOWBYTE(crc) != *(pDest + 2))	return ADFAIL_CRC;
				return ADFAIL_NON;
				break;
			default :
				if (bi < 0)	return ADFAIL_WITHOUTSTX;
				c_cDle[1][bi ++] = c;
				break;
			}
			bDle = false;
		}
		else if (!bDle && c == 0x10)	bDle = true;
		else {
			if (bi < 0)	return ADFAIL_WITHOUTSTX;
			c_cDle[1][bi ++] = c;
		}
		++ pDest;
		if (bi >= 256)	return ADFAIL_OVER;
	}
	return ADFAIL_ETX;
}

void CPole::IncreaseRed(int pdi, int errf)
{
	INCWORD(c_wRed[pdi][errf]);
	if (errf > LCF_ALL)	INCWORD(c_wRed[pdi][LCF_ALL]);
	//++ c_cGray[pdi][errf];
}

WORD CPole::CaptureRed(WORD* pBuf)
{
	memcpy(pBuf, &c_wRed, sizeof(WORD) * PDEVID_MAX * LCF_MAX);
	return sizeof(WORD) * PDEVID_MAX * LCF_MAX;
}

BYTE CPole::GetPDIFromCurID(int curi)
{
	return c_cSvcSequence[curi];
}

BYTE CPole::GetCurIDFromPDI(int pdi)
{
	// 200218
	GETPAPER(pPaper);
	int iLimit = pPaper->GetDeviceExFromRecip() ? PDEVEX_LENG : PDEV_LENG;
	for (int n = 0; n < iLimit; n ++) {
		if (c_cSvcSequence[n] == pdi)	return (BYTE)n;
	}
	return 0;
}

BYTE CPole::GetPreviousPDI(int pdi)
{
	// 200218
	GETPAPER(pPaper);
	int curi = (int)GetCurIDFromPDI(pdi);
	if (curi > 0)	-- curi;
	else	curi = pPaper->GetDeviceExFromRecip() ? PDEVEX_LENG - 1 : PDEV_LENG - 1;
	return c_cSvcSequence[curi];
}

void CPole::Timeout(int pdi)
{
	// 200218
	//if (pdi >= PDEVID_PAU && pdi <= PDEVID_TRS)	c_bRecvMon[PDEVID_PAU] = false;
	if (pdi >= PDEVID_PAU && pdi <= PDEVID_BMS)	c_bRecvMon[PDEVID_PAU] = false;
	else	c_bRecvMon[pdi] = false;

	GETVERSE(pVerse);
	if (!pVerse->Repair(pdi, LCF_RTIMEOUT))	IncreaseRed(pdi, LCF_RTIMEOUT);
}

bool CPole::SendPau()
{
	int previ = (int)GetPreviousPDI((int)PDEVID_PAU);
	if (c_bRecvMon[PDEVID_PAU])	Timeout(previ);		// trs -> pau, previous cycle is trs
	GETVERSE(pVerse);
	pVerse->SetCloseSvcID(previ);

	// 180910
	CVerse::PPSHARE pPsv = pVerse->GetShareVars();
	int vi = pVerse->GetValidPisSide();
	if (pPsv->piscf.fl[vi].a.t.s.ref.b.mma && pPsv->piscf.fl[vi].a.t.s.ref.b.sim) {
		c_bRecvMon[PDEVID_PAU] = false;
		return true;
	}
	else {
		PPAURINFO pPaur = (PPAURINFO)pVerse->Decorate(PDEVID_PAU);
		bool bRes = true;
		GETPAPER(pPaper);
		if (pPaper->IsCab() && pPaper->IsMain())
			bRes = c_nd[PDEVID_PAU].pDev->Send((BYTE*)pPaur, SIZE_PAUSR);
		if (!bRes) {
			IncreaseRed(PDEVID_PAU, LCF_SEND);
			TRACK("%s>PAU>ERR:send failure!\n", c_szOwnerName);
			return false;
		}
		else	c_bRecvMon[PDEVID_PAU] = true;
		return bRes;
	}
}

bool CPole::SendPis()
{
	int previ = (int)GetPreviousPDI((int)PDEVID_PIS);
	if (c_bRecvMon[PDEVID_PAU])	Timeout(previ);	// pau -> pis
	GETVERSE(pVerse);
	pVerse->SetCloseSvcID(previ);

	PPISRINFO pPisr = (PPISRINFO)pVerse->Decorate(PDEVID_PIS);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_PAU].pDev->Send((BYTE*)pPisr, SIZE_PISSR);
	if (!bRes) {
		IncreaseRed(PDEVID_PIS, LCF_SEND);
		TRACK("%s>PIS>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[PDEVID_PAU] = true;
	return bRes;
}

bool CPole::SendTrs()
{
	int previ = (int)GetPreviousPDI(PDEVID_TRS);
	if (c_bRecvMon[PDEVID_PAU])	Timeout(previ);		// pis -> trs
	GETVERSE(pVerse);
	pVerse->SetCloseSvcID(previ);

	PTRSRINFO pTrsr = (PTRSRINFO)pVerse->Decorate(PDEVID_TRS);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_PAU].pDev->Send((BYTE*)pTrsr, SIZE_TRSSR);
	if (!bRes) {
		IncreaseRed(PDEVID_TRS, LCF_SEND);
		TRACK("%s>TRS>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[PDEVID_PAU] = true;
	return bRes;
}

bool CPole::SendFdu()
{
	int previ = (int)GetPreviousPDI(PDEVID_FDU);
	if (c_bRecvMon[PDEVID_PAU])	Timeout(previ);
	GETVERSE(pVerse);
	pVerse->SetCloseSvcID(previ);

	PFDURINFO pFdur = (PFDURINFO)pVerse->Decorate(PDEVID_FDU);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_PAU].pDev->Send((BYTE*)pFdur, SIZE_FDUSR);
	if (!bRes) {
		IncreaseRed(PDEVID_FDU, LCF_SEND);
		TRACK("%s>FDU>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[PDEVID_PAU] = true;
	return bRes;
}

bool CPole::SendBms()
{
	int previ = (int)GetPreviousPDI(PDEVID_BMS);
	if (c_bRecvMon[PDEVID_PAU])	Timeout(previ);
	GETVERSE(pVerse);
	pVerse->SetCloseSvcID(previ);

	PBMSRINFO pBmsr = (PBMSRINFO)pVerse->Decorate(PDEVID_BMS);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_PAU].pDev->Send((BYTE*)pBmsr, SIZE_BMSSR);
	if (!bRes) {
		IncreaseRed(PDEVID_BMS, LCF_SEND);
		TRACK("%s>BMS>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[PDEVID_PAU] = true;
	return bRes;
}

bool CPole::SendAto()
{
	if (c_nd[PDEVID_ATO].pTerm == NULL)	return false;

	if (c_bRecvMon[PDEVID_ATO])	Timeout(PDEVID_ATO);
	GETVERSE(pVerse);
	PATORINFO pAtor = (PATORINFO)pVerse->Decorate(PDEVID_ATO);
	WORD wEncLeng = DleEncoder((BYTE*)pAtor, sizeof(SRATO));
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_ATO].pTerm->Send(c_cDle[0], wEncLeng);
	if (!bRes) {
		IncreaseRed(PDEVID_ATO, LCF_SEND);
		TRACK("%s>ATO>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[PDEVID_ATO] = true;

	return true;
}

bool CPole::SendHdu()
{
	if (c_nd[PDEVID_HDU].pTerm == NULL)	return false;

	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	WORD wLength = XWORD(pHdur->wLength);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_HDU].pTerm->Send((BYTE*)pHdur, wLength * 2 + 9);
	if (!bRes) {
		IncreaseRed(PDEVID_HDU, LCF_SEND);
		TRACK("%s>HDU>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	return true;
}

bool CPole::SendHduA()
{
	if (c_nd[PDEVID_HDU].pTerm == NULL)	return false;

	GETVERSE(pVerse);
	PHDURINFO pHdur = (PHDURINFO)pVerse->GetInfo(false, PDEVID_HDU, LI_CURR);
	WORD wLength = XWORD(pHdur->wLength);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab())	// && pPaper->IsMain())
		bRes = c_nd[PDEVID_HDU].pTerm->Send((BYTE*)pHdur, wLength * 2 + 9);
	if (!bRes) {
		IncreaseRed(PDEVID_HDU, LCF_SEND);
		TRACK("%s>HDU>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	return true;
}

bool CPole::SendHdu(BYTE* pBuf, WORD wLength)
{
	if (c_nd[PDEVID_HDU].pTerm == NULL)	return false;

	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab() && pPaper->IsMain())
		bRes = c_nd[PDEVID_HDU].pTerm->Send(pBuf, wLength);
	if (!bRes) {
		IncreaseRed(PDEVID_HDU, LCF_SEND);
		TRACK("%s>HDU>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	return true;
}

bool CPole::SendHduA(BYTE* pBuf, WORD wLength)
{
	if (c_nd[PDEVID_HDU].pTerm == NULL)	return false;

	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab())	// && pPaper->IsMain())
		bRes = c_nd[PDEVID_HDU].pTerm->Send(pBuf, wLength);
	if (!bRes) {
		IncreaseRed(PDEVID_HDU, LCF_SEND);
		TRACK("%s>HDU>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	return true;
}

bool CPole::SendRtd()
{
	if (c_nd[PDEVID_RTD].pDev == NULL)	return false;

	GETVERSE(pVerse);
	if (c_bRecvMon[PDEVID_RTD]) {
		c_bRecvMon[PDEVID_RTD] = false;		// Timeout()을 호출하면 repair를 하기때문에 하지않는다.
		pVerse->RtdClosure(CVerse::RCLSTYPE_TIMEOUT);
	}

	PRTDBINFO pRtdr = (PRTDBINFO)pVerse->Decorate(PDEVID_RTD);
	bool bRes = true;
	GETPAPER(pPaper);
	if (pPaper->IsCab())	// && pPaper->IsMain())	// RTD는 메인이든 보조든 같이 보낸다
		bRes = c_nd[PDEVID_RTD].pDev->Send((BYTE*)pRtdr, SIZE_RTDSB);

	if (!bRes) {
		IncreaseRed(PDEVID_RTD, LCF_SEND);
		TRACK("%s>RTD>ERR:send failure!\n", c_szOwnerName);
		return false;
	}
	else	c_bRecvMon[PDEVID_RTD] = true;
	return true;
}

// PAU -> PIS -> TRS
bool CPole::SendSvc()
{
	if (c_nd[PDEVID_PAU].pDev == NULL)	return false;

	// 200218
	//if (++ c_cSvcCurID > 2)	c_cSvcCurID = 0;
	GETPAPER(pPaper);
	//BYTE cLimit = pPaper->GetDeviceExFromRecip() ? 4 : 2;
	//if (++ c_cSvcCurID > cLimit)	c_cSvcCurID = 0;
	c_cLimit = pPaper->GetDeviceExFromRecip() ? PDEVEX_LENG - 1 : PDEV_LENG - 1;
	if (++ c_cSvcCurID > c_cLimit)	c_cSvcCurID = 0;
	//BYTE id = c_cSvcSequence[c_cSvcCurID];
	BYTE pdi = GetPDIFromCurID((int)c_cSvcCurID);		// 3, 5, 4, 6, 7
	switch (pdi) {
	case PDEVID_PAU :	return SendPau();	break;
	case PDEVID_PIS :	return SendPis();	break;
	// 200218
	//default :			return SendTrs();	break;
	case PDEVID_TRS :	return SendTrs();	break;
	case PDEVID_FDU :	return SendFdu();	break;
	default :			return SendBms();	break;
	}
	return true;
}

bool CPole::Setup()
{
	for (int n = 0; n < REALPDEVCH_MAX; n ++) {
		if (n < PDEVID_RTD) {
			KILLC(c_nd[n].pTerm);
			c_nd[n].pTerm = new CTerm(this, c_hReservoir, c_layer[n].pszName);
			if (!c_nd[n].pTerm->Open(c_layer[n].iCh, c_layer[n].dwBps, c_layer[n].wCycleTime)) {
				TRACK("%s>%s>ERR:intial failure!\n", c_szOwnerName, c_layer[n].pszName);
				delete c_nd[n].pTerm;
				c_nd[n].pTerm = NULL;
				return false;
			}
			//else	TRACK("%s>%s:intial.\n", c_szOwnerName, c_layer[n].init.pszName);
		}
		else {
			KILLC(c_nd[n].pDev);
			c_nd[n].pDev = new CDev(this, c_hReservoir, c_layer[n].pszName);
			// 171124
			//if (!c_nd[n].pDev->Open(c_layer[n].iCh, c_layer[n].wAddr, c_layer[n].dwBps, c_layer[n].wCycleTime)) {
			if (!c_nd[n].pDev->Open(c_layer[n].iCh, 0xffff, c_layer[n].dwBps, c_layer[n].wCycleTime)) {
				TRACK("%s>%s>ERR:intial failure!\n", c_szOwnerName, c_layer[n].pszName);
				delete c_nd[n].pDev;
				c_nd[n].pDev = NULL;
				return false;
			}
			//else	TRACK("%s>%s:intial.\n", c_szOwnerName, c_layer[u].init.pszName);
		}
	}
	c_cSvcCurID = 0xff;
	return true;
}

void CPole::ReceiveRtd(WORD wLeng, BYTE cState, bool bReal)
{
	GETVERSE(pVerse);
	GETPAPER(pPaper);
	CUCUTOFF();
	c_bRecvMon[PDEVID_RTD] = false;
	if (!bReal) {
		IncreaseRed(PDEVID_RTD, LCF_RTIMEOUT);
		c_bRecvMon[PDEVID_RTD] = false;		// Timeout()을 호출하면 repair를 하기때문에 하지않는다.
		pVerse->RtdClosure(CVerse::RCLSTYPE_TIMEOUT);
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>RTD>ERR:timeout!(%d)\n", c_szOwnerName, c_nd[PDEVID_PA].pDev->GetCycleTime());
#endif
		return;
	}

	if ((cState & ~8) != 0) {
		IncreaseRed(PDEVID_RTD, LCF_RCRC);
		pVerse->RtdClosure(CVerse::RCLSTYPE_FAILCRC);
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>RTD>ERR:status!(0x%02X)\n", c_szOwnerName, cState);
#endif
		return;
	}

	if (wLeng != SIZE_RTDSB) {
		IncreaseRed(PDEVID_RTD, LCF_RSIZE);
		pVerse->RtdClosure(CVerse::RCLSTYPE_FAILSIZE);
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>RTD>ERR:size!(%d)\n", c_szOwnerName, wLeng);
#endif
		return;
	}

	PRTDBINFO pRtda = (PRTDBINFO)pVerse->GetInfo(true, PDEVID_RTD, LI_CURR);
	c_mtx.Lock();
	c_nd[PDEVID_RTD].pDev->Bunch((BYTE*)pRtda, wLeng);

	// check addr
	if (pRtda->wAddr != LADD_RTD) {
		IncreaseRed(PDEVID_RTD, LCF_RADDR);
		pVerse->RtdClosure(CVerse::RCLSTYPE_FAILADD);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>RTD>ERR:address!(0x%04X)\n", c_szOwnerName, pRtda->wAddr);
#endif
		return;
	}

	// check control byte
	if (pRtda->cCtrl != DEVCHAR_CTL && pRtda->t.s.cFlow != DEVFLOW_ANS) {
		IncreaseRed(PDEVID_RTD, LCF_RECHO);
		pVerse->RtdClosure(CVerse::RCLSTYPE_FAILFLOW);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>RTD>ERR:ctrl(0x%02X) flow(0x%02X)!\n", c_szOwnerName, pRtda->cCtrl, pRtda->t.s.cFlow);
#endif
		return;
	}

	// check respond
	if (pRtda->t.s.pack.cItem == RTDITEM_CLOSECURRENT || pRtda->t.s.pack.cItem == RTDITEM_SAVELOGBOOK ||
		pRtda->t.s.pack.cItem == RTDITEM_SAVETROUBLE || pRtda->t.s.pack.cItem == RTDITEM_SAVEINSPECT) {		// RTDITEM_SAVEFULLSDASET
		PRTDBINFO pRtdr = (PRTDBINFO)pVerse->GetInfo(false, PDEVID_RTD, LI_CURR);
		if (memcmp(&pRtdr->t.s.pack.pad, &pRtda->t.s.pack.pad, sizeof(RTDPAD))) {
			IncreaseRed(PDEVID_RTD, LCF_RFRAME);
			pVerse->RtdClosure(CVerse::RCLSTYPE_FAILRESPOND);
			c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
			TRACK("%s>RTD>ERR:incorrect respond A!\n", c_szOwnerName);
#endif
			return;
		}
	}
	pVerse->RtdClosure(CVerse::RCLSTYPE_SUCCESS);
	// 181001
	GETARCHIVE(pArch);
	pArch->RtdTroubleSuccess();

	c_mtx.Unlock();
#if	defined(DEBUGPOLE_STATETEXT)
	TRACK("%s>RTD:receive %d bytes\n", c_szOwnerName, wLeng);
#endif

	SendTag(TAG_RECEIVEPOLE, c_pParent, PDEVID_RTD);
}

void CPole::ReceiveSvc(WORD wLeng, BYTE cState, bool bReal)
{
	GETVERSE(pVerse);
	GETPAPER(pPaper);
	CUCUTOFF();

	// 200218
	//if (c_cSvcCurID > 2)	return;		// 보조 TC에서 송신 동기가 맞지 않으면 잘못된 ID가 나타날 수 있다!!!!!
	BYTE cLimit = pPaper->GetDeviceExFromRecip() ? PDEVEX_LENG - 1 : PDEV_LENG - 1;
	if (c_cSvcCurID > cLimit)	return;		// 보조 TC에서 송신 동기가 맞지 않으면 잘못된 ID가 나타날 수 있다!!!!!

	c_mtx.Lock();
	//BYTE pdi = c_cSvcSequence[c_cSvcCurID];
	BYTE pdi = GetPDIFromCurID((int)c_cSvcCurID);		// 3, 5, 4, 6, 7
	c_bRecvMon[PDEVID_PAU] = false;
	//PVOID p = pVerse->GetInfo(true, pdi, LI_CURR);
	//WORD wTakeLength = pVerse->GetLength(true, pdi);

	if (!bReal) {
		if (!pPaper->IsMain()) {
			//IncreaseRed(PDEVID_UKD, LCF_RECHO);			// aux has timeout!
			c_mtx.Unlock();
			TRACK("%s>DEV>ERR!!!!!!!!!!:%s() UNREACHABLE LINE B !!!!!!!!\n", c_szOwnerName, __FUNCTION__);
			return;
		}
		Timeout(pdi);
		c_mtx.Unlock();
		return;
	}

	// check status
	if ((cState & ~8) != 0) {
		if (!pVerse->Repair(pdi, LCF_RCRC))	IncreaseRed(pdi, LCF_RCRC);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>%s>ERR:status!(0x%02X)\n", c_szOwnerName, c_layer[pdi].pszName, cState);
#endif
		return;
	}

	// 171123
	LOCALPREFIX prefix;
	c_nd[PDEVID_PAU].pDev->GetPrefix(&prefix);

	// check control char.
	if (prefix.cCtrl != DEVCHAR_CTL) {
		if (!pVerse->Repair(pdi, LCF_RFRAME))	IncreaseRed(pdi, LCF_RFRAME);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>%s>ERR:control char.!(0x%02X)\n", c_szOwnerName, c_layer[pdi].pszName, prefix.cCtrl);
#endif
		return;
	}

	// check address
	if (prefix.wAddr != c_layer[pdi].wAddr) {
		// 200218
		//if (!pPaper->IsMain() && (prefix.wAddr == LADD_PAU || prefix.wAddr == LADD_PIS || prefix.wAddr == LADD_TRS)) {
		if (!pPaper->IsMain() && (prefix.wAddr == LADD_PAU || prefix.wAddr == LADD_PIS ||
			prefix.wAddr == LADD_TRS || prefix.wAddr == LADD_FDU || prefix.wAddr == LADD_BMS)) {
			switch (prefix.wAddr) {
			case LADD_PAU :	pdi = PDEVID_PAU;	break;
			case LADD_PIS :	pdi = PDEVID_PIS;	break;
			// 200218
			//default :		pdi = PDEVID_TRS;	break;
			case LADD_TRS :	pdi = PDEVID_TRS;	break;
			case LADD_FDU :	pdi = PDEVID_FDU;	break;
			default :		pdi = PDEVID_BMS;	break;
			}
		}
		else {
			if (!pVerse->Repair(pdi, LCF_RADDR))	IncreaseRed(pdi, LCF_RADDR);
			c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
			TRACK("%s>%s>ERR:address!(0x%04X)\n", c_szOwnerName, c_layer[pdi].pszName, prefix.wAddr);
#endif
			return;
		}
	}

	// check flow control
	if (prefix.cFlow != DEVFLOW_ANS) {
		if (!pPaper->IsMain() && prefix.cFlow == DEVFLOW_REQ) {
			//c_cSvcCurID = pdi - PDEVID_PAU;		// expect next receiving...
			c_cSvcCurID = GetCurIDFromPDI(pdi);
			c_mtx.Unlock();
			return;
		}
		if (pVerse->Repair(pdi, LCF_RFRAME))	IncreaseRed(pdi, LCF_RFRAME);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>%s>ERR:flow char.!(0x%02X)\n", c_szOwnerName, c_layer[pdi].pszName, prefix.cFlow);
#endif
		return;
	}
	//c_cSvcCurID = pdi - PDEVID_PAU;		// 171123, for aux. when sending device and receiving device are different
	// 200218
	//c_cSvcCurID = GetCurIDFromPDI(pdi);
	if (!pPaper->IsMain())	c_cSvcCurID = GetCurIDFromPDI(pdi);		// aux synchronizes to main

	PVOID p = pVerse->GetInfo(true, pdi, LI_CURR);
	WORD wTakeLength = pVerse->GetLength(true, pdi);
	c_nd[PDEVID_PAU].pDev->Bunch((BYTE*)p, wTakeLength);

	// check size
	if (wLeng != wTakeLength) {
		if (!pVerse->Repair(pdi, LCF_RSIZE))	IncreaseRed(pdi, LCF_RSIZE);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>%s>ERR:size!(%d)\n", c_szOwnerName, c_layer[pdi].pszName, wLeng);
#endif
		return;
	}

	pVerse->Success(pdi);
	c_cSvcReceivedID = pdi;
	c_mtx.Unlock();
#if	defined(DEBUGPOLE_STATETEXT)
	else	TRACK("%s>%s:receive %d bytes\n", c_szOwnerName, c_layer[pdi].init.pszName, wLeng);
#endif
	SendTag(TAG_RECEIVEPOLE, c_pParent, pdi);	//MAKEDWORD(bReal, pdi));
}

void CPole::TakeReceiveDev(DWORD dw)
{
	TAGUNPAR(dw, wLeng, state, ch);
	bool bReal = wLeng >= 0x8000 ? false : true;

	switch (ch) {
	case PSCCHB_RTD :	ReceiveRtd(wLeng, state, bReal);	break;
	case PSCCHB_SVC :	ReceiveSvc(wLeng, state, bReal);	break;
	default :
		IncreaseRed(PDEVID_UKD, LCF_RSIZE);
		TRACK("%s>DEV>ERR:receive unknown id from ch.%d!\n", c_szOwnerName, ch);
		break;
	}
}

void CPole::ReceiveAto(WORD wLeng, BYTE cState, bool bReal)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	c_mtx.Lock();
	c_bRecvMon[PDEVID_ATO] = false;
	GETVERSE(pVerse);
	PATOAINFO pAtoa = (PATOAINFO)pVerse->GetInfo(true, PDEVID_ATO, LI_CURR);
	if (pAtoa == NULL) {
		c_mtx.Unlock();
		TRACK("%s>TERM>ERR!!!!!!!!!!:%s() UNREACHABLE LINE A !!!!!!!!\n", c_szOwnerName, __FUNCTION__);
		return;
	}
	// check timeout
	if (!bReal) {
		if (!pPaper->IsMain()) {
			IncreaseRed(PDEVID_UKD, LCF_RECHO);			// aux has timeout!
			c_mtx.Unlock();
			TRACK("%s>TERM>ERR!!!!!!!!!!:%s() UNREACHABLE LINE B !!!!!!!!\n", c_szOwnerName, __FUNCTION__);
			return;
		}
		memset(pAtoa, 0, pVerse->GetLength(true, PDEVID_ATO));
		if (!pVerse->Repair(PDEVID_ATO, LCF_RTIMEOUT))	IncreaseRed(PDEVID_ATO, LCF_RTIMEOUT);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>ATO>ERR:timeout!(%d)\n", c_szOwnerName, c_nd[PDEVID_ATO].pTerm->GetCycleTime());
#endif
		return;
	}

	c_nd[PDEVID_ATO].pTerm->Bunch(c_cDle[2], wLeng);
	int res = DleDecoder(c_cDle[2], wLeng);
	if (res != ADFAIL_NON) {
		if (res == ADFAIL_SIZE && pPaper->IsCab() && !pPaper->IsMain()) {
			c_mtx.Unlock();
			return;
		}
		if (!pVerse->Repair(PDEVID_ATO, (BYTE)res))	IncreaseRed(PDEVID_ATO, res);
		c_mtx.Unlock();
#if	defined(DEBUGPOLE_ERRORTEXT)
		TRACK("%s>ATO>ERR:%d!\n", c_szOwnerName, res);
#endif
		return;
	}
	memcpy(pAtoa, c_cDle[1], sizeof(SAATO));
	pAtoa->t.s.cReserve1 = DEVCHAR_CTL;		// ?????
	pVerse->Success(PDEVID_ATO);
	c_mtx.Unlock();
#if	defined(DEBUGPOLE_STATETEXT)
	TRACK("%s>ATO:receive %d bytes.\n", c_szOwnerName, wLeng);
#endif

	SendTag(TAG_RECEIVEPOLE, c_pParent, PDEVID_ATO);	//MAKEDWORD(bReal, PDEVID_ATO));
}

void CPole::ReceiveHdu(WORD wLeng, BYTE cState, bool bReal)
{
	GETPAPER(pPaper);
	CUCUTOFF();

	GETVERSE(pVerse);
	PHDUAINFO pHdu = (PHDUAINFO)pVerse->GetInfo(true, PDEVID_HDU, LI_CURR);
	if (pHdu == NULL) {
		TRACK("%s>TERM>ERR!!!!!!!!!!:%s() UNREACHABLE LINE A !!!!!!!!\n", c_szOwnerName, __FUNCTION__);
		return;
	}
	c_mtx.Lock();
	c_nd[PDEVID_HDU].pTerm->Bunch((BYTE*)pHdu, wLeng);
	c_mtx.Unlock();

	if (wLeng >= 4 && pHdu->cEsc == 0x1b && ((pHdu->cSeq == 'I' && pHdu->cont.inst.cLeng == 1) || pHdu->cSeq == 'A')) {
		//pVerse->SetLineState(PDEVID_HDU, true);
		pVerse->SetHduRecvLength(wLeng);
		BYTE cmd = pHdu->cSeq == 'I' ? pHdu->cont.inst.cCmd : pHdu->cSeq;
		SendTag(TAG_RECEIVEPOLE, c_pParent, MAKEDWORD(PDEVID_HDU, cmd));
		// 180118, for monit
		GETPAPER(pPaper);
		if (pPaper->GetLabFromRecip()) {
			HDUHOARD hhd;
			memcpy(&hhd, pHdu, sizeof(HDUHOARD));
			printf("HDU resp:");
			for (WORD w = 0; w < wLeng; w ++)	printf("%02X , ", hhd.c[w]);
			printf(" - %d\n", wLeng);
		}
	}
}

void CPole::TakeReceiveTerm(DWORD dw)
{
	TAGUNPAR(dw, wLeng, state, ch);
	bool bReal = wLeng >= 0x8000 ? false : true;

	if (ch == PSCCHA_ATO)	ReceiveAto(wLeng, state, bReal);
	else	ReceiveHdu(wLeng, state, bReal);
}

BEGIN_TAGTABLE(CPole, CPump)
	_TAGMAP(TAG_RECEIVEDEV, CPole::TakeReceiveDev)
	_TAGMAP(TAG_RECEIVETERM, CPole::TakeReceiveTerm)
END_TAGTABLE();

ENTRY_CONTAINER(CPole)
	SCOOP(&c_cSvcCurID,			sizeof(BYTE),		"Svid")
	SCOOP(&c_cSvcReceivedID,	sizeof(BYTE),		"")
	SCOOP(&c_cLimit,			sizeof(BYTE),		"")
	//SCOOP(&c_wHduSendTime,		sizeof(WORD),		"HduT")
	//SCOOP(&c_wRed[PDEVID_ATO],	(sizeof(WORD) * LCF_MAX),	"ATOF")
	//SCOOP(&c_wRed[PDEVID_PAU],	(sizeof(WORD) * LCF_MAX),	"PAUF")
	//SCOOP(&c_wRed[PDEVID_PIS],	(sizeof(WORD) * LCF_MAX),	"PISF")
	//SCOOP(&c_wRed[PDEVID_TRS],	(sizeof(WORD) * LCF_MAX),	"TRSF")
EXIT_CONTAINER()
