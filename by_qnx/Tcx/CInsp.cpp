/*
 * CInsp.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CInsp.h"
#include "CTcx.h"

#define	RAATO	pTcx->GetAto(FALSE, FALSE)
#define	RRATO	c_pLcFirm->rAto

//#define	IsSameItem(n)	(RAATO.nFlow == LOCALFLOW_INSPECT && RAATO.ct.insp.nItem == n)
#define	IsSameItem(n)	(n == n)
#define	IsEnd()			(RAATO->d.ip.nState == ATOINSPSTATE_END)
#define	IsOK()			(RAATO->d.ip.res.b.ok)
#define	IsNG()			(RAATO->d.ip.res.b.ng)
#define	IsResult(n)		(RAATO->d.ip.nSubItem & n)
#define	IsWaitEnd()		(c_pTcRef->real.insp.wTimer == 0)

#define	HEXTODEC(w)		((w >> 8) * 100 + (w & 0xff))

#define	SNDNEXT()\
	do {\
		++ c_pTcRef->real.insp.nAtoCmd;\
		c_nSubStep = c_pTcRef->real.insp.nSubStep + 1;\
		c_pTcRef->real.insp.nSubStep = INSPECTSND_DELAY;\
	} while(0)

#define	NextSubStep()		++ c_pTcRef->real.insp.nSubStep
#define	GotoSubStep(step)	c_pTcRef->real.insp.nSubStep = step
#define	NextAuxStep()		++ c_pTcRef->real.insp.nAuxStep
#define	GotoAuxStep(step)	c_pTcRef->real.insp.nAuxStep = step

#define	SKIP_ECUINSP

#define	BELL()\
	do {\
		pTcx->Buzz(500 / TIME_INTERVAL);\
	} while(0)

CInsp::CInsp()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
	c_bCmd = FALSE;
	c_bPause = FALSE;
	c_bDelay = FALSE;
	c_bWait = FALSE;
}

CInsp::~CInsp()
{
}

const WORD CInsp::c_wAtoPage[2][INSPECTSND_MAX][INSPECTMODE_MAX] = {
		{{ 2221, 2221, 2221, 2221, 2221, 2221 },	// door left
		 { 2221, 2221, 2221, 2221, 2221, 2221 },	// door right
		 { 2222, 2222, 2222, 2222, 2222, 2222 },	// eb
		 { 2222, 2222, 2222, 2222, 2222, 2222 },	// fsb
		 { 2223, 2223, 2223, 2223, 2223, 2223 },	// speed code low
		 { 2224, 2224, 2224, 2224, 2224, 2224 },	// speed code high
		 { 2224, 2224, 2224, 2224, 2224, 2224 },	// sensitivity
		 { 2225, 2225, 2225, 2225, 2225, 2225 },	// over speed low
		 { 2225, 2225, 2225, 2225, 2225, 2225 },	// over speed high
		 { 0,    0,    0,    0,    0,    0 },
		 { 0,    0,    0,    0,    0,    0 }},
		{{ 2231, 2231, 2231, 2231, 2231, 2231 },	// door left
		 { 2231, 2231, 2231, 2231, 2231, 2231 },	// door right
		 { 2232, 2232, 2232, 2232, 2232, 2232 },	// eb
		 { 2232, 2232, 2232, 2232, 2232, 2232 },	// fsb
		 { 2233, 2233, 2233, 2233, 2233, 2233 },	// speed code low
		 { 2234, 2234, 2234, 2234, 2234, 2234 },	// speed code high
		 { 2234, 2234, 2234, 2234, 2234, 2234 },	// sensitivity
		 { 2235, 2235, 2235, 2235, 2235, 2235 },	// over speed low
		 { 2235, 2235, 2235, 2235, 2235, 2235 },	// over speed high
		 { 0,    0,    0,    0,    0,    0 },
		 { 0,    0,    0,    0,    0,    0 }},
};

const WORD	CInsp::c_wBcStep[SIZE_PRESSURELIST][2] = {
		{ 8, 72 }, { 6, 73 }, { 4, 74 }, { 2, 75 }, { 1, 76 },
		{ 0, 75 }, { 1, 74 }, { 2, 73 }, { 4, 72 }, { 6, 71 },
		{ 8, 77 }, { 0, 0 }
};

#define	MIN_BCP		20
#define	MAX_BCP		20

const WORD	CInsp::c_wAsp[2][8] = {
		{ 206, 196, 201, 170, 170, 196, 201, 206 },	// min
		{ 395, 384, 390, 358, 358, 384, 390, 395 }	// max
};

const WORD	CInsp::c_wBcp[10][8] = {
		// Modified 2011/10/17-5)
		{ 128, 109, 115, 115, 115, 109, 115, 128 },	// 0, hb
		{ 275, 262, 263, 255, 255, 262, 263, 275 },	// 1, eb
		{ 218, 204, 196, 196, 191, 204, 207, 218 },	// 2, b7
		{ 195, 180, 183, 184, 174, 192, 183, 195 },	// 3, b6
		{ 176, 160, 162, 164, 164, 160, 162, 176 },	// 4, b5
		{ 151, 135, 136, 138, 138, 135, 136, 151 },	// 5, b4
		{ 128, 111, 113, 114, 114, 111, 113, 128 },	// 6, b3
		{  91,  88,  89,  92,  84,  88,  89, 105 },	// 7, b2
		{  82,  64,  65,  67,  67,  64,  65,  82 },	// 8, b1
		/*{ 128, 109, 115, 115, 115, 109, 115, 128 },	// 0, hb
		{ 275, 262, 263, 255, 255, 262, 263, 275 },	// 1, eb
		{ 218, 204, 207, 209, 209, 204, 207, 218 },	// 2, b7
		{ 195, 180, 183, 184, 184, 180, 183, 195 },	// 3, b6
		{ 176, 160, 162, 164, 164, 160, 162, 176 },	// 4, b5
		{ 151, 135, 136, 138, 138, 135, 136, 151 },	// 5, b4
		{ 128, 111, 113, 114, 114, 111, 113, 128 },	// 6, b3
		{ 105,  88,  89,  92,  92,  88,  89, 105 },	// 7, b2
		{  82,  64,  65,  67,  67,  64,  65,  82 },	// 8, b1*/
		/*{ 112,  85,  86, 104, 104,  85,  86, 112 },	// 0, hb
		{ 276, 231, 235, 246, 246, 231, 235, 276 },	// 1, eb
		{ 234, 193, 196, 210, 210, 193, 196, 234 },	// 2, b7
		{ 208, 170, 173, 187, 187, 170, 173, 208 },	// 3, b6
		{ 187, 151, 153, 169, 169, 151, 153, 187 },	// 4, b5
		{ 159, 127, 128, 145, 145, 127, 128, 159 },	// 5, b4
		{ 134, 104, 106, 123, 123, 104, 106, 134 },	// 6, b3
		{ 109,  82,  83, 101, 101,  82,  83, 109 },	// 7, b2
		{  84,  59,  60,  79,  79,  59,  60,  84 },	// 8, b1*/
		{  47,  27,  27,  47,  47,  27,  27,  47 }	// 9, N
};

const CInsp::INSPECTFLOW	CInsp::c_flow[] = {
		{ 0,               NULL,					{ 0, 0,    0,    0,    0,    0    } },
		{ INSPECTITEM_SND, &CInsp::SignalnDoorItem,	{ 0, 2220, 2220, 2220, 2220, 2220 } },	// 1
		{ INSPECTITEM_SIV, &CInsp::SivItem,			{ 0, 2140, 2140, 2240, 2240, 2240 } },	// 2
		{ INSPECTITEM_EXT, &CInsp::ExtItem,			{ 0, 2140, 2140, 2240, 2240, 2240 } },	// 4
		{ INSPECTITEM_ECU, &CInsp::EcuItem,			{ 0, 2251, 2251, 2251, 2251, 2251 } },	// 8
		{ INSPECTITEM_BCP, &CInsp::BcpItem,			{ 0, 2250, 2250, 2250, 2250, 2250 } },	// 0x10
		{ INSPECTITEM_V3F, &CInsp::V3fItem,			{ 0, 2260, 2260, 2260, 2260, 2260 } },	// 0x20
		{ INSPECTITEM_PWR, &CInsp::PwrItem,			{ 0, 2261, 2261, 2261, 2261, 2261 } },	// 0x40
		{ INSPECTITEM_COOL, &CInsp::CoolItem,		{ 0, 2285, 2285, 2285, 2285, 2285 } },	// 0x80
		{ INSPECTITEM_HEAT, &CInsp::HeatItem,		{ 0, 2286, 2286, 2286, 2286, 2286 } },	// 0x100
		{ INSPECTITEM_CMSB, &CInsp::CmsbItem,		{ 0, 2270, 2270, 2270, 2270, 2270 } },	// 0x200
		{ INSPECTITEM_DOORB, &CInsp::DoorBackupItem,{ 0, 2287, 2287, 2287, 2287, 2287 } },	// 0x400
		{ INSPECTITEM_TCMS, &CInsp::TcmsItem,		{ 0, 2281, 2281, 2281, 2281, 2281 } },	// 0x800
		{ INSPECTITEM_LAMP, &CInsp::LampItem,		{ 0, 2280, 2280, 2280, 2280, 2280 } },	// 0x1000
		{ 0,				NULL,					{ 0, 0,    0,    0,    0,    0    } }
};

const WORD CInsp::c_wSivJudge[2][2] = {
		{ 594, 606 }, { 361, 399 }
};

const WORD CInsp::c_wHangupMsgWithBell[] = {
		71, 72, 73, 74, 75, 76, 77, 78,
		79, 80,
		91, 95, 96, 99, 111, 114, 118, 119,
		0
};

void CInsp::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
	pTcx->TimeRegister(&c_pTcRef->real.insp.wTimer);
	pTcx->TimeRegister(&c_pTcRef->real.insp.wDelay);
	pTcx->TimeRegister(&c_pTcRef->real.insp.wTimeOuter);
}

void CInsp::WaitBegin(WORD sec)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Lock();
	pTcx->InterTrigger(&c_pTcRef->real.insp.wTimer, sec * 100);	// 1000 / TIME_INTERVAL
	pTcx->Unlock();
}

void CInsp::BitCtrl(BOOL bCtrl, WORD wRef, WORD* pTar)
{
	*pTar &= ~wRef;
	if (bCtrl)	*pTar |= (wRef & 0xff);
	else	*pTar |= (wRef & 0xff00);
}

void CInsp::ChgPage(WORD page)
{
	if (c_pTcRef->real.insp.wPage != page) {
		c_pTcRef->real.insp.wPage = page;
		if (c_pTcRef->real.insp.wPage != c_pTcRef->real.insp.wPrevPage) {
			c_pTcRef->real.insp.wPrevPage = c_pTcRef->real.insp.wPage;
			CTcx* pTcx = (CTcx*)c_pParent;
			BELL();
		}
	}
}

void CInsp::HangupWithBell(WORD msg)
{
	BOOL bell = FALSE;
	for (UCURV n = 0; c_wHangupMsgWithBell[n] != 0; n ++) {
		if (msg == c_wHangupMsgWithBell[n]) {
			bell = TRUE;
			break;
		}
	}
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Hangup(msg);
	if (bell)	BELL();
}

BYTE CInsp::WaitRepeatOrSkip()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_bPause) {
		if (c_pTcRef->real.du.nSw == DUKEY_UP || c_pTcRef->real.du.nSw == DUKEY_DOWN) {
			c_bPause = FALSE;
			BYTE sw = c_pTcRef->real.du.nSw == DUKEY_UP ? INSPKEY_REPEAT : INSPKEY_SKIP;
			c_pTcRef->real.du.nSw = 0;
			c_pTcRef->real.du.nTerm &= ~(DUTERM_REPEAT | DUTERM_SKIP);
			pTcx->InterTrigger(&c_pTcRef->real.insp.wTimeOuter, 0);
			return sw;
		}
		else if (c_pTcRef->real.insp.wTimeOuter == 0)	c_pTcRef->real.du.nSw = DUKEY_ESC;
	}
	else {
		c_pTcRef->real.du.nTerm |= DUTERM_REPEAT;
		if (c_pTcRef->real.insp.nMode < INSPECTMODE_EACHDEV ||
				(c_pTcRef->real.insp.nStep == 1 &&
				c_pTcRef->real.insp.nAuxStep < SNDINSPECTSTEP_END))
			c_pTcRef->real.du.nTerm |= DUTERM_SKIP;
		pTcx->InterTrigger(&c_pTcRef->real.insp.wTimeOuter, TIMEOUT_INSPECT);
		c_bPause = TRUE;
	}
	return INSPKEY_NON;
}

