/*
 * CLook.cpp
 *
 *  Created on: 2011. 2. 21
 *      Author: Che
 */

#include "math.h"

#include "CLook.h"
#include "CTcx.h"

CLook::CLook()
{
	c_pParent = NULL;
	c_pLcFirm = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
	c_bPowerOn = TRUE;
	//c_wHeavy = 0;
}

CLook::~CLook()
{
}

const CLook::NOTCHFORM CLook::c_notchA[] = {
/*B8*/	{ 327, 605, -1024, 121, 725 },
/*B7*/	{ 606, 769, -TEBEMAX_BRAKE, 100, 725 },
/*B6*/	{ 770, 925, -720, 87, 642 },
/*B5*/	{ 926, 1105, -624, 74, 557 },
/*B4*/	{ 1106, 1269, -501, 59, 471 },
/*B3*/	{ 1270, 1424, -388, 46, 384 },
/*B2*/	{ 1425, 1588, -276, 33, 297 },
/*B1*/	{ 1589, 1719, -163, 19, 210 },
/*N */	{ 1720, 2130, 0, 0, 116 },
/*p.5*/	{ 2131, 2277, 112, 11, 204 },
/*P1*/	{ 2278, 2531, 286, 28, 296 },
/*P2*/	{ 2532, 2884, 528, 51, 492 },
/*P3*/	{ 2885, 3236, 765, 75, 687 },
/*P4*/	{ 3237, 3613, TEBEMAX_POWER, 100, 900 },
		{ 0, 0, 0, 0, 0 }
};

// Modified 2013/01/28
const CLook::NOTCHFORM CLook::c_notchB[] = {
/*B8*/	{ 246, 535, -1024, 121, 725 },
/*B7*/	{ 536, 744, -TEBEMAX_BRAKE, 100, 725 },
/*B6*/	{ 745, 904, -720, 87, 642 },
/*B5*/	{ 905, 1071, -624, 74, 557 },
/*B4*/	{ 1072, 1243, -501, 59, 471 },
/*B3*/	{ 1244, 1403, -388, 46, 384 },
/*B2*/	{ 1404, 1563, -276, 33, 297 },
/*B1*/	{ 1564, 1804, -163, 19, 210 },
/*N */	{ 1805, 2091, 0, 0, 116 },
/*p.5*/	{ 2092, 2345, 112, 11, 204 },
/*P1*/	{ 2346, 2648, 286, 28, 296 },
/*P2*/	{ 2649, 3000, 528, 51, 492 },
/*P3*/	{ 3001, 3365, 765, 75, 687 },
/*P4*/	{ 3366, 3718, TEBEMAX_POWER, 100, 900 },
		{ 0, 0, 0, 0, 0 }
};

void CLook::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
	pTcx->TimeRegister(&c_pTcRef->real.bat.wTimer);
	pTcx->TimeRegister(&c_pTcRef->real.speed.wZvTimer);
}

void CLook::GetMascon(PMCDSTICK pDs)
{
	pDs->inst.v = READANALOG(OWN_SPACEA, c_pTcRef->real.cond.nAichDsv);
	BOOL find = FALSE;
	UCURV n;
	if (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY) {
		for (n = 0; c_notchA[n].min != 0; n++) {
			if (pDs->inst.v < c_notchA[n].min)
				break;
			if (pDs->inst.v >= c_notchA[n].min && pDs->inst.v <= c_notchA[n].max) {
				find = TRUE;
				break;
			}
		}

		if (find) {
			pDs->nIndex = n;
			pDs->inst.vTbeB = c_notchA[n].vTbeB;
			pDs->inst.percent = c_notchA[n].percent;
			pDs->bValid = TRUE;
		}
		else {
			//if (n == 0 || c_notch[n].min == 0)
			pDs->bValid = FALSE;
		}
	}
	else {
		for (n = 0; c_notchB[n].min != 0; n++) {
			if (pDs->inst.v < c_notchB[n].min)
				break;
			if (pDs->inst.v >= c_notchB[n].min && pDs->inst.v <= c_notchB[n].max) {
				find = TRUE;
				break;
			}
		}

		if (find) {
			pDs->nIndex = n;
			pDs->inst.vTbeB = c_notchB[n].vTbeB;
			pDs->inst.percent = c_notchB[n].percent;
			pDs->bValid = TRUE;
		}
		else {
			//if (n == 0 || c_notch[n].min == 0)
			pDs->bValid = FALSE;
		}
	}
}

// Modified 2012/03/05 ... begin
//void CLook::GetMascon(PINSTRUCTA pInst, BYTE nIndex)
//{
//	if (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY) {
//		pInst->v = (c_notchA[nIndex].max - c_notchA[nIndex].min) / 2 + c_notchA[nIndex].min;
//		pInst->vTbeB = c_notchA[nIndex].vTbeB;
//		pInst->percent = c_notchA[nIndex].percent;
//		//pInst->bValid = TRUE;
//	}
//	else {
//		pInst->v = (c_notchB[nIndex].max - c_notchB[nIndex].min) / 2 + c_notchB[nIndex].min;
//		pInst->vTbeB = c_notchB[nIndex].vTbeB;
//		pInst->percent = c_notchB[nIndex].percent;
//		//pInst->bValid = TRUE;
//	}
//}
// ... end

WORD CLook::GetNotchRescue(BYTE n)
{
	if (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY)
		return c_notchA[n].rescue;
	else	return c_notchB[n].rescue;
}

void CLook::CheckVoltage()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (!GETTBIT(OWN_SPACEA, TCDIB_BATKN))
		SHOT(SCENE_ID_HEADCAR, 528);
	else	CUT(SCENE_ID_HEADCAR, 528);
	if (!GETTBIT(OWN_SPACEA, TCDIB_BATK))
		SHOT(SCENE_ID_HEADCAR, 529);
	else	CUT(SCENE_ID_HEADCAR, 529);

	DWORD v = (DWORD) READANALOG(OWN_SPACEA, c_pTcRef->real.cond.nAichBat);
	c_pTcRef->real.bat.v = v * BATVOLTAGE_LGCRANGE / BATVOLTAGE_PSCRANGE;
	if (c_pTcRef->real.bat.v > BATVOLTAGE_NORMAL) {
		CLRTBIT(OWN_SPACEA, TCDOB_BATK);
		CUT(SCENE_ID_HEADCAR, 226);
		CUT(SCENE_ID_HEADCAR, 224);
	}
	else if (c_pTcRef->real.bat.v <= BATVOLTAGE_WARNING ||
			!GETTBIT(OWN_SPACEA, TCDIB_BATKN)) {
		SHOT(SCENE_ID_HEADCAR, 226);
		if (c_pTcRef->real.nDuty < 2) {
			if (c_pTcRef->real.bat.v <= BATVOLTAGE_FAULT
					&& GETTBIT(OWN_SPACEA, TCDIB_BATKN)) {
				if (!c_pTcRef->real.bat.bState) {
					pTcx->InterTrigger(&c_pTcRef->real.bat.wTimer, TIME_BATFAULT);
					c_pTcRef->real.bat.bState = TRUE;
				}
				else if (c_pTcRef->real.bat.wTimer == 0) {
					SHOT(SCENE_ID_HEADCAR, 224);
					SETTBIT(OWN_SPACEA, TCDOB_BATK);	// all power off
				}
			}
			else {
				c_pTcRef->real.bat.bState = FALSE;
				pTcx->InterTrigger(&c_pTcRef->real.bat.wTimer, 0);
			}
		}
	}
}

void CLook::CtrlEach()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if ((c_pTcRef->real.op.nLightMonitor & LIGHT_MONITOR) &&
			c_pTcRef->real.op.wLightMonTimer == 0) {
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			// Modified 2012/11/21
			if (pTcx->GetBusCondition(n + CAR_CC)) {
				if (c_pTcRef->real.op.nLightMonitor & LIGHT_ON) {
					if (!GETCBIT(n, CCDIB_LK1)) {
						if (IsWakeup(n))	SHOT(n, 521);
					}
					else	CUT(n, 521);
					if (!GETCBIT(n, CCDIB_LK2)) {
						if (IsWakeup(n))	SHOT(n, 523);
					}
					else	CUT(n, 523);
				}
				else {
					if (GETCBIT(n, CCDIB_LK1)) {
						if (IsWakeup(n))	SHOT(n, 522);
					}
					else	CUT(n, 522);
					if (GETCBIT(n, CCDIB_LK2)) {
						if (IsWakeup(n))	SHOT(n, 524);
					}
					else	CUT(n, 524);
				}
			}
		}
		//c_pTcRef->real.op.nLightMonitor &= ~LIGHT_MONITOR;
	}

	// Modified 2012/11/14 ... cause no time delay when tc exchanging, occur 521...
	//if (GETTBIT(OWN_SPACEA, TCDIB_LCS1)) {
	if (GETTBIT(CAR_HEAD, TCDIB_LCS1)) {
		if (!(c_pTcRef->real.op.nLightMonitor & LIGHT_ON)) {
			pTcx->InterTrigger(&c_pTcRef->real.op.wLightMonTimer, TIME_LIGHTMON);
			CUT(SCENE_ID_ALLCAR, 522);
			CUT(SCENE_ID_ALLCAR, 524);
		}
		if (!(c_pTcRef->real.ext.nState & EXTSTATE_REDUCTIONB))
			pTcx->ObsCtrl(TRUE, CCDOB_K1L, ALLCAR_FLAGS);
		pTcx->ObsCtrl(TRUE, CCDOB_K2L, ALLCAR_FLAGS);
		c_pTcRef->real.op.nLightMonitor = LIGHT_MONITOR | LIGHT_ON;
	}
	else {
		if (c_pTcRef->real.op.nLightMonitor & LIGHT_ON) {
			pTcx->InterTrigger(&c_pTcRef->real.op.wLightMonTimer, TIME_LIGHTMON);
			CUT(SCENE_ID_ALLCAR, 521);
			CUT(SCENE_ID_ALLCAR, 523);
		}
		pTcx->ObsCtrl(FALSE, CCDOB_K1L, ALLCAR_FLAGS);
		pTcx->ObsCtrl(FALSE, CCDOB_K2L, ALLCAR_FLAGS);
		c_pTcRef->real.op.nLightMonitor = LIGHT_MONITOR;
	}
	if (!GETTBIT(OWN_SPACEA, TCDIB_LCS2))
		pTcx->ObsCtrl(TRUE, CCDOB_K3L, ALLCAR_FLAGS);
	else	pTcx->ObsCtrl(FALSE, CCDOB_K3L, ALLCAR_FLAGS);

	// Modified 2012/11/12 ... begin
	//BOOL lcd = GETTBIT(OWN_SPACEA, TCDIB_LCDK) ? FALSE : TRUE;
	BOOL lcd = GETTBIT(OWN_SPACEA, TCDIB_LCDK) ? TRUE : FALSE;
	// ... end
	if (lcd) { // lcd on
		pTcx->LcdCtrl(FALSE, (c_pTcRef->real.ext.nState & EXTSTATE_REDUCTIONB) ? FALSE : TRUE); // lcd1 off-on
		pTcx->LcdCtrl(TRUE, TRUE); // lcd2 on
		// if reduction mode so do not
	}
	else {
		pTcx->LcdCtrl(FALSE, FALSE);
		pTcx->LcdCtrl(TRUE, FALSE);
	}

	if (c_pTcRef->real.nDuty < 2) {
		if (c_pTcRef->real.ca.fire.wStep > 0) {
			CtrlEachEmer(&c_pTcRef->real.ca.fire, FALSE);
			if (c_pTcRef->real.ca.fire.wStep == 0) {
				if (c_pTcRef->real.ca.emer.wRep != 0) {
					c_pTcRef->real.ca.emer.wRep = 0;
					c_pTcRef->real.ca.emer.wStep = 0;
				}
			}
		}
		if (c_pTcRef->real.ca.emer.wStep > 0) {
			CtrlEachEmer(&c_pTcRef->real.ca.emer, TRUE);
			if (c_pTcRef->real.ca.emer.wStep == 0) {
				if (c_pTcRef->real.ca.fire.wRep != 0) {
					c_pTcRef->real.ca.fire.wRep = 0;
					c_pTcRef->real.ca.fire.wStep = 0;
				}
			}
		}
	}
}

