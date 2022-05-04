/*
 * CPocket.cpp
 *
 *  Created on: 2010. 12. 23
 *      Author: Che
 */

#include <Draft.h>
#include <Msg.h>

#include "CPocket.h"
#include "CCcx.h"

#define	POCKETCH_SIV	MF_HDLCA
#define	POCKETCH_V3F	MF_HDLCA
#define	POCKETCH_ECU	MF_HDLCB
#define	POCKETCH_DCUL	MF_HDLCC
#define	POCKETCH_DCUR	MF_HDLCD
#define	POCKETCH_CMSB	MF_HDLCE

CPocket::CPocket()
	: CAnt()
{
	c_pParent = NULL;
	c_mail = -1;
	c_wActiveDev = 0;
	c_pDoz = NULL;
	c_paSiv = NULL;
	c_paV3f = NULL;
	c_paEcu = NULL;
	c_paDcul = NULL;
	c_paDcur = NULL;
	c_doorFlow = -1;
	for (UCURV n = 0; n < DEVID_MAX; n ++) {
		c_pDev[n] = NULL;
		c_wPocketDeb[n] = 0;
	}
	memset((PVOID)&c_aSiv, 0, sizeof(SIVAINFO));
	memset((PVOID)&c_aV3f, 0, sizeof(V3FAINFO));
	memset((PVOID)&c_aEcu, 0, sizeof(ECUAINFO));
	memset((PVOID)&c_aCmsb, 0, sizeof(CMSBAINFO));
	memset((PVOID)&c_aDcul, 0, sizeof(DCUAINFO));
	memset((PVOID)&c_aDcur, 0, sizeof(DCUAINFO));
	for (UCURV n = 0; n < 8; n ++) {
		c_wDoorDeb[n] = 0;
		memset((PVOID)&c_aDcua[n], 0, sizeof(DCUAINFO));
	}
}

CPocket::CPocket(PVOID pParent, int mq)
	: CAnt(pParent, mq, (PSZ)"CPocket")
{
	c_pParent = pParent;
	c_mail = mq;
	c_wActiveDev = 0;
	c_pDoz = NULL;
	c_paSiv = NULL;
	c_paV3f = NULL;
	c_paEcu = NULL;
	c_paDcul = NULL;
	c_paDcur = NULL;
	c_doorFlow = -1;
	for (UCURV n = 0; n < DEVID_MAX; n ++) {
		c_pDev[n] = NULL;
		c_wPocketDeb[n] = 0;
	}
	memset((PVOID)&c_aSiv, 0, sizeof(SIVAINFO));
	memset((PVOID)&c_aV3f, 0, sizeof(V3FAINFO));
	memset((PVOID)&c_aEcu, 0, sizeof(ECUAINFO));
	memset((PVOID)&c_aCmsb, 0, sizeof(CMSBAINFO));
	memset((PVOID)&c_aDcul, 0, sizeof(DCUAINFO));
	memset((PVOID)&c_aDcur, 0, sizeof(DCUAINFO));
	for (UCURV n = 0; n < 8; n ++) {
		c_wDoorDeb[n] = 0;
		memset((PVOID)&c_aDcua[n], 0, sizeof(DCUAINFO));
	}
}

CPocket::CPocket(PVOID pParent, int mq, PSZ pszOwner)
	: CAnt(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	c_mail = mq;
	c_wActiveDev = 0;
	c_pDoz = NULL;
	c_paSiv = NULL;
	c_paV3f = NULL;
	c_paEcu = NULL;
	c_paDcul = NULL;
	c_paDcur = NULL;
	c_doorFlow = -1;
	for (UCURV n = 0; n < DEVID_MAX; n ++) {
		c_pDev[n] = NULL;
		c_wPocketDeb[n] = 0;
	}
	memset((PVOID)&c_aSiv, 0, sizeof(SIVAINFO));
	memset((PVOID)&c_aV3f, 0, sizeof(V3FAINFO));
	memset((PVOID)&c_aEcu, 0, sizeof(ECUAINFO));
	memset((PVOID)&c_aCmsb, 0, sizeof(CMSBAINFO));
	memset((PVOID)&c_aDcul, 0, sizeof(DCUAINFO));
	memset((PVOID)&c_aDcur, 0, sizeof(DCUAINFO));
	for (UCURV n = 0; n < 8; n ++) {
		c_wDoorDeb[n] = 0;
		memset((PVOID)&c_aDcua[n], 0, sizeof(DCUAINFO));
	}
}