WORD CInsp::GetAtoPage()
{
	return c_wAtoPage[c_pTcRef->real.insp.nId][c_pTcRef->real.insp.nSubStep][c_pTcRef->real.insp.nRefMode];
}

void CInsp::AtoRequest(BOOL bCmd, BYTE nBcdCmd, WORD delay)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Lock();
	RRATO.r.it.nItem = nBcdCmd;
	pTcx->InterTrigger(&c_pTcRef->real.insp.wTimer, delay * 100);	// 1000 / TIME_INTERVAL
	c_pTcRef->real.insp.nReqCount = bCmd != 0 ? COUNT_INSPECTREQ : 0;
	RRATO.r.it.cmd.a = 0;
	RRATO.r.it.cmd.b.req = bCmd;
	RRATO.r.it.nSpr3[0] = RRATO.r.it.nSpr3[1] = 0;
	pTcx->Unlock();
}

void CInsp::BitsToOK(BYTE ref, WORD* pW)
{
	for (UCURV n = 0; n < 8; n ++) {
		if (ref & (1 << n)) {
			*pW &= ~(0x100 << n);
			*pW |= (1 << n);
		}
	}
}

WORD CInsp::BitsToNG(WORD w)
{
	WORD res = w;
	for (UCURV n = 0; n < 8; n ++) {
		if ((res & (0x101 << n)) == (0x101 << n))
			res &= ~(1 << n);
	}
	return res;
}

BOOL CInsp::AtoInspect(WORD delay, WORD flag, WORD* pRes)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_bCmd) {
		if (!IsSameItem(pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd)) ||
				IsWaitEnd() || IsEnd()) {
			*pRes &= ~flag;
			if (IsEnd() && IsOK())	*pRes |= (flag & 0xff);
			else	*pRes |= (flag & 0xff00);
			c_bCmd = FALSE;
			return TRUE;
		}
	}
	else {
		AtoRequest(TRUE, pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd), delay);
		*pRes |= flag;
		c_bCmd = TRUE;
	}
	return FALSE;
}

BOOL CInsp::AtoInspect(WORD delay, WORD flag, WORD* pRes, WORD* pSubRes)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_bCmd) {
		if (RAATO->d.ip.nSubItem != (BYTE)c_wBuf[0]) {
			c_wBuf[0] = RAATO->d.ip.nSubItem;
			BitsToOK((BYTE)c_wBuf[0], pSubRes);
		}
		if (!IsSameItem(pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd)) ||
				IsWaitEnd() || IsEnd()) {
			*pRes = *pSubRes = BitsToNG(*pSubRes);
			c_bCmd = FALSE;
			return TRUE;
		}
	}
	else {
		AtoRequest(TRUE, pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd), delay);
		*pRes = *pSubRes = flag;
		c_wBuf[0] = 0;
		c_bCmd = TRUE;
	}
	return FALSE;
}

BOOL CInsp::AtoInspect(WORD delay, WORD* pRes, WORD* pSubRes)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_bCmd) {
		if (!IsSameItem(pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd)) ||
				IsWaitEnd() || (IsEnd()/* && c_wDoorRes == c_wDoorWait*/)) {
			if (IsEnd()) {
				if (IsOK())	*pRes = 0x11;
				else	*pRes = 0x1100;
				*pSubRes = BitsToNG(*pSubRes);
				if (*pSubRes == c_wCcForm)	*pRes |= 2;
				else	*pRes |= 0x200;
			}
			else {
				*pRes = 0x1300;
				*pSubRes = BitsToNG(*pSubRes);
			}
			c_bCmd = FALSE;
			return TRUE;
		}
		else {
			if (IsEnd()) {
				if (IsOK())	*pRes &= ~0x1100;
				else	*pRes &= ~0x11;
			}
			BOOL bHead = c_pTcRef->real.nAddr > 2 ? TRUE : FALSE;	// 0->TC1, 1->TC0
			BOOL bDir = (c_pTcRef->real.insp.nAtoCmd & 1) ? TRUE : FALSE;	// 0->right, 1->left
			BYTE open;
			if (bDir == bHead)	open = 0xf0;
			else	open = 0xf;
			for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				BYTE door = c_pDoz->ccs[n].real.nInput[CCDI_DOOR];
				switch (c_nDoorStep[n]) {
				case 0 :
					if (door == open) {
						c_nDoorStep[n] = 1;
						*pSubRes &= ~(0x101 << (n + HEADCCBYLENGTHA));
					}
					break;
				case 1 :
					if (door == 0) {
						c_nDoorStep[n] = 2;
						*pSubRes &= ~(0x100 << (n + HEADCCBYLENGTHA));
						*pSubRes |= (1 << (n + HEADCCBYLENGTHA));
						c_wDoorRes |= (1 << n);
					}
					break;
				default :
					break;
				}
			}
		}
	}
	else {
		c_wCcForm = c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA;
		AtoRequest(TRUE, pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd), delay);
		*pRes = 0x1313;
		*pSubRes = c_wCcForm | c_wCcForm << 8;
		c_wDoorWait = c_wDoorRes = 0;
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			c_nDoorStep[n] = 0;
			c_wDoorWait |= (1 << n);
		}
		c_bCmd = TRUE;
	}
	return FALSE;
}