void CLook::CtrlEachEmer(PCAREMER pCe, BOOL bRef)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (pCe->wStep) {
	case 1:
		SETTBIT(OWN_SPACEA, TCDOB_BPA);
		pTcx->ToFlash(TCDOB_PAAK, TRUE);
		c_pTcRef->real.ca.wSwDeb = DEBOUNCE_SW;
		pTcx->InterTrigger(&c_pTcRef->real.ca.wPaapbTimer, TIME_PAAPB);
		++ pCe->wStep;
		break;
	case 2:
		if (GETTBIT(CAR_HEAD, TCDIB_PALS) || c_pTcRef->real.ca.wPaapbTimer == 0) {
			if (c_pTcRef->real.ca.wSwDeb > 0 && --c_pTcRef->real.ca.wSwDeb == 0) {
				CLRTBIT(OWN_SPACEA, TCDOB_BPA);
				pTcx->ToFlash(TCDOB_PAAK, FALSE);
				SETTBIT(OWN_SPACEA, TCDOB_PAAK);
				c_pTcRef->real.ca.wSwDeb = DEBOUNCE_SW;
				pTcx->InterTrigger(&c_pTcRef->real.ca.wPaapbTimer, TIME_PAAPBAGAIN);
				++ pCe->wStep;
				//if (bRef && GETTBIT(CAR_HEAD, TCDIB_PAAPB)) {
				//	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
				//		if (c_pTcRef->real.ca.emer.wBuf & (1 << n))	CUT(n, 551);
				//	}
				//}
			}
			SHOT(SCENE_ID_HEADCAR, 552);
		}
		else	c_pTcRef->real.ca.wSwDeb = DEBOUNCE_SW;
		break;
	case 3:
		if (!GETTBIT(CAR_HEAD, TCDIB_PALS)) {
			if (c_pTcRef->real.ca.wSwDeb > 0 && --c_pTcRef->real.ca.wSwDeb == 0) {
				c_pTcRef->real.ca.wSwDeb = DEBOUNCE_SW;
				++ pCe->wStep;
			}
		}
		else	c_pTcRef->real.ca.wSwDeb = DEBOUNCE_SW;
		break;
	case 4:
		if (GETTBIT(CAR_HEAD, TCDIB_PALS) || c_pTcRef->real.ca.wPaapbTimer == 0) {
			if (c_pTcRef->real.ca.wSwDeb > 0 && --c_pTcRef->real.ca.wSwDeb == 0) {
				CLRTBIT(OWN_SPACEA, TCDOB_PAAK);
				pTcx->ObsCtrl(FALSE, CCDOB_EIAL, ALLCAR_FLAGS);
				pCe->wRep = 0;
				pCe->wStep = 0;
				pTcx->InterTrigger(&c_pTcRef->real.ca.wPaapbTimer, 0);
				CUT(SCENE_ID_HEADCAR, 552);
				if (bRef) {
					for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
						if (c_pTcRef->real.ca.emer.wBuf & (1 << n))	CUT(n, 551);
					}
					if (!GETTBIT(CAR_HEAD, TCDIB_PALS))	SHOT(SCENE_ID_HEADCAR, 554);
				}
			}
		}
		else {
			c_pTcRef->real.ca.wSwDeb = DEBOUNCE_SW;
		}
		break;
	default:
		break;
	}
}

BOOL CLook::ModeHandle(BYTE mode)
{
	if (c_pTcRef->real.op.nMode == mode)	return FALSE;
	if ((c_pTcRef->real.op.nMode == OPMODE_AUTOREADY ||
			c_pTcRef->real.op.nMode == OPMODE_AUTO) && mode == OPMODE_AUTOREADY)
		return FALSE;	// auto, auto ready
	//if ((c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY || c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
	//		c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) && mode == OPMODE_DRIVERLESSREADY)
	//	return FALSE;	// driverless, driverless ready

	// Modified 2011/10/20 - cause drvless do off TCDOB_HCR
	//if ((c_pTcRef->real.nObligation & OBLIGATION_MAIN) &&
	//		(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
	//		c_pTcRef->real.op.nMode != OPMODE_DRIVERLESSREADY &&
	//		c_pTcRef->real.op.nMode != OPMODE_DRIVERLESS &&
	//		c_pTcRef->real.op.nMode != OPMODE_NON) {
	//	SETTBIT(OWN_SPACEA, TCDOB_HCR);
	//	c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_INHIBITA;
	//}

	CTcx* pTcx = (CTcx*)c_pParent;
	BYTE lamp;
	WORD flash;
	switch (mode) {
	case OPMODE_EMERGENCY :
		c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_ACTIVE;
		lamp = TCDOM_EMLP;
		flash = 0xffff;
		EDIT(SCENE_ID_HEADCAR, 601, 612, 609);
		break;
	case OPMODE_MANUAL :
		c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_ACTIVE;
		lamp = TCDOM_MMLP;
		flash = 0xffff;
		EDIT(SCENE_ID_HEADCAR, 601, 612, 603);
		// Modified 2012/11/26
		c_pTcRef->real.door.nCuri = 0;
		break;
	case OPMODE_YARD :
		c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_ACTIVE;
		lamp = TCDOM_YMLP;
		flash = 0xffff;
		EDIT(SCENE_ID_HEADCAR, 601, 612, 604);
		// Modified 2012/11/26
		c_pTcRef->real.door.nCuri = 0;
		break;
	case OPMODE_AUTOREADY :
		c_pTcRef->real.atoCmd.nCmd |= INSTCMD_ACTIVE;
		lamp = TCDOM_AMLP | TCDOM_AMLP1;
		flash = TCDOB_AUML1;
		EDIT(SCENE_ID_HEADCAR, 601, 612, 605);
		break;
	case OPMODE_AUTO :
		c_pTcRef->real.atoCmd.nCmd |= INSTCMD_ACTIVE;
		lamp = TCDOM_AMLP | TCDOM_AMLP1;
		flash = 0xffff;
		EDIT(SCENE_ID_HEADCAR, 601, 612, 602);
		// Modified 2012/11/26
		c_pTcRef->real.door.nCuri = 0;
		break;
	case OPMODE_DRIVERLESS :
		c_pTcRef->real.atoCmd.nCmd |= INSTCMD_ACTIVE;
		lamp = TCDOM_DMLP;
		flash = 0xffff;
		EDIT(SCENE_ID_HEADCAR, 601, 612, 601);
		break;
	case OPMODE_DRIVERLESSREADY :
		c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_ACTIVE;
		lamp = TCDOM_DMLP;
		flash = TCDOB_DRML;
		EDIT(SCENE_ID_HEADCAR, 601, 611, 612);
		break;
	default :
		c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_ACTIVE;
		lamp = TCDOM_EMLP | TCDOM_YMLP | TCDOM_MMLP | TCDOM_AMLP | TCDOM_DMLP | TCDOM_AMLP1;
		flash = 0;
		break;
	}

	c_pTcRef->real.op.nMode = mode;
	if (flash == 0)	OpModeLamp(lamp, LP_OFF);
	else {
		OpModeLamp(lamp, LP_ON);
		if (flash < 0xffff)	pTcx->ToFlash(flash, TRUE);
	}
	//if (flash > 0 && flash < 0xffff) {
	//	OpModeLamp(lamp, LP_OFF);
	//	pTcx->ToFlash(flash, TRUE);
	//	if (flashc > 0 && flashc < 0xffff)
	//		pTcx->ToFlash(flashc, TRUE);
	//}
	//else	OpModeLamp(lamp, flash != 0 ? LP_ON : LP_OFF);

	if (c_pTcRef->real.atoCmd.nCmd & INSTCMD_ACTIVE)
		SHOT(SCENE_ID_HEADCAR, 679);
	else	CUTS(SCENE_ID_HEADCAR, 623, 625);

	// Modified 2012/09/06
	// Appended 2012/03/29 ... begin
	//if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && c_pTcRef->real.op.nMode != OPMODE_MANUAL)
	//	c_pTcRef->real.op.nState &= ~OPERATESTATE_WITHPWM;
	// ... end

	return TRUE;
}

void CLook::OpModeLamp(BYTE lp, BYTE state)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->ToFlash(TCDOB_EMML, FALSE);
	pTcx->ToFlash(TCDOB_YAML, FALSE);
	pTcx->ToFlash(TCDOB_MAML, FALSE);
	pTcx->ToFlash(TCDOB_AUML, FALSE);
	pTcx->ToFlash(TCDOB_AUML1, FALSE);
	pTcx->ToFlash(TCDOB_DRML, FALSE);
	BYTE lamp = FEEDTBYTE(OWN_SPACEA, TCDO_MODELAMP);
	lamp &= ~TCDOM_MODELAMPES;
	if (state == LP_ON)	lamp |= lp;
	WRITETBYTE(OWN_SPACEA, TCDO_MODELAMP, lamp);
}

void CLook::NonMode(BYTE atcm, BYTE atci)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (IsNotRescue()) {
		ModeHandle(OPMODE_NON);
		CLRTBIT(OWN_SPACEA, TCDOB_DPLP);	// mode xch
		c_pTcRef->real.op.nAutoStep = 0;
		if (GetSettingBits(atcm) > 1)
			pTcx->ShotMainDev(SCENE_ID_HEADCAR, 608);
		else if (!atci) {
			if (c_pTcRef->real.op.wWaitAtcValid == 0) {
				pTcx->ShotMainDev(SCENE_ID_HEADCAR, 606);
				pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcValid, TIME_WAITATCVALID);
			}
		}
		else if (atcm == 0) {
			if (c_pTcRef->real.op.wWaitAtcMode == 0) {
				pTcx->ShotMainDev(SCENE_ID_HEADCAR, 607);
				pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, TIME_WAITATCMODE);
			}
		}
		else	pTcx->ShotMainDev(SCENE_ID_HEADCAR, 611);
	}
}

