/*
 * CPack.cpp
 *
 *  Created on: 2011. 2. 24
 *      Author: Che
 */

#include <time.h>

#include "CPack.h"
#include "CTcx.h"

CPack::CPack()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
}

CPack::~CPack()
{
}

void CPack::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
}

void CPack::Package()
{
	PackTime(FALSE);
	//c_pDoz->tcs[OWN_SPACEA].real.wAddr = GETLWORD(c_pTcRef->real.nAddr | BUSADDR_HIGH);
	//c_pDoz->tcs[OWN_SPACEA].real.nFlowNo = 1;
	CTcx* pTcx = (CTcx*)c_pParent;
	DWORD dw = pTcx->GetThumbWheel();

	// Appended 2012/10/09
	c_pDoz->tcs[OWN_SPACEA].real.nRealAddr = c_pTcRef->real.nAddr;
	c_pDoz->tcs[OWN_SPACEA].real.nCallAddr = c_pTcRef->real.dir.nCallID = c_pDoz->tcs[CAR_HEAD].real.nRealAddr;
	c_pDoz->tcs[OWN_SPACEA].real.wVersion = MAKEWORD(VERSION_HIGH(VERSION_TCX), VERSION_LOW(VERSION_TCX));
	c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[0] = (BYTE)((dw >> 16) & 0xff);
	c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[1] = (BYTE)((dw >> 8) & 0xff);
	c_pDoz->tcs[OWN_SPACEA].real.nThumbWheel[2] = (BYTE)(dw & 0xff);

	// Modified 2012/12/25
	if (c_pTcRef->real.cycle.wCtrlDisable > 0)	c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl |= SLAVECCCTRL_INVALIDOUT;
	else	c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl &= ~SLAVECCCTRL_INVALIDOUT;
	// Modified 2013/02/05
	if (FBTBIT(OWN_SPACEA, TCDOB_HCR))	c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl &= ~SLAVECCCTRL_DISABLEDEVICEERRORCOUNT;
	else	c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl |= SLAVECCCTRL_DISABLEDEVICEERRORCOUNT;
	// Modified 2013/03/11
	if (c_pTcRef->real.nState & STATE_ENABLEOVERHAUL)	c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl |= SLAVETCCTRL_ENABLEOVERHAUL;
	else	c_pDoz->tcs[OWN_SPACEA].real.nSlaveCtrl &= ~SLAVETCCTRL_ENABLEOVERHAUL;

	// Modified 2012/11/12 ... benin
	if (pTcx->GetPartner())	c_pDoz->tcs[OWN_SPACEA].real.nHint |= HINT_PARTNER;
	else	c_pDoz->tcs[OWN_SPACEA].real.nHint &= ~HINT_PARTNER;
	// ... end

	PENVARCHEXTENSION pEnv = pTcx->GetEnv();
	if (c_pDoz->tcs[OWN_SPACEA].real.nIssue & ISSUE_ENVIROMENTSET) {
		c_pDoz->tcs[OWN_SPACEA].real.each.wId = TCINFORM_ENV;
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.each.info.env, (PVOID)pEnv, sizeof(ENVARCHEXTENSION));
	}
	for (UCURV n = 0; n < 16; n ++) {
		WORD dia = pEnv->real.w[n / 4][n % 4];
		if (dia >= MIN_WHEELDIA && dia <= MAX_WHEELDIA)
			c_pDoz->tcs[OWN_SPACEA].real.nDia[n] = (BYTE)(dia - MIN_WHEELDIA);
		else	c_pDoz->tcs[OWN_SPACEA].real.nDia[n] = (BYTE)(DEFAULT_WHEELDIA - MIN_WHEELDIA);
	}
	// Appended 12/03/06 ... begin
	if (IsRescueTrain() || IsRescuePassiveDrive() || IsRescuePassiveUndergo())
		c_pDoz->tcs[OWN_SPACEA].real.pwm.nMode = PWMMODE_INPUTA;
	else	c_pDoz->tcs[OWN_SPACEA].real.pwm.nMode = PWMMODE_OUTPUT;
	// ... end

	if (c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) {
		BOOL bSivNor, bV3fNor, bEcuNor;
		bSivNor = bV3fNor = bEcuNor = TRUE;
		for (BYTE n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			WORD deves = pTcx->GetCarDevices(n);
			//BOOL bus = pTcx->GetRealBusCondition(n);

			if ((deves & DEVBEEN_SIV) && //bus &&
					//pTcx->CheckSivAddr(&c_pDoz->ccs[n].real.aSiv) &&
					(c_pTcRef->real.wSivTrace & (1 << n))) {
				PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_SIV, n);
				if (!(pTa->nState & TRACESTATE_LOAD)) {
					if (pTcx->GetTraceLength(TRACEID_SIV, n) > 0)
						pTcx->TraceBegin(TRACEID_SIV, n);
				}
				c_pDoz->tcs[OWN_SPACEA].real.cSiv.wTracing |= (1 << n);
				bSivNor = FALSE;
			}
			else	c_pDoz->tcs[OWN_SPACEA].real.cSiv.wTracing &= ~(1 << n);

			if ((deves & DEVBEEN_V3F) && //bus &&
					//pTcx->CheckV3fAddr(&c_pDoz->ccs[n].real.eV3f) &&
					(c_pTcRef->real.wV3fTrace & (1 << n))) {
				PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_V3F, n);
				if (!(pTa->nState & TRACESTATE_LOAD)) {
					if (pTcx->GetTraceLength(TRACEID_V3F, n) > 0)
						pTcx->TraceBegin(TRACEID_V3F, n);
				}
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.wTracing |= (1 << n);
				bV3fNor = FALSE;
			}
			else	c_pDoz->tcs[OWN_SPACEA].real.cV3f.wTracing &= ~(1 << n);

			if ((deves & DEVBEEN_ECU) && //bus &&
					//pTcx->CheckEcuAddr(&c_pDoz->ccs[n].real.aEcu)) {
					(c_pTcRef->real.wEcuTrace & (1 << n))) {
				PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_ECU, n);
				if (!(pTa->nState & TRACESTATE_LOAD)) {
					if (pTcx->GetTraceLength(TRACEID_ECU, n) > 0)
						pTcx->TraceBegin(TRACEID_ECU, n);
				}
				c_pDoz->tcs[OWN_SPACEA].real.cEcu.wTracing |= (1 << n);
				bEcuNor = FALSE;
			}
			else	c_pDoz->tcs[OWN_SPACEA].real.cEcu.wTracing &= ~(1 << n);
		}
		if (bSivNor)	c_pDoz->tcs[OWN_SPACEA].real.cSiv.wTraceClear = c_pTcRef->real.wSivTraceClear;
		if (bV3fNor) {
			// Modified 2013/03/12
			c_pDoz->tcs[OWN_SPACEA].real.cV3f.wRstCmd = c_pTcRef->real.dev.wV3fRstCmd;

			c_pDoz->tcs[OWN_SPACEA].real.cV3f.wTraceClear = c_pTcRef->real.wV3fTraceClear;
			if (c_pTcRef->real.wDevTimerSet > 0)
				c_pDoz->tcs[OWN_SPACEA].real.cV3f.wTimeValid =
					_SHIFTV(__V3FA) | _SHIFTV(__V3FB) | _SHIFTV(__V3FC) | _SHIFTV(__V3FD);
			else	c_pDoz->tcs[OWN_SPACEA].real.cV3f.wTimeValid = 0;
		}
		if (bEcuNor) {
			c_pDoz->tcs[OWN_SPACEA].real.cEcu.wTraceClear = c_pTcRef->real.wEcuTraceClear;
			if (c_pTcRef->real.wDevTimerSet > 0)
				c_pDoz->tcs[OWN_SPACEA].real.cEcu.wCals = 0xf;
			else	c_pDoz->tcs[OWN_SPACEA].real.cEcu.wCals = 0;
		}
		if (c_pTcRef->real.wDevTimerSet > 0)	-- c_pTcRef->real.wDevTimerSet;
	}
	// Modified 2013/01/10
	c_pDoz->tcs[OWN_SPACEA].real.cDcu.nMode = (c_pTcRef->real.door.nMode == DOORMODE_AOAC) ? DOORMODE_AOAC : 0;

	c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd = 0;
	if (GETTBIT(OWN_SPACEA, TCDIB_DM)) {
		c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd |= DRVLCMD_DMI;
		if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS)
			c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd |= DRVLCMD_ING;
		c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd &= ~(DRVLCMD_KUP | DRVLCMD_KDN);
		PATCAINFO paAtc = pTcx->GetActiveAtc(TRUE);
		if (paAtc != NULL) {
			if (paAtc->d.st.req.b.kup)	c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd |= DRVLCMD_KUP;
			if (paAtc->d.st.req.b.kdn)	c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd |= DRVLCMD_KDN;
			if (c_pTcRef->real.op.nKdnState & KDNSTATE_SETTLE)
				c_pDoz->tcs[OWN_SPACEA].real.nDrvlCmd |= DRVLCMD_KDNSETTLE;
		}
	}

	// Append 2012/12/03
	c_pDoz->tcs[OWN_SPACEA].real.acmd.wLocali = c_pTcRef->real.lm.wValidate;	// only service device
	memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.mcDrv, (PVOID)&c_pTcRef->real.mcDrv, sizeof(MCDSTICK));

	PackLocal();
	if (c_pTcRef->real.nObligation & OBLIGATION_MAIN) {	// local text
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.rAtc,
				(PVOID)&c_pLcFirm->rAtc[0], sizeof(ATCRINFO));
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.rAto,
				(PVOID)&c_pLcFirm->rAto, sizeof(ATORINFO));
	}
	c_pTcRef->real.cycle.wState &= ~TCSTATE_LOCALPACK;
	PBUCKET pBucket = pTcx->GetBucket();
	if (pBucket != NULL) {
		if (!(pBucket->wTicker & BEILTICKER_ATCR)) {
			if (c_pTcRef->real.cond.nActiveAtc != (LOCALID_ATCB + 1))
				memcpy((PVOID)&pBucket->atc.f.r, (PVOID)&c_pLcFirm->rAtc[0], SIZE_ATCSR);
			else	memcpy((PVOID)&pBucket->atc.f.r, (PVOID)&c_pLcFirm->rAtc[1], SIZE_ATCSR);
			pBucket->wTicker |= BEILTICKER_ATCR;
		}
		if (!(pBucket->wTicker & BEILTICKER_ATOR)) {
			memcpy((PVOID)&pBucket->ato.f.r, (PVOID)&c_pLcFirm->rAto, SIZE_ATOSR);
			pBucket->wTicker |= BEILTICKER_ATOR;
		}
	}
	// Modified 2013/01/28
	memset((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.arm, 0, sizeof(ARM_ARCH));
	PARM_ARCH pArm = pTcx->GetFilm();
	if (pArm != NULL)	memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.arm, (PVOID)pArm, sizeof(ARM_ARCH));
	// Modified 2013/02/19
	if (c_pTcRef->real.nState & STATE_BEENUSB)	c_pDoz->tcs[OWN_SPACEA].real.nIssue |= ISSUE_BEENUSB;
	else	c_pDoz->tcs[OWN_SPACEA].real.nIssue &= ~ISSUE_BEENUSB;
}

