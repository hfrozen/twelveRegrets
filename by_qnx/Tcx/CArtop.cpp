/*
 * CArtop.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include "CArtop.h"
#include "CTcx.h"

#define	INITVARDEB()	c_uMcDrvVarDeb = DEB_MCDRV
#define	INITPOSDEB()	c_uMcDrvPosDeb = DEB_MCDRV

CArtop::CArtop()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
	c_nrbt.nState = 0;
	c_nrbt.wTimer = 0;
	INITVARDEB();
	INITPOSDEB();
	//c_wPwmDeb = DEBOUNCE_PWM;
	c_bCapedEpcorSrc = FALSE;
	c_bEpcor = c_bEpcorBuf = FALSE;
	c_wEpcorDeb = 0;
	//memset((PVOID)&c_ms, 0, sizeof(MOVESTATE));
}

CArtop::~CArtop()
{
}

void CArtop::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
	pTcx->TimeRegister(&c_pTcRef->real.ms.hb.wTimer);
	pTcx->TimeRegister(&c_nrbt.wTimer);
}

void CArtop::ReleaseAll()
{
	ForceFullServiceBrake(FALSE);
	ForceHoldBrake(FALSE);
	c_pTcRef->real.ms.hb.wState &= ~HOLDBRAKE_ACTIVE;
}

void CArtop::RecoverAll()
{
	if (c_pTcRef->real.ms.wBrake & (MSTATE_FSBATMWB | MSTATE_FSBATWRECKDIR | MSTATE_NORMALFSB))
		ForceFullServiceBrake(TRUE);
	if (c_pTcRef->real.ms.wBrake & MSTATE_HOLDINGBRAKE) {
		ForceHoldBrake(TRUE);
		c_pTcRef->real.ms.hb.wState |= HOLDBRAKE_ACTIVE;
	}
}

// Modified 2013/02/13
//void CArtop::PwmToPower(BYTE convSign)
//{
//	//if (convSign == PWM_CONVTOBA)	c_pTcRef->real.rescPwr.nWork |= 1;
//	//else	c_pTcRef->real.rescPwr.nWork |= 2;
//	CTcx* pTcx = (CTcx*)c_pParent;
//	PWMARCH pwm;
//	memcpy((PVOID)&pwm, (PVOID)&c_pDoz->ccs[__PWM].real.pwm, sizeof(PWMARCH));
//	WORD period = pwm.high + pwm.period;
//	BOOL bValid = TRUE;
//	if (period > PWMPERIOD_MAX || period < PWMPERIOD_MIN ||
//			pwm.high == 0 || pwm.period == 0) {
//		// Modified 2013/02/05
//		//SHOT(SCENE_ID_HEADCAR, 592);
//		//if (-- c_wPwmDeb == 0) {
//		//	c_pTcRef->real.rescPwr.v = 10;
//		//	c_wPwmDeb = DEBOUNCE_PWM;
//		//}
//		// hold previous value
//		if (++ c_pTcRef->real.pwmbk.nFail >= COUNT_PWMFAIL) {
//			SHOT(SCENE_ID_HEADCAR, 592);
//			c_pTcRef->real.rescPwr.v = 10;
//			c_pTcRef->real.pwmbk.nFail = COUNT_PWMFAIL;
//			bValid = FALSE;
//		}
//		else {
//			pwm.high = c_pTcRef->real.pwmbk.high;
//			pwm.period = c_pTcRef->real.pwmbk.period;
//			bValid = TRUE;
//		}
//	}
//	else {
//		c_pTcRef->real.pwmbk.high = pwm.high;
//		c_pTcRef->real.pwmbk.period = pwm.period;
//		c_pTcRef->real.pwmbk.nFail = 0;
//		bValid = TRUE;
//	}
//
//	if (bValid) {
//		c_pTcRef->real.rescPwr.v = (WORD) ((DWORD)(pwm.period - PWMOFFSET_ZERO) * PWMOFFSET_SPAN / (DWORD) period);
//		//c_wPwmDeb = DEBOUNCE_PWM;
//		CUT(SCENE_ID_HEADCAR, 592);
//		WORD max = convSign == PWM_CONVTOBA ? 85 : 90;
//		if (c_pTcRef->real.rescPwr.v < 10 || c_pTcRef->real.rescPwr.v > max) {
//			SHOT(SCENE_ID_HEADCAR, 593);
//			if (c_pTcRef->real.rescPwr.v < 10)	c_pTcRef->real.rescPwr.v = 10;
//			else	c_pTcRef->real.rescPwr.v = max;
//		}
//		else	CUT(SCENE_ID_HEADCAR, 593);
//	}
//	long l = (long)(c_pTcRef->real.rescPwr.v - 10);
//	switch (convSign) {
//	case PWM_CONVTOBA :
//		c_pTcRef->real.rescPwr.vTbeB = (_TV_)(-870L * l / 75L);
//		c_pTcRef->real.rescPwr.percent = (WORD)(l * 100L / 75L);
//		break;
//	case PWM_CONVTOB :
//		c_pTcRef->real.rescPwr.vTbeB = (_TV_)(-1085L * l / 80L);	// 72.5%(-10%) -> -848
//		c_pTcRef->real.rescPwr.percent = (WORD)(l * 128L / 80L);	// 72.5%(-10%) -> 100%
//		break;
//	default :
//		c_pTcRef->real.rescPwr.vTbeB = (_TV_)(1024L * l / 80L);
//		c_pTcRef->real.rescPwr.percent = (WORD)(l * 100L / 80L);
//		break;
//	}
//}
//

WORD CArtop::CalcWithRounded(long v, long num, long den)
{
	WORD w = (WORD)(v * num / den);
	if (w < 100) {
		WORD rem = (WORD)(v * num % den);
		if (rem >= (den / 2))	++ w;
	}
	return w;
}

void CArtop::PwmToPower(BYTE convSign)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PWMARCH pwm;
	memcpy((PVOID)&pwm, (PVOID)&c_pDoz->ccs[__PWM].real.pwm, sizeof(PWMARCH));
	WORD period = pwm.high + pwm.period;
	BOOL bValid = TRUE;
	// check frequency
	if (period > PWMPERIOD_MAX || period < PWMPERIOD_MIN ||
			pwm.high == 0 || pwm.period == 0) {
		if (++ c_pTcRef->real.pwmbk.nDebFreq >= DEB_PWM) {
			// Modified 2013/03/20
			//SHOT(SCENE_ID_HEADCAR, 592);
			if (!GETTBIT(OWN_SPACEA, TCDIB_EPCOR))	SHOT(SCENE_ID_HEADCAR, 592);
			c_pTcRef->real.rescPwr.v = 10;
			c_pTcRef->real.pwmbk.nDebFreq = DEB_PWM;
			bValid = FALSE;
		}
		else {
			pwm.high = c_pTcRef->real.pwmbk.high;
			pwm.period = c_pTcRef->real.pwmbk.period;
			bValid = TRUE;
		}
	}
	else {
		c_pTcRef->real.pwmbk.high = pwm.high;
		c_pTcRef->real.pwmbk.period = pwm.period;
		c_pTcRef->real.pwmbk.nDebFreq = 0;
		bValid = TRUE;
		CUT(SCENE_ID_HEADCAR, 592);
	}
	// check duty
	if (bValid) {
		c_pTcRef->real.rescPwr.v = (WORD) (((DWORD)(pwm.period - PWMOFFSET_ZERO) * PWMOFFSET_SPAN) * 10L / (DWORD) period);
		// Modified 2013/03/22
		if (c_pTcRef->real.rescPwr.v < 95 /*100*/ || c_pTcRef->real.rescPwr.v > 950 /*900*/) {
			if (++ c_pTcRef->real.pwmbk.nDebDuty >= DEB_PWM) {
				// Modified 2013/03/20
				//SHOT(SCENE_ID_HEADCAR, 593);
				if (!GETTBIT(OWN_SPACEA, TCDIB_EPCOR))	SHOT(SCENE_ID_HEADCAR, 593);
				c_pTcRef->real.rescPwr.v = 100;
				c_pTcRef->real.pwmbk.nDebDuty = DEB_PWM;
			}
			else	c_pTcRef->real.rescPwr.v = c_pTcRef->real.pwmbk.v;
		}
		else {
			// Modified 2013/03/22
			if (c_pTcRef->real.rescPwr.v < 100)	c_pTcRef->real.rescPwr.v = 100;
			else if (c_pTcRef->real.rescPwr.v > 900)	c_pTcRef->real.rescPwr.v = 900;

			c_pTcRef->real.pwmbk.v = c_pTcRef->real.rescPwr.v;
			c_pTcRef->real.pwmbk.nDebDuty = 0;
			CUT(SCENE_ID_HEADCAR, 593);
		}
	}

	long l = (long)(c_pTcRef->real.rescPwr.v - 100);
	switch (convSign) {
	// Modified 2013/02/26
	//case PWM_CONVTOBA :
	//case PWM_CONVTOB :
	//	c_pTcRef->real.rescPwr.vTbeB = (_TV_)(-TEBEMAX_BRAKE * l / 80L);
	//	c_pTcRef->real.rescPwr.percent = (WORD)(l * 100L / 80L);
	//	break;
	case PWM_CONVTOBA :
		c_pTcRef->real.rescPwr.vTbeB = (_TV_)(-870L * l / 750L);
		//c_pTcRef->real.rescPwr.percent = (WORD)(l * 100L / 750L);
		c_pTcRef->real.rescPwr.percent = CalcWithRounded(l, 100L, 750L);
		break;
	case PWM_CONVTOB :
		if (l > 625)	l = 625;	// 62.5
		c_pTcRef->real.rescPwr.vTbeB = (_TV_)(-1070L * l / 800L);	// 72.5%(-10%, PWM output at B7) -> -836
		//c_pTcRef->real.rescPwr.percent = (WORD)(l * 128L / 800L);	// 72.5%(-10%, PWM output at B7) -> 100%
		c_pTcRef->real.rescPwr.percent = CalcWithRounded(l, 128L, 800L);	// 72.5%(-10%, PWM output at B7) -> 100%
		break;
	default :
		c_pTcRef->real.rescPwr.vTbeB = (_TV_)(TEBEMAX_POWER * l / 800L);
		//c_pTcRef->real.rescPwr.percent = (WORD)(l * 100L / 800L);
		c_pTcRef->real.rescPwr.percent = CalcWithRounded(l, 100L, 800L);
		break;
	}
}

