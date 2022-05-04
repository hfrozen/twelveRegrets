/*
 * CLocal.cpp
 *
 *  Created on: 2010. 11. 16
 *      Author: Che
 */

#include <Draft.h>
#include "CLocal.h"
#include "CTcx.h"

#define	CPU_COM10		0xff

#define	LOCALCH_ATCA	MF_HDLCA
#define	LOCALCH_ATCB	MF_HDLCB
#define	LOCALCH_ATO		MF_HDLCC
#define	LOCALCH_TWC		MF_HDLCD
#define	LOCALCH_DUC		CPU_COM10
#define	LOCALCH_CSC		MF_HDLCE
#define	LOCALCH_INT		MF_HDLCF

CLocal::CLocal()
	: CAnt()
{
	c_pParent = NULL;
	c_mail = -1;
	for (UCURV n = 0; n < LOCALID_SIZE; n ++)
		c_pDev[n] = NULL;
	c_pDuc = NULL;
	c_uInterFlow = -1;
	c_bAlter = TRUE;
	c_pTcRef = NULL;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
	c_wAck = 0;
	memset((PVOID)&c_atcChk, 0, sizeof(ATCCHECK));
	c_prio = 10;
	c_wCycle = 0;
}

CLocal::CLocal(PVOID pParent, int mq)
	: CAnt(pParent, mq, (PSZ)"CLocal")
{
	c_pParent = pParent;
	c_mail = mq;
	for (UCURV n = 0; n < LOCALID_SIZE; n ++)
		c_pDev[n] = NULL;
	c_pDuc = NULL;
	c_uInterFlow = -1;
	c_bAlter = TRUE;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
	c_wAck = 0;
	memset((PVOID)&c_atcChk, 0, sizeof(ATCCHECK));
	c_prio = 10;
	c_wCycle = 0;
}

CLocal::CLocal(PVOID pParent, int mq, PSZ pszOwner)
	: CAnt(pParent, mq, pszOwner)
{
	c_pParent = pParent;
	c_mail = mq;
	for (UCURV n = 0; n < LOCALID_SIZE; n ++)
		c_pDev[n] = NULL;
	c_pDuc = NULL;
	c_uInterFlow = -1;
	c_bAlter = TRUE;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
	c_wAck = 0;
	memset((PVOID)&c_atcChk, 0, sizeof(ATCCHECK));
	c_prio = 10;
	c_wCycle = 0;
}

CLocal::~CLocal()
{
	for (UCURV n = 0; n < LOCALID_SIZE; n ++)
		KILL(c_pDev[n]);
	KILL(c_pDuc);
}

const CLocal::INITARCH CLocal::c_initArch[] = {
	{ LOCALCH_ATCA,	LOCALADD_ATC,	MFMODE_HDLC,	MFBPS_38400,
		sizeof(ATCRINFO),	sizeof(ATCAINFO),	30, (PSZ)"ATCA"},
	{ LOCALCH_ATCB,	LOCALADD_ATC,	MFMODE_HDLC,	MFBPS_38400,
		sizeof(ATCRINFO),	sizeof(ATCAINFO),	30, (PSZ)"ATCB"},
	{ LOCALCH_ATO,	LOCALADD_ATO,	MFMODE_HDLC,	MFBPS_38400,
		sizeof(ATORINFO),	sizeof(ATOAINFO),	30, (PSZ)"ATO" },
	{ LOCALCH_TWC,	LOCALADD_TWC,	MFMODE_HDLC,	MFBPS_38400,
		100,	100,	20, (PSZ)"TWC" },
	{ LOCALCH_DUC,	LOCALADD_DUC,	MFMODE_ASYNC,	MFBPS_115200,
		sizeof(SRDUCINFO),	sizeof(SADUCINFO),	0, (PSZ)"DU" },
	{ LOCALCH_CSC,	LOCALADD_CSC,	MFMODE_ASYNC,	MFBPS_19200,
		sizeof(CMMRINFO),	sizeof(CSCAINFO),	0, (PSZ)"CSC" },
	{ LOCALCH_INT,	LOCALADD_PIS,	MFMODE_ASYNC,	MFBPS_19200,
		sizeof(CMMRINFO),	sizeof(PISAINFO),	0, (PSZ)"PIS" },
	{ LOCALCH_INT,	LOCALADD_TRS,	MFMODE_ASYNC,	MFBPS_19200,
		sizeof(CMMRINFO),	sizeof(TRSAINFO),	0, (PSZ)"TRS" },
	{ LOCALCH_INT,	LOCALADD_HTC,	MFMODE_ASYNC,	MFBPS_19200,
		sizeof(CMMRINFO),	sizeof(HTCAINFO),	0, (PSZ)"HTC" },
	{ LOCALCH_INT,	LOCALADD_PSD,	MFMODE_ASYNC,	MFBPS_19200,
		sizeof(CMMRINFO),	sizeof(PSDAINFO),	0, (PSZ)"PSD" }
};