void CPack::PackLocal()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PackTime(TRUE);

	// for COMMON
	c_pDoz->tcs[OWN_SPACEA].real.rCmm.wAddr = GETLWORD(LOCALADD_PIS);
	PATOAINFO paAto = pTcx->GetAto(FALSE);
	if (OWN_SPACEA < 2) {
		// Modified 2013/02/18
		if (c_pTcRef->real.wTerminalBackupTimer != 0)
			memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.rCmm.nPrst, (PVOID)&c_pTcRef->real.nTerminalCodeBackup, SIZE_VALIDTERMINALCODE);
		else {
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.nsc.a = paAto != NULL ? (paAto->d.st.nNstn | 0x70) : 0;	// text 4
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.nNext = paAto != NULL ? paAto->d.st.nNsn : 0;		// text 5
			// Modified 2013/01/11
			if (paAto != NULL && paAto->d.st.nDsn != 0)
				c_pTcRef->real.atoCmd.nDsn = paAto->d.st.nDsn;
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.nDest = c_pTcRef->real.atoCmd.nDsn;
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.wTno = (WORD)pTcx->ToBcd(c_pTcRef->real.cond.wTrainNo);	//(WORD)pTcx->ToBcd((WORD)paAto->d.st.wTtn);
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.nPrst = paAto != NULL ? paAto->d.st.nPsn : 0;
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.info.b.dcw = paAto != NULL ? paAto->d.st.info.b.dcw : 0;

			if (paAto != NULL && paAto->d.st.info.b.ddns) {
				c_pDoz->tcs[OWN_SPACEA].real.rCmm.info.b.nsddr = TRUE;
				c_pDoz->tcs[OWN_SPACEA].real.rCmm.info.b.nsddl = FALSE;
			}
			else {
				c_pDoz->tcs[OWN_SPACEA].real.rCmm.info.b.nsddr = FALSE;
				c_pDoz->tcs[OWN_SPACEA].real.rCmm.info.b.nsddl = TRUE;
			}
		}
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.info.b.rts = c_pLcFirm->aHtc.info.b.rst;

		c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.a = 0;
		switch (c_pTcRef->real.door.nMode) {
		case DOORMODE_AOAC :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.domaa = TRUE;
			break;
		case DOORMODE_AOMC :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.domam = TRUE;
			break;
		case DOORMODE_MOMC :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.dommm = TRUE;
			break;
		default :
			break;
		}
		switch (c_pTcRef->real.op.nMode) {
		case OPMODE_EMERGENCY :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.dme = TRUE;
			break;
		case OPMODE_MANUAL :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.dmm = TRUE;
			break;
		//case OPMODECAB_YARD :
		//	c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.dme = TRUE;
		//	break;
		case OPMODE_AUTO :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.dma = TRUE;
			break;
		case OPMODE_DRIVERLESS :
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.inst.b.dmr = TRUE;
			break;
		default :
			break;
		}
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.a = 0;
		//if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
		//	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)
		//		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.hcr = TRUE;
		//	else	c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.hcr = FALSE;
		//}
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.rCmm.temp, c_pLcFirm->aHtc.temp, 16);
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++)
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.nPW[n] = (BYTE)pTcx->GetPassengerWeightRate(n);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nEm = (BYTE)c_pTcRef->real.ca.emer.wRep;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nFr = (BYTE)c_pTcRef->real.ca.fire.wBuf;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.wStsd = (WORD)c_pTcRef->real.intg.dbDist;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.zvr =
			(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) ? TRUE : FALSE;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.dol =
			(c_pTcRef->real.door.nRefer & DOORREF_LEFT) ? TRUE : FALSE;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.dor =
			(c_pTcRef->real.door.nRefer & DOORREF_RIGHT) ? TRUE : FALSE;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.siv1 = (GETCBIT(__SIVA, CCDIB_SIVK)) ? TRUE : FALSE;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.siv0 = (GETCBIT(__SIVB, CCDIB_SIVK)) ? TRUE : FALSE;
		//c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.frd = c_pTcRef->real.ca.fire.wRep != 0 ? TRUE : FALSE;
		//c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.emc = c_pTcRef->real.ca.emer.wRep != 0 ? TRUE : FALSE;

		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.esk =
			(c_pTcRef->real.ext.nState & EXTSTATE_REDUCTIONA) ? TRUE : FALSE;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.lrr =
			(c_pTcRef->real.ext.nState & EXTSTATE_REDUCTIONB) ? TRUE : FALSE;
		if (c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.siv1 ||
				c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.siv0)
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.siva = TRUE;
		else	c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.siva = FALSE;
		if (GETTBIT(OWN_SPACEA, TCDIB_LCS1))	c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.lpo = TRUE;
		else	c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.lpo = FALSE;

		BYTE stick = pTcx->GetTrainDir(TRUE);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.fwd = stick == 1 ? TRUE : FALSE;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.req.b.rvs = stick == 2 ? TRUE : FALSE;

		PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nAtcSpeed = paAtc != NULL ? paAtc->d.st.nSpd : 0;
		UCURV si = c_pTcRef->real.nDuty < 2 ? __SIVA : __SIVB;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nSivV = c_pDoz->ccs[si].real.eSiv.d.st.st.nEd;
		c_pDoz->tcs[OWN_SPACEA].real.rCmm.nSivI = c_pDoz->ccs[si].real.eSiv.d.st.st.nIs;
	}
	else {
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.rCmm.nPrst, (PVOID)&c_pDoz->tcs[CAR_HEAD].real.rCmm.nPrst, sizeof(CMMRINFO) - 9);
	}

	if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
		if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)
			c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.hcr = TRUE;
		else	c_pDoz->tcs[OWN_SPACEA].real.rCmm.cmd.b.hcr = FALSE;
	}

	// for ATC
	c_pLcFirm->rAtc[0].wAddr =
	c_pLcFirm->rAtc[1].wAddr = GETLWORD(LOCALADD_ATC);
	c_pLcFirm->rAtc[0].nCtrl =
	c_pLcFirm->rAtc[1].nCtrl = LOCALCTRL;
	if (c_pTcRef->real.nLocalDataType[LOCALID_ATCA] == DATATYPE_TRACE) {
		PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_ATC, 0);
		if (!(pTa->nState & TRACESTATE_LOAD)) {
			if (pTcx->GetTraceLength(TRACEID_ATC, 0) > 0)
				pTcx->TraceBegin(TRACEID_ATC, 0);
			else	MSGLOG("[PACK]ATCA Can not trace cause trace length.\r\n");
		}
		if (pTa->nState & TRACESTATE_LOAD) {
			c_pLcFirm->rAtc[0].nFlow = LOCALFLOW_TRACE;
			//c_pLcFirm->rAtc[0].r.tc.nIndex = (BYTE)pTa->wiTc;
			PackAtc(&c_pLcFirm->rAtc[0]);
		}
		else {
			pTa->nState = 0;
			c_pTcRef->real.nLocalDataType[LOCALID_ATCA] = DATATYPE_NORMAL;
			c_pLcFirm->rAtc[0].nFlow = LOCALFLOW_NORMAL;
			PackAtc(&c_pLcFirm->rAtc[0]);
		}
	}
	else {
		c_pLcFirm->rAtc[0].nFlow = LOCALFLOW_NORMAL;
		PackAtc(&c_pLcFirm->rAtc[0]);
	}
	if (c_pTcRef->real.nLocalDataType[LOCALID_ATCB] == DATATYPE_TRACE) {
		PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_ATC, 0);
		if (!(pTa->nState & TRACESTATE_LOAD)) {
			if (pTcx->GetTraceLength(TRACEID_ATC, 0) > 0)
				pTcx->TraceBegin(TRACEID_ATC, 0);
			else	MSGLOG("[PACK]ATCB Can not trace cause trace length.\r\n");
		}
		if (pTa->nState & TRACESTATE_LOAD) {
			c_pLcFirm->rAtc[1].nFlow = LOCALFLOW_TRACE;
			//c_pLcFirm->rAtc[1].r.tc.nIndex = (BYTE)pTa->wiTc;
			PackAtc(&c_pLcFirm->rAtc[1]);
		}
		else {
			pTa->nState = 0;
			c_pTcRef->real.nLocalDataType[LOCALID_ATCB] = DATATYPE_NORMAL;
			c_pLcFirm->rAtc[1].nFlow = LOCALFLOW_NORMAL;
			PackAtc(&c_pLcFirm->rAtc[1]);
		}
	}
	else {
		c_pLcFirm->rAtc[1].nFlow = LOCALFLOW_NORMAL;
		PackAtc(&c_pLcFirm->rAtc[1]);
	}

	// for ATO
	c_pLcFirm->rAto.wAddr = GETLWORD(LOCALADD_ATO);
	c_pLcFirm->rAto.nCtrl = LOCALCTRL;
	switch (c_pTcRef->real.nLocalDataType[LOCALID_ATO]) {
	case DATATYPE_INSPECT :
		c_pLcFirm->rAto.nFlow = LOCALFLOW_INSPECT;
		break;
	case DATATYPE_TRACE :
		{
			PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_ATO, 0);
			if (!(pTa->nState & TRACESTATE_LOAD)) {
				if (pTcx->GetTraceLength(TRACEID_ATO, 0) > 0)
					pTcx->TraceBegin(TRACEID_ATO, 0);
				else	MSGLOG("[PACK]ATO Can not trace cause trace length.\r\n");
			}
			if (pTa->nState & TRACESTATE_LOAD) {
				c_pLcFirm->rAto.nFlow = LOCALFLOW_TRACE;
				//c_pLcFirm->rAto.r.tc.nIndex = (BYTE)pTa->wiTc;
				PackAto(&c_pLcFirm->rAto);
			}
			else {
				pTa->nState = 0;
				c_pTcRef->real.nLocalDataType[LOCALID_ATO] = DATATYPE_NORMAL;
				c_pLcFirm->rAto.nFlow = LOCALFLOW_NORMAL;
				PackAto(&c_pLcFirm->rAto);
			}
		}
		break;
	case DATATYPE_NORMAL :
		paAto = pTcx->GetAto(TRUE);
		if (paAto != NULL && paAto->nFlow == LOCALFLOW_NORMAL &&
				paAto->d.st.wTtn != 0 && paAto->d.st.wTtn != c_pTcRef->real.cond.wTrainNo) {
			PENVARCHEXTENSION pEnv = pTcx->GetEnv();
			pEnv->real.entire.elem.wTrainNo = c_pTcRef->real.cond.wTrainNo = paAto->d.st.wTtn;
			c_pTcRef->real.atoCmd.nState |= INSTSTATE_DISABLEEDITTN;	// ...no need...
			MSGLOG("[TC]Cause train no. change B.\r\n");
			pTcx->SaveEnv(3);	// by atc(different train no),cause train no.
		}
		c_pLcFirm->rAto.nFlow = LOCALFLOW_NORMAL;
		PackAto(&c_pLcFirm->rAto);
		break;

	default :
		break;
	}
	if (c_pTcRef->real.speed.nLocalCycle < 2)
		++ c_pTcRef->real.speed.nLocalCycle;
}