void CLook::ToDriverlessReady()
{
	 if (ModeHandle(OPMODE_DRIVERLESSREADY)) {
		CTcx* pTcx = (CTcx*)c_pParent;
		pTcx->EntryDriverless(FALSE);
	 }
}

void CLook::AbsoluteEmergency()
{
	ModeHandle(OPMODE_EMERGENCY);
	CLRTBIT(OWN_SPACEA, TCDOB_DPLP);	// mode xch
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, 0);
	c_pTcRef->real.op.nAutoStep = 0;
}

void CLook::ModeCtrl()
{
	// NOTE : this function must be run only on zero velocity and coast position !!!!!
	// tc1, tc0 always look at c_tcFirm[0], cause operation mode sync.
	// before, read di to c_tcFirm[0].nInput by master
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return;
	//if (!GETTBIT(OWN_SPACEA, TCDIB_C))	return;
	if (GETTBIT(OWN_SPACEA, TCDIB_P))	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	BYTE cabm = READTBYTE(OWN_SPACEA, TCDI_MODE) & TCDIM_ONLY;
	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	BYTE atcm = paAtc != NULL ? paAtc->d.st.mode.a : 0;
	BYTE atcs = paAtc != NULL ? paAtc->d.st.swi.a : 0;

	if (c_pTcRef->real.op.nCabm != cabm) {
		c_pTcRef->real.op.nCabm = cabm;
		c_pTcRef->real.op.nCabmDeb = DEB_OPMODE;
	}
	else if (c_pTcRef->real.op.nCabmDeb != 0)	-- c_pTcRef->real.op.nCabmDeb;

	if (c_pTcRef->real.op.nAtcMode != atcm) {
		c_pTcRef->real.op.nAtcMode = atcm;
		c_pTcRef->real.op.nAtcModeDeb = DEB_OPMODE;
	}
	else if (c_pTcRef->real.op.nAtcModeDeb != 0)	-- c_pTcRef->real.op.nAtcModeDeb;

	if (c_pTcRef->real.op.nAtcSw != atcs) {
		c_pTcRef->real.op.nAtcSw = atcs;
		c_pTcRef->real.op.nAtcSwDeb = DEB_OPMODE;
	}
	else if (c_pTcRef->real.op.nAtcSwDeb != 0)	-- c_pTcRef->real.op.nAtcSwDeb;

	if (c_pTcRef->real.op.nCabmDeb == 0 && c_pTcRef->real.op.nAtcModeDeb == 0 &&
			// Modified 2012/10/22
			c_pTcRef->real.op.nAtcSwDeb == 0) {
		cabm = c_pTcRef->real.op.nCabm;
		atcm = c_pTcRef->real.op.nAtcMode & ATCMODE_ONLY;
		BYTE atcv = c_pTcRef->real.op.nAtcMode & ATCMODE_VALID;
		if (cabm == TCDIM_EM) {
			ModeHandle(OPMODE_EMERGENCY);
			CLRTBIT(OWN_SPACEA, TCDOB_DPLP);	// mode xch
			pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, 0);
			c_pTcRef->real.op.nAutoStep = 0;
		}
		else if (cabm == TCDIM_MM && atcm == ATCMODE_MM && atcv) {
			ModeHandle(OPMODE_MANUAL);
			CLRTBIT(OWN_SPACEA, TCDOB_DPLP);	// mode xch
			pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, 0);
			c_pTcRef->real.op.nAutoStep = 0;
		}
		else if (cabm == TCDIM_DM) {	// Modified 2012/11/29 && IsNotRescue()) {
			if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY || c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) {
			}
			else if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS && atcm == ATCMODE_DM && atcv) {
			}
			else {
				if (atcm == 0 || !atcv) {
					 if (ModeHandle(OPMODE_DRIVERLESSREADY))
						pTcx->EntryDriverless(FALSE);
				}
				else	NonMode(atcm, atcv);
			}
		}
		else if (!cabm && atcm == ATCMODE_AM && atcv) {	// Modified 2012/11/29 && IsNotRescue()) {
			if (ModeHandle(OPMODE_AUTOREADY)) {
				CLRTBIT(OWN_SPACEA, TCDOB_DPLP);	// mode xch
				pTcx->InterTrigger(&c_pTcRef->real.op.wWaitSignalAutoB, TIME_WAITATOREADY);
				c_pTcRef->real.op.nAutoStep = AUTOSTEP_WAITATOR;
			}
			pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, 0);
		}
		else if (atcm == ATCMODE_YM && atcv) {	// Modified 2012/11/29 && IsNotRescue()) {
			// 2011/10/10 modified by ATC team
			//if (!cabm) {
				ModeHandle(OPMODE_YARD);
				CLRTBIT(OWN_SPACEA, TCDOB_DPLP);	// mode xch
				pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, 0);
				c_pTcRef->real.op.nAutoStep = 0;
			//}
		}
		// Modified 2012/10/22
		else if (!cabm && (c_pTcRef->real.op.nAtcSw & ATCSW_ONLY) == ATCSW_AI) {
			if (ModeHandle(OPMODE_AUTOREADY)) {
				CLRTBIT(OWN_SPACEA, TCDOB_DPLP);
				pTcx->InterTrigger(&c_pTcRef->real.op.wWaitSignalAutoB, TIME_WAITATCCMD);
				c_pTcRef->real.op.nAutoStep = AUTOSTEP_WAITATCCMD;
			}
		}
		else	NonMode(atcm, atcv);
	}

	// Modified 2013/11/02
	// Appended 2012/05/24 ... begin
	//if (c_pTcRef->real.nProperty & PROPERTY_USEEXTERNALMEM) {
	//	if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY || c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
	//			c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) {
	//		SETTBIT(OWN_SPACEA, TCDOB_BCID);
	//	}
	//	else	CLRTBIT(OWN_SPACEA, TCDOB_BCID);
	//}

	if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY || c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
				c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) {
		pTcx->ScanNuetralAtDriverless(TRUE);
	}
	else {
		if (c_pTcRef->real.atoCmd.nCmd & INSTCMD_INHIBITA)
			c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_INHIBITA;
	}
	// ... end
}

//BOOL CLook::ScanV3fResetSignals(PV3FEINFO peV3f)
//{
//	if (peV3f->d.st.st.cmd.b.rstreq ||
//			peV3f->d.st.st.deexc.b.hscbt ||
//			peV3f->d.st.st.deval.b.l1d || peV3f->d.st.st.deval.b.wdtd || peV3f->d.st.st.deval.b.clvd ||
//			peV3f->d.st.st.deval.b.gplvd || peV3f->d.st.st.deval.b.mocd || peV3f->d.st.st.deval.b.pud ||
//			peV3f->d.st.st.dedev.b.pgfd4 || peV3f->d.st.st.dedev.b.pgfd3 || peV3f->d.st.st.dedev.b.pgfd2 ||
//			peV3f->d.st.st.dedev.b.pgfd1 || peV3f->d.st.st.dedev.b.bcfd || peV3f->d.st.st.dedev.b.ld ||
//			peV3f->d.st.st.dedev.b.hotf)
//		return TRUE;
//	return FALSE;
//}

BOOL CLook::ScanV3fCutoutSignals(PV3FEINFO peV3f)
{
	if (peV3f->d.st.st.req.b.acoreq)	return TRUE;
	return FALSE;
}

//|| paV3f->d.st.dedev.b.cfd || paV3f->d.st.deval.b.mocd ||
//paV3f->d.st.deval.b.pud || paV3f->d.st.deval.b.l1d || psaV3f-

void CLook::SivAnnals(BYTE n, PSIVEINFO peSiv)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(peSiv->d.st.st.state.b.isoc, n, 201);
	pTcx->Annals(peSiv->d.st.st.state.b.dcov, n, 202);
	pTcx->Annals(peSiv->d.st.st.state.b.lovd, n, 203);
	pTcx->Annals(peSiv->d.st.st.vol.b.sivkx, n, 205);
	pTcx->Annals(peSiv->d.st.st.vol.b.iaoc, n, 206);
	pTcx->Annals(peSiv->d.st.st.vol.b.fcov, n, 207);
	pTcx->Annals(peSiv->d.st.st.vol.b.sivkax, n, 208);
	pTcx->Annals(peSiv->d.st.st.cur.b.iocu, n, 209);
	pTcx->Annals(peSiv->d.st.st.cur.b.iocv, n, 210);
	pTcx->Annals(peSiv->d.st.st.cur.b.iocw, n, 211);
	pTcx->Annals(peSiv->d.st.st.cur.b.lock, n, 212);
	pTcx->Annals(peSiv->d.st.st.over.b.ool, n, 213);
	pTcx->Annals(peSiv->d.st.st.over.b.osc, n, 214);
	pTcx->Annals(peSiv->d.st.st.over.b.aclv, n, 215);
	pTcx->Annals(peSiv->d.st.st.over.b.thuf, n, 216);
	pTcx->Annals(peSiv->d.st.st.over.b.acov, n, 217);
	pTcx->Annals(peSiv->d.st.st.ack.b.sivkf, n, 218);
	pTcx->Annals(peSiv->d.st.st.low.b.stf, n, 219);
	pTcx->Annals(peSiv->d.st.st.low.b.bcov, n, 220);
	pTcx->Annals(peSiv->d.st.st.low.b.bcoc, n, 221);
	pTcx->Annals(peSiv->d.st.st.low.b.thvf, n, 222);
	pTcx->Annals(peSiv->d.st.st.inst.b.thwf, n, 223);
	pTcx->Annals(peSiv->d.st.st.state.b.ssmk, n, 204);
	pTcx->Annals(peSiv->d.st.st.test.b.chkx, n, 261);
	pTcx->Annals(peSiv->d.st.st.ack.b.chkf, n, 262);
	pTcx->Annals(peSiv->d.st.st.ack.b.ivkf, n, 263);
}