// Appended 2012/03/06 ... begin
void CArtop::PowerToPwm(BYTE nIndex)
{
	//c_pTcRef->real.rescPwr.nWork |= 4;
	if (nIndex < MCDSV_B7POS)	nIndex = MCDSV_B7POS;
	// Modified 2012/11/14
	//if (IsRescueActiveDrive() && nIndex > MCDSV_NPOS)	nIndex = MCDSV_NPOS;
	c_pTcRef->real.rescPwr.v = c_pTcRef->real.mcDrv.inst.v;
	c_pTcRef->real.rescPwr.vTbeB = c_pTcRef->real.mcDrv.inst.vTbeB;
	c_pTcRef->real.rescPwr.percent = c_pTcRef->real.mcDrv.inst.percent;
	CTcx* pTcx = (CTcx*)c_pParent;
	WORD per10 = pTcx->GetNotchRescue(nIndex);
	PWMARCH pwm;
	//pwm.high = PWMPERIOD_NORMAL - per10 * 2;	// for invert at pilot
	pwm.high = per10 * 2;
	pwm.period = PWMPERIOD_NORMAL;
	pwm.nMode = PWMMODE_OUTPUT;
	memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.pwm, (PVOID)&pwm, sizeof(PWMARCH));
	if (IsRescueActiveDrive()) {
		CLRNBIT(OWN_SPACEA, NESTED_ROPR);
		CLRNBIT(OWN_SPACEA, NESTED_ROBR);
		if (nIndex < MCDSV_NPOS) {
			CLRTBIT(OWN_SPACEA, TCDOB_ROPR);
			SETTBIT(OWN_SPACEA, TCDOB_ROBR);
		}
		else if (nIndex > MCDSV_NPOS) {
			CLRTBIT(OWN_SPACEA, TCDOB_ROBR);
			SETTBIT(OWN_SPACEA, TCDOB_ROPR);
		}
		else {
			CLRTBIT(OWN_SPACEA, TCDOB_ROPR);
			CLRTBIT(OWN_SPACEA, TCDOB_ROBR);
		}
	}
	else if (IsRescueActiveUndergo()) {
		CLRTBIT(OWN_SPACEA, TCDOB_ROPR);
		CLRTBIT(OWN_SPACEA, TCDOB_ROBR);
		if (nIndex < MCDSV_NPOS) {
			CLRNBIT(OWN_SPACEA, NESTED_ROPR);
			SETNBIT(OWN_SPACEA, NESTED_ROBR);
		}
		else if (nIndex > MCDSV_NPOS) {
			CLRNBIT(OWN_SPACEA, NESTED_ROBR);
			SETNBIT(OWN_SPACEA, NESTED_ROPR);
		}
		else {
			CLRNBIT(OWN_SPACEA, NESTED_ROPR);
			CLRNBIT(OWN_SPACEA, NESTED_ROBR);
		}
	}
}
// ... end