void CPack::PackAtc(PATCRINFO prAtc)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	if (prAtc->nFlow == LOCALFLOW_TRACE) {
		PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_ATC, 0);
		prAtc->r.tc.nIndex = (BYTE)(pTa->wiTc);
	}
	else {
		//c_pLcFirm->rAtc[0].state.b.mir =
		//c_pLcFirm->rAtc[1].state.b.mir =
		//	(c_pTcRef->real.op.nMode == OPMODEATC_MANUAL ||
		//		c_pTcRef->real.op.nMode == OPMODECAB_MANUAL ||
		//			c_pTcRef->real.op.nMode == OPMODECAB_EMERGENCY) ? TRUE : FALSE;
		PATCAINFO paAtc = pTcx->GetActiveAtc(TRUE);
		prAtc->r.sr.state.b.afsba = paAtc != NULL ? paAtc->d.st.req.b.fsbr : 0;
		prAtc->r.sr.state.b.mcp = c_pTcRef->real.mcDrv.nIndex == MCDSV_NPOS ? TRUE : FALSE;
		prAtc->r.sr.state.b.mpp = c_pTcRef->real.mcDrv.nIndex > MCDSV_NPOS ? TRUE : FALSE;
		prAtc->r.sr.state.b.mfsbp = c_pTcRef->real.mcDrv.nIndex == MCDSV_B7POS ? TRUE : FALSE;
		//prAtc->r.sr.state.b.ebras = (c_pTcRef->real.ms.wBrake & MSTATE_EMERGENCYBRAKE) ? TRUE : FALSE;
		// Modified 2011/12/16 ... begin
		//prAtc->r.sr.state.b.ebras = GETTBIT(OWN_SPACEA, TCDIB_EPCOR) ? TRUE : FALSE;
		prAtc->r.sr.state.b.ebras = pTcx->GetEpcor();
		// ... end
		prAtc->r.sr.state.b.adc = GETTBIT(OWN_SPACEA, TCDIB_ADSCR) ? TRUE : FALSE;
		prAtc->r.sr.vSpr = 0;
	}
}