BOOL CInsp::SndInspect()
{
	switch (c_pTcRef->real.insp.nSubStep) {
	case INSPECTSND_BEGIN :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(30, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wDlr,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wDl)) {	// 1, 11
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wDlr & 0x300)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_DOORRIGHT :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(30, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wDrr,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wDr)) {	// 2, 12
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wDrr & 0x300)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_EB :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(20, 0x1111, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wBrk)) {	// 3, 13
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wBrk & 0x100)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_FSB :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(20, 0x2222, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wBrk)) {	// 4, 14
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wBrk & 0x200)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_SPEEDCODEL :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(30, 0xffff, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSlr,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSl)) {	// 5, 15
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSlr & 0xff00)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_SPEEDCODEH :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(30, 0xffff, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wShr,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSh)) {	// 6, 16
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wShr & 0xff00)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_SENSITIVITY :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(20, 0x101, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSenser,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSense)) {	// 7, 17
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wSenser & 0x100)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_OVERSPEEDL :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(60, 0xffff, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wOslr,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wOsl)) {	// 8, 18
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wOslr & 0xff00)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_OVERSPEEDH :
		ChgPage(GetAtoPage());
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	SNDNEXT();
		}
		else if (AtoInspect(60, 0x707, &c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wOshr,
				&c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wOsh)) {	// 9, 19
			if (c_spec.di.sig.atc[c_pTcRef->real.insp.nId].wOshr & 0x700)
				WaitRepeatOrSkip();
			else	SNDNEXT();
		}
		break;
	case INSPECTSND_END :
		return TRUE;
		break;
	case INSPECTSND_DELAY :
		if (!Delay(1))	return FALSE;
		GotoSubStep(c_nSubStep);
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::SignalnDoorItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nAuxStep) {
	case SNDINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.sig, 0, sizeof(DUINSPECTSIGNAL));
		c_spec.di.sig.atc[0].wDl = c_spec.di.sig.atc[0].wDr =
		c_spec.di.sig.atc[1].wDl = c_spec.di.sig.atc[1].wDr = 0xffff;
		pTcx->Lock();
		c_pTcRef->real.nLocalDataType[LOCALID_ATO] = DATATYPE_INSPECT;
		pTcx->Unlock();
		// begin inspect
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_pTcRef->real.insp.nId = 0xff;
		c_pTcRef->real.insp.nSubStep = 0xff;
		c_pTcRef->real.insp.nAtoCmd = 0;
		pTcx->Hangup(11);
		NextAuxStep();
		break;
	case SNDINSPECTSTEP_TESTTRS :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	NextAuxStep();
		}
		else if (c_bCmd) {
			if (IsSameItem(0) && !IsWaitEnd() && RAATO->d.ip.nstsf.a != 0x7f)
				return FALSE;
			c_bCmd = FALSE;
			if (RAATO->d.ip.nstsf.a == 0x7e)	c_spec.di.sig.wTrs = 0x77;
			else	c_spec.di.sig.wTrs = 0x7700;
			if (c_spec.di.sig.wTrs & 0x700)	WaitRepeatOrSkip();
			else	NextAuxStep();
		}
		else {
			AtoRequest(FALSE, 0, 3);
			c_spec.di.sig.wTrs = 0x7777;
			c_bCmd = TRUE;
		}
		break;
	case SNDINSPECTSTEP_TESTTRSDELAY :
		if (Delay(1)) {
			c_pTcRef->real.insp.nSubStep = INSPECTSND_BEGIN;
			c_pTcRef->real.insp.nId = 0;
			c_pTcRef->real.insp.nAtoCmd = 1;
			NextAuxStep();
		}
		break;
	case SNDINSPECTSTEP_TESTATCA :
		if (SndInspect()) {
			c_bCmd = FALSE;
			c_pTcRef->real.insp.nSubStep = INSPECTSND_BEGIN;
			c_pTcRef->real.insp.nId = 1;
			c_pTcRef->real.insp.nAtoCmd = 11;
			NextAuxStep();
		}
		break;
	case SNDINSPECTSTEP_TESTATCB :
		if (SndInspect()) {
			c_bCmd = FALSE;
			c_pTcRef->real.insp.nAtoCmd = 21;
			NextAuxStep();
		}
		break;
	case SNDINSPECTSTEP_TESTATO :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP)	NextAuxStep();
		}
		else if (c_bCmd) {
			if (!IsSameItem(pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd)) || IsWaitEnd() || IsEnd()) {
				c_bCmd = FALSE;
				if (IsEnd() && IsOK())	c_spec.di.sig.wAto = 0x11;
				else	c_spec.di.sig.wAto = 0x1100;
				if (c_spec.di.sig.wAto & 0x100)	WaitRepeatOrSkip();
				else	NextAuxStep();
			}
			else	return FALSE;
		}
		else {
			ChgPage(2239);	// ATO & TWC INSPECTION
			AtoRequest(TRUE, pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd), 20);
			c_spec.di.sig.wAto = 0x1111;
			c_bCmd = TRUE;
		}
		break;
	case SNDINSPECTSTEP_TESTATODELAY :
		if (Delay(1)) {
			c_pTcRef->real.insp.nAtoCmd = 31;
			NextAuxStep();
		}
		break;
	case SNDINSPECTSTEP_TESTTWC :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs == INSPKEY_SKIP) {
				pTcx->Hangup(19);
				NextAuxStep();
			}
		}
		else if (c_bCmd) {
			if (!IsSameItem(pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd)) || IsWaitEnd() || IsEnd()) {
				c_bCmd = FALSE;
				c_spec.di.sig.wAto &= ~0x2222;
				if (IsEnd() && IsOK())	c_spec.di.sig.wAto |= 0x22;
				else	c_spec.di.sig.wAto |= 0x2200;
				if (c_spec.di.sig.wAto & 0x200)	WaitRepeatOrSkip();
				else {
					pTcx->Hangup(19);
					NextAuxStep();
				}
			}
			else	return FALSE;
		}
		else {
			AtoRequest(TRUE, pTcx->ToBcd(c_pTcRef->real.insp.nAtoCmd), 20);
			c_spec.di.sig.wAto |= 0x2222;
			c_bCmd = TRUE;
		}
		break;
	case SNDINSPECTSTEP_SHUTOFF :
		if (Delay(1)) {
			pTcx->Lock();
			c_pTcRef->real.nLocalDataType[LOCALID_ATO] = DATATYPE_NORMAL;
			pTcx->Unlock();
			if (c_pTcRef->real.insp.nMode == INSPECTMODE_PDT ||
					c_pTcRef->real.insp.nMode == INSPECTMODE_DAILY ||
					c_pTcRef->real.insp.nMode == INSPECTMODE_MONTHLY)
				return TRUE;
			else	NextAuxStep();
		}
		break;
	case SNDINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nAuxStep = SNDINSPECTSTEP_BEGIN;
			}
		}
		else if (c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)	WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::SivItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case SIVINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.siv, 0, sizeof(DUINSPECTSIV));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_wBuf[0] = c_wBuf[1] = 0;
		c_wBuf[2] = c_pTcRef->real.insp.nMode < INSPECTMODE_MONTHLY ? 21 : 31;
		pTcx->Hangup(c_wBuf[2]);
		c_spec.di.siv.wRes = 0x3333;
		c_spec.di.siv.wStdV = 380;
		c_spec.di.siv.wStdF = 600;
		c_pDoz->tcs[OWN_SPACEA].real.cSiv.wSivsot = _SHIFTV(__SIVA) | _SHIFTV(__SIVB);
		WaitBegin(3);
		NextSubStep();
		break;
	case SIVINSPECTSTEP_WAITTE :
		if (_ISOCC(__SIVA) && c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.inst.b.sivte)	c_wBuf[0] |= 1;
		if (_ISOCC(__SIVB) && c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.inst.b.sivte)	c_wBuf[0] |= 2;
		if (c_wBuf[0] == 3) {
			WaitBegin(3);
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			if (c_wBuf[0] == 0) {
				pTcx->Hangup(c_wBuf[2] + 1);
				c_spec.di.siv.wRes &= 0xff00;
				GotoSubStep(SIVINSPECTSTEP_END);
			}
			else {
				if (c_wBuf[0] == 1) {
					pTcx->Hangup(c_wBuf[2] + 3);
					c_spec.di.siv.wRes &= 0xff0f;
				}
				else {
					pTcx->Hangup(c_wBuf[2] + 2);
					c_spec.di.siv.wRes &= 0xfff0;
				}
				WaitBegin(3);
				NextSubStep();
			}
		}
		break;
	case SIVINSPECTSTEP_WAITTC :
		if ((c_wBuf[0] & 1) && c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.inst.b.sivtc)	c_wBuf[1] |= 1;
		if ((c_wBuf[0] & 2) && c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.inst.b.sivtc)	c_wBuf[1] |= 2;
		if (c_wBuf[0] == c_wBuf[1])	NextSubStep();
		else if (IsWaitEnd()) {
			BYTE n = (c_wBuf[0] << 2) | c_wBuf[1];
			if (n == 13 || n == 14) {
				if (n == 13) {
					pTcx->Hangup(c_wBuf[2] + 3);
					c_spec.di.siv.wRes &= 0xff0f;
				}
				else {
					pTcx->Hangup(c_wBuf[2] + 2);
					c_spec.di.siv.wRes &= 0xfff0;
				}
				NextSubStep();
			}
			else {
				pTcx->Hangup(c_wBuf[2] + 1);
				c_spec.di.siv.wRes &= 0xff00;
				GotoSubStep(SIVINSPECTSTEP_END);
			}
		}
		break;
	case SIVINSPECTSTEP_RESULT :
		if (c_wBuf[1] & 1) {
			if (c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.inst.b.trof)
				c_spec.di.siv.item[0].f = pTcx->Conv(c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.nOf, 0, 250, (WORD)0, (WORD)1250);
			if (c_spec.di.siv.item[0].f >= c_wSivJudge[0][0] && c_spec.di.siv.item[0].f <= c_wSivJudge[0][1])
					c_spec.di.siv.wRes &= ~0x200;
			else	c_spec.di.siv.wRes &= ~2;
			if (c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.inst.b.treo)
				c_spec.di.siv.item[0].v = pTcx->Conv(c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.nEo, 0, 250, (WORD)0, (WORD)750);
			if (c_spec.di.siv.item[0].v >= c_wSivJudge[1][0] && c_spec.di.siv.item[0].v <= c_wSivJudge[1][1])
				c_spec.di.siv.wRes &= ~0x100;
			else	c_spec.di.siv.wRes &= ~1;
			c_spec.di.siv.item[0].i = pTcx->Conv(c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.nEd, 0, 250, (WORD)0, (WORD)3000);
			c_spec.di.siv.item[0].c = pTcx->Conv(c_pDoz->ccs[__SIVA].real.eSiv.d.st.st.nIs, 0, 250, (WORD)0, (WORD)750);
		}
		if (c_wBuf[1] & 2) {
			if (c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.inst.b.trof)
				c_spec.di.siv.item[1].f = pTcx->Conv(c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.nOf, 0, 250, (WORD)0, (WORD)1250);
			if (c_spec.di.siv.item[1].f >= c_wSivJudge[0][0] && c_spec.di.siv.item[1].f <= c_wSivJudge[0][1])
				c_spec.di.siv.wRes &= ~0x2000;
			else	c_spec.di.siv.wRes &= ~0x20;
			if (c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.inst.b.treo)
				c_spec.di.siv.item[1].v = pTcx->Conv(c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.nEo, 0, 250, (WORD)0, (WORD)750);
			if (c_spec.di.siv.item[1].v >= c_wSivJudge[1][0] && c_spec.di.siv.item[1].v <= c_wSivJudge[1][1])
				c_spec.di.siv.wRes &= ~0x1000;
			else	c_spec.di.siv.wRes &= ~0x10;
			c_spec.di.siv.item[1].i = pTcx->Conv(c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.nEd, 0, 250, (WORD)0, (WORD)3000);
			c_spec.di.siv.item[1].c = pTcx->Conv(c_pDoz->ccs[__SIVB].real.eSiv.d.st.st.nIs, 0, 250, (WORD)0, (WORD)750);
		}
		pTcx->Hangup(c_wBuf[2] + 8);
		NextSubStep();
		break;
	case SIVINSPECTSTEP_END :
		c_pDoz->tcs[OWN_SPACEA].real.cSiv.wSivsot = 0;
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = SIVINSPECTSTEP_BEGIN;
			}
		}
		//else if ((c_spec.di.siv.wRes & 0x3300) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)	WaitRepeatOrSkip();
		// cause at inspectmode_eachdev continue next item
		else if (c_spec.di.siv.wRes & 0x3300)	WaitRepeatOrSkip();
		else if (Delay(4))	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::ExtItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case EXTINSPECTSTEP_BEGIN :
		if (c_spec.di.siv.wRes != 0x33)	return TRUE;
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_spec.di.siv.wEsk = 0;
		c_spec.di.siv.wExtRes = 0x303;
		pTcx->Hangup(40);
		WaitBegin(3);
		NextSubStep();
		break;
	case EXTINSPECTSTEP_WAITMSG :
		if (IsWaitEnd()) {
			if (c_spec.di.siv.dwState == (TD_DON | (TD_DON << 7))) {
				pTcx->Hangup(41);
				c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = _SHIFTV(__SIVB);
				WaitBegin(5);
				NextSubStep();
			}
			else {
				pTcx->Hangup(59);	//51);
				c_spec.di.siv.wExtRes &= ~3;
				GotoSubStep(EXTINSPECTSTEP_RESETALL);
			}
		}
		break;
	case EXTINSPECTSTEP_WAITSIVK0 :
		if (!GETCBIT(__SIVB, CCDIB_SIVK)) {
			pTcx->Hangup(42);
			c_spec.di.siv.dwState = TD_DON;	// off sivk0
			WaitBegin(2);
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			pTcx->Hangup(45);		//42);
			c_spec.di.siv.wExtRes &= ~2;
			GotoSubStep(EXTINSPECTSTEP_RESETALL);
		}
		break;
	case EXTINSPECTSTEP_ESKONATSIV0 :
		if (IsWaitEnd()) {
			pTcx->Hangup(43);
			SETOBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
			WaitBegin(5);
			NextSubStep();
		}
		break;
	case EXTINSPECTSTEP_WAITESKC0 :
		if (GETCBIT(__ESK, CCDIB_ESKC)) {
			pTcx->Hangup(44);
			c_spec.di.siv.wEsk = 1;
			c_spec.di.siv.wExtRes &= ~0x200;
			WaitBegin(3);
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			pTcx->Hangup(46);		//43);
			c_spec.di.siv.wExtRes &= ~2;
			GotoSubStep(EXTINSPECTSTEP_ESKABSOFF);
		}
		break;
	case EXTINSPECTSTEP_OKSIV0 :
		if (IsWaitEnd())	NextSubStep();
		break;
	case EXTINSPECTSTEP_RESETESK0 :
		CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
		WaitBegin(5);
		NextSubStep();
		break;
	case EXTINSPECTSTEP_WAITRESETESK0 :
		if (!GETCBIT(__ESK, CCDIB_ESKC)) {
			c_spec.di.siv.wEsk = 0;
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			pTcx->Hangup(47);
			GotoSubStep(EXTINSPECTSTEP_RESETALL);
		}
		break;
	case EXTINSPECTSTEP_RESETSIVK0 :
		c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = 0;
		c_spec.di.siv.dwState = (TD_DON | (TD_DON << 7));
		WaitBegin(2);
		NextSubStep();
		break;
	case EXTINSPECTSTEP_WAITRESETSIVK0 :
		if (IsWaitEnd()) {
			if (c_spec.di.siv.dwState == (TD_DON | (TD_DON << 7))) {
				pTcx->Hangup(49);
				c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = _SHIFTV(__SIVA);
				WaitBegin(5);
				NextSubStep();
			}
			else {
				pTcx->Hangup(59);
				c_spec.di.siv.wExtRes &= ~1;
				GotoSubStep(EXTINSPECTSTEP_RESETALL);
			}
		}
		break;
	case EXTINSPECTSTEP_WAITSIVK1 :
		if (!GETCBIT(__SIVA, CCDIB_SIVK)) {
			pTcx->Hangup(50);
			c_spec.di.siv.dwState = (TD_DON << 7);
			WaitBegin(2);
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			pTcx->Hangup(53);
			c_spec.di.siv.wExtRes &= ~1;
			GotoSubStep(EXTINSPECTSTEP_RESETALL);
		}
		break;
	case EXTINSPECTSTEP_ESKONATSIV1 :
		if (IsWaitEnd()) {
			pTcx->Hangup(51);
			SETOBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
			WaitBegin(5);
			NextSubStep();
		}
		break;
	case EXTINSPECTSTEP_WAITESKC1 :
		if (GETCBIT(__ESK, CCDIB_ESKC)) {
			c_spec.di.siv.wEsk = 1;
			pTcx->Hangup(52);
			c_spec.di.siv.wExtRes &= ~0x100;
			WaitBegin(3);
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			pTcx->Hangup(54);
			c_spec.di.siv.wExtRes &= ~1;
			GotoSubStep(EXTINSPECTSTEP_ESKABSOFF);
		}
		break;
	case EXTINSPECTSTEP_OKSIV1 :
		if (IsWaitEnd())	NextSubStep();
		break;
	case EXTINSPECTSTEP_RESETESK1 :
		CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
		WaitBegin(5);
		NextSubStep();
		break;
	case EXTINSPECTSTEP_WAITRESETESK1 :
		if (!GETCBIT(__ESK, CCDIB_ESKC)) {
			pTcx->Hangup(60);
			c_spec.di.siv.wEsk = 0;
			GotoSubStep(EXTINSPECTSTEP_RESETALL);
		}
		else if (IsWaitEnd()) {
			pTcx->Hangup(55);
			GotoSubStep(EXTINSPECTSTEP_RESETALL);
		}
		break;
	case EXTINSPECTSTEP_ESKABSOFF :
		CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
		WaitBegin(3);
		NextSubStep();
		break;
	case EXTINSPECTSTEP_ESKABSOFFWAIT :
		if (IsWaitEnd())	NextSubStep();
		break;
	case EXTINSPECTSTEP_RESETALL :
		c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = 0;
		c_spec.di.siv.dwState = (TD_DON | (TD_DON << 7));
		WaitBegin(2);
		NextSubStep();
		break;
	case EXTINSPECTSTEP_WAITRESETALL :
		if (IsWaitEnd()) {
			if (c_bPause) {
				BYTE rs = WaitRepeatOrSkip();
				if (rs != INSPKEY_NON) {
					if (rs == INSPKEY_SKIP)	return TRUE;
					c_pTcRef->real.insp.nSubStep = EXTINSPECTSTEP_BEGIN;
				}
			}
			else if ((c_spec.di.siv.wExtRes & 0x300) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
				WaitRepeatOrSkip();
			else	return TRUE;
		}
		break;
	default :
		break;
	}

	return FALSE;
}