void CLook::V3fAnnals(BYTE n, PV3FEINFO peV3f)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(peV3f->d.st.st.dedev.b.hotf, n, 301);
	pTcx->Annals(peV3f->d.st.st.dedev.b.ld, n, 302);
	pTcx->Annals(peV3f->d.st.st.dedev.b.hbd, n, 303);
	pTcx->Annals(peV3f->d.st.st.dedev.b.bcfd, n, 304);
	pTcx->Annals(peV3f->d.st.st.dedev.b.pgfd1, n, 305);
	pTcx->Annals(peV3f->d.st.st.dedev.b.pgfd2, n, 306);
	pTcx->Annals(peV3f->d.st.st.dedev.b.pgfd3, n, 307);
	pTcx->Annals(peV3f->d.st.st.dedev.b.pgfd4, n, 308);
	pTcx->Annals(peV3f->d.st.st.deval.b.pud, n, 309);
	pTcx->Annals(peV3f->d.st.st.deval.b.chdf, n, 337);
	pTcx->Annals(peV3f->d.st.st.deval.b.mocd, n, 310);
	pTcx->Annals(peV3f->d.st.st.deval.b.gplvd, n, 311);
	pTcx->Annals(peV3f->d.st.st.deval.b.clvd, n, 312);
	pTcx->Annals(peV3f->d.st.st.deval.b.wdtd, n, 313);
	pTcx->Annals(peV3f->d.st.st.deval.b.l1d, n, 314);
	pTcx->Annals(peV3f->d.st.st.deval.b.bsd, n, 315);
	pTcx->Annals(peV3f->d.st.st.deexc.b.wsd, n, 316);
	pTcx->Annals(peV3f->d.st.st.deexc.b.ovd2, n, 317);
	pTcx->Annals(peV3f->d.st.st.deexc.b.ovd1, n, 318);
	pTcx->Annals(peV3f->d.st.st.deexc.b.eslvd, n, 319);
	pTcx->Annals(peV3f->d.st.st.deexc.b.fclvd, n, 320);
	pTcx->Annals(peV3f->d.st.st.deexc.b.lgd, n, 321);
	pTcx->Annals(peV3f->d.st.st.deexc.b.hscbt, n, 322);
	pTcx->Annals(peV3f->d.st.st.deexc.b.l1da, n, 323);
	pTcx->Annals(peV3f->d.st.st.gdf.b.gdfw, n, 324);
	pTcx->Annals(peV3f->d.st.st.gdf.b.gdfv, n, 325);
	pTcx->Annals(peV3f->d.st.st.gdf.b.gdfu, n, 326);
	// Modified 2013/02/05
	pTcx->Annals(peV3f->d.st.st.gdf.b.iocd, n, 358);
}

void CLook::EcuAnnals(BYTE n, PECUAINFO paEcu)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(paEcu->d.st.cm.fail.b.dvrf, n, 401);
	pTcx->Annals(paEcu->d.st.cm.fail.b.dvff, n, 402);
	pTcx->Annals(paEcu->d.st.cm.fail.b.emvf, n, 403);
	pTcx->Annals(paEcu->d.st.cm.fail.b.lwf, n, 404);
	pTcx->Annals(paEcu->d.st.cm.fail.b.rbedf, n, 405);
	pTcx->Annals(paEcu->d.st.cm.fail.b.ramf, n, 406);
	pTcx->Annals(paEcu->d.st.cm.fail.b.sde, n, 407);
	pTcx->Annals(paEcu->d.st.cm.fail.b.buf, n, 408);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.a4as, n, 409);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.a3as, n, 410);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.a2as, n, 411);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.a1as, n, 412);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.apsf, n, 413);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.bpsf, n, 414);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.fbapsf, n, 415);
	pTcx->Annals(paEcu->d.st.cm.failsig.b.rbapsf, n, 416);
	pTcx->Annals(paEcu->d.st.cm.abnor.b.rbeaf, n, 417);
	pTcx->Annals(paEcu->d.st.cm.abnor.b.tllf, n, 418);
	pTcx->Annals(paEcu->d.st.cm.abnor.b.bcf, n, 419);
	pTcx->Annals(paEcu->d.st.cm.abnor.b.abrc, n, 420);
	pTcx->Annals(paEcu->d.st.cm.axle.b.ss4a, n, 421);
	pTcx->Annals(paEcu->d.st.cm.axle.b.ss3a, n, 422);
	pTcx->Annals(paEcu->d.st.cm.axle.b.ss2a, n, 423);
	pTcx->Annals(paEcu->d.st.cm.axle.b.ss1a, n, 424);
	// Modified 2012/11/14
	//pTcx->Annals(paEcu->d.st.cm.axle.b.rbasp, n, 425);
	//pTcx->Annals(paEcu->d.st.cm.axle.b.fbasp, n, 426);
	pTcx->AnnalsMrp(paEcu->d.st.cm.axle.b.rbasp, n, 425);
	pTcx->AnnalsMrp(paEcu->d.st.cm.axle.b.fbasp, n, 426);
	pTcx->Annals(paEcu->d.st.cm.inst.b.pecuf, n, 480);
}

void CLook::PisAnnals(PPISAINFO paPis)
{
	// Modified 2013/11/02
	if (!IsPermitAid())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(paPis->fdd.b.f1, 0, 556);
	pTcx->Annals(paPis->fdd.b.f2, 7, 557);
	pTcx->Annals(paPis->state.b.paf1, 0, 560);
	pTcx->Annals(paPis->state.b.paf0, 7, 561);
	pTcx->Annals(paPis->state.b.cf1, 0, 562);
	pTcx->Annals(paPis->state.b.cf0, 7, 563);
	pTcx->Annals(paPis->state.b.slf1, 0, 564);
	pTcx->Annals(paPis->state.b.slf0, 7, 565);
	pTcx->Annals(paPis->state.b.srf1, 0, 566);
	pTcx->Annals(paPis->state.b.srf0, 7, 567);
	pTcx->Annals(paPis->pa.b.f1, 0, 568);
	pTcx->Annals(paPis->pa.b.f2, 1, 568);
	pTcx->Annals(paPis->pa.b.f3, 2, 568);
	pTcx->Annals(paPis->pa.b.f4, 3, 568);
	pTcx->Annals(paPis->pa.b.f5, 4, 568);
	pTcx->Annals(paPis->pa.b.f6, 5, 568);
	pTcx->Annals(paPis->pa.b.f7, 6, 568);
	pTcx->Annals(paPis->pa.b.f0, 7, 568);
	pTcx->Annals(paPis->emp1.b.f1, 0, 576);
	pTcx->Annals(paPis->emp1.b.f2, 1, 576);
	pTcx->Annals(paPis->emp1.b.f3, 2, 576);
	pTcx->Annals(paPis->emp1.b.f4, 3, 576);
	pTcx->Annals(paPis->emp1.b.f5, 4, 576);
	pTcx->Annals(paPis->emp1.b.f6, 5, 576);
	pTcx->Annals(paPis->emp1.b.f7, 6, 576);
	pTcx->Annals(paPis->emp1.b.f0, 7, 576);
	pTcx->Annals(paPis->emp2.b.f1, 0, 577);
	pTcx->Annals(paPis->emp2.b.f2, 1, 577);
	pTcx->Annals(paPis->emp2.b.f3, 2, 577);
	pTcx->Annals(paPis->emp2.b.f4, 3, 577);
	pTcx->Annals(paPis->emp2.b.f5, 4, 577);
	pTcx->Annals(paPis->emp2.b.f6, 5, 577);
	pTcx->Annals(paPis->emp2.b.f7, 6, 577);
	pTcx->Annals(paPis->emp2.b.f0, 7, 577);
}

void CLook::TrsAnnals(PTRSAINFO paTrs)
{
	// Modified 2013/11/02
	if (!IsPermitAid())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(paTrs->info.b.atnf, SCENE_ID_HEADCAR, 581);
	pTcx->Annals(paTrs->info.b.rapf, SCENE_ID_HEADCAR, 582);
	pTcx->Annals(paTrs->info.b.tcif, SCENE_ID_HEADCAR, 583);
	pTcx->Annals(paTrs->info.b.tcrpf, SCENE_ID_HEADCAR, 584);
}

void CLook::HtcAnnals(PHTCAINFO paHtc)
{
	// Modified 2013/11/02
	if (!IsPermitAid())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(paHtc->info.b.hvcf1, 0, 531);
	pTcx->Annals(paHtc->info.b.hvcf2, 7, 532);
	pTcx->Annals(paHtc->info.b.eskr, SCENE_ID_HEADCAR, 533);
	pTcx->Annals(paHtc->info.b.rst, SCENE_ID_HEADCAR, 534);
	pTcx->Annals(paHtc->fail.b.hcc1, 0, 535);
	pTcx->Annals(paHtc->fail.b.hcc2, 1, 535);
	pTcx->Annals(paHtc->fail.b.hcc3, 2, 535);
	pTcx->Annals(paHtc->fail.b.hcc4, 3, 535);
	pTcx->Annals(paHtc->fail.b.hcc5, 4, 535);
	pTcx->Annals(paHtc->fail.b.hcc6, 5, 535);
	pTcx->Annals(paHtc->fail.b.hcc7, 6, 535);
	pTcx->Annals(paHtc->fail.b.hcc0, 7, 535);
}

void CLook::PsdAnnals(PPSDAINFO paPsd)
{
	// Modified 2013/11/02
	if (!IsPermitAid())	return;

	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Annals(paPsd->info.b.psdf, SCENE_ID_HEADCAR, 587);
	pTcx->Annals(paPsd->info.b.wpsdf, SCENE_ID_HEADCAR, 588);
	pTcx->Annals(paPsd->info.b.psdo, SCENE_ID_HEADCAR, 589);
	pTcx->Annals(paPsd->info.b.psdc, SCENE_ID_HEADCAR, 590);
}