const TRLENGTH CLocal::c_wDataLengthByType[LOCALID_MAX + 1][DATATYPE_MAX] = {
		{{ SIZE_ATCSR, SIZE_ATCSA },
		 { SIZE_ATCTR, SIZE_ATCTA },
		 { SIZE_ATCSR, SIZE_ATCSA }},
		{{ SIZE_ATCSR, SIZE_ATCSA },
		 { SIZE_ATCTR, SIZE_ATCTA },
		 { SIZE_ATCSR, SIZE_ATCSA }},
		{{ SIZE_ATOSR, SIZE_ATOSA },
		 { SIZE_ATOTR, SIZE_ATOTA },
		 { SIZE_ATOIR, SIZE_ATOIA }},
		{{ 100, 100 },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(SRDUCINFO), sizeof(SADUCINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(CMMRINFO), sizeof(CSCAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(CMMRINFO), sizeof(PISAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(CMMRINFO), sizeof(TRSAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(CMMRINFO), sizeof(HTCAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ sizeof(CMMRINFO), sizeof(PSDAINFO) },
		 { 100, 100 },
		 { 100, 100 }},
		{{ 0, 0 },
		 { 0, 0 },
		 { 0, 0 }}
};


void CLocal::InitialModule()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
	c_pDucInfo = pTcx->GetDucInfo();
	memset((PVOID)&c_aAtcNormal[0], 0, sizeof(ATCAINFO));
	memset((PVOID)&c_aAtcNormal[1], 0, sizeof(ATCAINFO));
	memset((PVOID)&c_aAtcInsp[0], 0, sizeof(ATCAINFO));
	memset((PVOID)&c_aAtcInsp[1], 0, sizeof(ATCAINFO));
	memset((PVOID)&c_aAtcTrace[0], 0, sizeof(ATCAINFO));
	memset((PVOID)&c_aAtcTrace[1], 0, sizeof(ATCAINFO));
	memset((PVOID)&c_aAtoNormal, 0, sizeof(ATOAINFO));
	memset((PVOID)&c_aAtoInsp, 0, sizeof(ATOAINFO));
	memset((PVOID)&c_aAtoTrace, 0, sizeof(ATOAINFO));
	memset((PVOID)&c_aCsc, 0, sizeof(CSCAINFO));
	memset((PVOID)&c_aPis, 0, sizeof(PISAINFO));
	memset((PVOID)&c_aTrs, 0, sizeof(TRSAINFO));
	memset((PVOID)&c_aHtc, 0, sizeof(HTCAINFO));
	memset((PVOID)&c_aPsd, 0, sizeof(PSDAINFO));
}

int CLocal::InitDev()
{
	for (UCURV n = 0; n < LOCALID_SIZE; n ++) {
		MSGLOG("[LOCAL]%s channel.\r\n", c_initArch[n].pszName);
		if (n == LOCALID_DUC) {
			KILL(c_pDev[n]);
			KILL(c_pDuc);
			c_pDuc = new CPart(this, c_mail, c_initArch[n].pszName);
			if (!c_pDuc->VerifyMail()) {
				KILL(c_pDuc);
				MSGLOG("[LOCAL]ERROR:Queue open Failure at %s initialing...\r\n",
										c_initArch[n].pszName);
				return LOCALFAULT_OPENMAIL;
			}
			if (!c_pDuc->Open()) {
				MSGLOG("[LOCAL]ERROR:Open Failure at %s initialing...\r\n",
										c_initArch[n].pszName);
				return LOCALFAULT_OPENDEVICE;
			}
		}
		else {
			KILL(c_pDev[n]);
			c_pDev[n] = new CDevice(this, c_mail, c_initArch[n].pszName);
			c_pDev[n]->SetOwnerName(c_initArch[n].pszName);
			if (!c_pDev[n]->VerifyMail()) {
				KILL(c_pDev[n]);
				MSGLOG("[LOCAL]ERROR:Mail open Failure at %s initialing...\r\n",
									c_initArch[n].pszName);
				return LOCALFAULT_OPENMAIL;
			}
			//c_pDev[n]->SetPrio(++ c_prio);
			UCURV uRecvLength = c_initArch[n].uRecvLength;
			if (n == LOCALID_PIS) {
				for (UCURV m = n; m < LOCALID_MAX; m ++) {
					if (uRecvLength < c_initArch[n].uRecvLength)
						uRecvLength = c_initArch[n].uRecvLength;
				}
			}
			if (!c_pDev[n]->Open(c_initArch[n].uCh,
					c_initArch[n].wAddr, c_initArch[n].uMode, c_initArch[n].uBps,
					0, c_initArch[n].uSendLength, uRecvLength, c_initArch[n].uCycleTime)) {
				MSGLOG("[LOCAL]ERROR:Open failure at %s initializing.\r\n",
									c_initArch[n].pszName);
				return LOCALFAULT_OPENDEVICE;
			}
		}
	}

	c_pTcRef->real.lm.wTimeClose = 0;
	for (UCURV n = 0; n < LOCALID_MAX; n ++) {
		if (c_initArch[n].uCycleTime > 0)
			c_pTcRef->real.lm.wTimeClose |= (1 << n);
	}
	//CheckTimerConnection();
	return LOCALFAULT_NON;
}

/* modify by follow
if (c_pDev[LOCALID_ATC]) {
	PUTLWORD(c_pDoz->tcs[OWN_SPACEA].real.srAtc.wAddr, LOCALADD_ATC);
	c_pDev[LOCALID_ATC]->Send((BYTE*)&c_pDoz->tcs[OWN_SPACEA].real.srAtc, sizeof(SRATCINFO));
}*/
int CLocal::Send50ms()
{
	if (!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return LOCALFAULT_NON;	// send at liu1

	//CheckTimerConnection();
	if (c_wCycle < 30000)	++ c_wCycle;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_wAck &= 0xfff0;
	if ((c_pTcRef->real.lm.wAck & ((1 << LOCALID_ATCA) | (1 << LOCALID_ATCB))) != ((1 << LOCALID_ATCA) | (1 << LOCALID_ATCB))) {
		if (!(c_pTcRef->real.lm.wAck & (1 << LOCALID_ATCA))) {
			memset((PVOID)&c_pLcFirm->aAtcBuff[0], 0, sizeof(ATCAINFO));
			SendMail(MA_RECEIVELOCAL, c_pParent, MAKEDWORD(0, LOCALID_ATCA));
		}
		if (!(c_pTcRef->real.lm.wAck & (1 << LOCALID_ATCB))) {
			memset((PVOID)&c_pLcFirm->aAtcBuff[1], 0, sizeof(ATCAINFO));
			SendMail(MA_RECEIVELOCAL, c_pParent, MAKEDWORD(0, LOCALID_ATCB));
		}
		INCBYTE(c_pTcRef->real.cond.nAtcSkipCount);
	}

	c_pTcRef->real.lm.wAck &= ~((1 << LOCALID_ATCA) | (1 << LOCALID_ATCB) | (1 << LOCALID_ATO) | (1 << LOCALID_TWC));
	pTcx->RecordTime(TTIME_LOCALSEND);
	if (++ c_atcChk.nCycle >= CYCLE_ATCCHECK) {
		c_atcChk.bAlter ^= 1;
		c_atcChk.nCycle = 0;
	}
	c_pLcFirm->rAtc[0].r.sr.wChk =
	c_pLcFirm->rAtc[1].r.sr.wChk = c_atcChk.bAlter ? ATCCHKWORDA : ATCCHKWORDB;
	if (c_pTcRef->real.nLocalDataType[LOCALID_ATO] == DATATYPE_INSPECT) {
		if (c_pTcRef->real.insp.nReqCount == 0)	c_pLcFirm->rAto.r.it.cmd.b.req = FALSE;
		else	-- c_pTcRef->real.insp.nReqCount;
		c_pLcFirm->rAto.r.it.wChk = c_atcChk.bAlter ? ATCCHKWORDA : ATCCHKWORDB;
	}
	else	c_pLcFirm->rAto.r.sr.wChk = c_atcChk.bAlter ? ATCCHKWORDA : ATCCHKWORDB;
	if (c_pDev[LOCALID_ATCA] != NULL) {
		c_pDev[LOCALID_ATCA]->Send((BYTE*)&c_pLcFirm->rAtc[0],
				c_wDataLengthByType[LOCALID_ATCA][c_pTcRef->real.nLocalDataType[LOCALID_ATCA]].t);
	}
	if (c_pDev[LOCALID_ATCB] != NULL) {
		c_pDev[LOCALID_ATCB]->Send((BYTE*)&c_pLcFirm->rAtc[1],
				c_wDataLengthByType[LOCALID_ATCB][c_pTcRef->real.nLocalDataType[LOCALID_ATCB]].t);
	}
	if (c_pDev[LOCALID_ATO] != NULL) {
		c_pDev[LOCALID_ATO]->Send((BYTE*)&c_pLcFirm->rAto,
				c_wDataLengthByType[LOCALID_ATO][c_pTcRef->real.nLocalDataType[LOCALID_ATO]].t);
	}
	pTcx->RecordTime(TTIME_LOCALSENDED);
	return LOCALFAULT_NON;
}

int CLocal::Send100ms()
{
	if (!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return LOCALFAULT_NON;	// send at liu1

	c_wAck &= 0x2f;
	if (++ c_uInterFlow > 3)	c_uInterFlow = 0;

	LWORD addr;
	switch (c_uInterFlow) {
	case 0 :
		CheckTimeClose(LOCALID_PIS, (BYTE*)&c_pLcFirm->aPis, (BYTE*)&c_aPis);
		addr = LOCALADD_PIS;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nCnn =
			c_pTcRef->real.lm.nCode[_SERVICEID(LOCALID_PIS)] = GetCabNo(LOCALID_PIS);
		c_pTcRef->real.lm.wAck &= ~(1 << LOCALID_PIS);
		break;
	case 1 :
		CheckTimeClose(LOCALID_TRS, (BYTE*)&c_pLcFirm->aTrs, (BYTE*)&c_aTrs);
		addr = LOCALADD_TRS;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nCnn =
			c_pTcRef->real.lm.nCode[_SERVICEID(LOCALID_TRS)] = GetCabNo(LOCALID_TRS);
		c_pTcRef->real.lm.wAck &= ~(1 << LOCALID_TRS);
		break;
	case 2 :
		CheckTimeClose(LOCALID_HTC, (BYTE*)&c_pLcFirm->aHtc, (BYTE*)&c_aHtc);
		addr = LOCALADD_HTC;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nCnn =
			c_pTcRef->real.lm.nCode[_SERVICEID(LOCALID_HTC)] = GetCabNo(LOCALID_HTC);
		c_pTcRef->real.lm.wAck &= ~(1 << LOCALID_HTC);
		break;
	default :
		CheckTimeClose(LOCALID_PSD, (BYTE*)&c_pLcFirm->aPsd, (BYTE*)&c_aPsd);
		addr = LOCALADD_PSD;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nCnn =
			c_pTcRef->real.lm.nCode[_SERVICEID(LOCALID_PSD)] = GetCabNo(LOCALID_PSD);
		c_pTcRef->real.lm.wAck &= ~(1 << LOCALID_PSD);
		break;
	}
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.wAddr = GETLWORD(addr);
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.nFlow = LOCALFLOW_NORMAL;
	CalcBcc((BYTE*)&c_pDoz->tcs[OWN_SPACEA].real.rCmm, sizeof(CMMRINFO) - 4);
	if (c_pDev[LOCALID_INT] != NULL) {
		PINITARCH pIa = (PINITARCH)&c_initArch[c_uInterFlow + LOCALID_PIS];
		MFSTATUS status;
		c_pDev[LOCALID_INT]->GetStatus(&status);
		status.hdlc.addr = pIa->wAddr & 0xff;
		BYTE id = c_uInterFlow + LOCALID_PIS;
		status.hdlc.receive_count = c_wDataLengthByType[id][c_pTcRef->real.nLocalDataType[id]].r;
		c_pDev[LOCALID_INT]->SetStatus(&status);
		c_pDev[LOCALID_INT]->Send((BYTE*)&c_pDoz->tcs[OWN_SPACEA].real.rCmm,
				sizeof (CMMRINFO));
	}
	return LOCALFAULT_NON;
}

int CLocal::Send200ms()
{
	if (!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return LOCALFAULT_NON;	// send at liu1

	c_wAck &= ~0x20;
	CheckTimeClose(LOCALID_CSC, (BYTE*)&c_pLcFirm->aCsc, (BYTE*)&c_aCsc);
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.wAddr = GETLWORD(LOCALADD_CSC);
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.nFlow = LOCALFLOW_NORMAL;
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.nCnn =
		c_pTcRef->real.lm.nCode[_SERVICEID(LOCALID_CSC)] = GetCabNo(LOCALID_CSC);
	c_pTcRef->real.lm.wAck &= ~(1 << LOCALID_CSC);
	CalcBcc((BYTE*)&c_pDoz->tcs[OWN_SPACEA].real.rCmm, sizeof(CMMRINFO) - 4);	// exclude STX, ETX, BCC1, BCC2
	if (c_pDev[LOCALID_CSC] != NULL)
		c_pDev[LOCALID_CSC]->Send((BYTE*)&c_pDoz->tcs[OWN_SPACEA].real.rCmm, sizeof(CMMRINFO));
	return LOCALFAULT_NON;
}

int CLocal::SendDu()
{
	if (!(c_pTcRef->real.nObligation & OBLIGATION_MAIN))	return LOCALFAULT_NON;	// send at liu1

	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_pDuc != NULL)
		c_pDuc->Send((BYTE*)pTcx->GetDucSendInfo(), pTcx->GetDucSendLength(), pTcx->GetDucRecvLength());
	return LOCALFAULT_NON;
}

BYTE CLocal::GetCabNo(UCURV id)
{
	if (id >= LOCALID_MAX)	return 0x31;
	switch (id) {
	case LOCALID_ATCA :
	case LOCALID_ATCB :
	case LOCALID_ATO :
	case LOCALID_TWC :	return 0x31;	break;
	case LOCALID_CSC :
		if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x31;
		else	return 0x32;
		break;
	case LOCALID_PIS :
		if (!GETTBIT(CAR_HEAD, TCDIB_PISF) ||
				!(c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali & (1 << LOCALID_PIS))) {
			if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x33;
			else	return 0x34;
		}
		else {
			if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x31;
			else	return 0x32;
		}
		break;
	case LOCALID_TRS :
		if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x31;
		else	return 0x32;
		break;
	case LOCALID_HTC :
		if (!GETTBIT(CAR_HEAD, TCDIB_HTCF) ||
				!(c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali & (1 << LOCALID_HTC))) {
			if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x33;
			else	return 0x34;
		}
		else {
			if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x31;
			else	return 0x32;
		}
		break;
	case LOCALID_PSD :
		if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)	return 0x31;
		else	return 0x32;
		break;
	default :	return 0x31;	break;
	}
	return 0x31;
}
void CLocal::CalcBcc(BYTE* pDest, UCURV uLength)
{
	c_pDev[LOCALID_INT]->CalcBcc(pDest, uLength);
}

UCURV CLocal::CheckBcc(UCURV id)
{
	switch (id) {
	case LOCALID_CSC :	return CheckBcc((BYTE*)&c_pLcFirm->aCsc, sizeof(CSCAINFO));	break;
	case LOCALID_PIS :	return CheckBcc((BYTE*)&c_pLcFirm->aPis, sizeof(PISAINFO));	break;
	case LOCALID_TRS :	return CheckBcc((BYTE*)&c_pLcFirm->aTrs, sizeof(TRSAINFO));	break;
	case LOCALID_HTC :	return CheckBcc((BYTE*)&c_pLcFirm->aHtc, sizeof(HTCAINFO));	break;
	case LOCALID_PSD :	return CheckBcc((BYTE*)&c_pLcFirm->aPsd, sizeof(PSDAINFO));	break;
	default :	break;
	}
	return 0;
}

UCURV CLocal::CheckBcc(BYTE* pDest, UCURV uLength)
{
	return c_pDev[LOCALID_INT]->CheckBcc(pDest, uLength);
}

UCURV CLocal::CheckAtcReply(PATCAINFO paAtc)
{
	if (paAtc == NULL)	return FIRMERROR_ADDR;
	return FIRMERROR_NON;
}

UCURV CLocal::CheckAtoReply(PATOAINFO paAto)
{
	if (paAto == NULL)	return FIRMERROR_ADDR;
	return FIRMERROR_NON;
}

void CLocal::Dumbest(UCURV id)
{
	// Modified 2013/02/05
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return;
	if (id >= LOCALID_MAX)	return;
	CTcx* pTcx = (CTcx*)c_pParent;
	PDUMBDEV pDumb = pTcx->GetDumb();
	WORD* pv = NULL;
	BYTE n = OWN_SPACEA < 2 ? 0 : 1;
	switch (id) {
	case LOCALID_ATCA :
		pv = &pDumb->wAtc[0];
		break;
	case LOCALID_ATCB :
		pv = &pDumb->wAtc[1];
		break;
	case LOCALID_ATO :
	case LOCALID_TWC :
		pv = &pDumb->wAto;
		break;
	case LOCALID_CSC :
		pv = &pDumb->wCsc[n];
		break;
	case LOCALID_PIS :
		pv = &pDumb->wPis[n];
		break;
	case LOCALID_TRS :
		pv = &pDumb->wTrs[n];
		break;
	case LOCALID_HTC :
		pv = &pDumb->wHtc[n];
		break;
	case LOCALID_PSD :
		pv = &pDumb->wPsd[n];
		break;
	default :	pv = NULL;	break;
	}
	if (pv != NULL)	pTcx->CountDumb(pv);
	pTcx->CountEachDevError(CAR_MAX + id + 1);		// from 13
}

void CLocal::CheckTimeClose(UCURV id, BYTE* pDest, BYTE* pBkup)
{
	if (c_pTcRef->real.lm.wExpectation & (1 << id)) {
		UCURV uRLength = c_wDataLengthByType[id][c_pTcRef->real.nLocalDataType[id]].r;
		memset((PVOID)pDest, 0, uRLength);
		BYTE deb = id <= LOCALID_TWC ? DEBOUNCE_LOCALMONITA : DEBOUNCE_LOCALMONITB;
		if (++ c_pTcRef->real.lm.nDeb[id] >= deb) {
			LOCALERROR(*pDest, LOCALERROR_TIMEOUTB);	// by service device
			c_pTcRef->real.lm.nDeb[id] = deb;
			c_pTcRef->real.lm.wResult &= ~(1 << id);
			c_pTcRef->real.lm.wValidate &= ~(1 << id);
		}
		else {
			c_pTcRef->real.lm.wResult |= (1 << id);
			c_pTcRef->real.lm.wValidate |= (1 << id);
			if (pBkup != NULL)	memcpy((PVOID)pDest, (PVOID)pBkup, uRLength);
		}
		Dumbest(id);	// by timeout with no debounce
		SendMail(MA_RECEIVELOCAL, c_pParent, MAKEDWORD(TRUE, id));	// by timeout
	}
	c_pTcRef->real.lm.wExpectation |= (1 << id);
}

void CLocal::CheckTimerConnection()
{
	BOOL title = FALSE;
	int con = c_pDev[LOCALID_ATCA]->GetTimerConnection();
	if (con != -1) {
		MSGLOG("[LOCAL]FAIL:Cycle %d\r\n", c_wCycle);
		MSGLOG("[LOCAL]FAIL:ATCA Connection %d.\r\n", con);
		title = TRUE;
	}
	con = c_pDev[LOCALID_ATCB]->GetTimerConnection();
	if (con != -1) {
		if (!title)	MSGLOG("[LOCAL]FAIL:Cycle %d\r\n", c_wCycle);
		MSGLOG("[LOCAL]FAIL:ATCB Connection %d.\r\n", con);
		title = TRUE;
	}
	con = c_pDev[LOCALID_ATO]->GetTimerConnection();
	if (con != -1) {
		if (!title)	MSGLOG("[LOCAL]FAIL:Cycle %d\r\n", c_wCycle);
		MSGLOG("[LOCAL]FAIL:ATO Connection %d.\r\n", con);
		title = TRUE;
	}
	con = c_pDev[LOCALID_TWC]->GetTimerConnection();
	if (con != -1) {
		if (!title)	MSGLOG("[LOCAL]FAIL:Cycle %d\r\n", c_wCycle);
		MSGLOG("[LOCAL]FAIL:TWC Connection %d.\r\n", con);
		title = TRUE;
	}
}

void CLocal::TraceToNormalABS(UCURV id)
{
	if (id < LOCALID_MAX && c_pTcRef->real.nLocalDataType[id] == DATATYPE_TRACE)
		c_pTcRef->real.nLocalDataType[id] = DATATYPE_NORMAL;
}

BOOL CLocal::ToTrace(UCURV id)
{
	if (id < LOCALID_MAX && c_pTcRef->real.nLocalDataType[id] != DATATYPE_TRACE &&
			(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL)) {
		c_pTcRef->real.nLocalDataType[id] = DATATYPE_TRACE;
		return TRUE;
	}
	return FALSE;
}

void CLocal::AtcAnnals(PATCAINFO paAtc, BOOL bMain)
{
	if (paAtc == NULL)	return;
	// Modified 2013/11/02
	//if (!IsMainWarn())	return;
	if (!IsPermitMain())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	if (bMain) {
		pTcx->Annals(paAtc->d.st.fail.b.tccf, SCENE_ID_HEADCAR, 632);	// main
		pTcx->Annals(paAtc->d.st.fail.b.atcf, SCENE_ID_HEADCAR, 640);	// main
	}
	else {
		pTcx->Annals(paAtc->d.st.fail.b.tccf, SCENE_ID_HEADCAR, 633);	// sub
		pTcx->Annals(paAtc->d.st.fail.b.atcf, SCENE_ID_HEADCAR, 641);	// sub
	}
}

void CLocal::AtcAnnals(PATCAINFO paAtc)
{
	if (paAtc == NULL)	return;
	// Modified 2013/11/02
	//if (!IsMainWarn())	return;
	if (!IsPermitMain())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	//pTcx->Annals(paAtc->d.st.fail.b.adof, SCENE_ID_HEADCAR, 634);
	pTcx->Annals(paAtc->d.st.fail.b.msf, SCENE_ID_HEADCAR, 635);
	pTcx->Annals(paAtc->d.st.fail.b.spff, SCENE_ID_HEADCAR, 636);
	pTcx->Annals(paAtc->d.st.fail.b.dome, SCENE_ID_HEADCAR, 637);
	pTcx->Annals(paAtc->d.st.fail.b.bae, SCENE_ID_HEADCAR, 638);
	pTcx->Annals(paAtc->d.st.fail.b.saar, SCENE_ID_HEADCAR, 639);
	pTcx->Annals(paAtc->d.st.err.b.adcf, SCENE_ID_HEADCAR, 642);
	pTcx->Annals(paAtc->d.st.err.b.aacf, SCENE_ID_HEADCAR, 643);
	pTcx->Annals(paAtc->d.st.err.b.akuf, SCENE_ID_HEADCAR, 644);
	pTcx->Annals(paAtc->d.st.err.b.ncaf, SCENE_ID_HEADCAR, 645);
	pTcx->Annals(paAtc->d.st.err.b.dchf, SCENE_ID_HEADCAR, 646);
	pTcx->Annals(paAtc->d.st.err.b.mce, SCENE_ID_HEADCAR, 647);
	pTcx->Annals(paAtc->d.st.err.b.aduf, SCENE_ID_HEADCAR, 648);
	//pTcx->Annals(paAtc->d.st.pos.b.adcf, SCENE_ID_HEADCAR, 649);
	pTcx->Annals(paAtc->d.st.pos.b.nmto, SCENE_ID_HEADCAR, 684);
	pTcx->Annals(paAtc->d.st.pos.b.msop, SCENE_ID_HEADCAR, 685);
	pTcx->Annals(paAtc->d.st.pos.b.dcop, SCENE_ID_HEADCAR, 686);
	pTcx->Annals(paAtc->d.st.pos.b.mcop, SCENE_ID_HEADCAR, 687);
	pTcx->Annals(paAtc->d.st.sph.b.wkuf, SCENE_ID_HEADCAR, 691);
}

void CLocal::AtoAnnals(PATOAINFO paAto)
{
	if (paAto == NULL)	return;
	// Modified 2013/11/02
	//if (!IsMainWarn())	return;
	if (!IsPermitMain())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(paAto->d.st.pss.b.ipm, SCENE_ID_HEADCAR, 650);
	pTcx->Annals(paAto->d.st.pss.b.upm, SCENE_ID_HEADCAR, 651);
	pTcx->Annals(paAto->d.st.pss.b.utd, SCENE_ID_HEADCAR, 688);
	pTcx->Annals(paAto->d.st.psm.b.anmto, SCENE_ID_HEADCAR, 652);
	pTcx->Annals(paAto->d.st.psm.b.spc2f, SCENE_ID_HEADCAR, 653);
	pTcx->Annals(paAto->d.st.fail.b.itaf, SCENE_ID_HEADCAR, 654);
	pTcx->Annals(paAto->d.st.fail.b.spc1f, SCENE_ID_HEADCAR, 655);
	pTcx->Annals(paAto->d.st.fail.b.pssmf, SCENE_ID_HEADCAR, 656);
	pTcx->Annals(paAto->d.st.fail.b.scef, SCENE_ID_HEADCAR, 657);
	//pTcx->Annals(paAto->d.st.fail.b.atbf, SCENE_ID_HEADCAR, 658);
	pTcx->Annals(paAto->d.st.fail.b.tdbf, SCENE_ID_HEADCAR, 659);
	pTcx->Annals(paAto->d.st.fail.b.orf, SCENE_ID_HEADCAR, 660);
	pTcx->Annals(paAto->d.st.fail.b.srf, SCENE_ID_HEADCAR, 661);
	pTcx->Annals(paAto->d.st.state.b.ator, SCENE_ID_HEADCAR, 662);
	pTcx->Annals(paAto->d.st.state.b.ccf, SCENE_ID_HEADCAR, 663);
	//pTcx->Annals(paAto->d.st.arm.b.itm, SCENE_ID_HEADCAR, 664);
	pTcx->Annals(paAto->d.st.arm.b.aaso, SCENE_ID_HEADCAR, 665);
	pTcx->Annals(paAto->d.st.arm.b.tcf, SCENE_ID_HEADCAR, 666);
	pTcx->Annals(paAto->d.st.arm.b.stsr, SCENE_ID_HEADCAR, 667);
	pTcx->Annals(paAto->d.st.arm.b.hf, SCENE_ID_HEADCAR, 668);
	pTcx->Annals(paAto->d.st.arm.b.atcf2, SCENE_ID_HEADCAR, 669);
	pTcx->Annals(paAto->d.st.arm.b.atcf1, SCENE_ID_HEADCAR, 670);
}

/*	Make this function
vodi CLocal::Receive(BYTE* pDesc, UCURV length)
{
}
*/

// Modified 2013/01/07
PATOAINFO CLocal::GetAto(BOOL type)
{
	if (type)	return &c_aAtoNormal;
	else	return &c_aAtoInsp;
}

void CLocal::MaReceiveDevice(DWORD dwParam)
{
	DRECV dr;
	dr.dw = dwParam;
	BOOL bState = (dr.info.wLength & 0x8000) ? TRUE : FALSE;
	dr.info.wLength &= 0x7fff;

	UCURV id;
	BYTE* pDest;
	BYTE* pBkup = NULL;
	BYTE* pBkupI = NULL;
	BYTE* pBkupT = NULL;
	switch (dr.info.nCh) {
	case LOCALCH_ATCA :
		id = LOCALID_ATCA;
		pDest = (BYTE*)&c_pLcFirm->aAtcBuff[0];
		pBkup = (BYTE*)&c_aAtcNormal[0];
		pBkupI = (BYTE*)&c_aAtcInsp[0];
		pBkupT = (BYTE*)&c_aAtcTrace[0];
		break;
	case LOCALCH_ATCB :
		id = LOCALID_ATCB;
		pDest = (BYTE*)&c_pLcFirm->aAtcBuff[1];
		pBkup = (BYTE*)&c_aAtcNormal[1];
		pBkupI = (BYTE*)&c_aAtcInsp[1];
		pBkupT = (BYTE*)&c_aAtcTrace[1];
		break;
	case LOCALCH_ATO :
		id = LOCALID_ATO;
		pDest = (BYTE*)&c_pLcFirm->aAto;
		pBkup = (BYTE*)&c_aAtoNormal;
		pBkupI = (BYTE*)&c_aAtoInsp;
		pBkupT = (BYTE*)&c_aAtoTrace;
		break;
	case LOCALCH_DUC :
		id = LOCALID_DUC;
		pDest = (BYTE*)&c_pDucInfo->saDuc;
		break;
	case LOCALCH_CSC :
		id = LOCALID_CSC;
		pDest = (BYTE*)&c_pLcFirm->aCsc;
		pBkup = (BYTE*)&c_aCsc;
		break;
	case LOCALCH_INT :
		switch (c_uInterFlow) {
		case 0 :
			id = LOCALID_PIS;
			pDest = (BYTE*)&c_pLcFirm->aPis;
			pBkup = (BYTE*)&c_aPis;
			break;
		case 1 :
			id = LOCALID_TRS;
			pDest = (BYTE*)&c_pLcFirm->aTrs;
			pBkup = (BYTE*)&c_aTrs;
			break;
		case 2 :
			id = LOCALID_HTC;
			pDest = (BYTE*)&c_pLcFirm->aHtc;
			pBkup = (BYTE*)&c_aHtc;
			break;
		default :
			id = LOCALID_PSD;
			pDest = (BYTE*)&c_pLcFirm->aPsd;
			pBkup = (BYTE*)&c_aPsd;
			break;
		}
		break;
	default :
		return;
	}

	BOOL result = FALSE;
	UCURV uRLength = c_wDataLengthByType[id][c_pTcRef->real.nLocalDataType[id]].r;
	if (!bState) {
		memset((PVOID)pDest, 0, uRLength);
			//++ c_pTcRef->real.dwLocalErrorTimeout[id];
		LOCALERROR(*pDest, LOCALERROR_TIMEOUTA);	// by main device
	}
	// Modified 2012/12/03
	else {
		c_wAck |= (1 << id);
		if (dr.info.wLength == uRLength) {
			UCURV pid = (id < LOCALID_PIS) ? id : LOCALID_INT;
			if (dr.info.nState == 0) {	// MFB_SUCCESS
				c_pDev[pid]->Combine(pDest, uRLength);
				// Modified 2012/12/25
				//BYTE adi = id <= LOCALID_TWC ? 0 : 1;
				//LWORD addr;
				//memcpy((PVOID)&addr, (PVOID)(pDest + adi), sizeof(LWORD));
				//if (GETLWORD(addr) == c_initArch[id].wAddr)	result = TRUE;
				//else	LOCALERROR(*pDest, LOCALERROR_ADDR);
				LWORD addr;
				if (id <= LOCALID_TWC) {
					memcpy((PVOID)&addr, (PVOID)pDest, sizeof(LWORD));
					if (GETLWORD(addr) == c_initArch[id].wAddr)	result = TRUE;
					else	LOCALERROR(*pDest, LOCALERROR_ADDR);
				}
				else {
					if (*pDest != LOCALFLOW_STX)	LOCALERROR(*pDest, LOCALERROR_STX);
					else {
						memcpy((PVOID)&addr, (PVOID)(pDest + 1), sizeof(LWORD));
						if (GETLWORD(addr) == c_initArch[id].wAddr) {
							if ((CheckBcc(id)) == 0)	result = TRUE;
							else	LOCALERROR(*pDest, LOCALERROR_BCC);
						}
						else	LOCALERROR(*pDest, LOCALERROR_ADDR);
					}
				}
			}
			else {
				memset((PVOID)pDest, 0, uRLength);
				LOCALERROR(*pDest, LOCALERROR_MFB);
			}
		}
		else {
			memset((PVOID)pDest, 0, uRLength);
			LOCALERROR(*pDest, LOCALERROR_LENGTH);
		}
	}

	c_pTcRef->real.lm.wExpectation &= ~(1 << id);
	if (result) {
		c_pTcRef->real.lm.wResult |= (1 << id);
		c_pTcRef->real.lm.nDeb[id] = 0;
		// Modified 2013/01/07
		//if ((id == LOCALID_ATCA || id == LOCALID_ATCB || id == LOCALID_ATO) &&
		//		*(pDest + 3) == LOCALFLOW_TRACE && pBkupT != NULL)
		//	pBkup = pBkupT;
		if (id <= LOCALID_TWC) {
			if (*(pDest + 3) == LOCALFLOW_TRACE && pBkupT != NULL)
				pBkup = pBkupT;
			else if (*(pDest + 3) == LOCALFLOW_INSPECT && pBkupI != NULL)
				pBkup = pBkupI;
		}
		if (pBkup != NULL)
			memcpy((PVOID)pBkup, (PVOID)pDest, uRLength);
		c_pTcRef->real.lm.wValidate |= (1 << id);
	}
	else {
		BYTE deb = id <= LOCALID_TWC ? DEBOUNCE_LOCALMONITA : DEBOUNCE_LOCALMONITB;
		if (++ c_pTcRef->real.lm.nDeb[id] >= deb) {
			c_pTcRef->real.lm.nDeb[id] = deb;
			c_pTcRef->real.lm.wResult &= ~(1 << id);
			c_pTcRef->real.lm.wValidate &= ~(1 << id);
		}
		else {
			c_pTcRef->real.lm.wResult |= (1 << id);
			c_pTcRef->real.lm.wValidate |= (1 << id);
			if (pBkup != NULL)
				memcpy((PVOID)pDest, (PVOID)pBkup, uRLength);
		}
		Dumbest(id);	// by unmatch frame and timeout with no debounce
	}
	SendMail(MA_RECEIVELOCAL, c_pParent, MAKEDWORD(bState, id));
}

void CLocal::MaReceivePart(DWORD dwParam)
{
	//c_pDuc->Receive(c_nDucBulk);
	WORD w = c_pDuc->Receive(c_nDucBulk);
	printf("[DUCI]");
	for (WORD n = 0; n < w; n ++)
		printf("%X ", c_nDucBulk[n]);
	printf("\n");
	SendMail(MA_RECEIVEDUC, c_pParent, (DWORD)(&c_nDucBulk));
}

BEGIN_MAIL(CLocal, CAnt)
	_MAIL(MA_RECEIVEDEVICE, CLocal::MaReceiveDevice)
	_MAIL(MA_RECEIVEPART, CLocal::MaReceivePart)
END_MAIL();