void CInsp::EcuResult(BYTE nCid, BYTE nRes, BYTE kind)
{
	PDUINSPECTECUITEM pEcu;
	switch (kind) {
	case 0 :
		pEcu = &c_spec.di.ecu.as;
#ifdef	SKIP_ECUINSP
		nRes = 3;
#endif
		break;
	case 1 :	pEcu = &c_spec.di.ecu.svc;	break;
	default :	pEcu = &c_spec.di.ecu.eb;	break;
	}
	BYTE sh = nCid + HEADCCBYLENGTHA;
	BitCtrl(nRes & 8 ? TRUE : FALSE, 0x101 << sh, &pEcu->w);
	BitCtrl(nRes & 4 ? TRUE : FALSE, 0x101 << sh, &pEcu->req);
	BitCtrl(nRes & 2 ? TRUE : FALSE, 0x101 << sh, &pEcu->as);
	BitCtrl(nRes & 1 ? TRUE : FALSE, 0x101 << sh, &pEcu->bc);
}

BOOL CInsp::EcuInspect(BYTE nKind)
{
	BYTE n;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nAuxStep) {
	case INSPECTECU_BEGIN :
		pTcx->Hangup(61);
		c_wBuf[2] = c_wBuf[3] = 0;
		WaitBegin(1);
		NextAuxStep();
		break;
	case INSPECTECU_DELAY :
		if (IsWaitEnd()) {
			switch (nKind) {	// with rqt
			case 0 :	n = 0x00;	break;	// anti skid
			case 1 :	n = 0x38;	break;	// fsb
			default :	n = 0x58;	break;	// eb
			}
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.set.a = n;
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.wRqt = 0;
			for (BYTE m = 0; m < c_pTcRef->real.cf.nLength; m ++)
				c_pDoz->tcs[OWN_SPACEA].real.cEcu.wRqt |= (1 << m);
			WaitBegin(7);	// +1
			NextAuxStep();
		}
		break;
	case INSPECTECU_WAITOOT :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (c_pDoz->ccs[n].real.aEcu.d.st.cm.set.b.oot)
				c_wBuf[2] |= (1 << (n + HEADCCBYLENGTHA));
		}
		if (c_wBuf[2] == c_wBuf[1]) {
			WaitBegin(7);	// +1
			NextAuxStep();
		}
		else if (IsWaitEnd()) {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				if ((c_wBuf[1] & (1 << (n + HEADCCBYLENGTHA))) &&
						!(c_wBuf[2] & (1 << (n + HEADCCBYLENGTHA))))
					EcuResult(n, 0, nKind);
			}
			if (c_wBuf[2] == 0) {
				pTcx->Hangup(62);
				GotoAuxStep(INSPECTECU_JUDGE);
			}
			else	NextAuxStep();
		}
		break;
	case INSPECTECU_WAITTC :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (c_pDoz->ccs[n].real.aEcu.d.st.cm.set.b.tc)
				c_wBuf[3] |= (1 << (n + HEADCCBYLENGTHA));
		}
		if (c_wBuf[3] == c_wBuf[2])	NextAuxStep();
		else if (IsWaitEnd()) {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				if ((c_wBuf[2] & (1 << (n + HEADCCBYLENGTHA))) &&
						!(c_wBuf[3] & (1 << (n + HEADCCBYLENGTHA))))
					EcuResult(n, 0, nKind);
			}
			if (c_wBuf[3] == 0) {
				pTcx->Hangup(62);
				GotoAuxStep(INSPECTECU_JUDGE);
			}
			else	NextAuxStep();
		}
		break;
	case INSPECTECU_RESULT :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (c_wBuf[3] & (1 << (n + HEADCCBYLENGTHA))) {
				c_wBuf[2] = c_pDoz->ccs[n].real.aEcu.d.st.nTestResult;
#ifdef	SKIP_ECUINSP
				if (nKind != 0)	c_wBuf[2] |= 0x80;
				else	c_wBuf[2] |= 0x30;
#endif
				EcuResult(n, c_wBuf[2] >> 4, nKind);
			}
		}
		NextAuxStep();
		break;
	case INSPECTECU_JUDGE :
		if (pTcx->Hangdown() != 62)	pTcx->Hangup(63);
		if (nKind == 0) {
			c_spec.di.ecu.wAsRes = 0;
			if (c_spec.di.ecu.as.bc & 0xff00)	c_spec.di.ecu.wAsRes |= 0x100;
			else	c_spec.di.ecu.wAsRes |= 1;
			if (c_spec.di.ecu.as.as & 0xff00)	c_spec.di.ecu.wAsRes |= 0x200;
			else	c_spec.di.ecu.wAsRes |= 2;
		}
		else if (nKind == 1) {
			c_spec.di.ecu.wRes &= 0xf0f0;
			if (c_spec.di.ecu.svc.w & 0xff00)	c_spec.di.ecu.wRes |= 0x100;
			else	c_spec.di.ecu.wRes |= 1;
			if (c_spec.di.ecu.svc.req & 0xff00)	c_spec.di.ecu.wRes |= 0x200;
			else	c_spec.di.ecu.wRes |= 2;
			if (c_spec.di.ecu.svc.as & 0xff00)	c_spec.di.ecu.wRes |= 0x400;
			else	c_spec.di.ecu.wRes |= 4;
			if (c_spec.di.ecu.svc.bc & 0xff00)	c_spec.di.ecu.wRes |= 0x800;
			else	c_spec.di.ecu.wRes |= 8;
		}
		else {
			c_spec.di.ecu.wRes &= 0xf0f;
			if (c_spec.di.ecu.eb.w & 0xff00)	c_spec.di.ecu.wRes |= 0x1000;
			else	c_spec.di.ecu.wRes |= 0x10;
			if (c_spec.di.ecu.eb.req & 0xff00)	c_spec.di.ecu.wRes |= 0x2000;
			else	c_spec.di.ecu.wRes |= 0x20;
			if (c_spec.di.ecu.eb.as & 0xff00)	c_spec.di.ecu.wRes |= 0x4000;
			else	c_spec.di.ecu.wRes |= 0x40;
			if (c_spec.di.ecu.eb.bc & 0xff00)	c_spec.di.ecu.wRes |= 0x8000;
			else	c_spec.di.ecu.wRes |= 0x80;
		}
		WaitBegin(3);
		NextAuxStep();
		break;
	case INSPECTECU_END :
		c_pDoz->tcs[OWN_SPACEA].real.cEcu.set.a = 0;
		c_pDoz->tcs[OWN_SPACEA].real.cEcu.wRqt = 0;
		if (IsWaitEnd())	return TRUE;
		break;
	default :
		break;
	}

	return FALSE;
}