BOOL CLook::CheckV3f(UCURV nCcid, PDEVICESTATE pDev)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Modified 2012/03/12 ... begin
	//if (pTcx->CheckV3fReply(nCcid, &c_pDoz->ccs[nCcid].real.eV3f) == FIRMERROR_NON) {
	WORD res = pTcx->CheckV3fReply(nCcid, &c_pDoz->ccs[nCcid].real.eV3f);
	if (res == FIRMERROR_NON) {
		c_v3fd.wFailDck &= ~(1 << nCcid);
		// ... end
		if (c_pDoz->ccs[nCcid].real.eV3f.nFlow == DEVFLOW_TRACEA)	return TRUE;
		WORD cur = (WORD)c_pDoz->ccs[nCcid].real.eV3f.d.st.st.nIm;
		if (cur > c_v3fd.wMaxCurrent)	c_v3fd.wMaxCurrent = cur;
		for (UCURV n = 0; n < 4; n ++)
			c_v3fd.dwSpeedBuff[c_v3fd.wLength ++] = c_pDoz->ccs[nCcid].real.eV3f.d.st.dwSpd[n];

		V3fAnnals(nCcid, &c_pDoz->ccs[nCcid].real.eV3f);
		if (ScanV3fCutoutSignals(&c_pDoz->ccs[nCcid].real.eV3f)) {
			c_v3fd.wFmccoCmd |= (1 << nCcid);
			if (pDev->wFmcco & (1 << nCcid)) {
				if (pDev->wFmccm & (1 << nCcid)) { // step 3, monitor vco to off
					if (!c_pDoz->ccs[nCcid].real.eV3f.d.st.st.req.b.vco)
						SHOT(nCcid, 329);
					else	CUT(nCcid, 329);
				}
				else { // step 2, monitor vco to on
					if (c_pDoz->ccs[nCcid].real.eV3f.d.st.st.req.b.vco) {
						pDev->wFmccm |= (1 << nCcid);
						CUT(nCcid, 328);
					}
					else	SHOT(nCcid, 328);
				}
			}
			else { // step 1, for vco monitor
				pDev->wFmcco |= (1 << nCcid);
				pDev->wFmccm &= ~(1 << nCcid);
				// Modified 2012/03/12
				//++ c_v3fd.wFmccoLength;
				SHOT(nCcid, 351);
			}
			CUT(nCcid, 330);
			return FALSE;
		}
		else {
			c_v3fd.wFmccoCmd &= ~(1 << nCcid);
			if (pDev->wFmcco & (1 << nCcid)) { // step 1, for vco monitor
				pDev->wFmccm |= (1 << nCcid);
				pDev->wFmcco &= ~(1 << nCcid);
				CUT(nCcid, 351);
				CUT(nCcid, 328);
				CUT(nCcid, 329);
			}
			else if (pDev->wFmccm & (1 << nCcid)) { // step 2
				if (c_pDoz->ccs[nCcid].real.eV3f.d.st.st.req.b.vco)
					SHOT(nCcid, 330);
				else { // end
					pDev->wFmccm &= ~(1 << nCcid);
					CUT(nCcid, 330);
				}
			}
			// Modified 2013/11/02
			//if (ScanV3fResetSignals(&c_pDoz->ccs[nCcid].real.eV3f))
			//	c_v3fd.wRstCmd |= (1 << nCcid);
			//else	c_v3fd.wRstCmd &= ~(1 << nCcid);
			if (c_pDoz->ccs[nCcid].real.eV3f.d.st.st.cmd.b.sqts) {
				pDev->wV3fSqts |= (1 << nCcid);
			}
			else	pDev->wV3fSqts &= ~(1 << nCcid);

			// Modified 2013/03/12
			if (c_pDoz->ccs[nCcid].real.eV3f.d.st.st.cmd.b.rstreq) {
				pDev->wV3fRstReq |= (1 << nCcid);
				SHOT(nCcid, 357);
			}
			else {
				pDev->wV3fRstReq &= ~(1 << nCcid);
				CUT(nCcid, 357);
			}
			if ((pDev->wV3fRstCmd & (1 << nCcid)) && c_pDoz->ccs[nCcid].real.eV3f.d.st.st.cmd.b.rstack) {
				pDev->wV3fRstCmd &= ~(1 << nCcid);
				CUT(nCcid, 359);
			}
			CUT(nCcid, 336);
		}
		return TRUE;
	}
	// Appended 2012/03/12 ... begin
	else if (res == FIRMERROR_FAULT)
		c_v3fd.wFailDck |= (1 << nCcid);
	// ... end
	return FALSE;
}

BOOL CLook::CheckEcu(UCURV nCcid, PDEVICESTATE pDev)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL di, sdi;
	if (GETCBIT(nCcid, CCDIB_NRBD)) {
		pDev->wNrbi |= (1 << nCcid);
		di = TRUE;
		SHOT(nCcid, 434);
	}
	else {
		di = FALSE;
		CUT(nCcid, 434);
	}
	if (pTcx->CheckEcuReply(nCcid, &c_pDoz->ccs[nCcid].real.aEcu) == FIRMERROR_NON) {
		if (c_pDoz->ccs[nCcid].real.aEcu.nFlow == ECUFLOW_TDTA)	return TRUE;
		if (c_pDoz->ccs[nCcid].real.aEcu.d.st.cm.set.b.nnrbd) {
			pDev->wNrbc |= (1 << nCcid);
			sdi = TRUE;
			SHOT(nCcid, 433);
		}
		else {
			sdi = FALSE;
			CUT(nCcid, 433);
		}
		if (di != sdi)
			SHOT(nCcid, 435);
		else	CUT(nCcid, 435);

		if (pTcx->CheckEcuReply(nCcid ^ 1, &c_pDoz->ccs[nCcid ^ 1].real.aEcu, FALSE) == FIRMERROR_NON) {
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.nAsoc[nCcid] =
				c_pDoz->ccs[nCcid ^ 1].real.aEcu.d.st.nAsp;
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.wPecuf &= ~(1 << nCcid);
		}
		else {
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.nAsoc[nCcid] = 0;
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.wPecuf |= (1 << nCcid);
		}
		EcuAnnals(nCcid, &c_pDoz->ccs[nCcid].real.aEcu);
		return TRUE;
	}
	return FALSE;
}

BOOL CLook::CheckCmsb(UCURV nCcid)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL bRes = TRUE;
	if (pTcx->CheckCmsbReply(nCcid, &c_pDoz->ccs[nCcid].real.aCmsb) != FIRMERROR_NON)
		bRes = FALSE;
	else {
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.lvr)
			SHOT(nCcid, 507);
		else	CUT(nCcid, 507);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.ovr)
			SHOT(nCcid, 508);
		else	CUT(nCcid, 508);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.invol)
			SHOT(nCcid, 506);
		else	CUT(nCcid, 506);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.invoc)
			SHOT(nCcid, 504);
		else	CUT(nCcid, 504);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.olr)
			SHOT(nCcid, 505);
		else	CUT(nCcid, 505);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.os)
			SHOT(nCcid, 509);
		else	CUT(nCcid, 509);
		// Modified 2013/11/02
		if (c_pDoz->ccs[nCcid].real.aCmsb.state.b.lcmg)
			SHOT(nCcid, 536);
		else	CUT(nCcid, 536);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.p15)
			SHOT(nCcid, 537);
		else	CUT(nCcid, 537);
		if (c_pDoz->ccs[nCcid].real.aCmsb.fail.b.p5)
			SHOT(nCcid, 538);
		else	CUT(nCcid, 538);
	}
	return bRes;
}

void CLook::CmsbLoad(BOOL cmdA, BOOL cmdB)
{
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[0].b.cmg2 = FALSE;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[0].b.cmg1 = cmdA;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[0].b.cmr = cmdA;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[1].b.cmg2 = cmdB;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[1].b.cmg1 = FALSE;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.inst[1].b.cmr = cmdB;
}

void CLook::CopyDcuErrorCount(WORD count, UCURV sid, UCURV did, UCURV cid)
{
	BYTE id = sid * 4 + did;
	if (cid == (c_pTcRef->real.cf.nLength - 1))	id ^= 7;
	CTcx* pTcx = (CTcx*)c_pParent;
	PDUMBDEV pDumb = pTcx->GetDumb();
	pDumb->wDcu[cid + HEADCCBYLENGTHA][id] = count;
}

void CLook::CalcV3fSpeed()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_v3fd.wLength < 1) {
		c_pTcRef->real.speed.uLength = 0;
		c_pTcRef->real.speed.dwV3fs = 0L;
		SHOT(SCENE_ID_HEADCAR, 335);
	}
	else {
		c_pTcRef->real.speed.uLength = (UCURV)c_v3fd.wLength;
		if (c_v3fd.wLength == 1)
			c_pTcRef->real.speed.dwV3fs = c_v3fd.dwSpeedBuff[0];
		else {
			double avr = 0.f;
			WORD w;
			for (w = 0; w < c_v3fd.wLength; w ++)
				avr += (double)c_v3fd.dwSpeedBuff[w];
			avr /= (double)c_v3fd.wLength;
			double deviation = 0.f;
			for (w = 0; w < c_v3fd.wLength; w ++)
				deviation += fabs(avr - (double)c_v3fd.dwSpeedBuff[w]);
			deviation /= (double)c_v3fd.wLength;
			// Modified 2013/02/05
			if (deviation < 1.0f)	deviation = 1.0f;

			long v = (long)(avr - deviation);
			if (v < 0)	v = 0;
			DWORD min = (DWORD)v;
			DWORD max = (DWORD)(avr + deviation);
			avr = 0.f;
			WORD leng = 0;
			for (w = 0; w < c_v3fd.wLength; w ++) {
				if (c_v3fd.dwSpeedBuff[w] >= min && c_v3fd.dwSpeedBuff[w] <= max) {
					avr += (double)c_v3fd.dwSpeedBuff[w];
					++leng;
				}
			}
			c_pTcRef->real.speed.dwV3fs = (DWORD)(avr / leng);
		}
		CUT(SCENE_ID_HEADCAR, 335);
	}
}

// Modified 2012/01/16 ... begin
//void CLook::CmsbCtrl(WORD sivf, BOOL cmd)
void CLook::CmsbCtrl(WORD sivf)
{
	CTcx* pTcx = (CTcx*)c_pParent;

	// Modified 2012/02/27 ... begin
	//if (cmd && sivf == 0 &&
	if (sivf == 0 &&
	// ... end
			(GETCBIT(__SIVA, CCDIB_SIVK) ||
			GETCBIT(__SIVB, CCDIB_SIVK) ||
			GETCBIT(__ESK, CCDIB_ESKC)) &&
			((_ISOCC(__CMSBA) && GETCBIT(__CMSBA, CCDIB_CMG)) ||
			(_ISOCC(__CMSBB) && GETCBIT(__CMSBB, CCDIB_CMG)))) {
		BOOL cmdA, cmdB;
		cmdA = cmdB = FALSE;

		if (c_pTcRef->real.op.nCmsbMonitor & CMSBMON_POWERA)	cmdA = TRUE;
		else if (c_pTcRef->real.op.nCmsbMonitor & CMSBMON_READYA) {
			if (c_pTcRef->real.op.wCmsbTimer[0] == 0) {
				c_pTcRef->real.op.nCmsbMonitor &= ~CMSBMON_READYA;
				c_pTcRef->real.op.nCmsbMonitor |= CMSBMON_POWERA;
				pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[0],
						(DELAY_CMSBFRONT * 1000 + TIME_CMSBONMONITOR) / TIME_INTERVAL);
				CUT(__CMSBA, 502);
				cmdA = TRUE;
			}
		}
		else {
			if (_ISOCC(__CMSBA)) {
				c_pTcRef->real.op.nCmsbMonitor |= CMSBMON_READYA;
				pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[0], TIME_CMSBRUNREADY);
			}
		}

		if (c_pTcRef->real.op.nCmsbMonitor & CMSBMON_POWERB)	cmdB = TRUE;
		else if (c_pTcRef->real.op.nCmsbMonitor & CMSBMON_READYB) {
			if (c_pTcRef->real.op.wCmsbTimer[1] == 0) {
				c_pTcRef->real.op.nCmsbMonitor &= ~CMSBMON_READYB;
				c_pTcRef->real.op.nCmsbMonitor |= CMSBMON_POWERB;
				pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[1],
						(DELAY_CMSBFRONT * 1000 + TIME_CMSBONMONITOR) / TIME_INTERVAL);
				CUT(__CMSBB, 502);
				cmdB = TRUE;
			}
		}
		else {
			if (_ISOCC(__CMSBB)) {
				c_pTcRef->real.op.nCmsbMonitor |= CMSBMON_READYB;
				pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[1], TIME_CMSBRUNREADY);
			}
		}

		CmsbLoad(cmdA, cmdB);
	}
	else {
		if (c_pTcRef->real.op.nCmsbMonitor & (CMSBMON_READYA | CMSBMON_READYB)) {
			pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[0], 0);
			pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[1], 0);
			c_pTcRef->real.op.nCmsbMonitor &= ~(CMSBMON_READYA | CMSBMON_READYB);
		}
		else if (c_pTcRef->real.op.nCmsbMonitor & (CMSBMON_POWERA | CMSBMON_POWERB)) {
			if (c_pTcRef->real.op.nCmsbMonitor & CMSBMON_POWERA) {
				c_pTcRef->real.op.nCmsbMonitor &= ~CMSBMON_POWERA;
				pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[0],
						(DELAY_CMSBFRONT * 1000 + TIME_CMSBOFFMONITOR) / TIME_INTERVAL);
				CUT(__CMSBA, 503);
			}
			if (c_pTcRef->real.op.nCmsbMonitor & CMSBMON_POWERB) {
				c_pTcRef->real.op.nCmsbMonitor &= ~CMSBMON_POWERB;
				pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[1],
						(DELAY_CMSBREAR * 1000 + TIME_CMSBOFFMONITOR) / TIME_INTERVAL);
				CUT(__CMSBB, 503);
			}
		}
		CmsbLoad(FALSE, FALSE);
	}
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.nCmst[0] = DELAY_CMSBFRONT;
	c_pDoz->tcs[OWN_SPACEA].real.cCmsb.nCmst[1] = DELAY_CMSBREAR;

	if (_ISOCC(__CMSBA) && pTcx->GetBusCondition(_REALCC(__CMSBA)))
		CmsbPowerMonit(__CMSBA);
	if (_ISOCC(__CMSBB) && pTcx->GetBusCondition(_REALCC(__CMSBB)))
		CmsbPowerMonit(__CMSBB);
}

