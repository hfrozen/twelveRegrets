/*
 * CExt.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CExt.h"
#include "CTcx.h"

CExt::CExt()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
}

CExt::~CExt()
{
}

void CExt::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pDoz = pTcx->GetTcDoz();
	pTcx->TimeRegister(&c_pTcRef->real.ext.wTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ext.wChgTimer);
	pTcx->TimeRegister(&c_pTcRef->real.ext.wEskMonTimer);
}

void CExt::Check()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (!pTcx->GetRealBusCondition(_REALCC(__SIVA)) ||
			!pTcx->GetRealBusCondition(_REALCC(__SIVB)) ||
			!pTcx->GetRealBusCondition(_REALCC(__ESK))) {
		if ((c_pTcRef->real.ext.nStep == EXTSTEP_WAITESK ||
				c_pTcRef->real.ext.nStep == EXTSTEP_POSTLUDE) &&
				c_pTcRef->real.ext.wEskMonTimer == 0) {
			c_pTcRef->real.ext.nStep = EXTSTEP_PRELUDE;
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.esk = FALSE;
			CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, 0);
			SHOT(SCENE_ID_HEADCAR, 239);
			LoadControl(FALSE);
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.lrr = FALSE;
			CLRTBIT(OWN_SPACEA, TCDOB_ESLP);
		}
		return;
	}

	UCURV cause = 0;
	switch (c_pTcRef->real.ext.nStep) {
	case EXTSTEP_PRELUDE :
		if ((cause = CheckEnterExt()) != 0) {
			c_pTcRef->real.ext.wCause = cause;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTSTABILITY);
			c_pTcRef->real.ext.nStep = EXTSTEP_STABILITY;
			CUT(SCENE_ID_HEADCAR, 239);
		}
		break;

	case EXTSTEP_STABILITY:
		if (c_pTcRef->real.ext.wTimer == 0) {
			//c_pDoz->tcs[OWN_SPACEA].real.srCmm.req.b.lrr = TRUE;
			LoadControl(TRUE);
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITUNSIVK);
			c_pTcRef->real.ext.nStep = EXTSTEP_WAITUNSIVK;
		}
		else if (CheckEnterExt() == 0) {
			c_pTcRef->real.ext.nStep = EXTSTEP_PRELUDE;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, 0);
		}
		break;

	case EXTSTEP_WAITUNSIVK :
		if (CheckExitExt()) {
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, 0);
			c_pTcRef->real.ext.nStep = EXTSTEP_PRELUDE;
		}
		else if ((!(c_pTcRef->real.ext.wCause & 0x80) && !GETCBIT(__SIVA, CCDIB_SIVK)) ||
			((c_pTcRef->real.ext.wCause & 0x80) && !GETCBIT(__SIVB, CCDIB_SIVK))) {
			//c_pDoz->tcs[OWN_SPACEA].real.srCmm.req.b.lrr = TRUE;
			//LoadControl(TRUE);
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITREDUCTION);
			c_pTcRef->real.ext.nStep = EXTSTEP_WAITREDUCTION;
			CUT(__SIVA, 230);
			CUT(__SIVB, 230);
		}
		else if (c_pTcRef->real.ext.wTimer == 0) {
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITUNSIVK);
			SHOT((c_pTcRef->real.ext.wCause & 0x80) ? __SIVB : __SIVA, 230);
		}
		break;

	case EXTSTEP_WAITREDUCTION :
		if (CheckExitExt()) {
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, 0);
			c_pTcRef->real.ext.nStep = EXTSTEP_PRELUDE;
		}
		else if (c_pTcRef->real.ext.wTimer == 0) {
			SETOBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
			//c_pTcRef->real.ext.nState |= EXTSTATE_REDUCTIONA;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITESK);
			pTcx->InterTrigger(&c_pTcRef->real.ext.wEskMonTimer, TIME_ESKMON);
			c_pTcRef->real.ext.nStep = EXTSTEP_WAITESK;
		}
		//if (c_pDoz->tcs[OWN_SPACEA].real.saHtc.info.b.eskr) {
		//	SETOBIT(OWN_SPACEA, T1, CCDOB_ESK);
		//	pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITESK);
		//	c_pTcRef->real.ext.nStep = EXTSTEP_WAITESK;
		//}
		break;

	case EXTSTEP_WAITESK :
		if (CheckExitExt()) {
			c_pTcRef->real.ext.nStep = EXTSTEP_POSTLUDE;
		}
		else if (GETCBIT(__ESK, CCDIB_ESKC)) {
			SETTBIT(OWN_SPACEA, TCDOB_ESLP);
			c_pTcRef->real.ext.nState |= EXTSTATE_REDUCTIONA;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, 0);
			c_pTcRef->real.ext.nStep = EXTSTEP_POSTLUDE;
			CUT(__ESK, 232);
		}
		else if (c_pTcRef->real.ext.wTimer == 0) {
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITESK);
			SHOT(__ESK, 232);
		}
		pTcx->InterTrigger(&c_pTcRef->real.ext.wEskMonTimer, TIME_ESKMON);
		break;

	case EXTSTEP_POSTLUDE :
		if (CheckExitExt()) {
			//c_pDoz->tcs[OWN_SPACEA].real.srCmm.req.b.esk = FALSE;
			CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
			//c_pTcRef->real.ext.nState &= ~EXTSTATE_REDUCTIONA;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITUNESK);
			c_pTcRef->real.ext.nStep = EXTSTEP_WAITUNESK;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wEskMonTimer, 0);
		}
		else if (((c_pTcRef->real.ext.wCause & 0x80) && GETCBIT(__SIVB, CCDIB_SIVK)) ||
				(!(c_pTcRef->real.ext.wCause & 0x80) && GETCBIT(__SIVA, CCDIB_SIVK))) {
			c_pTcRef->real.ext.nStep = EXTSTEP_WAITUNESK;
			CLROBIT(OWN_SPACEA, __ESK, CCDOB_ESK);
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITUNESK);
			SHOT((c_pTcRef->real.ext.wCause & 0x80) ? __SIVB : __SIVA, 235);
			pTcx->InterTrigger(&c_pTcRef->real.ext.wEskMonTimer, 0);
		}
		else {
			pTcx->InterTrigger(&c_pTcRef->real.ext.wEskMonTimer, TIME_ESKMON);
		}
		break;

	case EXTSTEP_WAITUNESK :
		if (!GETCBIT(__ESK, CCDIB_ESKC)) {
			c_pTcRef->real.ext.nState &= ~EXTSTATE_REDUCTIONA;
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, 0);
			c_pTcRef->real.ext.nStep = EXTSTEP_PRELUDE;
			LoadControl(FALSE);
			//c_pDoz->tcs[OWN_SPACEA].real.srCmm.req.b.lrr = FALSE;
			CLRTBIT(OWN_SPACEA, TCDOB_ESLP);
			CUT(__ESK, 233);
		}
		else if (c_pTcRef->real.ext.wTimer == 0) {
			pTcx->InterTrigger(&c_pTcRef->real.ext.wTimer, TIME_EXTWAITUNESK);
			SHOT(__ESK, 233);
		}
		break;

	default :
		break;
	}
}

UCURV CExt::CheckEnterExt()
{
	UCURV sivf;
	if ((sivf = ScanExtNode(__SIVA)) == 0) {
		if ((sivf = ScanExtNode(__SIVB)) != 0)
			sivf |= 0x80;

	}
	return sivf;
}

BOOL CExt::CheckExitExt()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (((c_pTcRef->real.ext.wCause & 0x80) &&
		!GETCBIT(__SIVB, CCDIB_IVF) && GETCBIT(__SIVB, CCDIB_IVS) && !GETCBIT(__SIVB, CCDIB_SIVFR)) ||	// Check SIVK ?????
		(!(c_pTcRef->real.ext.wCause & 0x80) &&
		!GETCBIT(__SIVA, CCDIB_IVF) && GETCBIT(__SIVA, CCDIB_IVS) && !GETCBIT(__SIVA, CCDIB_SIVFR))) {
		if ((c_pTcRef->real.ext.wCause & 0x7f) == (CCDIB_SIVFR + 1))
			SHOT((c_pTcRef->real.ext.wCause & 0x80) ? __SIVB : __SIVA, 236);
		else if ((c_pTcRef->real.ext.wCause & 0x7f) == (CCDIB_IVF + 1))
			SHOT((c_pTcRef->real.ext.wCause & 0x80) ? __SIVB : __SIVA, 237);
		else if ((c_pTcRef->real.ext.wCause & 0x7f) == (CCDIB_IVS + 1))
			SHOT((c_pTcRef->real.ext.wCause & 0x80) ? __SIVB : __SIVA, 238);
		return TRUE;
	}
	// Modified 2013/11/02
	if (!IsPantoUp()) {
		SHOT(SCENE_ID_HEADCAR, 240);
		return TRUE;
	}
	return FALSE;
}

UCURV CExt::ScanExtNode(BYTE nCcid)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	UCURV extf = 0;
	if (GETCBIT(nCcid, CCDIB_SIVFR)) {
		extf = CCDIB_SIVFR + 1;
		SHOT(nCcid, 246);
	}
	else {
		if (GETCBIT(nCcid, CCDIB_IVF)) {
			extf = CCDIB_IVF + 1;
			SHOT(nCcid, 228);
		}
		else {
			if (!GETCBIT(nCcid, CCDIB_IVS)) {
				extf = CCDIB_IVS + 1;
				SHOT(nCcid, 229);
			}
			else {
				if (c_pDoz->ccs[nCcid].real.eSiv.d.st.st.test.b.chkx)	// !!!!! need CheckSivReply()
					SHOT(nCcid, 225);
				else	CUT(nCcid, 225);
				CUT(nCcid, 229);
				// Modified 2013/11/02
				//CUTS(nCcid, 236, 238);
				CUTS(nCcid, 236, 240);
			}
			CUT(nCcid, 228);
		}
		CUT(nCcid, 246);
	}
	return extf;
}

void CExt::LoadControl(BOOL cmd)
{
	//if (cmd)	LoadReduction(FALSE, TRUE);
	//else	LoadReduction(TRUE, TRUE);
	CTcx* pTcx = (CTcx*)c_pParent;
	if (cmd) {
		// Modified 2012/11/12
		//if (!GETTBIT(OWN_SPACEA, TCDIB_LCDK))
		if (GETTBIT(OWN_SPACEA, TCDIB_LCDK))
			LoadReduction(FALSE, TRUE);	// lcd1 = off, lcd2 = on
		else	LoadReduction(FALSE, FALSE);	// lcd1 = off, lcd2 = off
		c_pTcRef->real.ext.nState |= EXTSTATE_REDUCTIONB;
		if (GETTBIT(OWN_SPACEA, TCDIB_LCS1))
			pTcx->ObsCtrl(FALSE, CCDOB_K1L, ALLCAR_FLAGS);
		pTcx->InterTrigger(&c_pTcRef->real.ext.wChgTimer, TIME_CHGREDUCTION);
		SHOT(SCENE_ID_HEADCAR, 241);
	}
	else {
		// Modified 2012/11/12
		//if (!GETTBIT(OWN_SPACEA, TCDIB_LCDK))
		if (GETTBIT(OWN_SPACEA, TCDIB_LCDK))
			LoadReduction(TRUE, TRUE);	// lcd1 = on, lcd2 = on;
		else	LoadReduction(FALSE, FALSE);	// lcd1 = off, lcd2 = off
		c_pTcRef->real.ext.nState &= ~EXTSTATE_REDUCTIONB;
		// Modified 2012/12/12
		c_pTcRef->real.ext.nState |= EXTSTATE_SCANESCAPE;
		if (GETTBIT(OWN_SPACEA, TCDIB_LCS1))
			pTcx->ObsCtrl(TRUE, CCDOB_K1L, ALLCAR_FLAGS);
		pTcx->InterTrigger(&c_pTcRef->real.ext.wChgTimer, TIME_CHGREDUCTION);
		CUT(SCENE_ID_HEADCAR, 241);
	}
}

void CExt::LoadReduction(BOOL cmd1, BOOL cmd2)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->LcdCtrl(FALSE, cmd1);
	pTcx->LcdCtrl(TRUE, cmd2);
}