BOOL CInsp::EcuItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case ECUINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.ecu, 0, sizeof(DUINSPECTECU));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_spec.di.ecu.wRes = 0xffff;
		c_spec.di.ecu.wAsRes = 0x0303;
		c_spec.di.ecu.svc.w = c_spec.di.ecu.svc.req = c_spec.di.ecu.svc.as = c_spec.di.ecu.svc.bc =
			c_spec.di.ecu.eb.w = c_spec.di.ecu.eb.req = c_spec.di.ecu.eb.as = c_spec.di.ecu.eb.bc =
				c_spec.di.ecu.as.as = c_spec.di.ecu.as.bc =
				(c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA) |
					(c_pTcRef->real.cid.wAll << (HEADCCBYLENGTHA + 8));
		c_wBuf[1] = (BYTE)(c_spec.di.ecu.wRes & 0xff);
		pTcx->Hangup(71);
		WaitBegin(60);
		NextSubStep();
		// break;
	case ECUINSPECTSTEP_WAITNPOS :
		if (c_spec.di.bc.wMcp == MCDSV_NPOS) {
			//c_pDoz->tcs[OWN_SPACEA].real.rEcu.d.st.set.b.rqt = TRUE;
			WaitBegin(10);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(ECUINSPECTSTEP_SHUTOFF);
		break;
	case ECUINSPECTSTEP_WAITC :
		if (GETTBIT(OWN_SPACEA, TCDIB_C)) {
			c_pTcRef->real.insp.nAuxStep = INSPECTECU_BEGIN;
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(ECUINSPECTSTEP_SHUTOFF);
		break;
	case ECUINSPECTSTEP_SERVICE :
		if (EcuInspect(1)) {
			c_pTcRef->real.insp.nAuxStep = INSPECTECU_BEGIN;
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(ECUINSPECTSTEP_SHUTOFF);
		break;
	case ECUINSPECTSTEP_EMER :
		if (EcuInspect(2)) {
			c_pTcRef->real.insp.nAuxStep = INSPECTECU_BEGIN;
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(ECUINSPECTSTEP_SHUTOFF);
		break;
	case ECUINSPECTSTEP_ANTI :
		if (EcuInspect(0)) {
			c_pTcRef->real.insp.nAuxStep = INSPECTECU_BEGIN;
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	NextSubStep();
		break;
	case ECUINSPECTSTEP_SHUTOFF :
		c_pDoz->tcs[OWN_SPACEA].real.cEcu.set.a = 0;
		c_pDoz->tcs[OWN_SPACEA].real.cEcu.wRqt = 0;
		pTcx->Hangup(75);
		NextSubStep();
		break;
	case ECUINSPECTSTEP_WAITB7POS :
		if (c_spec.di.bc.wMcp == MCDSV_B7POS)	NextSubStep();
		break;
	case ECUINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = ECUINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.ecu.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}

	return FALSE;
}

void CInsp::JudgePressure(UCURV id, WORD pos)
{
	ReadPressure(TRUE);
	if (id < SIZE_PRESSURELIST) {
		for (BYTE n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			//WORD as = c_spec.di.bc.wAsp[n + HEADCCBYLENGTHA];
			WORD bc = c_spec.di.bc.wBcp[n + HEADCCBYLENGTHA];
			c_spec.di.bc.wRes &= ~(0x101 << (n + HEADCCBYLENGTHA));
			WORD bcmin, bcmax;
			if (pos < 9) {
				// Modified 2011/101/17-5)
				double tol = (double)c_wBcp[pos][n] * 0.25f;	//0.1f;
				double db;
				if ((double)c_wBcp[pos][n] > tol)	db = (double)c_wBcp[pos][n] - tol;
				else	db = 0.f;
				bcmin = (WORD)db;
				db = (double)c_wBcp[pos][n] + tol;
				bcmax = (WORD)db;
			}
			else {
				bcmin = 0;
				bcmax = 10;
			}
			if (bc >= bcmin && bc <= bcmax)
				c_spec.di.bc.wRes |= (1 << (n + HEADCCBYLENGTHA));
			else	c_spec.di.bc.wRes |= (0x100 << (n + HEADCCBYLENGTHA));
		}
	}
	BottleBcp(id, TRUE);
}

void CInsp::ReadPressure(BOOL bCmd)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	for (BYTE n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if (bCmd) {
			c_spec.di.bc.wAsp[n + HEADCCBYLENGTHA] = (WORD)(pTcx->GetRealAsp(n) * 100.f);
			c_spec.di.bc.wBcp[n + HEADCCBYLENGTHA] = (WORD)(pTcx->GetRealBcp(n) * 100.f);
		}
		else	c_spec.di.bc.wAsp[n + HEADCCBYLENGTHA] = c_spec.di.bc.wBcp[n + HEADCCBYLENGTHA] = 0;
	}
}

void CInsp::InitPressure()
{
	c_spec.di.bc.wRes = (c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA) |
						(c_pTcRef->real.cid.wAll << (HEADCCBYLENGTHA + 8));
	/*for (BYTE n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		c_spec.di.bc.wAsp[n + HEADCCBYLENGTHA] = c_spec.di.bc.wBcp[n + HEADCCBYLENGTHA] = 0;
	}*/
}

void CInsp::BottleBcp(UCURV id, BOOL bDir)
{
	if (id < SIZE_PRESSURELIST) {
		if (bDir) {
			c_spec.pi.wRes[id] = c_spec.di.bc.wRes;
			c_spec.pi.wEtc[id] = c_spec.di.bc.wEtc;
			c_spec.pi.wMcp[id] = c_spec.di.bc.wMcp;
			for (UCURV n = 0; n < 8; n ++) {
				c_spec.pi.wAsp[id][n] = c_spec.di.bc.wAsp[n];
				c_spec.pi.wBcp[id][n] = c_spec.di.bc.wBcp[n];
			}
		}
		else {
			c_spec.di.bc.wRes = c_spec.pi.wRes[id];
			c_spec.di.bc.wEtc = c_spec.pi.wEtc[id];
			c_spec.di.bc.wMcp = c_spec.pi.wMcp[id];
			for (UCURV n = 0; n < 8; n ++) {
				c_spec.di.bc.wAsp[n] = c_spec.pi.wAsp[id][n];
				c_spec.di.bc.wBcp[n] = c_spec.pi.wBcp[id][n];
			}
		}
	}
}

BOOL CInsp::BcpItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case BCPINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.bc, 0, sizeof(DUINSPECTPRESS));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		//c_spec.di.bc.wRes = (c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA) |
		//					(c_pTcRef->real.cid.wAll << (HEADCCBYLENGTHA + 8));
		InitPressure();
		ReadPressure(FALSE);
		c_spec.di.bc.wEtc = 0x707;
		pTcx->Hangup(80);
		WaitBegin(60);
		NextSubStep();
		// break;
	case BCPINSPECTSTEP_WAITHBCOS :
		if (GETTBIT(OWN_SPACEA, TCDIB_HBCOS)) {
			pTcx->Hangup(71);
			c_wBuf[1] = c_wBuf[2] = 0;
			WaitBegin(60);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd()) {
			pTcx->Hangup(79);
			GotoSubStep(BCPINSPECTSTEP_WAITUHBCOS);
		}
		break;
	case BCPINSPECTSTEP_READLOOP :
		if (c_wBcStep[c_wBuf[1]][1] == 0)	GotoSubStep(BCPINSPECTSTEP_CHECKEB);
		else if (c_spec.di.bc.wMcp == c_wBcStep[c_wBuf[1]][0]) {
			pTcx->Hangup(66);
			InitPressure();
			WaitBegin(6);	// +1
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd()) {
			pTcx->Hangup(79);
			GotoSubStep(BCPINSPECTSTEP_WAITUHBCOS);
		}
		break;
	case BCPINSPECTSTEP_DELAY :
		if (IsWaitEnd()) {
			JudgePressure((UCURV)c_wBuf[2], c_spec.di.bc.wMcp + 1);
			pTcx->Hangup(67);
			WaitBegin(2);
			NextSubStep();
		}
		break;
	case BCPINSPECTSTEP_VIEW :
		if (IsWaitEnd()) {
			pTcx->Hangup(c_wBcStep[c_wBuf[1]][1]);
			++ c_wBuf[1];
			++ c_wBuf[2];
			GotoSubStep(BCPINSPECTSTEP_READLOOP);
		}
		break;
	case BCPINSPECTSTEP_WAITNPOS :
		if (c_spec.di.bc.wMcp == MCDSV_NPOS) {
			pTcx->Hangup(77);
			WaitBegin(10);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd()) {
			pTcx->Hangup(79);
			GotoSubStep(BCPINSPECTSTEP_WAITUHBCOS);
		}
		break;
	case BCPINSPECTSTEP_CHECKEB :
		if (GETTBIT(OWN_SPACEA, TCDIB_EBS)) {
			pTcx->Hangup(66);
			InitPressure();
			WaitBegin(6);	// +1
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd()) {
			pTcx->Hangup(79);
			GotoSubStep(BCPINSPECTSTEP_WAITUHBCOS);
		}
		break;
	case BCPINSPECTSTEP_DELAYB :
		if (IsWaitEnd()) {
			JudgePressure(11, 1);
			pTcx->Hangup(67);
			WaitBegin(2);
			NextSubStep();
		}
		break;
	case BCPINSPECTSTEP_VIEWB :
		if (IsWaitEnd()) {
			pTcx->Hangup(78);
			WaitBegin(10);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd()) {
			pTcx->Hangup(79);
			GotoSubStep(BCPINSPECTSTEP_WAITUHBCOS);
		}
		break;
	case BCPINSPECTSTEP_WAITUEB :
		if (!GETTBIT(OWN_SPACEA, TCDIB_EBS)) {
			pTcx->Hangup(79);
			WaitBegin(10);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd()) {
			pTcx->Hangup(79);
			GotoSubStep(BCPINSPECTSTEP_WAITUHBCOS);
		}
		break;
	case BCPINSPECTSTEP_WAITUHBCOS :
		if (!GETTBIT(OWN_SPACEA, TCDIB_HBCOS)) {
			pTcx->Hangup(0);
			NextSubStep();
		}
		break;
	case BCPINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = BCPINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.bc.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}

	return FALSE;
}

void CInsp::V3fResult(BYTE nCid, BYTE nRes)
{
	// Modified 2013/11/02
	BYTE sh = nCid;		// + HEADCCBYLENGTHA;
	BitCtrl(nRes & 1 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wPwr);
	BitCtrl(nRes & 2 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wBoot);
	BitCtrl(nRes & 4 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wRam);
	BitCtrl(nRes & 8 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wLock);
	BitCtrl(nRes & 0x10 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wOvd);
	BitCtrl(nRes & 0x20 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wOcd);
	BitCtrl(nRes & 0x40 ? TRUE : FALSE, 0x101 << sh, &c_spec.di.v3f.wLoad);
}

BOOL CInsp::V3fItem()
{
	BYTE n;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case V3FINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.v3f, 0, sizeof(DUINSPECTV3F));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_wBuf[1] = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if (deves & DEVBEEN_V3F)	c_wBuf[1] |= (1 << (n + HEADCCBYLENGTHA));
		}
		c_spec.di.v3f.wRes = 0xefef;
		c_spec.di.v3f.wPwr = c_spec.di.v3f.wBoot = c_spec.di.v3f.wRam = c_spec.di.v3f.wLock =
			c_spec.di.v3f.wOvd = c_spec.di.v3f.wOcd = c_spec.di.v3f.wLoad =
				c_wBuf[1] | (c_wBuf[1] << 8);
		pTcx->Hangup(91);
		WaitBegin(10);
		NextSubStep();
		break;
	case V3FINSPECTSTEP_WAITSECURYTY :
		if (GETTBIT(OWN_SPACEA, TCDIB_SBS)) {
			pTcx->Hangup(92);
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.a = 0;
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.wVfsot =
				_SHIFTV(__V3FA) | _SHIFTV(__V3FB) | _SHIFTV(__V3FC) | _SHIFTV(__V3FD);
			switch (c_pTcRef->real.insp.nMode) {
			case INSPECTMODE_NON :
			case INSPECTMODE_PDT :
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.b.pdt = TRUE;
				break;
			case INSPECTMODE_DAILY :
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.b.day = TRUE;
				break;
			default :
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.b.month = TRUE;
				break;
			}
			c_wBuf[2] = c_wBuf[3] = 0;
			WaitBegin(10);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(V3FINSPECTSTEP_JUDGE);
		break;
	case V3FINSPECTSTEP_WAITTCE0 :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if ((deves & DEVBEEN_V3F) && c_pDoz->ccs[n].real.eV3f.d.st.st.test.b.tce0)
				c_wBuf[2] |= (1 << (n + HEADCCBYLENGTHA));
		}
		if (c_wBuf[2] == c_wBuf[1]) {
			WaitBegin(10);
			NextSubStep();
		}
		else if (IsWaitEnd()) {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				if ((c_wBuf[1] & (1 << (n + HEADCCBYLENGTHA))) &&
						!(c_wBuf[2] & (1 << (n + HEADCCBYLENGTHA))))
					V3fResult(n, 0);
			}
			if (c_wBuf[2] == 0) {
				pTcx->Hangup(93);
				WaitBegin(1);
				GotoSubStep(V3FINSPECTSTEP_WAIT);
			}
			else	NextSubStep();
		}
		break;
	case V3FINSPECTSTEP_WAITTCE1 :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if ((deves & DEVBEEN_V3F) && c_pDoz->ccs[n].real.eV3f.d.st.st.test.b.tce1)
				c_wBuf[3] |= (1 << (n + HEADCCBYLENGTHA));
		}
		if (c_wBuf[3] == c_wBuf[2])	NextSubStep();
		else if (IsWaitEnd()) {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				if ((c_wBuf[2] & (1 << (n + HEADCCBYLENGTHA))) &&
						!(c_wBuf[3] & (1 << (n + HEADCCBYLENGTHA))))
					V3fResult(n, 0);
			}
			if (c_wBuf[3] == 0) {
				pTcx->Hangup(94);
				WaitBegin(1);
				GotoSubStep(V3FINSPECTSTEP_WAIT);
			}
			else	NextSubStep();
		}
		break;
	case V3FINSPECTSTEP_RESULT :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (c_wBuf[3] & (1 << n + HEADCCBYLENGTHA))
				V3fResult(n, c_pDoz->ccs[n].real.eV3f.d.st.st.nItem);
		}
		GotoSubStep(V3FINSPECTSTEP_JUDGE);
		break;
	case V3FINSPECTSTEP_WAIT :
		if (IsWaitEnd())	NextSubStep();
		break;
	case V3FINSPECTSTEP_JUDGE :
		c_spec.di.v3f.wRes = 0;
		if (c_spec.di.v3f.wPwr & 0xff)	c_spec.di.v3f.wRes |= 1;
		else	c_spec.di.v3f.wRes |= 0x100;
		if (c_spec.di.v3f.wBoot & 0xff)	c_spec.di.v3f.wRes |= 2;
		else	c_spec.di.v3f.wRes |= 0x200;
		if (c_spec.di.v3f.wRam & 0xff)	c_spec.di.v3f.wRes |= 4;
		else	c_spec.di.v3f.wRes |= 0x400;
		if (c_spec.di.v3f.wLock & 0xff)	c_spec.di.v3f.wRes |= 8;
		else	c_spec.di.v3f.wRes |= 0x800;
		if (c_spec.di.v3f.wOvd & 0xff)	c_spec.di.v3f.wRes |= 0x20;
		else	c_spec.di.v3f.wRes |= 0x2000;
		if (c_spec.di.v3f.wOcd & 0xff)	c_spec.di.v3f.wRes |= 0x40;
		else	c_spec.di.v3f.wRes |= 0x4000;
		if (c_spec.di.v3f.wLoad & 0xff)	c_spec.di.v3f.wRes |= 0x80;
		else	c_spec.di.v3f.wRes |= 0x8000;
		c_wBuf[3] = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if (deves & DEVBEEN_V3F)
				c_spec.di.v3f.wVer[c_wBuf[3] ++] = c_pDoz->ccs[n].real.eV3f.d.st.st.nVer;
		}
		c_pDoz->tcs[OWN_SPACEA].real.cV3f.wVfsot = 0;
		c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.a = 0;
		pTcx->Hangup(95);
		NextSubStep();
		break;
	case V3FINSPECTSTEP_WAITUNSECURYTY :
		if (!GETTBIT(OWN_SPACEA, TCDIB_SBS)) {
			pTcx->Hangup(0);
			NextSubStep();
		}
		break;
	case V3FINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = V3FINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.v3f.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::PwrItem()
{
	BYTE n;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case DRVINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.drv, 0, sizeof(DUINSPECTDRIVE));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_wBuf[1] = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if (deves & DEVBEEN_V3F)	c_wBuf[1] |= (1 << (n + HEADCCBYLENGTHA));
		}
		c_spec.di.drv.wRes = 0xefef;
		pTcx->Hangup(91);
		WaitBegin(10);
		NextSubStep();
		break;
	case DRVINSPECTSTEP_WAITSECURYTY :
		if (GETTBIT(OWN_SPACEA, TCDIB_SBS)) {
			c_pTcRef->real.ms.wMask |= MOVESTATE_MASKSHB;
			pTcx->Hangup(96);
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.a = 0;
			switch (c_pTcRef->real.insp.nMode) {
			case INSPECTMODE_NON :
			case INSPECTMODE_PDT :
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.b.pdt = TRUE;
				break;
			case INSPECTMODE_DAILY :
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.b.day = TRUE;
				break;
			default :
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.b.month = TRUE;
				break;
			}
			WaitBegin(10);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(DRVINSPECTSTEP_RESULT);
		break;
	case DRVINSPECTSTEP_WAITP1 :
		if (c_spec.di.bc.wMcp == 10) {
			pTcx->Hangup(97);
			WaitBegin(5);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	GotoSubStep(DRVINSPECTSTEP_RESULT);
		break;
	case DRVINSPECTSTEP_WAITONLY :
		if (IsWaitEnd())	NextSubStep();
		break;
	case DRVINSPECTSTEP_RESULT :
		c_wBuf[1] = 0;
		c_spec.di.drv.wRes = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if (deves & DEVBEEN_V3F) {
				if (pTcx->CheckV3fReply(n, &c_pDoz->ccs[n].real.eV3f, FALSE) == FIRMERROR_NON) {
					BOOL bRes = TRUE;
					c_spec.di.drv.wCapV[c_wBuf[1]] =
						pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nFc, 0, 250, (WORD)0, (WORD)2000);
					if (c_spec.di.drv.wCapV[c_wBuf[1]] == 0)	bRes = FALSE;
					c_spec.di.drv.wTqC[c_wBuf[1]] =
						pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nTrqRef, 0, 250, (WORD)0, (WORD)2000);
					if (c_spec.di.drv.wTqC[c_wBuf[1]] == 0)		bRes = FALSE;
					c_spec.di.drv.wRtqC[c_wBuf[1]] =
						pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nIq, 0, 250, (WORD)0, (WORD)2000);
					if (c_spec.di.drv.wRtqC[c_wBuf[1]] == 0)	bRes = FALSE;
					c_spec.di.drv.tbe[c_wBuf[1]] =
						(SHORT)pTcx->ConvS(GETLWORD(c_pDoz->ccs[n].real.eV3f.d.st.st.vTbeL), -1024, 1024, -100, 100);
					c_spec.di.drv.wVl[c_wBuf[1]] =
						pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nVl, 0, 250, (WORD)0, (WORD)100);
					// Modified 2013/11/02
					//if (bRes)	c_spec.di.drv.wRes |= (1 << (n + HEADCCBYLENGTHA));
					//else	c_spec.di.drv.wRes |= (0x100 << (n + HEADCCBYLENGTHA));
					if (bRes)	c_spec.di.drv.wRes |= (1 << n);
					else	c_spec.di.drv.wRes |= (0x100 << n);
				}
				// Modified 2013/11/02
				//else	c_spec.di.drv.wRes |= (0x100 << (n + HEADCCBYLENGTHA));
				else	c_spec.di.drv.wRes |= (0x100 << n);
				++ c_wBuf[1];
			}
		}
		pTcx->Hangup(99);
		c_pTcRef->real.ms.wMask &= ~MOVESTATE_MASKSHB;
		WaitBegin(10);
		NextSubStep();
		break;
	case DRVINSPECTSTEP_WAITUNSECURYTY :
		if (!GETTBIT(OWN_SPACEA, TCDIB_SBS) && c_spec.di.bc.wMcp == MCDSV_B7POS) {
			pTcx->Hangup(0);
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.a = 0;
			c_pTcRef->real.ms.wMask &= ~MOVESTATE_MASKSHB;
			NextSubStep();
		}
		break;
	case DRVINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = DRVINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.drv.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