void CPack::PackAto(PATORINFO prAto)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	switch (prAto->nFlow) {
	case LOCALFLOW_INSPECT :
		break;
	case LOCALFLOW_TRACE :
		{
			PTRACEARCH pTa = pTcx->GetTraceArch(TRACEID_ATO, 0);
			prAto->r.tc.nIndex = (BYTE)(pTa->wiTc);
		}
		break;
	default :
		{
			prAto->r.sr.wTnc = c_pTcRef->real.cond.wTrainNo;
			prAto->nFlow = LOCALFLOW_NORMAL;
			DWORD dw = pTcx->GetThumbWheel();
			prAto->r.sr.nFno = (BYTE)pTcx->ToHex((BYTE)(dw & 0xff));
			prAto->r.sr.nTrs0 = prAto->r.sr.nTrs1 = prAto->r.sr.nTrs2 = 0;
			prAto->r.sr.fail.a = 0;
			prAto->r.sr.set.b.atoinh = ((c_pTcRef->real.atoCmd.nCmd & INSTCMD_INHIBIT) ||
										(c_pTcRef->real.atoCmd.nCmd & INSTCMD_INHIBITA)) ? TRUE : FALSE;
			prAto->r.sr.set.b.caba = (c_pTcRef->real.atoCmd.nCmd & INSTCMD_CABIN) ? TRUE : FALSE;
			//if (c_pLcFirm->rAto.ct.nor.set.b.atoinh)	c_pLcFirm->rAto.ct.nor.set.b.actato = FALSE;
			//else
			prAto->r.sr.set.b.actato = (c_pTcRef->real.atoCmd.nCmd & INSTCMD_ACTIVE) ? TRUE : FALSE;
			prAto->r.sr.set.a |= ((c_pTcRef->real.cf.nLength << 4) & 0xf0);
			prAto->r.sr.mode.a &= ~ATOMODE_ONLY;
			switch (c_pTcRef->real.op.nMode) {
			case OPMODE_DRIVERLESS :	prAto->r.sr.mode.b.aamd = TRUE;	break;
			case OPMODE_AUTO :			prAto->r.sr.mode.b.aama = TRUE;	break;
			case OPMODE_YARD :			prAto->r.sr.mode.b.aamy = TRUE;	break;
			case OPMODE_MANUAL :		prAto->r.sr.mode.b.aamm = TRUE;	break;
			case OPMODE_EMERGENCY :		prAto->r.sr.mode.b.aame = TRUE;	break;
			default :					break;
			}
			prAto->r.sr.mode.b.adc = (GETTBIT(OWN_SPACEA, TCDIB_ADSCR)) ? TRUE : FALSE;
			// This line excuted at auto mode
			//prAto->r.sr.vTbeB =
			//	(c_pTcRef->real.atoCmd.nState & INSTSTATE_SIGN) ?
			//			- c_pTcRef->real.atoCmd.v : c_pTcRef->real.atoCmd.v;
			prAto->r.sr.vTbeB = INVERTWORD(pTcx->GetV3fTbe());
			// Modified 2011/12/28 ...begin
			//prAto->r.sr.vAsp = (_TV_)(c_pTcRef->real.speed.dwAtc / (DWORD)10);
			prAto->r.sr.vAsp = (_TV_)(c_pTcRef->real.speed.dwV3fs / (DWORD)10);
			// ... end
			//prAto->r.sr.vBpa = prAto->r.sr.vPpa = 10000;
			BYTE total = pTcx->GetV3fQuantity();
			BYTE fail = pTcx->GetBitQuantity(c_pTcRef->real.dev.wFailV3f, TRUE);
			if (fail > total)	prAto->r.sr.vPpa = 0;
			else	prAto->r.sr.vPpa = (WORD)(total - fail) * 10000 / (WORD)total;
			total = pTcx->GetEcuQuantity();
			fail = pTcx->GetBitQuantity(c_pTcRef->real.dev.wFailEcu, TRUE);
			if (fail > total)	prAto->r.sr.vBpa = 0;
			else	prAto->r.sr.vBpa = (WORD)(total - fail) * 10000 / (WORD)total;
			prAto->r.sr.wSpr = 0;
		}
		break;
	}
}