void CArtop::Drive()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Appended 2012/03/06
	//c_pTcRef->real.rescPwr.nWork = 0;
	// Modified 2012/11/29
	//if (IsNotRescue()) {
	// Modified 2012/02/21
	//if (IsNotRescue() || IsRescueActiveDrive()) {
	if (!GETTBIT(OWN_SPACEA, TCDIB_P)) {
		BYTE stick = pTcx->GetTrainDir(TRUE);
		if (stick == 3) {
			c_pTcRef->real.ms.wBrake |= MSTATE_FSBATWRECKDIR;
			if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
				ForceFullServiceBrake(TRUE);
				SHOT(SCENE_ID_HEADCAR, 142);
			}
			return;
		}
		else {
			c_pTcRef->real.ms.wBrake &= ~MSTATE_FSBATWRECKDIR;
			if (stick != c_pTcRef->real.ms.cs.nState) {
				c_pTcRef->real.ms.cs.bValid = FALSE;
				c_pTcRef->real.ms.cs.nState = stick;
				c_pTcRef->real.ms.cs.wDeb = DEBOUNCE_STICK;
			}
			else if (c_pTcRef->real.ms.cs.wDeb > 0 && -- c_pTcRef->real.ms.cs.wDeb == 0) {
				c_pTcRef->real.ms.cs.bValid = TRUE;
				c_pTcRef->real.ms.nDir = c_pTcRef->real.ms.cs.nState;
			}
			if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE)	CUT(SCENE_ID_HEADCAR, 142);
		}
	}
	// Modified 2012/03/06 ... begin
	//}
	// ... end

	if (!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE)) {
		pTcx->Powering(FALSE, FALSE, c_pTcRef->real.ms.nDir, 0);
		return;
	}

	// Modified 2012/03/06 ... begin
	//AtoToTbe(pTcx->GetAto(FALSE)->d.st.vTbeB);		// for fsb at mwb
	if (IsNotRescue())	AtoToTbe(pTcx->GetAto(FALSE)->d.st.vTbeB);		// for fsb at mwb
	// Modified 2012/11/29
	//if (IsNotRescue() || IsRescueActiveDrive())	AtoToTbe(pTcx->GetAto(FALSE)->d.st.vTbeB);		// for fsb at mwb
	// No need ATO because unconditional emergency mode at rescue mode

	// Modified 2012/09/17 ... begin
	if(CheckSecurityBrake())	c_pTcRef->real.ms.wBrake |= MSTATE_SECURITYBRAKE;
	else	c_pTcRef->real.ms.wBrake &= ~MSTATE_SECURITYBRAKE;

	if (CheckParkingRelease())	c_pTcRef->real.ms.wBrake |= MSTATE_PARKINGBRAKE;
	else	c_pTcRef->real.ms.wBrake &= ~MSTATE_PARKINGBRAKE;

	// Modified 2012/09/26
	ScanEmergency();

	if (IsNotRescue()) {
		//ScanEmergency();
		// Modified 2013/11/02
		if (!(c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY)) {
			if (CheckBccoOverNine())	c_pTcRef->real.ms.wBrake |= MSTATE_BCCOOVERNINE;
			else	c_pTcRef->real.ms.wBrake &= ~MSTATE_BCCOOVERNINE;
		}

		if (CheckFullServiceBrake())	c_pTcRef->real.ms.wBrake |= MSTATE_NORMALFSB;
		else	c_pTcRef->real.ms.wBrake &= ~MSTATE_NORMALFSB;
	}

	// Modified 2013/01/28
	if (!FBTBIT(OWN_SPACEA, TCDOB_TCR)) {
		// tcr on when tail car or halfway neutral...
		if (CheckHoldBrake())	c_pTcRef->real.ms.wBrake |= MSTATE_HOLDINGBRAKE;
		else	c_pTcRef->real.ms.wBrake &= ~MSTATE_HOLDINGBRAKE;
	}

	if (CheckNonReleaseBrake())	c_pTcRef->real.ms.wBrake |= MSTATE_ECUNRBDETECT;
	else	c_pTcRef->real.ms.wBrake &= ~MSTATE_ECUNRBDETECT;

	CheckCompulsoryRelease();
	// end

	if (c_pTcRef->real.op.nMode == OPMODE_AUTO ||
			c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS) {
		if (!GETTBIT(OWN_SPACEA, TCDIB_C))
			SHOT(SCENE_ID_HEADCAR, 103);
		else {
			if (pTcx->GetLocalCondition(LOCALID_ATO)) {
				if (c_pTcRef->real.atoCmd.nState & INSTSTATE_VALID) {
					if (c_pTcRef->real.atoCmd.vTbeB > 0) {
						if ((c_pTcRef->real.ms.wBrake & MSTATE_PWRDISABLE) ||
							(!GETTBIT(OWN_SPACEA, TCDIB_ADSBS) &&
							!GETTBIT(OWN_SPACEA, TCDIB_ADSCR))) {
							pTcx->PoweringMask();
							// Modified 2013/03/20
							if (c_pTcRef->real.ms.wBrake & MSTATE_PWRDISABLE)
								SHOT(SCENE_ID_HEADCAR, ScanBrakeShot(c_pTcRef->real.ms.wBrake & MSTATE_PWRDISABLE, TRUE));	// 105-112
							else	SHOT(SCENE_ID_HEADCAR, 113);
						}
						else {
							pTcx->Powering(TRUE, FALSE, 1, c_pTcRef->real.atoCmd.vTbeB);	// c_pTcRef->real.ms.nDir = 0
							CUTS(SCENE_ID_HEADCAR, 105, 113);
							CUTS(SCENE_ID_HEADCAR, 623, 625);
						}
					}
					// Modified 2011/12/14 - begin
					//else	pTcx->Powering(FALSE, TRUE, 1, c_pTcRef->real.atoCmd.vTbeB);
					else if (c_pTcRef->real.atoCmd.vTbeB < 0)
						pTcx->Powering(FALSE, TRUE, 1, c_pTcRef->real.atoCmd.vTbeB);
					else	pTcx->Powering(FALSE, FALSE, 1, c_pTcRef->real.atoCmd.vTbeB);
					// - end
					//CUTS(SCENE_ID_HEADCAR, 623, 625);
					CUT(SCENE_ID_HEADCAR, 104);
				}
				else	SHOT(SCENE_ID_HEADCAR, 104);
			}
			CUT(SCENE_ID_HEADCAR, 103);
		}
	}
	else if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ||
			c_pTcRef->real.op.nMode == OPMODE_MANUAL ||
			c_pTcRef->real.op.nMode == OPMODE_YARD) {
		// Modified 2012/03/06 ... begin
		BOOL bValid = TRUE;
		UCURV notch = 0;
		_TV_ tbe = 0;
		if (IsRescueTrain()) {
			notch = 4;
			PwmToPower(PWM_CONVTOBA);
			tbe = c_pTcRef->real.rescPwr.vTbeB;
		}
		// Modified 2013/02/05
		//else if (IsRescuePassiveUndergo()) {
		//	if (GETTBIT(CAR_TAIL, TCDIB_ROBR)) {
		//		notch = 4;
		//		PwmToPower(PWM_CONVTOB);
		//	}
		//	else {
		//		notch = 1;
		//		c_pTcRef->real.rescPwr.vTbeB = 0;
		//		c_pTcRef->real.rescPwr.percent = 0;
		//	}
		//	tbe = c_pTcRef->real.rescPwr.vTbeB;
		//}
		//else if (IsRescuePassiveDrive()) {
		else if (IsRescuePassiveUndergo() || IsRescuePassiveDrive()) {
			if (GETTBIT(OWN_SPACEA, TCDIB_ROPR)) {
				notch = 2;
				PwmToPower(PWM_CONVTOP);
			}
			else if (GETTBIT(OWN_SPACEA, TCDIB_ROBR)) {
				notch = 4;
				PwmToPower(PWM_CONVTOB);
			}
			else {
				notch = 1;
				c_pTcRef->real.rescPwr.vTbeB = 0;
				c_pTcRef->real.rescPwr.percent = 0;
			}
			tbe = c_pTcRef->real.rescPwr.vTbeB;
		}
		else {
			if (c_pTcRef->real.dead.nActive == DEADMAN_POWERMASK) {
				pTcx->PoweringMask();
				if (IsRescueActiveDrive() || IsRescueActiveUndergo())	PowerToPwm(MCDSV_NPOS);	// Power cut cause deadman
				bValid = FALSE;
			}
			else {
				if (!c_pTcRef->real.mcDrv.bValid) {
					pTcx->Powering(FALSE, TRUE, c_pTcRef->real.ms.nDir, TBE_BRAKEMAX);
					if (IsRescueActiveDrive() || IsRescueActiveUndergo())	PowerToPwm(MCDSV_B7POS);	// brake max cause !bValid
					SHOT(SCENE_ID_HEADCAR, 119);
					bValid = FALSE;
				}
				else {
					notch = GETTBIT(OWN_SPACEA, TCDIB_C) ? 1 : 0;
					if (GETTBIT(OWN_SPACEA, TCDIB_P))	notch |= 2;
					if (GETTBIT(OWN_SPACEA, TCDIB_B))	notch |= 4;
					else if (c_pTcRef->real.mcDrv.nIndex < MCDSV_B7POS)	notch |= 4;
					tbe = c_pTcRef->real.mcDrv.inst.vTbeB;
					CUT(SCENE_ID_HEADCAR, 119);
				}
			}
		}

		if (bValid) {
			if (notch == 1 || notch == 2 || notch == 4) {
				switch (notch) {
				case 2 :	// power
					if (tbe > 0) {
						WORD mask = IsNotRescue() ? MSTATE_PWRDISABLE : MSTATE_PWRDISABLERESCUE;
						// Modified 2013/11/02
						//if (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY)
						//	mask &= ~MSTATE_BCCOOVERNINE;				// ignore at laboratory, cause always in this condition
						if (c_pTcRef->real.ms.wMask & MOVESTATE_MASKSHB)
							mask &= ~(MSTATE_PARKINGBRAKE | MSTATE_SECURITYBRAKE);
						if ((c_pTcRef->real.ms.wBrake & mask) ||
							(!GETTBIT(OWN_SPACEA, TCDIB_ADSBS) &&
							!GETTBIT(OWN_SPACEA, TCDIB_ADSCR))) {
							// Modified 2013/11/02... begin
							if (c_pTcRef->real.dev.wV3fSqts != 0 && c_pTcRef->real.ptflow.nPos == 0 && (c_pTcRef->real.ms.wBrake & MSTATE_PARKINGBRAKE)) {
								// VVVF Sequence test...
								pTcx->Powering(TRUE, FALSE, c_pTcRef->real.ms.nDir, tbe, TRUE);
								SHOT(SCENE_ID_HEADCAR, 144);
							}
							else {
							// ... end
								pTcx->PoweringMask();
								if (IsRescueActiveDrive() || IsRescueActiveUndergo())	PowerToPwm(MCDSV_NPOS);
								// Modified 2013/03/20
								if (c_pTcRef->real.ms.wBrake & mask)	// MSTATE_PWRDISABLE)
									SHOT(SCENE_ID_HEADCAR, ScanBrakeShot(c_pTcRef->real.ms.wBrake & mask, FALSE));	// 120-127
								else	SHOT(SCENE_ID_HEADCAR, 128);
							}
						}
						else {
							// Modified 2013/02/05
							//if (IsNotRescue() || IsRescuePassiveDrive() || IsRescueActiveUndergo() || IsRescueActiveDrive())
							//	pTcx->Powering(TRUE, FALSE, c_pTcRef->real.ms.nDir, tbe);
							pTcx->Powering(TRUE, FALSE, c_pTcRef->real.ms.nDir, tbe);
							if (IsRescueActiveDrive() || IsRescueActiveUndergo())	PowerToPwm(c_pTcRef->real.mcDrv.nIndex);
							// Modified 2013/03/04
							//if (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY)
							//	pTcx->InterTrigger(&c_pTcRef->real.cond.wReleaseTimer, 0);	// at lab.
							CUTS(SCENE_ID_HEADCAR, 120, 128);
						}
						INITVARDEB();
						//CUT(SCENE_ID_HEADCAR, 115);
						CUT(SCENE_ID_HEADCAR, 117);
					}
					else {
						if (c_uMcDrvVarDeb != 0 && -- c_uMcDrvVarDeb == 0) {
							if (IsNotRescue() || IsRescueActiveUndergo() || IsRescueActiveDrive()) {	// use mascon
								//SHOT(SCENE_ID_HEADCAR, 115);
								SHOT(SCENE_ID_HEADCAR, 117);
							}
							pTcx->PoweringMask();
						}
					}
					break;
				case 4 :	// brake
					if (tbe < 0) {
						pTcx->Powering(FALSE, TRUE, c_pTcRef->real.ms.nDir, tbe);
						if (IsRescueActiveDrive() || IsRescueActiveUndergo())	PowerToPwm(c_pTcRef->real.mcDrv.nIndex);
						INITVARDEB();
						//CUT(SCENE_ID_HEADCAR, 115);
						CUT(SCENE_ID_HEADCAR, 118);
					}
					else {
						if (c_uMcDrvVarDeb != 0 && -- c_uMcDrvVarDeb == 0) {
							if (IsNotRescue() || IsRescueActiveUndergo() || IsRescueActiveDrive()) {	// use mascon
								//SHOT(SCENE_ID_HEADCAR, 115);
								SHOT(SCENE_ID_HEADCAR, 118);
							}
							pTcx->PoweringMask();
						}
					}
					CUT(SCENE_ID_HEADCAR, 144);
					break;
				default :	// coast
					if (tbe == 0) {
						pTcx->Powering(FALSE, FALSE, c_pTcRef->real.ms.nDir, 0);
						if (IsRescueActiveDrive() || IsRescueActiveUndergo())	PowerToPwm(MCDSV_NPOS);
						INITVARDEB();
						//CUT(SCENE_ID_HEADCAR, 115);
						CUT(SCENE_ID_HEADCAR, 116);
					}
					else {
						if (c_uMcDrvVarDeb != 0 && -- c_uMcDrvVarDeb == 0) {
							if (IsNotRescue() || IsRescueActiveUndergo() || IsRescueActiveDrive()) {	// use mascon
								//SHOT(SCENE_ID_HEADCAR, 115);
								SHOT(SCENE_ID_HEADCAR, 116);
							}
							pTcx->PoweringMask();
						}
					}
					CUT(SCENE_ID_HEADCAR, 144);
					break;
				}
				INITPOSDEB();
				CUT(SCENE_ID_HEADCAR, 114);
			}
			else {
				if (c_uMcDrvPosDeb != 0 && -- c_uMcDrvPosDeb == 0) {
					if (IsNotRescue() || IsRescueActiveUndergo() || IsRescueActiveDrive())	// use mascon
						SHOT(SCENE_ID_HEADCAR, 114);
					pTcx->PoweringMask();
				}
			}
			CUT(SCENE_ID_HEADCAR, 119);
		}
	}
	// ... end
}