void CInsp::JudgeHtc(BYTE judge, WORD* pSubRes, WORD flag, WORD* pRes)
{
	BOOL bRes = TRUE;
	*pSubRes = 0;
	for (UCURV n = 0; n < 8; n ++) {
		if (judge & (1 << n)) {
			*pSubRes |= (1 << (n + HEADCCBYLENGTHA + 8));
			bRes = FALSE;
		}
		else	*pSubRes |= (1 << (n + HEADCCBYLENGTHA));
	}
	if (bRes)	*pRes |= flag;
	else	*pRes |= (flag << 8);
}

BOOL CInsp::CoolItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case COOLINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.cool, 0, sizeof(DUINSPECTCOOL));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		pTcx->Hangup(102);
		c_spec.di.cool.wRes = 0x7f7f;
		c_spec.di.cool.wCool[0] = c_spec.di.cool.wCool[1] =
		c_spec.di.cool.wFan[0] = c_spec.di.cool.wFan[1] =
		c_spec.di.cool.wExhaust =
		c_spec.di.cool.wDamper[0] = c_spec.di.cool.wDamper[1] =
			(c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA) |
				(c_pTcRef->real.cid.wAll << (HEADCCBYLENGTHA + 8));
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.hvts = TRUE;
		WaitBegin(10);
		NextSubStep();
		break;
	case COOLINSPECTSTEP_WAITEXC :
		if (c_pLcFirm->aHtc.test.b.exc) {
			WaitBegin(300);
			NextSubStep();
		}
		else if (IsWaitEnd())	GotoSubStep(COOLINSPECTSTEP_ALLNG);
		break;
	case COOLINSPECTSTEP_WAITEND :
		if (c_pLcFirm->aHtc.test.b.end)
			NextSubStep();
		else if (IsWaitEnd())	GotoSubStep(COOLINSPECTSTEP_ALLNG);
		break;
	case COOLINSPECTSTEP_JUDGE :
		pTcx->Hangup(103);
		c_spec.di.cool.wRes = 0;
		JudgeHtc(c_pLcFirm->aHtc.nCoolArm[0], &c_spec.di.cool.wCool[0], 1, &c_spec.di.cool.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nCoolArm[1], &c_spec.di.cool.wCool[1], 2, &c_spec.di.cool.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nFanArm[0], &c_spec.di.cool.wFan[0], 4, &c_spec.di.cool.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nFanArm[1], &c_spec.di.cool.wFan[1], 8, &c_spec.di.cool.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nExhaustArm, &c_spec.di.cool.wExhaust, 0x10, &c_spec.di.cool.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nEdamArm, &c_spec.di.cool.wDamper[0], 0x20, &c_spec.di.cool.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nFdamArm, &c_spec.di.cool.wDamper[1], 0x40, &c_spec.di.cool.wRes);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.hvts = FALSE;
		WaitBegin(2);
		GotoSubStep(COOLINSPECTSTEP_WAIT);
		break;
	case COOLINSPECTSTEP_ALLNG :
		pTcx->Hangup(108);
		c_spec.di.cool.wRes = 0;
		JudgeHtc(0xff, &c_spec.di.cool.wCool[0], 1, &c_spec.di.cool.wRes);
		JudgeHtc(0xff, &c_spec.di.cool.wCool[1], 2, &c_spec.di.cool.wRes);
		JudgeHtc(0xff, &c_spec.di.cool.wFan[0], 4, &c_spec.di.cool.wRes);
		JudgeHtc(0xff, &c_spec.di.cool.wFan[1], 8, &c_spec.di.cool.wRes);
		JudgeHtc(0xff, &c_spec.di.cool.wExhaust, 0x10, &c_spec.di.cool.wRes);
		JudgeHtc(0xff, &c_spec.di.cool.wDamper[0], 0x20, &c_spec.di.cool.wRes);
		JudgeHtc(0xff, &c_spec.di.cool.wDamper[1], 0x40, &c_spec.di.cool.wRes);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.hvts = FALSE;
		WaitBegin(2);
		NextSubStep();
	case COOLINSPECTSTEP_WAIT :
		if (IsWaitEnd())	NextSubStep();
		break;
	case COOLINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = COOLINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.cool.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::HeatItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case HEATINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.heat, 0, sizeof(DUINSPECTHEAT));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		pTcx->Hangup(106);
		c_spec.di.heat.wRes = 0x707;
		c_spec.di.heat.wHeat[0] = c_spec.di.heat.wHeat[1] = c_spec.di.heat.wHeat[2] =
			(c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA) |
				(c_pTcRef->real.cid.wAll << (HEADCCBYLENGTHA + 8));
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.htts = TRUE;
		WaitBegin(10);
		NextSubStep();
		break;
	case HEATINSPECTSTEP_WAITEXC :
		if (c_pLcFirm->aHtc.test.b.exc) {
			WaitBegin(300);
			NextSubStep();
		}
		else if (IsWaitEnd())	GotoSubStep(HEATINSPECTSTEP_ALLNG);
		break;
	case HEATINSPECTSTEP_WAITEND :
		if (c_pLcFirm->aHtc.test.b.end)	NextSubStep();
		else if (IsWaitEnd())	GotoSubStep(HEATINSPECTSTEP_ALLNG);
		break;
	case HEATINSPECTSTEP_JUDGE :
		pTcx->Hangup(107);
		c_spec.di.heat.wRes = 0;
		JudgeHtc(c_pLcFirm->aHtc.nHeatArm[0], &c_spec.di.heat.wHeat[0], 1, &c_spec.di.heat.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nHeatArm[1], &c_spec.di.heat.wHeat[1], 2, &c_spec.di.heat.wRes);
		JudgeHtc(c_pLcFirm->aHtc.nHeatArm[2], &c_spec.di.heat.wHeat[2], 4, &c_spec.di.heat.wRes);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.htts = FALSE;
		WaitBegin(2);
		GotoSubStep(HEATINSPECTSTEP_WAIT);
		break;
	case HEATINSPECTSTEP_ALLNG :
		pTcx->Hangup(109);
		c_spec.di.heat.wRes = 0;
		JudgeHtc(0xff, &c_spec.di.heat.wHeat[0], 1, &c_spec.di.heat.wRes);
		JudgeHtc(0xff, &c_spec.di.heat.wHeat[1], 2, &c_spec.di.heat.wRes);
		JudgeHtc(0xff, &c_spec.di.heat.wHeat[2], 4, &c_spec.di.heat.wRes);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.htts = FALSE;
		WaitBegin(2);
		NextSubStep();
	case HEATINSPECTSTEP_WAIT :
		if (IsWaitEnd())	NextSubStep();
		break;
	case HEATINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = HEATINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.heat.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::CmsbItem()
{
	BYTE n;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case CMSBINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.cmsb, 0, sizeof(DUINSPECTCMSB));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_wBuf[1] = c_wBuf[2] = c_wBuf[3] = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			if (deves & DEVBEEN_CMSB)	c_wBuf[1] |= (1 << (n + HEADCCBYLENGTHA));
			c_clock[n] = 0;
		}
		pTcx->Hangup(111);
		c_spec.di.cmsb.wRes = c_wBuf[1] | (c_wBuf[1] << 8);
		WaitBegin(30);
		NextSubStep();
		break;
	case CMSBINSPECTSTEP_WAITCMG :
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (c_wBuf[1] & (1 << (n + HEADCCBYLENGTHA))) {
				BYTE m = n < 4 ? 0 : 1;
				if (!(c_wBuf[2] & (1 << (n + HEADCCBYLENGTHA)))) {
					if (GETCBIT(n, CCDIB_CMG)) {
						c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[m].b.cmr = TRUE;
						c_spec.di.cmsb.wRes |= m == 0 ? 8 : 0x80;
						c_clock[n] = ClockCycles();
						c_wBuf[2] |= (1 << (n + HEADCCBYLENGTHA));
					}
				}
				else {
					if (!GETCBIT(n, CCDIB_CMG)) {
						if (!(c_wBuf[3] & (1 << (n + HEADCCBYLENGTHA)))) {
							uint64_t clk = ClockCycles();
							double sec = (double)(clk - c_clock[n]) / (double)c_pTcRef->real.cps;
							c_spec.di.cmsb.wTime[m] = (WORD)sec;
							if (c_spec.di.cmsb.wTime[m] <= 120) {
								c_spec.di.cmsb.wRes &= ~(1 << (n + HEADCCBYLENGTHA + 8));
								c_spec.di.cmsb.wRes |= (1 << (n + HEADCCBYLENGTHA));
							}
							else {
								c_spec.di.cmsb.wRes &= ~(1 << (n + HEADCCBYLENGTHA));
								c_spec.di.cmsb.wRes |= (1 << (n + HEADCCBYLENGTHA + 8));
							}
							c_wBuf[3] |= (1 << (n + HEADCCBYLENGTHA));
						}
					}
				}
			}
		}
		if (c_wBuf[2] == c_wBuf[1])	pTcx->Hangup(112);
		if (c_wBuf[3] == c_wBuf[1]) {
			pTcx->Hangup(113);
			NextSubStep();
		}
		if (c_bWait && IsWaitEnd())	NextSubStep();
		break;
	case CMSBINSPECTSTEP_JUDGE :
		c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[0].b.cmr =
			c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[1].b.cmr = FALSE;
		c_spec.di.cmsb.wRes &= ~0x88;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if ((c_spec.di.cmsb.wRes & (0x101 << (n + HEADCCBYLENGTHA))) ==
				(0x101 << (n + HEADCCBYLENGTHA)))
				c_spec.di.cmsb.wRes &= ~(1 << (n + HEADCCBYLENGTHA));
		}
		WaitBegin(1);
		NextSubStep();
		break;
	case CMSBINSPECTSTEP_SHUTDOWN :
		if (IsWaitEnd())	NextSubStep();
		break;
	case CMSBINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = CMSBINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.cmsb.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::DoorBackupItem()
{
	UCURV n;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case DOORBKUPINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.doorb, 0, sizeof(DUINSPECTDOORB));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_wCcForm = (BYTE)(c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA);
		c_spec.di.doorb.wRes = c_spec.di.doorb.wRight = c_spec.di.doorb.wLeft = 0;
		pTcx->Hangup(118);
		NextSubStep();
		break;
	case DOORBKUPINSPECTSTEP_WAITMM :
		if (c_pTcRef->real.door.nMode == DOORMODE_MOMC) {
			c_spec.di.doorb.wRes = 0x0101;
			c_spec.di.doorb.wRight = c_spec.di.doorb.wLeft = c_wCcForm | c_wCcForm << 8;
			c_wDoorWait = c_wDoorRes = 0;
			pTcx->Hangup(120);
			WaitBegin(1);
			NextSubStep();
		}
		break;
	case DOORBKUPINSPECTSTEP_OPENRIGHT :
		if (IsWaitEnd()) {
			pTcx->ObsCtrl(TRUE, CCDOB_OCRDO, ALLCAR_FLAGS);
			WaitBegin(8);
			NextSubStep();
		}
		break;
	case DOORBKUPINSPECTSTEP_OPENEDRIGHT :
		if (IsWaitEnd() || c_wDoorWait == c_wCcForm) {
			WaitBegin(1);
			NextSubStep();
		}
		else {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				BYTE door = c_pDoz->ccs[n].real.nInput[CCDI_DOOR];
				if ((door & 0xf0) == 0xf0)	c_wDoorWait |= (1 << n);
			}
		}
		break;
	case DOORBKUPINSPECTSTEP_CLOSERIGHT :
		if (IsWaitEnd()) {
			pTcx->ObsCtrl(FALSE, CCDOB_OCRDO, ALLCAR_FLAGS);
			WaitBegin(8);
			NextSubStep();
		}
		break;
	case DOORBKUPINSPECTSTEP_CLOSEDRIGHT :
		if (IsWaitEnd() || c_wDoorRes == c_wCcForm) {
			WaitBegin(1);
			NextSubStep();
		}
		else {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				BYTE door = c_pDoz->ccs[n].real.nInput[CCDI_DOOR];
				if ((door & 0xf0) == 0)	c_wDoorRes |= (1 << n);
			}
		}
		break;
	case DOORBKUPINSPECTSTEP_JUDGERIGHT :
		c_spec.di.doorb.wRight = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if ((c_wDoorWait & (1 << n)) && (c_wDoorRes & (1 << n)))
				c_spec.di.doorb.wRight |= (1 << ((n ^ 1) + HEADCCBYLENGTHA));
			else	c_spec.di.doorb.wRight |= (1 << (((n ^ 1) + HEADCCBYLENGTHA) + 8));
		}
		if (c_spec.di.doorb.wRight & 0xff00)	c_spec.di.doorb.wRes = 0x100;
		else	c_spec.di.doorb.wRes = 1;
		WaitBegin(1);
		NextSubStep();
		break;
	case DOORBKUPINSPECTSTEP_OPENLEFT :
		if (IsWaitEnd()) {
			c_spec.di.doorb.wRes |= 0x1010;
			c_spec.di.doorb.wLeft = c_wCcForm | c_wCcForm << 8;
			c_wDoorWait = c_wDoorRes = 0;
			pTcx->ObsCtrl(TRUE, CCDOB_OCLDO, ALLCAR_FLAGS);
			WaitBegin(8);
			NextSubStep();
		}
		break;
	case DOORBKUPINSPECTSTEP_OPENEDLEFT :
		if (IsWaitEnd() || c_wDoorWait == c_wCcForm) {
			WaitBegin(1);
			NextSubStep();
		}
		else {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				BYTE door = c_pDoz->ccs[n].real.nInput[CCDI_DOOR];
				if ((door & 0xf) == 0xf)	c_wDoorWait |= (1 << n);
			}
		}
		break;
	case DOORBKUPINSPECTSTEP_CLOSELEFT :
		if (IsWaitEnd()) {
			pTcx->ObsCtrl(FALSE, CCDOB_OCLDO, ALLCAR_FLAGS);
			WaitBegin(8);
			NextSubStep();
		}
		break;
	case DOORBKUPINSPECTSTEP_CLOSEDLEFT :
		if (IsWaitEnd() || c_wDoorRes == c_wCcForm) {
			WaitBegin(1);
			NextSubStep();
		}
		else {
			for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				BYTE door = c_pDoz->ccs[n].real.nInput[CCDI_DOOR];
				if ((door & 0xf) == 0)	c_wDoorRes |= (1 << n);
			}
		}
		break;
	case DOORBKUPINSPECTSTEP_JUDGELEFT :
		c_spec.di.doorb.wLeft = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if ((c_wDoorWait & (1 << n)) && (c_wDoorRes & (1 << n)))
				c_spec.di.doorb.wLeft |= (1 << ((n ^ 1) + HEADCCBYLENGTHA));
			else	c_spec.di.doorb.wLeft |= (1 << (((n ^ 1) + HEADCCBYLENGTHA) + 8));
		}
		c_spec.di.doorb.wRes &= 0x101;
		if (c_spec.di.doorb.wLeft & 0xff00)	c_spec.di.doorb.wRes |= 0x1000;
		else	c_spec.di.doorb.wRes |= 0x10;
		pTcx->Hangup(121);
		WaitBegin(2);
		NextSubStep();
		break;
	case DOORBKUPINSPECTSTEP_TURNAA :
		if (IsWaitEnd()) {
			pTcx->Hangup(119);
			NextSubStep();
		}
		break;
	case DOORBKUPINSPECTSTEP_WAITAA :
		if (c_pTcRef->real.door.nMode == DOORMODE_AOAC)	NextSubStep();
		break;
	case DOORBKUPINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = DOORBKUPINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.doorb.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::TcmsItem()
{
	BYTE n;
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case TCMSINSPECTSTEP_BEGIN :
		memset((PVOID)&c_spec.di.tcms, 0, sizeof(DUINSPECTTCMS));
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_spec.di.tcms.wRes = 0x303;
		c_spec.di.tcms.wTc = 0x8181;
		for (n = 0; n < 4; n ++)	c_spec.di.tcms.wTcVer[n] = 0;
		for (n = 0; n < 8; n ++)	c_spec.di.tcms.wCcVer[n] = 0;
		c_spec.di.tcms.wCc = (c_pTcRef->real.cid.wAll << HEADCCBYLENGTHA) |
							(c_pTcRef->real.cid.wAll << (HEADCCBYLENGTHA + 8));
		pTcx->Hangup(116);
		c_wBuf[0] =(WORD)((c_pDoz->tcs[CAR_HEAD].real.nHcrInfo - 1) & 2);
		WaitBegin(1);
		NextSubStep();
		break;
	case TCMSINSPECTSTEP_WAIT :
		if (IsWaitEnd())	NextSubStep();
		break;
	case TCMSINSPECTSTEP_JUDGE :
		c_spec.di.tcms.wRes = 0;
		if (c_wBuf[0] == 0) {	// TC1
			c_spec.di.tcms.wTc = 1;
			c_spec.di.tcms.wTcVer[0] = HEXTODEC(c_pDoz->tcs[CAR_HEAD].real.wVersion);
			if (pTcx->GetBusCondition(CAR_HEADBK)) {
				c_spec.di.tcms.wTc |= 2;
				c_spec.di.tcms.wTcVer[1] = HEXTODEC(c_pDoz->tcs[CAR_HEADBK].real.wVersion);
			}
			else	c_spec.di.tcms.wTc |= 0x200;
			if (pTcx->GetBusCondition(CAR_TAIL)) {
				c_spec.di.tcms.wTc |= 0x80;
				c_spec.di.tcms.wTcVer[2] = HEXTODEC(c_pDoz->tcs[CAR_TAIL].real.wVersion);
			}
			else	c_spec.di.tcms.wTc |= 0x8000;
			if (pTcx->GetBusCondition(CAR_TAILBK)) {
				c_spec.di.tcms.wTc |= 0x40;
				c_spec.di.tcms.wTcVer[3] = HEXTODEC(c_pDoz->tcs[CAR_TAILBK].real.wVersion);
			}
			else	c_spec.di.tcms.wTc |= 0x4000;
		}
		else {				// TC0
			c_spec.di.tcms.wTc = 0x80;
			c_spec.di.tcms.wTcVer[2] = HEXTODEC(c_pDoz->tcs[CAR_HEAD].real.wVersion);
			if (pTcx->GetBusCondition(CAR_HEADBK)) {
				c_spec.di.tcms.wTc |= 0x40;
				c_spec.di.tcms.wTcVer[3] = HEXTODEC(c_pDoz->tcs[CAR_HEADBK].real.wVersion);
			}
			else	c_spec.di.tcms.wTc |= 0x4000;
			if (pTcx->GetBusCondition(CAR_TAIL)) {
				c_spec.di.tcms.wTc |= 1;
				c_spec.di.tcms.wTcVer[0] = HEXTODEC(c_pDoz->tcs[CAR_TAIL].real.wVersion);
			}
			else	c_spec.di.tcms.wTc |= 0x100;
			if (pTcx->GetBusCondition(CAR_TAILBK)) {
				c_spec.di.tcms.wTc |= 2;
				c_spec.di.tcms.wTcVer[1] = HEXTODEC(c_pDoz->tcs[CAR_TAILBK].real.wVersion);
			}
			else	c_spec.di.tcms.wTc |= 0x200;
		}
		if (c_spec.di.tcms.wTc & 0xff00)	c_spec.di.tcms.wRes = 0x100;
		else	c_spec.di.tcms.wRes = 1;
		c_spec.di.tcms.wCc = 0;
		for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (pTcx->GetBusCondition(n + CAR_CC)) {
				c_spec.di.tcms.wCc |= (1 << (n + HEADCCBYLENGTHA));
				c_spec.di.tcms.wCcVer[n + HEADCCBYLENGTHA] = HEXTODEC(c_pDoz->ccs[n].real.wVersion);
			}
			else	c_spec.di.tcms.wCc |= (0x100 << (n + HEADCCBYLENGTHA));
		}
		if (c_spec.di.tcms.wCc & 0xff00)	c_spec.di.tcms.wRes |= 0x200;
		else	c_spec.di.tcms.wRes |= 2;
		pTcx->Hangup(117);
		WaitBegin(2);
		NextSubStep();
	case TCMSINSPECTSTEP_WAITA :
		if (IsWaitEnd())	NextSubStep();
		break;
	case TCMSINSPECTSTEP_END :
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = TCMSINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.tcms.wRes & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;

		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::LampItem()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (c_pTcRef->real.insp.nSubStep) {
	case LAMPINSPECTSTEP_BEGIN :
		ChgPage(c_flow[c_pTcRef->real.insp.nStep].wPage[c_pTcRef->real.insp.nRefMode]);
		c_pTcRef->real.du.nSw = 0;
		c_spec.di.wLamp = 0x101;
		pTcx->Hangup(114);
		WaitBegin(10);
		NextSubStep();
	case LAMPINSPECTSTEP_WAIT :
		//{
		//	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
		//	if (paAtc != NULL) {
		//		c_pTcRef->real.insp.nAbsLamp = paAtc->d.st.req.b.lampt;
		//	}
		//	else	c_pTcRef->real.insp.nAbsLamp = 0;
		//}
		if (c_pTcRef->real.du.nSw == DUKEY_OK || c_pTcRef->real.du.nSw == DUKEY_NG) {
			c_spec.di.wLamp = c_pTcRef->real.du.nSw == DUKEY_OK ? 1 : 0x100;
			c_pTcRef->real.du.nSw = 0;
			pTcx->Hangup(115);
			WaitBegin(2);
			NextSubStep();
		}
		else if (c_bWait && IsWaitEnd())	NextSubStep();
		break;
	case LAMPINSPECTSTEP_WAITA :
		//c_pTcRef->real.insp.nAbsLamp = 0;
		if (IsWaitEnd())	NextSubStep();
		break;
	case LAMPINSPECTSTEP_END :
		//c_pTcRef->real.insp.nAbsLamp = 0;
		if (c_bPause) {
			BYTE rs = WaitRepeatOrSkip();
			if (rs != INSPKEY_NON) {
				if (rs == INSPKEY_SKIP)	return TRUE;
				c_pTcRef->real.insp.nSubStep = LAMPINSPECTSTEP_BEGIN;
			}
		}
		else if ((c_spec.di.wLamp & 0xff00) || c_pTcRef->real.insp.nMode == INSPECTMODE_EACHDEV)
			WaitRepeatOrSkip();
		else	return TRUE;
		break;
	default :
		break;
	}
	return FALSE;
}