void CPack::PackTime(BOOL cmd)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PDEVTIME pDevt = pTcx->GetDevt();
	FormatDevTime(pDevt);
	if (!cmd) {
		PTBTIME pTbt = pTcx->GetTbt();
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.time,
				(PVOID)pTbt, sizeof(TBTIME));
	}
	else {
		memcpy((PVOID)&c_pDoz->tcs[OWN_SPACEA].real.rCmm.time,
				(PVOID)pDevt, sizeof(DEVTIME));
		if (c_pTcRef->real.nLocalDataType[LOCALID_ATCA] == DATATYPE_NORMAL) {
			memcpy((PVOID)&c_pLcFirm->rAtc[0].r.sr.time, (PVOID)pDevt, sizeof(DEVTIME));
			memcpy((PVOID)&c_pLcFirm->rAtc[1].r.sr.time, (PVOID)pDevt, sizeof(DEVTIME));
		}
		if (c_pTcRef->real.nLocalDataType[LOCALID_ATO] == DATATYPE_NORMAL)
			memcpy((PVOID)&c_pLcFirm->rAto.r.sr.time, (PVOID)pDevt, sizeof(DEVTIME));
	}
}

void CPack::FormatTime(PTBTIME pTb)
{
	if (c_pTcRef->real.nDuty == 0 || !(c_pTcRef->real.cycle.wState & TCSTATE_FRAMEOPEN)) {
		// if the bus slave, and opened frame from bus,
		// do not rely on the system time,
		// must be synchronized to the bus time
		CTcx* pTcx = (CTcx*)c_pParent;
		time_t cur = time(NULL);
		struct tm* lc = localtime(&cur);
		pTb->year = pTcx->ToBcd((WORD)(lc->tm_year + 1900));
		pTb->month = pTcx->ToBcd((BYTE)(lc->tm_mon + 1));
		pTb->day = pTcx->ToBcd((BYTE)lc->tm_mday);
		pTb->hour = pTcx->ToBcd((BYTE)lc->tm_hour);
		pTb->minute = pTcx->ToBcd((BYTE)lc->tm_min);
		pTb->sec = pTcx->ToBcd((BYTE)lc->tm_sec);
		pTb->msec = 0;
	}
}

void CPack::FormatDevTime(PDEVTIME pDev)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PTBTIME pTbt = pTcx->GetTbt();
	if (c_pTcRef->real.nDuty == 0)	FormatTime(pTbt);
	pDev->year = (BYTE)(pTbt->year & 0xff);
	pDev->month = pTbt->month;
	pDev->day = pTbt->day;
	pDev->hour = pTbt->hour;
	pDev->minute = pTbt->minute;
	pDev->sec = pTbt->sec;
}