WORD CArtop::ScanBrakeShot(WORD brake, BOOL mode)
{
	int shot = 0;
	// Modified 2013/03/20
//	if (c_pTcRef->real.ms.wBrake & MSTATE_EMERGENCYBRAKE)			shot = 105;
//	else if (c_pTcRef->real.ms.wBrake & MSTATE_NORMALFSB)			shot = 106;
//	else if (c_pTcRef->real.ms.wBrake & (MSTATE_FSBATMWB | MSTATE_FSBATWRECKDIR))	shot = 107;
//	else if (c_pTcRef->real.ms.wBrake & MSTATE_BCCOOVERNINE)		shot = 108;
//	else if (c_pTcRef->real.ms.wBrake & MSTATE_SECURITYBRAKE)		shot = 109;
//	else if (c_pTcRef->real.ms.wBrake & MSTATE_PARKINGBRAKE)		shot = 110;
//	else if (c_pTcRef->real.ms.wBrake & MSTATE_ECUNRBDETECT)		shot = 111;
//	//else if (c_pTcRef->real.ms.wBrake & MSTATE_DEADMANPOWERMASK)	shot = 112;
//	else if (c_pTcRef->real.dead.nActive != 0)		shot = 112;
//	else	shot = 150;
	if (brake & MSTATE_EMERGENCYBRAKE)			shot = 105;
	else if (brake & MSTATE_NORMALFSB)			shot = 106;
	else if (brake & (MSTATE_FSBATMWB | MSTATE_FSBATWRECKDIR))	shot = 107;
	else if (brake & MSTATE_BCCOOVERNINE)		shot = 108;
	else if (brake & MSTATE_SECURITYBRAKE)		shot = 109;
	else if (brake & MSTATE_PARKINGBRAKE)		shot = 110;
	else if (brake & MSTATE_ECUNRBDETECT)		shot = 111;
	//else if (brake & MSTATE_DEADMANPOWERMASK)	shot = 112;
	else if (c_pTcRef->real.dead.nActive != 0)		shot = 112;
	else	shot = 150;
	if (!mode && shot != 0)	shot += 15;
	return shot;
}