BOOL CInsp::Delay(WORD delay)
{
	if (c_bDelay) {
		if (c_pTcRef->real.insp.wDelay != 0)	return FALSE;
		c_bDelay = FALSE;
		return TRUE;
	}
	else {
		c_pTcRef->real.insp.wDelay = delay * 100;
		c_bDelay = TRUE;
		return FALSE;
	}
}

void CInsp::Exit()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Lock();
	c_pTcRef->real.nLocalDataType[LOCALID_ATO] = DATATYPE_NORMAL;
	pTcx->Unlock();
	c_pTcRef->real.ms.wMask &= ~MOVESTATE_MASKSHB;
	CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
	c_bDelay = FALSE;
	c_bCmd = FALSE;
	//c_pTcRef->real.insp.nAbsLamp = 0;
	pTcx->InterTrigger(&c_pTcRef->real.insp.wTimer, 0);
	pTcx->Hangup(0);
	//while(!Delay(1));
	c_pDoz->tcs[OWN_SPACEA].real.cSiv.wPanto = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cSiv.wSivsot = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cEcu.set.a = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cEcu.wRqt = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cV3f.test.a = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cV3f.wVfsot = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[0].b.cmr = FALSE;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[1].b.cmr = FALSE;
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.hvts = FALSE;
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.htts = FALSE;
}