void CLook::CmsbPowerMonit(BYTE nCcid)
{
	CTcx* pTcx = (CTcx*)c_pParent;

	// Modified 2011/12/18 ... begin
	BYTE flag = __CMSBA == nCcid ? CMSBMON_READYA : CMSBMON_READYB;
	if (c_pTcRef->real.op.nCmsbMonitor & flag)	return;
	// ... end

	BYTE tid = nCcid < 3 ? 0 : 1;
	flag = __CMSBA == nCcid ? CMSBMON_POWERA : CMSBMON_POWERB;

	// Modified 2012/01/18
	BOOL cmdf = (c_pTcRef->real.op.nCmsbMonitor & flag) ? TRUE : FALSE;
	BOOL monf = c_pDoz->ccs[nCcid].real.aCmsb.state.b.invon ? TRUE : FALSE;
	if (cmdf == monf) {
		pTcx->InterTrigger(&c_pTcRef->real.op.wCmsbTimer[tid], 0);
		if (cmdf)	SHOT(nCcid, 511);
		else	CUT(nCcid, 511);
		CUT(nCcid, 502);
		CUT(nCcid, 503);
	}
	else {
		if (c_pTcRef->real.op.wCmsbTimer[tid] == 0) {
			if (cmdf)	SHOT(nCcid, 502);
			else	SHOT(nCcid, 503);
			CUT(nCcid, 511);
		}
	}
}
// ... end

BYTE CLook::GetSettingBits(BYTE bits)
{
	BYTE all = 0;
	for (UCURV n = 0; n < 16; n ++) {
		if (bits & (1 << n))
			++ all;
	}
	return all;
}