BOOL CArtop::ScanEmergency()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (CheckEmergencyBrake()) {
		c_pTcRef->real.ms.wBrake |= MSTATE_EMERGENCYBRAKE;
		SETTBIT(OWN_SPACEA, TCDOB_EBLP);
		return TRUE;
	}
	else {
		if (c_pTcRef->real.ms.wBrake & MSTATE_EMERGENCYBRAKE) {
			// emergency braking...
			// Modified 2012/09/26
			if (((c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) && c_pTcRef->real.mcDrv.bValid) ||
					IsRescue()) {
			//if (((c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) &&
				//c_pTcRef->real.mcDrv.bValid /*&&
				// Modified 2011.09.26
				//c_pTcRef->real.mcDrv.nIndex <= MCDSV_B7POS*/)) {
				//||
				//(c_pTcRef->real.nDuty > 1 && !FBTBIT(CAR_HEAD, TCDOB_EBLP))) {
				c_pTcRef->real.ms.wBrake &= ~MSTATE_EMERGENCYBRAKE;
				CLRTBIT(OWN_SPACEA, TCDOB_EBLP);
				CUT(SCENE_ID_HEADCAR, 428);
				return FALSE;	// exit from emergency
			}
			return TRUE;
		}
		return FALSE;
	}
}

BOOL CArtop::CheckEmergencyBrake()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL epcor = GETTBIT(OWN_SPACEA, TCDIB_EPCOR) ? TRUE : FALSE;

	// Modified 2011/12/16 ... begin
	if (epcor != c_bEpcorBuf) {
		c_bEpcorBuf = epcor;
		c_wEpcorDeb = DEBOUNCE_EPCOR;
	}
	else if (c_wEpcorDeb > 0 && -- c_wEpcorDeb == 0) {
		c_bEpcor = c_bEpcorBuf;
	}
	epcor = c_bEpcor;
	// ... end

	c_pDoz->tcs[OWN_SPACEA].real.cV3f.state.b.epcor = epcor;
	if (epcor) {
		SHOT(SCENE_ID_HEADCAR, 460);
		if (!GETTBIT(OWN_SPACEA, TCDIB_EBRM)) {
			SHOT(SCENE_ID_HEADCAR, 455);
			c_bCapedEpcorSrc = TRUE;
		}
		// Modified 2013/03/20
		//else if (!GETTBIT(OWN_SPACEA, TCDIB_EBRA)) {
		//	SHOT(SCENE_ID_HEADCAR, 456);
		//	c_bCapedEpcorSrc = TRUE;
		//}
		//else if (GETTBIT(OWN_SPACEA, TCDIB_EBS)) {
		//	SHOT(SCENE_ID_HEADCAR, 457);
		//	c_bCapedEpcorSrc = TRUE;
		//}
		//else if (c_pTcRef->real.dev.wMrp != 0) {
		//	SHOT(SCENE_ID_HEADCAR, 443);
		//	c_bCapedEpcorSrc = TRUE;
		//}
		//else if (!c_bCapedEpcorSrc) {
		//	SHOT(SCENE_ID_HEADCAR, 428);
		//	c_bCapedEpcorSrc = TRUE;
		//}
		//else {
		//	// c_bCapedEpcorSrc is TRUE, wait !epcor
		//}
		if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && !GETTBIT(OWN_SPACEA, TCDIB_EBRA)) {
			SHOT(SCENE_ID_HEADCAR, 456);
			c_bCapedEpcorSrc = TRUE;
		}
		if (GETTBIT(OWN_SPACEA, TCDIB_EBS)) {
			SHOT(SCENE_ID_HEADCAR, 457);
			c_bCapedEpcorSrc = TRUE;
		}
		if (c_pTcRef->real.dev.wMrp != 0) {
			SHOT(SCENE_ID_HEADCAR, 443);
			c_bCapedEpcorSrc = TRUE;
		}
		if (!c_bCapedEpcorSrc) {
			SHOT(SCENE_ID_HEADCAR, 428);
			c_bCapedEpcorSrc = TRUE;
		}
	}
	else {
		c_bCapedEpcorSrc = FALSE;
		CUT(SCENE_ID_HEADCAR, 428);
		CUT(SCENE_ID_HEADCAR, 443);
		CUTS(SCENE_ID_HEADCAR, 455, 457);
		CUT(SCENE_ID_HEADCAR, 460);
	}

	BOOL ebcos = GETTBIT(OWN_SPACEA, TCDIB_EBCOS) ? TRUE : FALSE;
	if (ebcos)	SETTBIT(OWN_SPACEA, TCDOB_EBCOSLP);
	else	CLRTBIT(OWN_SPACEA, TCDOB_EBCOSLP);
	if (ebcos)	SHOT(SCENE_ID_HEADCAR, 459);
	else	CUT(SCENE_ID_HEADCAR, 459);

	return epcor;
}