BOOL CInsp::Check()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_pTcRef->real.insp.nStep != 3) {	// non extension
		c_spec.di.siv.dwState = 0;
		if (!pTcx->GetBusCondition(_REALCC(__SIVA)))
			c_spec.di.siv.dwState |= TD_DCOMF;
		else {
			UCURV res = pTcx->CheckSivReply(__SIVA, &c_pDoz->ccs[__SIVA].real.eSiv, FALSE);
			if (res == FIRMERROR_ADDR)	c_spec.di.siv.dwState |= TD_DCOMF;
			else if (res == FIRMERROR_FAULT)	c_spec.di.siv.dwState |= TD_DFAULT;
			else	c_spec.di.siv.dwState |= TD_DON;
		}
		if (!pTcx->GetBusCondition(_REALCC(__SIVB)))
			c_spec.di.siv.dwState |= (TD_DCOMF << 7);
		else {
			UCURV res = pTcx->CheckSivReply(__SIVB, &c_pDoz->ccs[__SIVB].real.eSiv, FALSE);
			if (res == FIRMERROR_ADDR)	c_spec.di.siv.dwState |= (TD_DCOMF << 7);
			else if (res == FIRMERROR_FAULT)	c_spec.di.siv.dwState |= (TD_DFAULT << 7);
			else	c_spec.di.siv.dwState |= (TD_DON << 7);
		}
	}
	if (c_pTcRef->real.mcDrv.bValid)	c_spec.di.bc.wMcp = (WORD)c_pTcRef->real.mcDrv.nIndex;
	else	c_spec.di.bc.wMcp = 15;
	if (c_pTcRef->real.dev.wNrbi != 0 || c_pTcRef->real.dev.wNrbc != 0)	c_spec.di.bc.wEtc |= 1;
	else	c_spec.di.bc.wEtc &= ~1;
	if (GETTBIT(OWN_SPACEA, TCDIB_EBRA))	c_spec.di.bc.wEtc |= 2;
	else	c_spec.di.bc.wEtc &= ~2;
	if (GETTBIT(OWN_SPACEA, TCDIB_EBS))		c_spec.di.bc.wEtc |= 4;
	else	c_spec.di.bc.wEtc &= ~4;

	if (c_pTcRef->real.insp.nStep == 0) {
		++ c_pTcRef->real.insp.nStep;
		c_pTcRef->real.insp.nAuxStep = 0;
		c_pTcRef->real.insp.nSubStep = 0;
		c_pTcRef->real.du.nSw = 0;
		c_pTcRef->real.insp.wPage = 0;
		c_bCmd = FALSE;
		if (c_pTcRef->real.insp.nMode < INSPECTMODE_EACHDEV)
			memset((PVOID)&c_spec.di, 0, sizeof(DUINSPECTINFO));
		c_spec.di.wMode = (WORD)c_pTcRef->real.insp.nRefMode;
		TBTIME tb;
		pTcx->GetTbTime(&tb);
		memcpy((PVOID)&c_spec.time, (PVOID)&tb, sizeof(TBTIME));
		c_spec.di.wYear = (WORD)tb.year;
		c_spec.di.wDate = ((WORD)tb.month << 8) | (WORD)tb.day;
		c_spec.di.wTime = ((WORD)tb.hour << 8) | (WORD)tb.minute;
		c_spec.di.wSecond = (WORD)tb.sec;
	}

	if (c_flow[c_pTcRef->real.insp.nStep].func == NULL ||
			c_pTcRef->real.du.nSw == DUKEY_EXIT || c_pTcRef->real.du.nSw == DUKEY_ESC) {
		if (c_pTcRef->real.du.nSw == DUKEY_EXIT || c_pTcRef->real.du.nSw == DUKEY_ESC) {
			c_pTcRef->real.du.nSw = 0;
			c_bPause = c_bCmd = FALSE;
			Exit();
		}
		return TRUE;
	}
	if (!(c_pTcRef->real.insp.wItem & c_flow[c_pTcRef->real.insp.nStep].wItem))
		++ c_pTcRef->real.insp.nStep;
	else {
		INSPECTF func = c_flow[c_pTcRef->real.insp.nStep].func;
		if ((this->*func)()) {
			++ c_pTcRef->real.insp.nStep;
			c_pTcRef->real.insp.nSubStep = 0;
		}
	}
	return FALSE;
}

void CInsp::ResetStep()
{
	c_pTcRef->real.insp.nStep = c_pTcRef->real.insp.nSubStep = c_pTcRef->real.insp.nAuxStep = 0;
	c_pTcRef->real.insp.nReqCount = 0;
	c_pTcRef->real.du.nTerm &= ~(DUTERM_REPEAT | DUTERM_SKIP);
	c_bPause = c_bCmd = FALSE;
}

WORD CInsp::GetPage()
{
	if (c_pTcRef->real.insp.nMode == INSPECTMODE_VIEW) {
		switch (c_pTcRef->real.insp.nId) {
		case 0 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return c_flow[1].wPage[c_pTcRef->real.insp.nRefMode];		// signal & door
			break;
		case 1 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2221;		// atc1 door
			break;
		case 2 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2222;		// atc1 eb fsr
			break;
		case 3 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2223;		// atc1 speed low
			break;
		case 4 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2224;		// atc1 speed high & sensitivity
			break;
		case 5 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2225;		// atc1 over speed
			break;
		case 6 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2231;		// atc2 door
			break;
		case 7 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2232;		// atc2 eb fsr
			break;
		case 8 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2233;		// atc2 speed low
			break;
		case 9 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2234;		// atc2 speed high & sensitivity
			break;
		case 10 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2235;		// atc2 over speed
			break;
		case 11 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SND)
				return 2239;		// ato
			break;
		case 12 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_SIV)
				return c_flow[2].wPage[c_pTcRef->real.insp.nRefMode];		// siv & ext
			break;
		case 13 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_ECU)
				return c_flow[4].wPage[c_pTcRef->real.insp.nRefMode];		// ecu
			break;
		case 14 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_BCP)
				return c_flow[5].wPage[c_pTcRef->real.insp.nRefMode];		// bcp
			break;
		case 15 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_V3F)
				return c_flow[6].wPage[c_pTcRef->real.insp.nRefMode];		// v3f
			break;
		case 16 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_PWR)
				return c_flow[7].wPage[c_pTcRef->real.insp.nRefMode];		// pwr
			break;
		case 17 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_COOL)
				return c_flow[8].wPage[c_pTcRef->real.insp.nRefMode];		// cool
			break;
		case 18 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_HEAT)
				return c_flow[9].wPage[c_pTcRef->real.insp.nRefMode];		// heat
			break;
		case 19 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_CMSB)
				return c_flow[10].wPage[c_pTcRef->real.insp.nRefMode];		// cmsb
			break;
		case 20 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_DOORB)
				return c_flow[11].wPage[c_pTcRef->real.insp.nRefMode];		// door backup
			break;
		case 21 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_TCMS)
				return c_flow[12].wPage[c_pTcRef->real.insp.nRefMode];		// tcms
			break;
		case 22 :
			if (c_pTcRef->real.insp.wItem & INSPECTITEM_LAMP)
				return c_flow[13].wPage[c_pTcRef->real.insp.nRefMode];		// lamp
			break;
		default :	return 0;	break;
		}
		return 0xffff;
	}
	return c_pTcRef->real.insp.wPage;
}