CPocket::~CPocket()
{
	for (UCURV n = 0; n < DEVID_MAX; n ++)
		KILL(c_pDev[n]);
}

const CPocket::INITARCH CPocket::c_initArch[] = {
		{ POCKETCH_SIV,		DEVADD_SIV,		MFMODE_HDLC,	MFBPS_38400,
				sizeof(SIVRINFO),	sizeof(SIVAINFO),	35,	(PSZ)"SIV" },
		{ POCKETCH_V3F,		DEVADD_V3F,		MFMODE_HDLC,	MFBPS_115200,
				sizeof(V3FRINFO),	sizeof(V3FAINFO),	16,	(PSZ)"V3F" },
		{ POCKETCH_ECU, 	DEVADD_ECU,		MFMODE_HDLC,	MFBPS_38400,
				sizeof(ECURINFO),	sizeof(ECUAINFO),	40,	(PSZ)"ECU" },
		{ POCKETCH_DCUL,	DEVADD_DCU,		MFMODE_ASYNC,	MFBPS_19200,
				sizeof(DCURINFO),	sizeof(DCUAINFO),	50,	(PSZ)"DCUL" },
		{ POCKETCH_DCUR,	DEVADD_DCU,		MFMODE_ASYNC,	MFBPS_19200,
				sizeof(DCURINFO),	sizeof(DCUAINFO),	50,	(PSZ)"DCUR" },
		{ POCKETCH_CMSB,	DEVADD_CMSB,	MFMODE_ASYNC,	MFBPS_19200,
				sizeof(CMSBRINFO),	sizeof(CMSBAINFO),	40,	(PSZ)"CMSB" }
};