BOOL CArtop::CheckBccoOverNine()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if ((GetSettingBits(c_pTcRef->real.dev.wFailBccs1) +
			GetSettingBits(c_pTcRef->real.dev.wFailBccs2)) >=
			POWERMASK_BCCOLENGTH) {
		pTcx->PoweringMask();
		SHOT(SCENE_ID_HEADCAR, 427);
		return TRUE;
	}
	else {
		CUT(SCENE_ID_HEADCAR, 427);
		return FALSE;
	}
}

BOOL CArtop::CheckFullServiceBrake()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL bAto = (c_pTcRef->real.op.nMode == OPMODE_AUTO ||
			c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS) ? TRUE : FALSE;
	BOOL bFsb = FALSE;
	BOOL bAtoinh = FALSE;
	BYTE src = 0;
	// Modified 2013/11/02
	//if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY) {
	if (c_pTcRef->real.op.nMode == OPMODE_MANUAL || c_pTcRef->real.op.nMode == OPMODE_YARD ||
			c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS || c_pTcRef->real.op.nMode == OPMODE_AUTO) {
		PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
		if (paAtc != NULL && paAtc->d.st.req.b.fsbr) {		// common
			SHOT(SCENE_ID_HEADCAR, 461);
			bFsb = TRUE;
			src = 1;
		}
		// Modified 2013/03/12
		//else if (c_pTcRef->real.dev.wNrbi != 0 || c_pTcRef->real.dev.wNrbc != 0) {	// common
		//	SHOT(SCENE_ID_HEADCAR, 466);
		//	bFsb = TRUE;
		//	src = 2;
		//	if (bAto) {
		//		SHOT(SCENE_ID_HEADCAR, 621);
		//		bAtoinh = TRUE;
		//	}
		//}
		// Modified 2012/12/12
		//else if (GetSettingBits(c_pTcRef->real.dev.wFailCC) >= FSBTERM_CC) {
		//	bFsb = TRUE;
		//	src = 3;
		//	if (bAto) {
		//		SHOT(SCENE_ID_HEADCAR, 620);
		//		bAtoinh = TRUE;
		//	}
		//}
		else if (c_pTcRef->real.atoCmd.nAtc1FaultCount >= FSBTERM_FAULTATCCOUNT &&
				c_pTcRef->real.atoCmd.nAtc2FaultCount >= FSBTERM_FAULTATCCOUNT) {
			SHOT(SCENE_ID_HEADCAR, 472);
			bFsb = TRUE;
			src = 4;
		}
		if (!GETTBIT(OWN_SPACEA, TCDIB_ADSBS) &&
				!GETTBIT(OWN_SPACEA, TCDIB_ADSCR) &&
				!(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF)) {	// common
			SHOT(SCENE_ID_HEADCAR, 467);
			bFsb = TRUE;
			src = 5;
		}
	}
	// Modified 2013/11/02
	//else if (c_pTcRef->real.op.nMode == OPMODE_NON) {
	else if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY){
		if (c_pTcRef->real.op.nMode == OPMODE_NON) {
			SHOT(SCENE_ID_HEADCAR, 465);
			src = 6;
		}
		else {
			SHOT(SCENE_ID_HEADCAR, 473);
			src = 7;
		}
		bFsb = TRUE;
		if (bAto)	bAtoinh = TRUE;
	}

	if (!bFsb) {
		if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
				c_pTcRef->real.op.nMode == OPMODE_AUTO) {
			if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS &&
					c_pTcRef->real.atoCmd.nAtoFaultCount >= FSBTERM_FAULTATCCOUNT) {
				SHOT(SCENE_ID_HEADCAR, 474);
				bFsb = TRUE;
				src = 8;
			}
			// Modified 2013/11/02
			else if (GETTBIT(OWN_SPACEA, TCDIB_P) ||
				GETTBIT(OWN_SPACEA, TCDIB_B) || !GETTBIT(OWN_SPACEA, TCDIB_C)) {
				// mascon not place at C
				SHOT(SCENE_ID_HEADCAR, 468);
				bFsb = TRUE;
				src = 12;
			}
			else if (GetSettingBits(c_pTcRef->real.dev.wFailV3f) >= c_pTcRef->real.cid.fsbTerm.nV3f) {
				SHOT(SCENE_ID_HEADCAR, 462);
				bFsb = TRUE;
				src = 9;
				if (bAto) {
					SHOT(SCENE_ID_HEADCAR, 617);
					bAtoinh = TRUE;
				}
			}
			else if (GetSettingBits(c_pTcRef->real.dev.wFailEcu) >= c_pTcRef->real.cid.fsbTerm.nEcu) {
				SHOT(SCENE_ID_HEADCAR, 463);
				bFsb = TRUE;
				src = 10;
				if (bAto) {
					SHOT(SCENE_ID_HEADCAR, 618);
					bAtoinh = TRUE;
				}
			}
			else if ((GetSettingBits(c_pTcRef->real.dev.wFailBccs1) +
					GetSettingBits(c_pTcRef->real.dev.wFailBccs2)) >= c_pTcRef->real.cid.fsbTerm.nBcc) {
				SHOT(SCENE_ID_HEADCAR, 464);
				bFsb = TRUE;
				src = 11;
				if (bAto) {
					SHOT(SCENE_ID_HEADCAR, 619);
					bAtoinh = TRUE;
				}
			}
			// Modified 2013/11/02
			//else if (GETTBIT(OWN_SPACEA, TCDIB_P) ||
			//	GETTBIT(OWN_SPACEA, TCDIB_B) || !GETTBIT(OWN_SPACEA, TCDIB_C)) {
			//	// mascon not place at C
			//	SHOT(SCENE_ID_HEADCAR, 468);
			//	bFsb = TRUE;
			//	src = 11;
			//}
			else if (CheckFsbAtMwb()) {
				bFsb = TRUE;
				src = 13;
			}
		}
		else {
			// manual mode
			if (!bFsb) {
				if (c_pTcRef->real.dead.nActive == DEADMAN_FSB) {
					if (c_pTcRef->real.mcDrv.bValid && c_pTcRef->real.mcDrv.nIndex < MCDSV_NPOS) {
						c_pTcRef->real.dead.nActive = 0;
						SHOT(SCENE_ID_HEADCAR, 492);
						CUT(SCENE_ID_HEADCAR, 129);
						c_pTcRef->real.dead.wTimer = 0;
						bFsb = FALSE;
					}
					else {
						bFsb = TRUE;
						src = 14;
					}
				}
				//SHOT(SCENE_ID_HEADCAR, 490);
			}
		}
	}

	if (!(c_pTcRef->real.ms.wBrake & MSTATE_EMERGENCYBRAKE)) {
		ForceFullServiceBrake(bFsb);
		c_pTcRef->real.ms.nFsbSrc = src;
	}

	if (!bFsb) {
		CUTS(SCENE_ID_HEADCAR, 461, 469);
		CUTS(SCENE_ID_HEADCAR, 472, 475);
		CUT(SCENE_ID_HEADCAR, 491);
		if (bAto)	CUTS(SCENE_ID_HEADCAR, 617, 621);
	}
	if (bAto && bAtoinh)
		c_pTcRef->real.atoCmd.nCmd |= INSTCMD_INHIBIT;
	else	c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_INHIBIT;

	return bFsb;
}