void CLook::Check()
{
	// Modified 2012/01/19 ... begin
	//if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE))	return;
	// ... end

	DEVICESTATE dev;
	memset((PVOID) &dev, 0, sizeof(DEVICESTATE));
	dev.wMrp = c_pTcRef->real.dev.wMrp;
	dev.wFmcco = c_pTcRef->real.dev.wFmcco;
	dev.wFmccm = c_pTcRef->real.dev.wFmccm;
	dev.nSivkmon[0] = c_pTcRef->real.dev.nSivkmon[0];
	dev.nSivkmon[1] = c_pTcRef->real.dev.nSivkmon[1];
	// Inserted 2012/02/27
	dev.nState = c_pTcRef->real.dev.nState;
	// Modified 2013/11/02
	dev.wV3fRstCmd = c_pTcRef->real.dev.wV3fRstCmd;

	memset((PVOID) &c_v3fd, 0, sizeof(V3FARCH));
	CTcx* pTcx = (CTcx*)c_pParent;
	PDUMBDEV pDumb = pTcx->GetDumb();
	BYTE sivi, v3fi, cmsi;
	sivi = v3fi = cmsi = 0;
	BOOL bV3fTbeCapture = FALSE;
	BOOL bV3fFlow = c_pTcRef->real.nAddr < 3 ? TRUE : FALSE;
	c_pTcRef->real.intg.dbSivPower = c_pTcRef->real.intg.dbV3fPower = c_pTcRef->real.intg.dbRevival = 0;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD deves = pTcx->GetCarDevices(n);
		if (pTcx->GetBusCondition(n + CAR_CC)) {
			if (deves & DEVBEEN_SIV) {		// 1
				dev.nState |= 0x80;			// check step 1
				if (!GETCBIT(n, CCDIB_RFN))	SHOT(n, 244);
				else	CUT(n, 244);
				BYTE id = n < 2 ? 0 : 1;
				// Modified 2011/12/28 ... begin
				//if (pTcx->CheckSivReply(n, &c_pDoz->ccs[n].real.eSiv) != FIRMERROR_NON)
				//	dev.wFailSiv |= (1 << n);
				//else	SivAnnals(n, &c_pDoz->ccs[n].real.eSiv);
				if (pTcx->CheckSivReply(n, &c_pDoz->ccs[n].real.eSiv) != FIRMERROR_NON) {
					dev.wFailSiv |= (1 << n);
					dev.nSivkmon[id] = 0;
				}
				else {
					if (c_pDoz->ccs[n].real.eSiv.nFlow != DEVFLOW_TRACEA) {
						//c_pDoz->ccs[n].real.eSiv.d.st.power = 50.f;		// ?????
						c_pTcRef->real.intg.dbSivPower += c_pDoz->ccs[n].real.eSiv.d.st.power;
						SivAnnals(n, &c_pDoz->ccs[n].real.eSiv);
						WORD w = pTcx->Conv(c_pDoz->ccs[n].real.eSiv.d.st.st.nEd, 0, 250, (WORD)0, (WORD)3000);
						if (w >= VALID_SIVED) {
							if (dev.nSivkmon[id] < DEVMON_SIVK)	++ dev.nSivkmon[id];
						}
						else	dev.nSivkmon[id] = 0;
					}
				}
				// ... end

				BOOL mrp = GETCBIT(n, CCDIB_MRPS) ? FALSE : TRUE;
				if ((dev.wMrp & (1 << n)) && !mrp) {
					dev.wMrp &= ~(1 << n);
					CUT(n, 453);
					SHOT(n, 489);
				}
				else if (!(dev.wMrp & (1 << n)) && mrp) {
					dev.wMrp |= (1 << n);
					CUT(n, 489);
					// Modified 2012/11/14
					//SHOT(n, 453);
					pTcx->ShotMrp(n, 453);
				}
				// Modified 2013/02/05
				if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
					if (!pTcx->CheckSivAddr(&c_pDoz->ccs[n].real.eSiv))
						pTcx->CountDumb(&pDumb->wSiv[sivi]);
				}
				if (++ sivi > 1)	sivi = 1;
			}
			if ((deves & DEVBEEN_V3F) && (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE)) {	// 2
				dev.nState |= 0x40;		// check step 2
				if (!CheckV3f(n, &dev))
					dev.wFailV3f |= (1 << n);
				else {
					if (c_pDoz->ccs[n].real.eV3f.nFlow != DEVFLOW_TRACEA) {
						//c_pDoz->ccs[n].real.eV3f.d.st.power = 75.f;			// ?????
						//c_pDoz->ccs[n].real.eV3f.d.st.revival = 25.f;		// ?????
						c_pTcRef->real.intg.dbV3fPower += c_pDoz->ccs[n].real.eV3f.d.st.power;
						c_pTcRef->real.intg.dbRevival += c_pDoz->ccs[n].real.eV3f.d.st.revival;
						if (bV3fFlow) {		// 1 car
							if (!bV3fTbeCapture) {
								dev.vV3fTbeL = c_pDoz->ccs[n].real.eV3f.d.st.st.vTbeL;
								bV3fTbeCapture = TRUE;
							}
						}
						else {	// 0 car
							dev.vV3fTbeL = c_pDoz->ccs[n].real.eV3f.d.st.st.vTbeL;
						}
					}
				}
				if (!pTcx->CheckV3fAddr(&c_pDoz->ccs[n].real.eV3f))
					pTcx->CountDumb(&pDumb->wV3f[v3fi]);
				if (++ v3fi > 3)	v3fi = 3;
			}
			if (deves & DEVBEEN_CMSB) {		// 3
				dev.nState |= 0x20;		// check step 3
				if (!CheckCmsb(n))
					dev.wFailCmsb |= (1 << n);
				// Modified 2013/01/08
				//if (!pTcx->CheckCmsbAddr(&c_pDoz->ccs[n].real.aCmsb))
				//	pTcx->CountDumb(&pDumb->wCmsb[cmsi]);
				pDumb->wCmsb[cmsi] = c_pDoz->ccs[n].real.wCmsbErrorCount;

				if (++ cmsi > 1)	cmsi = 1;
			}
			// Modified 2013/02/05
			for (UCURV m = 0; m < 4; m ++) {
				CopyDcuErrorCount(c_pDoz->ccs[n].real.wDoorErrorCount[0][m], 0, m, n);
				CopyDcuErrorCount(c_pDoz->ccs[n].real.wDoorErrorCount[1][m], 1, m, n);
			}
			// Modified 2012/01/19 ... begin
			if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {	// 4
			// ... end
				dev.nState |= 0x10;		// check step 4
				if (!CheckEcu(n, &dev))	dev.wFailEcu |= (1 << n);
				if (!pTcx->CheckEcuAddr(&c_pDoz->ccs[n].real.aEcu))
					pTcx->CountDumb(&pDumb->wEcu[n + HEADCCBYLENGTHA]);
				if (!GETCBIT(n, CCDIB_BOUCS)) {
					dev.wBoucs |= (1 << n);
					SHOT(n, 450);
				}
				else	CUT(n, 450);
				if (!GETCBIT(n, CCDIB_BCCS1)) {
					dev.wFailBccs1 |= (1 << n);
					SHOT(n, 451);
				}
				else	CUT(n, 451);
				if (!GETCBIT(n, CCDIB_BCCS2)) {
					dev.wFailBccs2 |= (1 << n);
					SHOT(n, 452);
				}
				else	CUT(n, 452);
				if (READCBYTE(n, CCDI_DOOR) != 0) {
					//dev.wDoor |= (1 << n);
					dev.nState |= DEVSTATE_SOMEDOOR;
				}
				else	dev.nState &= ~DEVSTATE_SOMEDOOR;
				if (!GETCBIT(n, CCDIB_ASCS1)) {
					dev.wAscs1 |= (1 << n);
					SHOT(n, 470);
				}
				else	CUT(n, 470);
				if (!GETCBIT(n, CCDIB_ASCS2)) {
					dev.wAscs2 |= (1 << n);
					SHOT(n, 471);
				}
				else	CUT(n, 471);
				if (c_pTcRef->real.ext.wChgTimer == 0) {
					BOOL reduc = (c_pTcRef->real.ext.nState & EXTSTATE_REDUCTIONB) ? TRUE : FALSE;	// ?????
					if ((reduc && !GETCBIT(n, CCDIB_LRR)) ||
							(!reduc && GETCBIT(n, CCDIB_LRR))) {
						if (reduc) {
							CUT(n, 243);
							SHOT(n, 242);
						}
						else {
							CUT(n, 242);
							// Modified 2012/11/21
							// SHOT(n, 243);
							//if (GETTBIT(OWN_SPACEA, TCDIB_LCDK))	SHOT(n, 243);
							// Modified 2012/12/12
							if (GETTBIT(OWN_SPACEA, TCDIB_LCDK) && (c_pTcRef->real.ext.nState & EXTSTATE_SCANESCAPE))
								SHOT(n, 243);
						}
					}
					else	CUTS(n, 242, 243);	// reduc && LRR || !reduc && !LRR
					c_pTcRef->real.ext.nState &= ~EXTSTATE_SCANESCAPE;
				}
				// Modified 2013/11/02
				if (!GETCBIT(n, CCDIB_K1))	SHOT(n, 525);
				else {
					dev.wK1 |= (1 << n);
					CUT(n, 525);
				}
				if (!GETCBIT(n, CCDIB_K4))	SHOT(n, 526);
				else {
					dev.wK4 |= (1 << n);
					CUT(n, 526);
				}
				if (!GETCBIT(n, CCDIB_K6))	SHOT(n, 527);
				else {
					dev.wK6 |= (1 << n);
					CUT(n, 527);
				}

				if (OWN_SPACEA < 2) {
					if (GETCBIT(n, CCDIB_EMR) && !(c_pTcRef->real.ca.emer.wBuf & (1 << n))) {
						c_pTcRef->real.ca.emer.wBuf |= (1 << n);
						if (!(c_pTcRef->real.ca.emer.wRep & (1 << n))) {
							c_pTcRef->real.ca.emer.wRep |= (1 << n);
							SETOBIT(OWN_SPACEA, n, CCDOB_EIAL);
							SHOT(n, 551);
							CUT(SCENE_ID_HEADCAR, 554);
							if (c_pTcRef->real.ca.fire.wStep == 0 && c_pTcRef->real.ca.emer.wStep == 0)
								c_pTcRef->real.ca.emer.wStep = 1;
						}
					}
					else if (!GETCBIT(n, CCDIB_EMR) && (c_pTcRef->real.ca.emer.wBuf & (1 << n))) {
						c_pTcRef->real.ca.emer.wBuf &= ~(1 << n);
						CUT(n, 551);
					}

					if (GETCBIT(n, CCDIB_FDN) && !(c_pTcRef->real.ca.fire.wBuf & (1 << n))) {
						c_pTcRef->real.ca.fire.wBuf |= (1 << n);
						if (!(c_pTcRef->real.ca.fire.wRep & (1 << n))) {
							c_pTcRef->real.ca.fire.wRep |= (1 << n);
							SETOBIT(OWN_SPACEA, n, CCDOB_EIAL);
							if (c_pTcRef->real.ca.emer.wStep == 0 && c_pTcRef->real.ca.fire.wStep == 0)
								c_pTcRef->real.ca.fire.wStep = 1;
						}
					}
					//else if (!GETCBIT(n, CCDIB_FDN) && !FBOBIT(OWN_SPACEA, n, CCDOB_EIAL) &&
					else if (!GETCBIT(n, CCDIB_FDN) && c_pTcRef->real.ca.fire.wStep == 0 &&
							(c_pTcRef->real.ca.fire.wBuf & (1 << n)))
						c_pTcRef->real.ca.fire.wBuf &= ~(1 << n);
				}
				for (UCURV m = 0; m < 4; m ++) {
					if (!(pTcx->CheckDcuReply(&c_pDoz->ccs[n].real.aDcu[0][m], m + 1, n))) {
						if (!(c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY))
							pTcx->ShotMainDev(n, pTcx->XchAtZero(n, m + 713));
						// Modified 2013/01/08
						//pTcx->CountDumb(&pDumb->wDcu[n][m]);
					}
					else	CUT(n, pTcx->XchAtZero(n, m + 713));

					if (!(pTcx->CheckDcuReply(&c_pDoz->ccs[n].real.aDcu[1][m], m + 5, n))) {
						if (!(c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY))
							pTcx->ShotMainDev(n, pTcx->XchAtZero(n, m + 717));
						// Modified 2013/01/08
						//pTcx->CountDumb(&pDumb->wDcu[n][m + 4]);
					}
					else	CUT(n, pTcx->XchAtZero(n, m + 717));
				}
			// Inserted 2012/01/19
			}
		}
		else {
			dev.wFailCC |= (1 << n);
			if (deves & DEVBEEN_SIV) {	// 1
				dev.nState |= 0x80;		// check step 1
				dev.wFailSiv |= (1 << n);
				pTcx->CountDumb(&pDumb->wSiv[sivi]);
				if (++ sivi > 1)	sivi = 1;
				// Modified 2011/12/28 ... begin
				BYTE id = n < 2 ? 0 : 1;
				dev.nSivkmon[id] = 0;
				// ... end
			}
			// Modified 2012/01/19 ... begin
			if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
			// ... end
				if (deves & DEVBEEN_V3F) {	// 2
					dev.nState |= 0x40;		// check step 2
					dev.wFailV3f |= (1 << n);
					pTcx->CountDumb(&pDumb->wV3f[v3fi]);
					if (++ v3fi > 3)	v3fi = 3;
				}
				if (deves & DEVBEEN_CMSB) {	// 3
					dev.nState |= 0x20;		// check step 3
					dev.wFailCmsb |= (1 << n);
					// Modified 2013/01/08
					//pTcx->CountDumb(&pDumb->wCmsb[cmsi]);
					pDumb->wCmsb[cmsi] = pDumb->wBus[LENGTH_TC + n];
					if (++ cmsi > 1)	cmsi = 1;
				}
				dev.nState |= 0x10;			// check step 4
				dev.wFailEcu |= (1 << n);	// 4
				pTcx->CountDumb(&pDumb->wEcu[n]);
				// Modified 2013/01/08
				//for (UCURV m = 0; m < 8; m ++)
				//	pTcx->CountDumb(&pDumb->wDcu[n][m]);
				for (UCURV m = 0; m < 8; m ++)
					pDumb->wDcu[n][m] = pDumb->wBus[LENGTH_TC + n];
				dev.wFailBccs1 |= (1 << n);
				dev.wFailBccs2 |= (1 << n);
			// Inserted 2012/01/19
			}
		}
	}
	// Appended 2012/01/19
	if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {	// 5
		dev.nState |= 8;		// check step 5
		// Modified 2013/03/12
		//c_pDoz->tcs[OWN_SPACEA].real.cV3f.wRstCmd = c_v3fd.wRstCmd;
		// Modified 2012/03/12 ... begin
		//c_pDoz->tcs[OWN_SPACEA].real.cV3f.wFmccoCmd = c_v3fd.wFmccoCmd;
		//switch (c_v3fd.wFmccoLength) {
		c_pDoz->tcs[OWN_SPACEA].real.cV3f.wFmccoCmd = c_v3fd.wFmccoCmd | c_v3fd.wFailDck;
		BYTE length = pTcx->GetBitQuantity(c_pDoz->tcs[OWN_SPACEA].real.cV3f.wFmccoCmd, TRUE);
		switch (length) {
		// ... end
		case 0:
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp1co =
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp2co = FALSE;
			CUTS(SCENE_ID_HEADCAR, 352, 355);
			break;
		case 1:
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp1co = TRUE;
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp2co = FALSE;
			EDIT(SCENE_ID_HEADCAR, 353, 355, 352);
			break;
		case 2:
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp1co = FALSE;
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp2co = TRUE;
			EDIT(SCENE_ID_HEADCAR, 352, 355, 353);
			break;
		case 3:
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp1co =
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp2co = TRUE;
			EDIT(SCENE_ID_HEADCAR, 352, 355, 354);
			break;
		default:
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp1co =
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.pp2co = TRUE;
			EDIT(SCENE_ID_HEADCAR, 352, 354, 355);
			break;
		}
		// Appended 2012/03/29 ... begin
		//if (IsRescue() || (c_pTcRef->real.op.nState & OPERATESTATE_WITHPWM))
		if (IsRescue())	c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.rescue = TRUE;
		else	c_pDoz->tcs[OWN_SPACEA].real.cV3f.mode.b.rescue = FALSE;
		// ... end

		// Modified 2012/03/19 ... begin
		//if (GETTBIT(OWN_SPACEA, TCDIB_ADSCR)) {
		if (GETTBIT(CAR_HEAD, TCDIB_ADSCR)) {
		// ... end
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.inst.b.adc = TRUE;
			CLRTBIT(OWN_SPACEA, TCDOB_TDO);
		}
		else {
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.inst.b.adc = FALSE;
			SETTBIT(OWN_SPACEA, TCDOB_TDO);
		}
		c_pTcRef->real.speed.wMotorCurrent = c_v3fd.wMaxCurrent;

		// Modified 2012/11/21 ... begin
		if (GETTBIT(OWN_SPACEA, TCDIB_LCDK))	CUT(SCENE_ID_HEADCAR, 247);
		else	SHOT(SCENE_ID_HEADCAR, 247);
		// ... end
	// Inserted 2012/01/19
	}

	// Modified 2012/01/16 ... begin
	//CmsbCtrl(dev.wFailSiv, TRUE);
	dev.nState |= 4;			// check step 6
	//CmsbCtrl(dev.wFailSiv);		// 6
	CmsbCtrl(0);		// 6
	// ... end
	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE)) {
		pTcx->Lock();
		memcpy((PVOID)&c_pTcRef->real.dev, (PVOID)&dev, sizeof(DEVICESTATE));
		pTcx->Unlock();
		return;
	}

	dev.nState |= 2;			// check step 7
	CalcV3fSpeed();
	DWORD v = 0;
	// Modified 2013/02/05
	//if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY)
	//	v = c_pTcRef->real.speed.dwV3fs;
	//else	v = c_pTcRef->real.speed.dwAtcs;
	if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY))	// for running test at lab.
		v = c_pTcRef->real.speed.dwAtcs;
	else	v = c_pTcRef->real.speed.dwV3fs;

	if (v <= SPEED_ZERO) {
		if (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) {
			SETTBIT(OWN_SPACEA, TCDOB_ZVR);
			if (!GETTBIT(OWN_SPACEA, TCDIB_ADSCR)) {	// opened door
				c_pTcRef->real.intg.dbIntg = 0.f;
				c_pTcRef->real.intg.dbDist = 0.f;
			}
			if (c_pTcRef->real.power.inst.b.p && c_pTcRef->real.power.wTimer == 0)
				SHOT(SCENE_ID_HEADCAR, 51);
			else	CUT(SCENE_ID_HEADCAR, 51);
		}
		else {
			if (c_pTcRef->real.speed.nZv & SPEEDCOUNT_ZVF) {
				if (c_pTcRef->real.speed.wZvTimer == 0) {
					c_pTcRef->real.speed.nZv &= ~SPEEDCOUNT_ZVF;
					c_pTcRef->real.speed.nZv |= SPEEDDETECT_ZVF;
					SETTBIT(OWN_SPACEA, TCDOB_ZVR);
					if (c_pTcRef->real.cond.wReleaseTimer == 0)
						pTcx->InterTrigger(&c_pTcRef->real.cond.wReleaseTimer, 1);
					SHOT(SCENE_ID_HEADCAR, 53);
					CUT(SCENE_ID_HEADCAR, 745);
				}
			}
			else {
				c_pTcRef->real.speed.nZv |= SPEEDCOUNT_ZVF;
				pTcx->InterTrigger(&c_pTcRef->real.speed.wZvTimer, TIME_DETECTZV);
			}
		}
	}
	else {
		pTcx->InterTrigger(&c_pTcRef->real.speed.wZvTimer, 0);
		if (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF)
			pTcx->InterTrigger(&c_pTcRef->real.cond.wReleaseTimer, 0);	// at field
		c_pTcRef->real.speed.nZv = 0;
		CLRTBIT(OWN_SPACEA, TCDOB_ZVR);
		// Modified 2012/11/12
		c_pTcRef->real.door.nState &= ~DOORSTATE_ENABLEREOPEN;
		CUT(SCENE_ID_HEADCAR, 51);
		CUT(SCENE_ID_HEADCAR, 53);
	}

	uint64_t dt = ClockCycles();
	if (!c_bPowerOn) {
		uint64_t intv = dt - c_timeDist;
		double sec = (double)intv / (double)c_pTcRef->real.cps;
		double db = ((double)v * sec / 3600.f);
		c_pTcRef->real.intg.dbDist += db;		// distance between stations
		c_pTcRef->real.intg.dbDistLive += db;	// total distance at HCR
		if (c_pTcRef->real.nState & STATE_VALIDENVF) {
			PENVARCHEXTENSION pEnv = pTcx->GetEnv();
			pEnv->real.entire.elem.dbDist += db;
			if (db > 0.f)	c_pTcRef->real.nState |= STATE_SAVEENVF;
			if (c_pTcRef->real.intg.dbSivPower > 0.f) {
				c_pTcRef->real.intg.dbSivPowerLive += c_pTcRef->real.intg.dbSivPower;	// total power at HCR
				pEnv->real.entire.elem.dbSivPower += c_pTcRef->real.intg.dbSivPower;
				c_pTcRef->real.nState |= STATE_SAVEENVF;
			}
			if (c_pTcRef->real.intg.dbV3fPower > 0.f) {
				c_pTcRef->real.intg.dbV3fPowerLive += c_pTcRef->real.intg.dbV3fPower;	// total power at HCR
				pEnv->real.entire.elem.dbV3fPower += c_pTcRef->real.intg.dbV3fPower;
				c_pTcRef->real.nState |= STATE_SAVEENVF;
			}
			if (c_pTcRef->real.intg.dbRevival > 0.f) {
				pEnv->real.entire.elem.dbRevival += c_pTcRef->real.intg.dbRevival;
				c_pTcRef->real.nState |= STATE_SAVEENVF;
			}
		}
	}
	c_bPowerOn = FALSE;
	c_timeDist = dt;
	//c_pTcRef->real.scArm.nState |= ARMSTATE_VALIDDATA;		// ...no need...

	PATCAINFO paAtc = pTcx->GetActiveAtc(TRUE);
	BYTE pulse = paAtc != NULL ? paAtc->d.st.nAccp : 0;
	// Modified 2013/02/05
	//pTcx->PutDistanceFair((WORD)pulse, v);
	pTcx->PutDistanceFair((WORD)pulse, c_pTcRef->real.speed.dwAtcs);

	if (!(c_pTcRef->real.intg.nDistPulse & DISTPULSE_PUT)) {
		if ((c_pTcRef->real.intg.dbDist - c_pTcRef->real.intg.dbIntg) >= DISTPULSE_PERIOD) {
			c_pTcRef->real.intg.dbIntg += DISTPULSE_PERIOD;
			//c_pTcRef->real.intg.dbIntg = c_pTcRef->real.intg.dbDist;
			SETTBIT(OWN_SPACEA, TCDOB_DCALS);
			c_pTcRef->real.intg.nDistPulse |= DISTPULSE_PUT;
			pTcx->InterTrigger(&c_pTcRef->real.intg.wDistTimer, TIME_DISTPERIOD);
		}
	}
	if (GETTBIT(OWN_SPACEA, TCDIB_HES)) {
		if (!(c_pTcRef->real.bat.nState & BATSTATE_TCDIHES))	// ...no need...
			c_pTcRef->real.bat.nState |= BATSTATE_TCDIHES;
		pTcx->ObsCtrl(TRUE, CCDOB_BHEK, ALLCAR_FLAGS);
	}
	else {
		if (c_pTcRef->real.bat.nState & BATSTATE_TCDIHES)
			c_pTcRef->real.bat.nState &= ~BATSTATE_TCDIHES;
		pTcx->ObsCtrl(FALSE, CCDOB_BHEK, ALLCAR_FLAGS);
	}

	GetMascon(&c_pTcRef->real.mcDrv);
	if (c_pTcRef->real.mcDrv.bValid && c_pTcRef->real.mcDrv.nIndex == MCDSV_NPOS)
		CUT(SCENE_ID_HEADCAR, 492);
	if (c_pTcRef->real.nProperty & PROPERTY_PARTNER) {
		BYTE partner = OWN_SPACEA ^ 1;
		if (c_pTcRef->real.mcDrv.bValid != c_pDoz->tcs[partner].real.mcDrv.bValid ||
				c_pTcRef->real.mcDrv.nIndex != c_pDoz->tcs[partner].real.mcDrv.nIndex) {
			if (c_pTcRef->real.mcDrv.wFailSafeDeb < DEB_FAILSAFE &&
					++ c_pTcRef->real.mcDrv.wFailSafeDeb >= DEB_FAILSAFE) {
				SHOT(SCENE_ID_HEADCAR, 143);
			}
		}
		else {
			c_pTcRef->real.mcDrv.wFailSafeDeb = 0;
			CUT(SCENE_ID_HEADCAR, 143);
		}
	}

	if (!GETTBIT(CAR_HEAD, TCDIB_TRSF))
		SHOT(c_pTcRef->real.nAddr < 3 ? 0 : 7, c_pTcRef->real.nAddr < 3 ? 585 : 586);
	else	CUT(c_pTcRef->real.nAddr < 3 ? 0 : 7, c_pTcRef->real.nAddr < 3 ? 585 : 586);
	if (!GETTBIT(CAR_TAIL, TCDIB_TRSF))
		SHOT(c_pTcRef->real.nAddr < 3 ? 7 : 0, c_pTcRef->real.nAddr < 3 ? 586 : 585);
	else	CUT(c_pTcRef->real.nAddr < 3 ? 7 : 0, c_pTcRef->real.nAddr < 3 ? 586 : 585);
	if (!GETTBIT(CAR_HEAD, TCDIB_PISF))
		SHOT(c_pTcRef->real.nAddr < 3 ? 0 : 7, c_pTcRef->real.nAddr < 3 ? 558 : 559);
	else	CUT(c_pTcRef->real.nAddr < 3 ? 0 : 7, c_pTcRef->real.nAddr < 3 ? 558 : 559);
	if (!GETTBIT(CAR_TAIL, TCDIB_PISF))
		SHOT(c_pTcRef->real.nAddr < 3 ? 7 : 0, c_pTcRef->real.nAddr < 3 ? 559 : 558);
	else	CUT(c_pTcRef->real.nAddr < 3 ? 7 : 0, c_pTcRef->real.nAddr < 3 ? 559 : 558);
	if (!GETTBIT(CAR_HEAD, TCDIB_HTCF))
		SHOT(c_pTcRef->real.nAddr < 3 ? 0 : 7, c_pTcRef->real.nAddr < 3 ? 543 : 544);
	else	CUT(c_pTcRef->real.nAddr < 3 ? 0 : 7, c_pTcRef->real.nAddr < 3 ? 543 : 544);
	if (!GETTBIT(CAR_TAIL, TCDIB_HTCF))
		SHOT(c_pTcRef->real.nAddr < 3 ? 7 : 0, c_pTcRef->real.nAddr < 3 ? 544 : 543);
	else	CUT(c_pTcRef->real.nAddr < 3 ? 7 : 0, c_pTcRef->real.nAddr < 3 ? 544 : 543);

	// Modified 2011.09.26
	//if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
	//WORD heavy = pTcx->GetHeavyCode();
	//if (heavy > 0 && c_pTcRef->real.insp.nMode == INSPECTMODE_NON) {
	//	if (heavy != c_wHeavy || !(c_pTcRef->real.scArm.nState & ARMSTATE_BUZZHOLD)) {
	//		c_wHeavy = heavy;
	//		pTcx->BuzzCtrl(TRUE);
	//	}
	//}
	//else	pTcx->BuzzCtrl(FALSE);
	//}
	pTcx->Lock();
	memcpy((PVOID)&c_pTcRef->real.dev, (PVOID)&dev, sizeof(DEVICESTATE));
	pTcx->Unlock();
	pTcx->SetV3fTbe(c_pTcRef->real.dev.vV3fTbeL);
	// Modified 2012/05/24 ... begin
	if ((c_pTcRef->real.ptflow.cur.nState & (PTSTEP_MOVE | PTFLOW_UP)) != (PTSTEP_MOVE | PTFLOW_UP))
		CheckVoltage();
	// ... end
	CtrlEach();
}