const TRLENGTH CPocket::c_wDataLengthByType[DEVID_MAX + 1][DATATYPE_MAX] = {
		{{ sizeof(SIVRINFO), sizeof(SIVAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(V3FRINFO), sizeof(V3FAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(ECURINFO), sizeof(ECUAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(DCURINFO), sizeof(DCUAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(DCURINFO), sizeof(DCUAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(CMSBRINFO), sizeof(CMSBAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ 0, 0 },
		 { 0, 0 },
		 { 0, 0 }}
};

void CPocket::InitialModule()
{
	CCcx* pCcx = (CCcx*)c_pParent;
	c_wActiveDev = pCcx->GetActiveDev();
	c_pDoz = pCcx->GetCcDoz();
	c_paSiv = pCcx->GetaSivInfo();
	c_paV3f = pCcx->GetaV3fInfo();
	c_paEcu = pCcx->GetaEcuInfo();
	c_paDcul = pCcx->GetaDculInfo();
	c_paDcur = pCcx->GetaDcurInfo();
}

int CPocket::InitDev()
{
	for (UCURV n = 0; n < DEVID_MAX; n ++) {
		KILL(c_pDev[n]);
		if (c_wActiveDev & (1 << n)) {
			MSGLOG("[POCK]%s initialize.\r\n", c_initArch[n].pszName);
			c_pDev[n] = new CDevice(this, c_mail, c_initArch[n].pszName);
			if (!c_pDev[n]->VerifyMail()) {
				KILL(c_pDev[n]);
				MSGLOG("[POCK]ERROR:Mail failure at %s initializing.\r\n",
						c_initArch[n].pszName);
				return DEVFAULT_OPENMAIL;
			}
			if (!c_pDev[n]->Open(c_initArch[n].uCh,
					c_initArch[n].wAddr, c_initArch[n].uMode, c_initArch[n].uBps,
					0, c_initArch[n].uSendLength, c_initArch[n].uRecvLength,
					c_initArch[n].uCycleTime)) {
				MSGLOG("[POCK]ERROR:Open failure at %s initializing.\r\n",
						c_initArch[n].pszName);
				return DEVFAULT_OPENDEVICE;
			}
		}
	}
	return DEVFAULT_NON;
}

int CPocket::SendV3f()
{
	CCcx* pCcx = (CCcx*)c_pParent;
	pCcx->RecordTime(CTIME_V3FSEND);
	if (c_wActiveDev & DEVBEEN_V3F) {
		pCcx->LockA();
		c_pDev[DEVID_V3F]->Send((BYTE*)pCcx->GetrV3fInfo(), sizeof(V3FRINFO));
		pCcx->UnlockA();
	}
	pCcx->RecordTime(CTIME_V3FSENDED);
	return DEVFAULT_NON;
}

int CPocket::SendSivEcu()
{
	CCcx* pCcx = (CCcx*)c_pParent;
	pCcx->RecordTime(CTIME_ECUSEND);
	if (c_wActiveDev & DEVBEEN_SIV) {
		pCcx->LockA();
		c_pDev[DEVID_SIV]->Send((BYTE*)pCcx->GetrSivInfo(), pCcx->GetrSivLength());
		pCcx->UnlockA();
	}
	if (c_wActiveDev & DEVBEEN_ECU) {
		pCcx->LockA();
		c_pDev[DEVID_ECU]->Send((BYTE*)pCcx->GetrEcuInfo(), sizeof(ECURINFO));
		pCcx->UnlockA();
	}
	pCcx->RecordTime(CTIME_ECUSENDED);
	return DEVFAULT_NON;
}

int CPocket::SendDcu()
{
	CCcx* pCcx = (CCcx*)c_pParent;
	pCcx->RecordTime(CTIME_DCUSEND);
	if (c_wActiveDev & DEVBEEN_DCU) {
		if (++ c_doorFlow > 3)	c_doorFlow = 0;
		pCcx->SetDoorFlow((BYTE)c_doorFlow);
		pCcx->LockA();
		PDCURINFO prDcu = pCcx->GetrDcuInfo();
		prDcu->nDcua = c_doorFlow + 1;
		c_pDev[DEVID_DCU]->CalcBcc((BYTE*)prDcu, sizeof(DCURINFO) - 4);
		c_pDev[DEVID_DCU]->Send((BYTE*)prDcu, sizeof(DCURINFO));
		prDcu->nDcua = c_doorFlow + 5;
		c_pDev[DEVID_DCU2]->CalcBcc((BYTE*)prDcu, sizeof(DCURINFO) - 4);
		c_pDev[DEVID_DCU2]->Send((BYTE*)prDcu, sizeof(DCURINFO));
		pCcx->UnlockA();
	}
	pCcx->RecordTime(CTIME_DCUSENDED);
	return DEVFAULT_NON;
}

int CPocket::SendCmsb()
{
	CCcx* pCcx = (CCcx*)c_pParent;
	pCcx->RecordTime(CTIME_CMSBSEND);
	if (c_wActiveDev & DEVBEEN_CMSB) {
		pCcx->LockA();
		PCMSBRINFO prCmsb = pCcx->GetrCmsbInfo();
		c_pDev[DEVID_CMSB]->CalcBcc((BYTE*)prCmsb, sizeof(CMSBRINFO) - 4);
		c_pDev[DEVID_CMSB]->Send((BYTE*)prCmsb, sizeof(CMSBRINFO));
		pCcx->UnlockA();
	}
	pCcx->RecordTime(CTIME_CMSBSENDED);
	return DEVFAULT_NON;
}

void CPocket::MaReceiveDevice(DWORD dwParam)
{
	DRECV dr;
	dr.dw = dwParam;
	BOOL bState = (dr.info.wLength & 0x8000) ? TRUE : FALSE;
	dr.info.wLength &= 0x7fff;

	UCURV id;
	BYTE* pDest;
	BYTE* pBkup = NULL;
	UCURV uLength;
	BYTE adi;
	BYTE dni = -1;
	switch (dr.info.nCh) {
	case POCKETCH_SIV :
	//case POCKETCH_V3F :
		if (c_wActiveDev & DEVBEEN_V3F) {
			id = DEVID_V3F;
			pDest = (BYTE*)c_paV3f;
			pBkup = (BYTE*)&c_aV3f;
			adi = 0;
		}
		else if (c_wActiveDev & DEVBEEN_SIV) {
			id = DEVID_SIV;
			pDest = (BYTE*)c_paSiv;
			pBkup = (BYTE*)&c_aSiv;
			adi = 0;
		}
		else	return;
		break;

	case POCKETCH_ECU :
		if (c_wActiveDev & DEVBEEN_ECU) {
			id = DEVID_ECU;
			pDest = (BYTE*)c_paEcu;
			pBkup = (BYTE*)&c_aEcu;
			adi = 0;
		}
		else	return;
		break;

	case POCKETCH_DCUL :
		if (c_wActiveDev & DEVBEEN_DCU) {
			id = DEVID_DCU;
			dni = c_doorFlow;
			pDest = (BYTE*)c_paDcul;
			pBkup = (BYTE*)&c_aDcua[dni];
			adi = 1;
		}
		else	return;
		break;

	case POCKETCH_DCUR :
		if (c_wActiveDev & DEVBEEN_DCU) {
			id = DEVID_DCU2;
			dni = c_doorFlow + 4;
			pDest = (BYTE*)c_paDcur;
			pBkup = (BYTE*)&c_aDcua[dni];
			adi = 1;
		}
		else	return;
		break;

	case POCKETCH_CMSB :
		if (c_wActiveDev & DEVBEEN_CMSB) {
			id = DEVID_CMSB;
			pDest = (BYTE*)&c_pDoz->cc.real.aCmsb;
			pBkup = (BYTE*)&c_aCmsb;
			adi = 1;
		}
		else	return;
		break;

	default :
		return;
	}

	uLength = c_wDataLengthByType[id][c_wDataType[id]].r;
	memset((PVOID)pDest, 0, uLength);
	BOOL result = FALSE;
	if (bState) {
		if (dr.info.nState == 0) {
			c_pDev[id]->Combine(pDest, uLength);
			LWORD addr;
			memcpy((PVOID)&addr, (PVOID)(pDest + adi), sizeof(LWORD));
			// Modified 2012/10/08
			if (adi != 0) {
				if (*pDest == DEVFLOW_STX && GETLWORD(addr) == c_initArch[id].wAddr) {
					if (id == DEVID_DCU || id == DEVID_DCU2) {
						if (c_pDev[DEVID_DCU]->CheckBcc(pDest, sizeof(DCUAINFO)) == 0 &&
								*(pDest + 4) == (dni + 1))	result = TRUE;
					}
					else {
						if (c_pDev[DEVID_CMSB]->CheckBcc(pDest, sizeof(CMSBAINFO)) == 0)
							result = TRUE;
					}
				}
			}
			else {
				if (GETLWORD(addr) == c_initArch[id].wAddr)	result = TRUE;
			}
		}
	}

	WORD* pDeb = (id != DEVID_DCU && id != DEVID_DCU2) ? &c_wPocketDeb[id] : &c_wDoorDeb[dni];
	if (result) {
		*pDeb = 0;
		if (pBkup != NULL)	memcpy((PVOID)pBkup, (PVOID)pDest, uLength);
	}
	else {
		if (++ *pDeb >= DEBOUNCE_POCKETMONIT) {
			*pDeb = DEBOUNCE_POCKETMONIT;
			bState = FALSE;
		}
		else if (pBkup != NULL) {
			memcpy((PVOID)pDest, (PVOID)pBkup, uLength);
			bState = TRUE;
		}
	}
	SendMail(MA_RECEIVEPOCKET, c_pParent, MAKEDWORD(bState, id));
	CCcx* pCcx = (CCcx*)c_pParent;
	pCcx->SetDevDeb(&c_wPocketDeb[0]);
	pCcx->SetDoorDeb(&c_wDoorDeb[0]);
}

BEGIN_MAIL(CPocket, CAnt)
	_MAIL(MA_RECEIVEDEVICE, CPocket::MaReceiveDevice)
END_MAIL();