BOOL CArtop::CheckFsbAtMwb()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (!(c_pTcRef->real.atoCmd.nState & INSTSTATE_VALID))
		return FALSE;	// non fsb

	PATOAINFO paAto = pTcx->GetAto(FALSE);
	if (paAto == NULL)	return FALSE;

	if (paAto->d.st.vPwbB > 0) {
		// valid mwb
		if (c_pTcRef->real.atoCmd.vTbeB > 0) {	// powering
			CUTS(SCENE_ID_HEADCAR, 623, 624);
			SHOT(SCENE_ID_HEADCAR, 625);
			return TRUE;	// fsb
		}
		else if (c_pTcRef->real.atoCmd.vTbeB == 0) {	// no braking
			CUT(SCENE_ID_HEADCAR, 623);
			CUT(SCENE_ID_HEADCAR, 625);
			SHOT(SCENE_ID_HEADCAR, 624);
			return TRUE;	// fsb
		}
		else {
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.inst.b.mwb =
				c_pDoz->tcs[OWN_SPACEA].real.cEcu.inst.b.mwb = TRUE;
			AtoToTbe(paAto->d.st.vPwbB);
			pTcx->Powering(TRUE, FALSE, c_pTcRef->real.ms.nDir, c_pTcRef->real.atoCmd.vTbeB);
			CUTS(SCENE_ID_HEADCAR, 624, 625);
			SHOT(SCENE_ID_HEADCAR, 623);
		}
		return FALSE;	// not fsb
	}
	else {	// non mwb, so no need fsb
		CUTS(SCENE_ID_HEADCAR, 623, 625);
		return FALSE;	// not fsb
	}
}

void CArtop::ForceFullServiceBrake(BOOL bFsb)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDoz->tcs[OWN_SPACEA].real.cV3f.inst.b.fsb =
		c_pDoz->tcs[OWN_SPACEA].real.cEcu.inst.b.fsb = bFsb;
	c_pDoz->tcs[OWN_SPACEA].real.cEcu.wMsb = 0;
	c_pDoz->tcs[OWN_SPACEA].real.cV3f.wMsb = 0;
	if (bFsb) {
		pTcx->PoweringMask();
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (c_pTcRef->real.dev.wFailEcu & (n ^ 1)) {	// fair ecu ?
				c_pDoz->tcs[OWN_SPACEA].real.cEcu.wMsb |= (1 << n);
				WORD deves = pTcx->GetCarDevices(n);
				if (deves & DEVBEEN_V3F)
					c_pDoz->tcs[OWN_SPACEA].real.cV3f.wMsb |= (1 << n);
				SHOT(n, 491);
			}
			else	CUT(n, 491);
		}
	}
	else	CUT(SCENE_ID_ALLCAR, 491);
}

BOOL CArtop::CheckHoldBrake()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL hbr, hbcos;
	hbr = hbcos = FALSE;
	if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY) {
		if (IsRescuePassiveDrive()) {
			// Modified 2013/02/20
			//if (GETTBIT(OWN_SPACEA, TCDIB_ROBR) && (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF)) {
			// Modified 2013/03/12
			//if (!GETTBIT(OWN_SPACEA, TCDIB_ROPR) && (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF))
			if (!GETTBIT(CAR_HEAD, TCDIB_ROPR) && (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF))
				hbr = TRUE;
		}
		// Modified 2013/03/12
		else if (IsRescuePassiveUndergo()) {
			if (!GETTBIT(CAR_TAIL, TCDIB_ROPR) && (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF))
				hbr = TRUE;
		}
		else {
			if (c_pTcRef->real.mcDrv.bValid &&
					c_pTcRef->real.mcDrv.nIndex <= MCDSV_NPOS && (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF))
				hbr = TRUE;
		}
	}
	else {
		PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
		if (paAtc != NULL && paAtc->d.st.req.b.ahbr) {
			hbr = TRUE;
			SHOT(SCENE_ID_HEADCAR, 622);
		}
		else	CUT(SCENE_ID_HEADCAR, 622);
	}

	if (GETTBIT(OWN_SPACEA, TCDIB_HBCOS) &&
			((c_pTcRef->real.ms.wBrake & MSTATE_PARKINGBRAKE) || IsRescue()))	hbcos = TRUE;

	if (hbr) {
		if (!(c_pTcRef->real.ms.hb.wState & HOLDBRAKE_ACTIVE)) {
			c_pTcRef->real.ms.hb.wState |= HOLDBRAKE_ACTIVE;
			pTcx->InterTrigger(&c_pTcRef->real.ms.hb.wTimer,
					c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ? TIME_HOLDBRAKECAPTUREEMERGENCY : 0);
			hbr = (c_pTcRef->real.ms.hb.wTimer == 0) ? TRUE : FALSE;
		}
		else if (c_pTcRef->real.ms.hb.wTimer != 0)	hbr = FALSE;
	}
	else {
		if (c_pTcRef->real.ms.hb.wState & HOLDBRAKE_ACTIVE) {
			c_pTcRef->real.ms.hb.wState &= ~HOLDBRAKE_ACTIVE;
			pTcx->InterTrigger(&c_pTcRef->real.ms.hb.wTimer,
					c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ? TIME_HOLDBRAKERELEASEEMERGENCY : TIME_HOLDBRAKERELEASEATC);
			hbr = (c_pTcRef->real.ms.hb.wTimer == 0) ? FALSE : TRUE;
		}
		else if (c_pTcRef->real.ms.hb.wTimer != 0)	hbr = TRUE;
	}

	if (hbr)	hbr = !hbcos;
	ForceHoldBrake(hbr);
	return hbr;
}

