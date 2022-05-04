/*
 * CChaser.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CChaser.h"
#include "CTcx.h"

CChaser::CChaser()
{
	c_pTcRef = NULL;
	c_pDoz = NULL;
	c_pParent = NULL;
}

CChaser::~CChaser()
{
}

void CChaser::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pDoz = pTcx->GetTcDoz();

	memset((PVOID)&c_atcch.ta, 0, sizeof(TRACEARCH));
	memset((PVOID)&c_atoch.ta, 0, sizeof(TRACEARCH));
	for (UCURV n = 0; n < MAX_DEV_LENGTH; n ++) {
		memset((PVOID)&c_sivch[n].ta, 0, sizeof(TRACEARCH));
		memset((PVOID)&c_v3fch[n].ta, 0, sizeof(TRACEARCH));
		memset((PVOID)&c_ecuch[n].ta, 0, sizeof(TRACEARCH));
	}
}

BYTE CChaser::BitIndex(BYTE bits)
{
	for (BYTE n = 8; n > 0; n ++) {
		if (bits & (1 << (n - 1)))	return n;
	}
	return 0;
}

WORD CChaser::CalcProgress(WORD cur, WORD max)
{
	if (max > 0)	return (cur * 100 / max);
	return 0;
}

UCURV CChaser::CountEcuTraceMark(BYTE* p)
{
	UCURV n = 0;
	while (*p == 0xff) {
		++ n;
		++ p;
	}
	return n;
}

PTRACEARCH CChaser::GetTraceArch(UCURV di, UCURV ci)
{
	switch (di) {
	case TRACEID_SIV :
		return &c_sivch[ci].ta;
		break;
	case TRACEID_V3F :
		return &c_v3fch[ci].ta;
		break;
	case TRACEID_ECU :
		return &c_ecuch[ci].ta;
		break;
	case TRACEID_ATC :
		return &c_atcch.ta;
		break;
	case TRACEID_ATO :
		return &c_atoch.ta;
		break;
	default :
		break;
	}
	return NULL;
}

void CChaser::TraceBegin(UCURV di, UCURV ci)
{
	TRACEARCH* pTa;
	switch (di) {
	case TRACEID_SIV :
		pTa = &c_sivch[ci].ta;
		break;
	case TRACEID_V3F :
		pTa = &c_v3fch[ci].ta;
		break;
	case TRACEID_ECU :
		pTa = &c_ecuch[ci].ta;
		break;
	case TRACEID_ATC :
		pTa = &c_atcch.ta;
		ci = 0;
		break;
	case TRACEID_ATO :
		pTa = &c_atoch.ta;
		ci = 0;
		break;
	default :
		pTa = NULL;
		break;
	}
	if (pTa != NULL && pTa->wLength > 0 && pTa->nState == 0) {
		pTa->nState = TRACESTATE_LOAD;
		pTa->wiTc = 0;
		pTa->wiCc = 0;
		CTcx* pTcx = (CTcx*)c_pParent;
		pTcx->SetTraceProgress(di, ci, 0);
	}
}

void CChaser::TraceInitial()
{
	c_pTcRef->real.nLocalDataType[LOCALID_ATCA] =
	c_pTcRef->real.nLocalDataType[LOCALID_ATCB] =
	c_pTcRef->real.nLocalDataType[LOCALID_ATO] = DATATYPE_NORMAL;
	c_pTcRef->real.wTraceDev =
	c_pTcRef->real.wSivTrace =
	c_pTcRef->real.wV3fTrace =
	c_pTcRef->real.wEcuTrace =
	c_pTcRef->real.wSivTraceClear =
	c_pTcRef->real.wV3fTraceClear =
	c_pTcRef->real.wEcuTraceClear = 0;
	c_atcch.ta.nState =
	c_atoch.ta.nState =
	c_sivch[__SIVA].ta.nState =
	c_sivch[__SIVB].ta.nState =
	c_v3fch[__V3FA].ta.nState =
	c_v3fch[__V3FB].ta.nState =
	c_v3fch[__V3FC].ta.nState =
	c_v3fch[__V3FD].ta.nState = 0;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		c_ecuch[n].ta.nState = 0;
	}
}

void CChaser::AtcTraceCtrl(PATCAINFO paAtc)
{
	if (paAtc == NULL)	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	if (paAtc->nFlow == LOCALFLOW_TRACE) {
		if (c_atcch.ta.nState == TRACESTATE_LOAD) {
			//MSGLOG("[TRACE]Atc capture %d of %d\n", paAtc->d.tr.nBlock, c_atcch.ta.wBlockDev);
			if ((WORD)paAtc->d.tr.nIndex < c_atcch.ta.wDev) {
				memcpy((PVOID)&c_atcch.d[paAtc->d.tr.nIndex][0],
						(PVOID)&paAtc->d.tr.nText[0], TRACEBLENGTH_ATC);
				c_atcch.ta.wiTc = (WORD)paAtc->d.tr.nIndex + 1;
				pTcx->SetTraceProgress(TRACEID_ATC, 0,
						CalcProgress(c_atcch.ta.wiTc, c_atcch.ta.wDev));
				//MSGLOG("[TRACE]Atc block = %d\n", c_atcch.ta.wBlockiTc);
				if (c_atcch.ta.wiTc >= c_atcch.ta.wDev) {
					pTcx->SaveTrace(TRACEID_ATC, 0, (BYTE*)&c_atcch.d[0][0], (DWORD)(c_atcch.ta.wDev * TRACEBLENGTH_ATC));
					c_atcch.ta.nState = TRACESTATE_END;
					if (c_pTcRef->real.cond.nActiveAtc > 0)
						c_pTcRef->real.nLocalDataType[c_pTcRef->real.cond.nActiveAtc - 1] = DATATYPE_NORMAL;
				}
			}
		}
	}
	else if (paAtc->nFlow == LOCALFLOW_NORMAL) {
		WORD w = (WORD)(paAtc->d.st.tdws.a >> 4);
		if (w > MAXTRACELENGTH_ATC)	w = MAXTRACELENGTH_ATC;
		WORD v = w;
		w *= sizeof(ATCSTABLE);		// 8(event) * 30(sampling) * 20(data)
		WORD leng = w / TRACEBLENGTH_ATC;
		if (w % 100)	++ leng;
		if (leng != c_atcch.ta.wCur) {
			MSGLOG("[TRACE]ATC block = %d\n", leng);
			c_atcch.ta.wCur = leng;
			c_atcch.ta.nDeb = DEB_TRACING;
		}
		else if (c_atcch.ta.nDeb != 0 && -- c_atcch.ta.nDeb == 0) {
			c_atcch.ta.wDev = c_atcch.ta.wCur;
			c_atcch.ta.wLength = v;
			pTcx->SetTraceLength(TRACEID_ATC, 0, c_atcch.ta.wLength);
			MSGLOG("[TRACE]Atc has %d index.\r\n", c_atcch.ta.wDev);
		}
	}
}

void CChaser::AtoTraceCtrl(PATOAINFO paAto)
{
	if (paAto == NULL)	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	if (paAto->nFlow == LOCALFLOW_TRACE) {
		if (c_atoch.ta.nState == TRACESTATE_LOAD) {
			//MSGLOG("[TRACE]Ato capture %d of %d\n", paAto->d.tr.nBlock, c_atoch.ta.wBlockDev);
			if ((WORD)paAto->d.tr.nIndex < c_atoch.ta.wDev) {
				memcpy((PVOID)&c_atoch.d[paAto->d.tr.nIndex][0],
						(PVOID)&paAto->d.tr.nText[0], TRACEBLENGTH_ATO);
				c_atoch.ta.wiTc = (WORD)paAto->d.tr.nIndex + 1;
				pTcx->SetTraceProgress(TRACEID_ATO, 0,
						CalcProgress(c_atoch.ta.wiTc, c_atoch.ta.wDev));
				//MSGLOG("[TRACE]Ato block = %d\n", c_atoch.ta.wBlockiTc);
				if (c_atoch.ta.wiTc >= c_atoch.ta.wDev) {
					pTcx->SaveTrace(TRACEID_ATO, 0, (BYTE*)&c_atoch.d[0][0], (DWORD)(c_atoch.ta.wDev * TRACEBLENGTH_ATO));
					c_atoch.ta.nState = TRACESTATE_END;
					c_pTcRef->real.nLocalDataType[LOCALID_ATO] = DATATYPE_NORMAL;
				}
			}
		}
	}
	else if (paAto->nFlow == LOCALFLOW_NORMAL) {
		WORD w = (WORD)(paAto->d.st.info.a & 0xf);
		if (w > MAXTRACELENGTH_ATO)	w = MAXTRACELENGTH_ATO;
		WORD v = w;
		w *= sizeof(ATOSTABLE);		// 8(event) * 30(sampling) * 28(data)
		WORD leng = w / TRACEBLENGTH_ATO;
		if (w % 100)	++ leng;
		if (leng != c_atoch.ta.wCur) {
			MSGLOG("[TRACE]ATO block = %d\n", leng);
			c_atoch.ta.wCur = leng;
			c_atoch.ta.nDeb = DEB_TRACING;
		}
		else if (c_atoch.ta.nDeb != 0 && -- c_atoch.ta.nDeb == 0) {
			c_atoch.ta.wDev = c_atcch.ta.wCur;
			c_atoch.ta.wLength = v;
			pTcx->SetTraceLength(TRACEID_ATO, 0, c_atoch.ta.wLength);
			MSGLOG("[TRACE]Ato has %d index.\r\n", c_atoch.ta.wDev);
		}
	}
}

BOOL CChaser::Link()
{
	//if ((c_pTcRef->real.nObligation & OBLIGATION_TOP) != OBLIGATION_TOP)	return FALSE;
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) ||
			!(c_pTcRef->real.nObligation & OBLIGATION_HEAD ||
			!(c_pTcRef->real.nObligation & OBLIGATION_MAIN)))
		return FALSE;
	if (!(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL))	return FALSE;

	CTcx* pTcx = (CTcx*)c_pParent;

	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD deves = pTcx->GetCarDevices(n);
		BOOL bus = pTcx->GetRealBusCondition(n);

		if (deves & DEVBEEN_SIV) {
			if (bus && pTcx->CheckSivAddr(&c_pDoz->ccs[n].real.eSiv)) {
				if (c_pDoz->ccs[n].real.eSiv.nFlow == DEVFLOW_TRACEA) {
					if (c_sivch[n].ta.nState == TRACESTATE_LOAD) {
						if (c_pDoz->ccs[n].real.eSiv.d.tr.wIndex < c_sivch[n].ta.wDev && c_pDoz->ccs[n].real.eSiv.d.tr.wIndex < 0xffff) {
							memcpy((PVOID)&c_sivch[n].d[c_pDoz->ccs[n].real.eSiv.d.tr.wIndex][0],
										(PVOID)&c_pDoz->ccs[n].real.eSiv.d.tr.nText[0], TRACEBLENGTH_SIV);
							c_sivch[n].ta.wiTc = c_pDoz->ccs[n].real.eSiv.d.tr.wIndex + 1;
							pTcx->SetTraceProgress(TRACEID_SIV, n,
									CalcProgress(c_sivch[n].ta.wiTc, c_sivch[n].ta.wDev));
							if (c_sivch[n].ta.wiTc >= c_sivch[n].ta.wDev) {
								pTcx->SaveTrace(TRACEID_SIV, n, (BYTE*)&c_sivch[n].d[0][0], (DWORD)(c_sivch[n].ta.wDev * TRACEBLENGTH_SIV));
								c_sivch[n].ta.nState = TRACESTATE_END;
								c_pTcRef->real.wSivTrace &= ~(1 << n);
							}
						}
					}
				}
				else if (c_pDoz->ccs[n].real.eSiv.nFlow == DEVFLOW_NORMALA) {
					WORD w = pTcx->ToHex(c_pDoz->ccs[n].real.eSiv.d.st.st.nTrace);
					if (w > MAXTRACELENGTH_SIV)	w = MAXTRACELENGTH_SIV;
					WORD v = w;
					DWORD dw = (DWORD)w * (DWORD)sizeof(SIVSTABLE);
					WORD leng = (WORD)(dw / TRACEBLENGTH_SIV);
					if (dw % 100)	++ leng;
					if (leng != c_sivch[n].ta.wCur) {
						c_sivch[n].ta.wCur = leng;
						c_sivch[n].ta.nDeb = DEB_TRACING;
					}
					else if (c_sivch[n].ta.nDeb != 0 && -- c_sivch[n].ta.nDeb == 0) {
						c_sivch[n].ta.wDev = c_sivch[n].ta.wCur;
						c_sivch[n].ta.wLength = v;
						pTcx->SetTraceLength(TRACEID_SIV, n, c_sivch[n].ta.wLength);
						MSGLOG("[TRACE]Siv has %d index.\r\n", c_sivch[n].ta.wDev);
					}
				}
			}
		}

		if (deves & DEVBEEN_V3F) {
			if (bus && pTcx->CheckV3fAddr(&c_pDoz->ccs[n].real.eV3f)) {
				if (c_pDoz->ccs[n].real.eV3f.nFlow == DEVFLOW_TRACEA) {
					if (c_v3fch[n].ta.nState == TRACESTATE_LOAD) {
						if (c_pDoz->ccs[n].real.eV3f.d.tr.wIndex < c_v3fch[n].ta.wDev && c_pDoz->ccs[n].real.eV3f.d.tr.wIndex < 0xffff) {
							memcpy((PVOID)&c_v3fch[n].d[c_pDoz->ccs[n].real.eV3f.d.tr.wIndex][0],
									(PVOID)&c_pDoz->ccs[n].real.eV3f.d.tr.nText[0], TRACEBLENGTH_V3F);
							c_v3fch[n].ta.wiTc = c_pDoz->ccs[n].real.eV3f.d.tr.wIndex + 1;
							pTcx->SetTraceProgress(TRACEID_V3F, n,
									CalcProgress(c_v3fch[n].ta.wiTc, c_v3fch[n].ta.wDev));
							if (c_v3fch[n].ta.wiTc >= c_v3fch[n].ta.wDev) {
								pTcx->SaveTrace(TRACEID_V3F, n, (BYTE*)&c_v3fch[n].d[0][0], (DWORD)(c_v3fch[n].ta.wDev * TRACEBLENGTH_V3F));
								c_v3fch[n].ta.nState = TRACESTATE_END;
								c_pTcRef->real.wV3fTrace &= ~(1 << n);
							}
						}
					}
				}
				else if (c_pDoz->ccs[n].real.eV3f.nFlow == DEVFLOW_NORMALA) {
					WORD w = (DWORD)c_pDoz->ccs[n].real.eV3f.d.st.st.trace.a & 0xf;
					if (w > MAXTRACELENGTH_V3F)	w = MAXTRACELENGTH_V3F;
					WORD v = w;
					DWORD dw = (DWORD)w * (DWORD)sizeof(V3FSTABLE);
					WORD leng = (WORD)(dw / TRACEBLENGTH_V3F);
					if (dw % 100)	++ leng;
					if (leng != c_v3fch[n].ta.wCur) {
						c_v3fch[n].ta.wCur = leng;
						c_v3fch[n].ta.nDeb = DEB_TRACING;
					}
					else if (c_v3fch[n].ta.nDeb != 0 && -- c_v3fch[n].ta.nDeb == 0) {
						c_v3fch[n].ta.wDev = c_v3fch[n].ta.wCur;
						c_v3fch[n].ta.wLength = v;
						pTcx->SetTraceLength(TRACEID_V3F, n, c_v3fch[n].ta.wLength);
						MSGLOG("[TRACE]V3f has %d index.\r\n", c_v3fch[n].ta.wDev);
					}
				}
			}
		}

		if (deves & DEVBEEN_ECU) {
			if (bus & pTcx->CheckEcuAddr(&c_pDoz->ccs[n].real.aEcu)) {
				if (c_pDoz->ccs[n].real.aEcu.nFlow == ECUFLOW_TDTA) {
					if (c_ecuch[n].ta.nState == TRACESTATE_LOAD) {
						memcpy((PVOID)&c_ecuch[n].d.c[c_ecuch[n].ta.wiTc][0],
								(PVOID)&c_pDoz->ccs[n].real.aEcu.d.tr.n[0], sizeof(TAECU));
						++ c_ecuch[n].ta.wiTc;
						pTcx->SetTraceProgress(TRACEID_ECU, n,
								CalcProgress(c_ecuch[n].ta.wiTc, c_ecuch[n].ta.wDev));
						if (c_ecuch[n].ta.wiTc >= c_ecuch[n].ta.wDev) {
							pTcx->SaveTrace(TRACEID_ECU, n, (BYTE*)&c_ecuch[n].d.c[0][0], (DWORD)(c_ecuch[n].ta.wDev * sizeof(TAECU)));
							c_ecuch[n].ta.nState = TRACESTATE_END;
							c_pTcRef->real.wEcuTrace &= ~(1 << n);
						}
					}
				}
				else if (c_pDoz->ccs[n].real.aEcu.nFlow != ECUFLOW_TDTA) {
					WORD w = (WORD)BitIndex(c_pDoz->ccs[n].real.aEcu.d.st.nTrace & 0x1f);
					if (w > MAXTRACELENGTH_ECU)	w = MAXTRACELENGTH_ECU;
					WORD v = w;
					w *= MAXBLOCK_ECU;
					if (w != c_ecuch[n].ta.wCur) {
						c_ecuch[n].ta.wCur = w;
						c_ecuch[n].ta.nDeb = DEB_TRACING;
					}
					else if (c_ecuch[n].ta.nDeb != 0 && -- c_ecuch[n].ta.nDeb == 0) {
						c_ecuch[n].ta.wDev = c_ecuch[n].ta.wCur;
						c_ecuch[n].ta.wLength = v;
						pTcx->SetTraceLength(TRACEID_ECU, n, c_ecuch[n].ta.wLength);
						MSGLOG("[TRACE]Ecu has %d index.\r\n", c_ecuch[n].ta.wDev);
					}
				}
			}
		}
	}
	if (c_pTcRef->real.wTraceDev == 0)	return FALSE;
	if ((c_pTcRef->real.wTraceDev & 1) && !(c_atcch.ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 2) && !(c_atoch.ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 4) && !(c_sivch[__SIVA].ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 8) && !(c_sivch[__SIVB].ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 0x10) && !(c_v3fch[__V3FA].ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 0x20) && !(c_v3fch[__V3FB].ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 0x40) && !(c_v3fch[__V3FC].ta.nState & TRACESTATE_END))
		return FALSE;
	if ((c_pTcRef->real.wTraceDev & 0x80) && !(c_v3fch[__V3FD].ta.nState & TRACESTATE_END))
		return FALSE;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		if ((c_pTcRef->real.wTraceDev & (0x100 << n)) && !(c_ecuch[n].ta.nState & TRACESTATE_END))
			return FALSE;
	}
	TraceInitial();
	return TRUE;
}