BOOL CArtop::CheckNonReleaseBrake()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	if (c_pTcRef->real.op.nMode == OPMODE_AUTO || c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS) {
		if (c_pTcRef->real.atoCmd.vTbeB < 0 || (paAtc != NULL && paAtc->d.st.req.b.ahbr))
			pTcx->ObsCtrl(FALSE, CCDOB_NRB, ALLCAR_FLAGS);
		else	pTcx->ObsCtrl(TRUE, CCDOB_NRB, ALLCAR_FLAGS);
	}
	// Modified 2011/12/29 ... begin
	//else {
	//	if ((c_pTcRef->real.ms.wBrake & MSTATE_NRBDISABLE) || GETTBIT(OWN_SPACEA, TCDIB_B))
	//		pTcx->ObsCtrl(FALSE, CCDOB_NRB, ALLCAR_FLAGS);
	//	else	pTcx->ObsCtrl(TRUE, CCDOB_NRB, ALLCAR_FLAGS);
	//}
	else if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ||
			c_pTcRef->real.op.nMode == OPMODE_MANUAL ||
			c_pTcRef->real.op.nMode == OPMODE_YARD) {
		WORD mask = MSTATE_NRBDISABLE;
		if (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY)
			mask &= ~MSTATE_BCCOOVERNINE;
		if ((c_pTcRef->real.ms.wBrake & mask) ||
				((IsNotRescue() || IsRescueActiveUndergo() || IsRescueActiveDrive()) && GETTBIT(OWN_SPACEA, TCDIB_B)) ||
				(IsRescuePassiveDrive() && GETTBIT(CAR_HEAD, TCDIB_ROBR)) ||
				(IsRescuePassiveUndergo() && GETTBIT(CAR_TAIL, TCDIB_ROBR)))
			pTcx->ObsCtrl(FALSE, CCDOB_NRB, ALLCAR_FLAGS);
		else	pTcx->ObsCtrl(TRUE, CCDOB_NRB, ALLCAR_FLAGS);
	}
	// Modified 2013/03/12
	//else	return FALSE;
	// ... end

	// Modified 2013/03/12
	//if (c_pTcRef->real.dev.nEcuNrDetect != 0) {
	if (c_pTcRef->real.dev.wNrbi != 0 || c_pTcRef->real.dev.wNrbc != 0) {
		pTcx->PoweringMask();
		if (c_pTcRef->real.dev.wNrbi != 0)	SHOT(SCENE_ID_HEADCAR, 487);
		if (c_pTcRef->real.dev.wNrbc != 0)	SHOT(SCENE_ID_HEADCAR, 486);
		//if (c_pTcRef->real.dev.nEcuNrDetect == NRDETECTFROM_SDI) {
		//	SHOT(SCENE_ID_HEADCAR, 486);
		//	CUT(SCENE_ID_HEADCAR, 487);
		//}
		//else {
		//	SHOT(SCENE_ID_HEADCAR, 487);
		//	CUT(SCENE_ID_HEADCAR, 486);
		//}
		return TRUE;
	}
	CUTS(SCENE_ID_HEADCAR, 486, 487);
	return FALSE;
}

BOOL CArtop::CheckCompulsoryRelease()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (GETTBIT(OWN_SPACEA, TCDIB_CPRLS)) {
		SETTBIT(OWN_SPACEA, TCDOB_CPRLP);
		CTcx* pTcx = (CTcx*)c_pParent;
		WORD wNrb = c_pTcRef->real.dev.wNrbi | c_pTcRef->real.dev.wNrbc;
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (wNrb & (1 << n)) {
				SETOBIT(OWN_SPACEA, n, CCDOB_CPRL);
				c_pTcRef->real.dev.wCprl |= (1 << n);
				SHOT(n, 52);
			}
			else {
				CLROBIT(OWN_SPACEA, n, CCDOB_CPRL);
				c_pTcRef->real.dev.wCprl &= ~(1 << n);
				CUT(n, 52);
			}
		}
		SHOT(SCENE_ID_HEADCAR, 488);
		return TRUE;
	}
	CLRTBIT(OWN_SPACEA, TCDOB_CPRLP);
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++)
		CLROBIT(OWN_SPACEA, n, CCDOB_CPRL);
	c_pTcRef->real.dev.wCprl = 0;
	CUT(SCENE_ID_HEADCAR, 488);
	CUT(SCENE_ID_ALLCAR, 52);
	return FALSE;
}

BOOL CArtop::CheckSecurityBrake()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	// Modified 2012/03/05
	//if (GETTBIT(OWN_SPACEA, TCDIB_SBS)) {
	if (GETTBIT(CAR_HEAD, TCDIB_SBS) || GETTBIT(CAR_TAIL, TCDIB_SBS)) {
		if (!(c_pTcRef->real.ms.wMask & MOVESTATE_MASKSHB))
			pTcx->PoweringMask();
		SETTBIT(OWN_SPACEA, TCDOB_SBLP);
		SHOT(SCENE_ID_HEADCAR, 436);
		return TRUE;
	}
	CUT(SCENE_ID_HEADCAR, 436);
	CLRTBIT(OWN_SPACEA, TCDOB_SBLP);
	return FALSE;
}

BOOL CArtop::CheckParkingRelease()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	BOOL pb = !GETTBIT(OWN_SPACEA, TCDIB_PBRR) ? TRUE : FALSE;
	if (pb)	SETTBIT(OWN_SPACEA, TCDOB_PBLP);
	else	CLRTBIT(OWN_SPACEA, TCDOB_PBLP);
	BOOL bypass = GETTBIT(OWN_SPACEA, TCDIB_PBBPS) ? TRUE : FALSE;
	if (pb) {
		if (!bypass) {
			if (!(c_pTcRef->real.ms.wMask & MOVESTATE_MASKSHB))
				pTcx->PoweringMask();
			SHOT(SCENE_ID_HEADCAR, 438);
		}
		else {
			pb = FALSE;
			SHOT(SCENE_ID_HEADCAR, 437);
		}
	}
	else if (bypass)	SHOT(SCENE_ID_HEADCAR, 485);
	else {
		CUTS(SCENE_ID_HEADCAR, 437, 438);
		CUT(SCENE_ID_HEADCAR, 485);
		//CUT(SCENE_ID_HEADCAR, 492);
	}
	return pb;
}

void CArtop::AtoToTbe(_TV_ vTbe)
{
	c_pTcRef->real.atoCmd.vAto = vTbe;
	c_pTcRef->real.atoCmd.nState |= INSTSTATE_VALID;
	if (c_pTcRef->real.atoCmd.vAto < ATOTBE_MIN || c_pTcRef->real.atoCmd.vAto > ATOTBE_MAX) {
		c_pTcRef->real.atoCmd.nState &= ~INSTSTATE_VALID;
		if (c_pTcRef->real.atoCmd.vAto < ATOTBE_MIN)	c_pTcRef->real.atoCmd.v = ATOTBE_MIN;
		else	c_pTcRef->real.atoCmd.v = ATOTBE_MAX;
	}
	else	c_pTcRef->real.atoCmd.v = c_pTcRef->real.atoCmd.vAto;

	WORD max;
	if (c_pTcRef->real.atoCmd.v < 0) {
		c_pTcRef->real.atoCmd.nState |= INSTSTATE_SIGN;
		max = TEBEMAX_BRAKE;
		c_pTcRef->real.atoCmd.v = -(c_pTcRef->real.atoCmd.v);
	}
	else {
		c_pTcRef->real.atoCmd.nState &= ~INSTSTATE_SIGN;
		max = TEBEMAX_POWER;
	}
	c_pTcRef->real.atoCmd.vTbeB = c_pTcRef->real.atoCmd.v * max / ATOTBE_MAX;	// 10000;
	if (c_pTcRef->real.atoCmd.nState & INSTSTATE_SIGN)
		c_pTcRef->real.atoCmd.vTbeB = -c_pTcRef->real.atoCmd.vTbeB;
}

WORD CArtop::ConvPwb()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PATOAINFO paAto = pTcx->GetAto(FALSE);
	if (paAto != NULL) {
		if (paAto->d.st.vPwbB > 0) {
			if (paAto->d.st.vPwbB < ATOTBE_MAX) {
				WORD v = paAto->d.st.vPwbB * TEBEMAX_POWER / ATOTBE_MAX;
				return v;
			}
			return TEBEMAX_POWER;
		}
		return 0;
	}
	return 0;
}

BYTE CArtop::GetSettingBits(WORD bits)
{
	BYTE all = 0;
	for (UCURV n = 0; n < 16; n ++) {
		if (bits & (1 << n))
			++ all;
	}
	return all;
}
