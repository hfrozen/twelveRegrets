/*
 * CDuDoc.cpp
 *
 *  Created on: 2011. 1. 30
 *      Author: Che
 */

#include "CDuDoc.h"
#include "CTcx.h"

#define	HEXTODEC(w)		((w >> 8) * 100 + (w & 0xff))

CDuDoc::CDuDoc()
{
	c_pParent = NULL;
	c_pLcFirm = NULL;
	c_pTcRef = NULL;
	c_pDoz = NULL;
	c_pDucInfo = NULL;
	c_wReset = 2;
}

CDuDoc::~CDuDoc()
{
}

const WORD CDuDoc::c_wPattern[3][10] = {
	{ 1, 4, 3, 8, 8, 4, 3, 2, 0, 0 },
	{ 0, 1, 4, 8, 4, 3, 2, 0, 0, 0 },
	{ 0, 0, 1, 4, 3, 2, 0, 0, 0, 0 }
};

const BYTE CDuDoc::c_nHeadPile[] = {
	0x1b, 0x57, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x7f, 0x0d, 0x0a,
	0x1b, 0x57, 0x07, 0x30, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0xac, 0x0d, 0x0a
};
#define	SIZE_HEADPILE	24

const BYTE CDuDoc::c_nTailPile[] = {
	0x1b, 0x57, 0x07, 0x30, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0xab, 0x0d, 0x0a
};
#define	SIZE_TAILPILE	13

void CDuDoc::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
	c_pDucInfo = pTcx->GetDucInfo();
	memset((PVOID)&c_wDevTraceLength[0][0], 0, DEVID_MAX * MAX_DEV_LENGTH);
	memset((PVOID)&c_wDevTraceProgress[0][0], 0, DEVID_MAX * MAX_DEV_LENGTH);
	memset((PVOID)&c_nOsMsg, 0, SIZE_OSMSG);
}

BYTE CDuDoc::WrenchByte(BYTE n)
{
	BYTE res = 0;
	if (n & 1)	res |= 0x80;
	if (n & 2)	res |= 0x40;
	if (n & 4)	res |= 0x20;
	if (n & 8)	res |= 0x10;
	if (n & 0x10)	res |= 8;
	if (n & 0x20)	res |= 4;
	if (n & 0x40)	res |= 2;
	if (n & 0x80)	res |= 1;
	return res;
}

DWORD CDuDoc::XchAtZero(DWORD dw)
{
	union DWB {
		DWORD dw;
		BYTE n[4];
	};

	DWB dwb, res;
	dwb.dw = dw;
	res.n[0] = WrenchByte(dwb.n[0]);
	res.n[1] = WrenchByte(dwb.n[1]);
	res.n[2] = WrenchByte(dwb.n[2]);
	res.n[3] = WrenchByte(dwb.n[3]);
	return res.dw;
}

BOOL CDuDoc::IsValidTc(UCURV ti)
{
	WORD add = GETLWORD(c_pDoz->tcs[ti].real.wAddr);
	if ((add & 0x7f00) == BUSADDR_HIGH && //(add & 0xff) == (ti + 1) &&
			c_pDoz->tcs[CAR_HEAD].real.dwOrderID == c_pDoz->tcs[ti].real.dwOrderID)
		return TRUE;
	return FALSE;
}

BOOL CDuDoc::IsValidCc(UCURV ci)
{
	WORD add = GETLWORD(c_pDoz->ccs[ci].real.wAddr);
	if ((add & 0x7f00) == BUSADDR_HIGH && (add & 0xff) == (ci + CAR_CC + 1) &&
			c_pDoz->tcs[CAR_HEAD].real.dwOrderID == c_pDoz->ccs[ci].real.dwOrderID)
		return TRUE;
	return FALSE;
}

void CDuDoc::TuningCode(WORD code, WORD cid, PTMFAULTINFO pFi)
{
	pFi->wCarID = pFi->wLow = pFi->wReal = pFi->wHigh = 0;
	pFi->wCarID = cid;
	if (code > 500) {
		pFi->wReal = code;
		pFi->wHigh = code - 500;
	}
	else	pFi->wLow = code;
}

BYTE CDuDoc::GetTcInfo()
{
	BYTE res = 0;
	for (UCURV n = 0; n < LENGTH_TC; n ++) {
		BYTE addr = c_pDoz->tcs[n].real.nRealAddr;
		if (addr >= 1 && addr <= 4) {
			-- addr;
			res |= (1 << addr);
			if (GETLWORD(c_pDoz->tcs[n].real.wAddr) & 0x8000)
				res |= (0x10 << addr);
		}
	}
	return res;
}

//BYTE CDuDoc::GetTcFrontMap()
//{
//	switch (c_pTcRef->real.cf.nLength) {
//	case 4 :	return 2;	break;
//	case 6 :	return 1;	break;
//	default :	return 0;	break;
//	}
//	return 0;
//}

//				M1F	M2F	M1R	M2R
//	4:	id:		1	X	X	2
//		ret:	X	X	X	B5
//	6:	id:		1	X	3	4
//		ret:	B1	X	B5	B6
//	8:	id:		1	2	5	6
//		ret:	B1	B2	B5	B6
//BYTE CDuDoc::GetV3fMap(BYTE id)
//{
//	if (c_pTcRef->real.cf.nLength == 4) {
//		if (id == 2)	return 5;
//	}
//	else {
//		switch (id) {
//		case 1 :	return 1;	break;
//		case 2 :
//			if (c_pTcRef->real.cf.nLength == 8)	return 2;
//			else	return 0xff;
//			break;
//		case 5 :	return 5;	break;
//		case 6 :	return 6;	break;
//		default :	break;
//		}
//	}
//	return 0xff;
//}

//				M2F	M2R
//	4:	id:		X	2
//		ret:	X	B2
//	6:	id:		2	4
//		ret:	B2	B6
//	8:	id:		2	6
//		ret:	B2	B6
BYTE CDuDoc::GetCmsbMap(BYTE id)
{
	if (id == 2)	return 2;
	else if (c_pTcRef->real.cf.nLength == 6 && id == 4)
		return 6;
	else if (c_pTcRef->real.cf.nLength == 8 && id == 6)
		return 6;
	return 0xff;
}

//				M1F	M1R
//	4:	id:		X	1
//		ret:	X	B1
//	6:	id:		1	3
//		ret:	B1	B5
//	8:	id:		1	5
//		ret:	B1	B5
BYTE CDuDoc::GetAuxMap(BYTE id)
{
	if (id == 1)	return 1;
	else if (c_pTcRef->real.cf.nLength == 6 && id == 3)
		return 5;
	else if (c_pTcRef->real.cf.nLength == 8 && id == 5)
		return 5;
	return 0xff;
}

BYTE CDuDoc::GetDoorBypass(UCURV id)
{
	BYTE bypass = 0;
	for (UCURV n = 0; n < 4; n++) {
		if (c_pDoz->ccs[id].real.aDcu[0][n].state.b.diso)
			bypass |= (1 << n);
		if (c_pDoz->ccs[id].real.aDcu[1][n].state.b.diso)
			bypass |= (0x10 << n);
	}
	return bypass;
}

WORD CDuDoc::GetDoorFault(UCURV id)
{
	WORD fault = 0;
	for (UCURV n = 0; n < 4; n ++) {
		if (c_pDoz->ccs[id].real.aDcu[0][n].test.b.dcus)
			fault |= (1 << n);
		if (c_pDoz->ccs[id].real.aDcu[1][n].test.b.dcus)
			fault |= (0x10 << n);
	}
	return fault;
}

BYTE CDuDoc::CalcSum(WORD size, BOOL type)
{
	if (type)	c_pDucInfo->srDuc.wSendLength = (size - 3) / 2;
	BYTE sum = 0;
	BYTE* p = (BYTE*)&c_pDucInfo->srDuc.nEsc;
	for (UCURV n = 0; n < (size + 3); n ++)
		sum += *p ++;
	return sum;
}

BYTE CDuDoc::CalcSum(BYTE* p, BYTE size)
{
	BYTE sum = 0;
	while (size > 0) {
		sum += *p ++;
		-- size;
	}
	return sum;
}

BYTE CDuDoc::GetRealCid(BYTE cid, BOOL all)
{
	if (cid < 8) {
		if (cid > 0)	-- cid;
		else	cid = c_pTcRef->real.cf.nLength - 1;
		if (cid >= c_pTcRef->real.cf.nLength)	cid = 0;
	}
	else {
		if (!all)	cid = 0;
	}
	return cid;
}

void CDuDoc::Operate()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUOPERATE;
	oDuc.wCrew = 78;
	oDuc.glamp.a = 0;
	if (c_pTcRef->real.nAddr < 3)	oDuc.glamp.b.c1 = 1;
	else	oDuc.glamp.b.c0 = 1;
	oDuc.glamp.b.sysbk =
		(c_pTcRef->real.nAddr == 2 || c_pTcRef->real.nAddr == 4) ? 1 : 0;
	WORD w = c_pTcRef->real.ms.wBrake;
	if (w & MSTATE_HOLDINGBRAKE)	oDuc.glamp.b.hold = 1;
	if (w & (MSTATE_NORMALFSB | MSTATE_FSBATWRECKDIR | MSTATE_FSBATMWB))	oDuc.glamp.b.fs = 1;
	if (w & MSTATE_PARKINGBRAKE)	oDuc.glamp.b.park = 1;
	if (w & MSTATE_SECURITYBRAKE)	oDuc.glamp.b.security = 1;
	if (w & MSTATE_EMERGENCYBRAKE)	oDuc.glamp.b.emergency = 1;
	int id;
	if ((id = c_pTcRef->real.cond.nActiveAtc) == (LOCALID_ATCA + 1))
		oDuc.glamp.b.atcMain = 1;
	else if (id == (LOCALID_ATCB + 1))	oDuc.glamp.b.atcAux = 1;
	// Modified 2013/11/02
	//// Modified 2013/01/22
	////if (pTcx->GetAlarmCode() > 0) {
	//if (pTcx->GetActiveAlarmCode() > 0) {
	//	oDuc.glamp.b.error = TRUE;
	//	// Modified 2012/12/25
	//	//oDuc.glamp.b.flash = (c_pTcRef->real.scArm.nState & ARMSTATE_BUZZON) ? TRUE : FALSE;
	//	oDuc.glamp.b.flash = TRUE;
	//}

	if (IsRescue()) {
		oDuc.wOpMode = c_pTcRef->real.op.nMode == OPMODE_EMERGENCY ? 9 : 8;
		if (IsRescuePassiveDrive())			oDuc.wRescueMode = 2;
		else if (IsRescueActiveUndergo())	oDuc.wRescueMode = 4;
		else if (IsRescuePassiveUndergo())	oDuc.wRescueMode = 8;
		else if (IsRescueActiveDrive())		oDuc.wRescueMode = 0x10;
		else								oDuc.wRescueMode = 1;
		//switch (c_pTcRef->real.op.nRescueMode) {
		//case RESCUEMODE_PASSIVEDRIVE :
		//	oDuc.wRescueMode = 2;
		//	break;
		//case RESCUEMODE_ACTIVEUNDERGO :
		//	oDuc.wRescueMode = 4;
		//	break;
		//case RESCUEMODE_PASSIVEUNDERGO :
		//	oDuc.wRescueMode = 8;
		//	break;
		//case RESCUEMODE_ACTIVEDRIVE :
		//	oDuc.wRescueMode = 0x10;
		//	break;
		//default :
		//	oDuc.wRescueMode = 1;
		//	break;
		//}
	}
	else {
		switch (c_pTcRef->real.op.nMode) {
		case OPMODE_EMERGENCY :			oDuc.wOpMode = 4;	break;
		case OPMODE_MANUAL :			oDuc.wOpMode = 2;	break;
		case OPMODE_YARD :				oDuc.wOpMode = 5;	break;
		case OPMODE_AUTO :				oDuc.wOpMode = 1;	break;
		case OPMODE_AUTOREADY :			oDuc.wOpMode = 6;	break;
		case OPMODE_DRIVERLESS :		oDuc.wOpMode = 3;	break;
		case OPMODE_DRIVERLESSREADY :
		case OPMODE_DRIVERLESSEXIT :	oDuc.wOpMode = 7;	break;
		default :						oDuc.wOpMode = 0;	break;
		}
		oDuc.wRescueMode = 0;
	}

	oDuc.wPower = oDuc.wBrake = 0;
	// Modified 2013/02/26
	//SHORT vPer = pTcx->TbeToPercent(GETLWORD(c_pTcRef->real.power.vTbeL));
	//if (vPer < 0)	oDuc.wBrake = (WORD)(-vPer);
	//else	oDuc.wPower = (WORD)vPer;
	if (IsRescueTrain() || IsRescuePassiveUndergo() || IsRescuePassiveDrive()) {
		// calcurate with roundedoff
		if (c_pTcRef->real.rescPwr.vTbeB < 0)	oDuc.wBrake = c_pTcRef->real.rescPwr.percent;
		else	oDuc.wPower = c_pTcRef->real.rescPwr.percent;
	}
	else {
		// NEED ROUNDEDOFF !!!!!
		SHORT vPer = pTcx->TbeToPercent(GETLWORD(c_pTcRef->real.power.vTbeL));
		if (vPer < 0)	oDuc.wBrake = (WORD)(-vPer);
		else	oDuc.wPower = (WORD)vPer;
	}

	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	PATOAINFO paAto = pTcx->GetAto(FALSE);

	if (paAtc != NULL) {
		BYTE n = paAtc->d.st.sph.a & 0xfc;
		n |= ((paAtc->d.st.mode.a >> 6) & 3);
		oDuc.wAtcSpeedCode = MAKEWORD(n, paAtc->d.st.spl.a);
	}
	else	oDuc.wAtcSpeedCode = 0;
	oDuc.wDist = (WORD)(c_pTcRef->real.intg.dbDistLive / 1000.f);
	oDuc.wTrainNo = c_pTcRef->real.cond.wTrainNo;
	if (c_pTcRef->real.mcDrv.bValid)
		oDuc.wMascon = (2 << c_pTcRef->real.mcDrv.nIndex);
	//DWORD dwSpeed = c_pTcRef->real.speed.dwV3f;
	DWORD dwSpeed;
	// Modified 2013/02/05
	//if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY)
	//	dwSpeed = c_pTcRef->real.speed.dwV3fs;
	//else	dwSpeed = c_pTcRef->real.speed.dwAtcs;
	if (c_pTcRef->real.op.nMode != OPMODE_EMERGENCY && (c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY))	// for running test at lab.
		dwSpeed = c_pTcRef->real.speed.dwAtcs;
	else	dwSpeed = c_pTcRef->real.speed.dwV3fs;

	oDuc.wSpeed = (WORD)(dwSpeed / (DWORD)1000);	// m/h -> km/h
	dwSpeed %= 1000;
	if (dwSpeed >= 500)	++ oDuc.wSpeed;
	// Modified 2013/01/11
	//oDuc.wDestine = paAto != NULL ? (WORD)paAto->d.st.nDsn : 0;
	if (paAto != NULL && paAto->d.st.nDsn != 0)
		c_pTcRef->real.atoCmd.nDsn = paAto->d.st.nDsn;
	oDuc.wDestine = c_pTcRef->real.atoCmd.nDsn;
	oDuc.wNextStt = paAto != NULL ? (WORD)paAto->d.st.nNsn : 0;
	oDuc.wTheTrain = (WORD)(c_pTcRef->real.cf.nSerialNo / 10);
	oDuc.wTrainID = (WORD)(c_pTcRef->real.cf.nOrgNo);
	oDuc.wTrainLength = (WORD)(c_pTcRef->real.cf.nLength);
	//oDuc.wCarSel =
	TBTIME tb;
	pTcx->GetTbTime(&tb);
	oDuc.wYear = (WORD)tb.year;
	oDuc.wDate = ((WORD)tb.month << 8) | (WORD)tb.day;
	oDuc.wTime = ((WORD)tb.hour << 8) | (WORD)tb.minute;
	oDuc.wSecond = (WORD)tb.sec;
	// Modified 2013/11/02
	oDuc.hlamp.a = 0;
	if (c_wReset > 1)	-- c_wReset;
	else if (c_wReset == 1) {
		oDuc.wReset = 2;
		//MSGLOG("[DUC]Reset...\r\n");
	}
	else {
		oDuc.wReset = 0;

		// Modified 2013/11/02
		WORD alarm = pTcx->GetActiveAlarmCode(c_pTcRef->real.scArm.dwScrollIndex);
		TuningCode(alarm >> 4, alarm & 0xf, &oDuc.alarm);
		DWORD length = pTcx->GetAlarmLength();
		if (length > 2) {
			if (c_pTcRef->real.scArm.dwScrollIndex == 0) {
				oDuc.hlamp.b.ast0 = TRUE;
				oDuc.hlamp.b.ast1 = FALSE;
			}
			else if (c_pTcRef->real.scArm.dwScrollIndex == length) {
				oDuc.hlamp.b.ast0 =
				oDuc.hlamp.b.ast1 = TRUE;
			}
			else {
				oDuc.hlamp.b.ast0 = FALSE;
				oDuc.hlamp.b.ast1 = TRUE;
			}
		}
		else {
			oDuc.hlamp.b.ast0 =
			oDuc.hlamp.b.ast1 = FALSE;
		}
		alarm = pTcx->GetBellLength();
		if (alarm > 0)	oDuc.glamp.b.error = oDuc.glamp.b.flash = TRUE;

		WORD sign = pTcx->GetSignCode();
		TuningCode(sign >> 4, sign & 0xf, &oDuc.log);

		alarm = pTcx->GetSevereCode();
		c_pTcRef->real.scArm.nSevereID = oDuc.wSevereID = alarm & 0xf;
		c_pTcRef->real.scArm.wSevereCode = oDuc.wSevereCode = alarm >> 4;
	}

	//oDuc.hlamp.a = 0;
	if (c_pTcRef->real.hcr.nMasterID > 0 && c_pTcRef->real.hcr.nMasterID < 5) {
		if (c_pDoz->tcs[CAR_HEAD].real.nDrvlCmd & DRVLCMD_DMI) {
			if (c_pTcRef->real.hcr.nMasterID < 3) {
				oDuc.hlamp.b.hcab = TRUE;
				if (c_pDoz->tcs[CAR_HEAD].real.nDrvlCmd & DRVLCMD_ING)	oDuc.hlamp.b.hcf = TRUE;
			}
			else {
				oDuc.hlamp.b.tcab = TRUE;
				if (c_pDoz->tcs[CAR_HEAD].real.nDrvlCmd & DRVLCMD_ING)	oDuc.hlamp.b.tcf = TRUE;
			}
		}
		else {
			if (c_pTcRef->real.hcr.nMasterID < 3) {
				oDuc.hlamp.b.hcab = TRUE;
				// Modified 2012/12/25
				//oDuc.hlamp.b.hcf = c_pTcRef->real.ms.nDir == 1 ? TRUE : FALSE;
				//oDuc.hlamp.b.hcb = c_pTcRef->real.ms.nDir == 2 ? TRUE : FALSE;
				oDuc.hlamp.b.hcf = (c_pTcRef->real.ms.nDir & 1) ? TRUE : FALSE;
				oDuc.hlamp.b.hcb = (c_pTcRef->real.ms.nDir & 2) ? TRUE : FALSE;
			}
			else {
				oDuc.hlamp.b.tcab = TRUE;
				// Modified 2012/12/25
				//oDuc.hlamp.b.tcf = c_pTcRef->real.ms.nDir == 1 ? TRUE : FALSE;
				//oDuc.hlamp.b.tcb = c_pTcRef->real.ms.nDir == 2 ? TRUE : FALSE;
				oDuc.hlamp.b.tcf = (c_pTcRef->real.ms.nDir & 1) ? TRUE : FALSE;
				oDuc.hlamp.b.tcb = (c_pTcRef->real.ms.nDir & 2) ? TRUE : FALSE;
			}
		}
	}
	oDuc.insp.a = 0;
	if (pTcx->GetTrainDir(TRUE) == 1)	oDuc.insp.b.fwd = TRUE;
	if (!(c_pTcRef->real.ms.wBrake & MSTATE_SECURITYBRAKE))	oDuc.insp.b.nosecur = TRUE;
	if (c_pTcRef->real.mcDrv.nIndex == MCDSV_B7POS)	oDuc.insp.b.pb7 = TRUE;
	if (!IsRescue())	oDuc.insp.b.norescue = TRUE;
	if (GETTBIT(OWN_SPACEA, TCDIB_ADSCR))	oDuc.insp.b.adcl = TRUE;
	if (c_pTcRef->real.op.nMode == OPMODE_MANUAL)	oDuc.insp.b.manu = TRUE;
	if (c_pTcRef->real.door.nMode == DOORMODE_AOAC)	oDuc.insp.b.dmaa = TRUE;
	if (GETCBIT(pTcx->GetHeadCarID(), CCDIB_MRPS))	oDuc.insp.b.mrps = TRUE;
	if (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF)	oDuc.insp.b.zvr = TRUE;
	if (c_pTcRef->real.ms.wBrake & MSTATE_PARKINGBRAKE)	oDuc.insp.b.parking = TRUE;

	oDuc.wProgress = pTcx->GetBucketProgress();
	oDuc.wMsg = c_wMsg;

	oDuc.duc.a = 0;
	if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS || c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY) {
		if (c_pTcRef->real.op.nAutoStep & DRIVERLESSSTEP_WAITNUETRAL)
			oDuc.duc.b.drvlessMsg = TRUE;
	}
	oDuc.duc.b.kup = paAtc != NULL ? paAtc->d.st.req.b.kup : FALSE;
	oDuc.duc.b.kdn = paAtc != NULL ? paAtc->d.st.req.b.kdn : FALSE;
	oDuc.duc.b.progress = c_pTcRef->real.nState & STATE_PROGRESSING ? TRUE : FALSE;
	if (!(c_pTcRef->real.nState & STATE_DOWNLOADING)) {
		// Modified 2013/03/11
		//oDuc.duc.b.interview = c_pTcRef->real.nState & STATE_ENABLEOVERHAUL ? TRUE : FALSE;
		oDuc.duc.b.interview = c_pDoz->tcs[CAR_HEAD].real.nSlaveCtrl & SLAVETCCTRL_ENABLEOVERHAUL ? TRUE : FALSE;
		if ((c_pTcRef->real.insp.nMode < INSPECTMODE_PDT ||
				c_pTcRef->real.insp.nMode > INSPECTMODE_EACHDEV) &&
				FBTBIT(OWN_SPACEA, TCDOB_ZVR)) {
			oDuc.duc.b.usb = c_pTcRef->real.nState & STATE_BEENUSB ? TRUE : FALSE;
			// Modified 2013/03/05
			if (!oDuc.duc.b.usb && c_pTcRef->real.du.nDownloadItem == DUKEY_DIRBYDAY) {
				if (c_pTcRef->real.es.nTargetDuty < LENGTH_TC) {
					if ((c_pDoz->tcs[c_pTcRef->real.es.nTargetDuty].real.nIssue & ISSUE_BEENUSB) &&
							!(c_pDoz->tcs[c_pTcRef->real.es.nTargetDuty].real.nIssue & ISSUE_DOWNLOAD))
						oDuc.duc.b.usb = TRUE;
				}
			}
		}
	}
	if (c_pTcRef->real.insp.nMode != INSPECTMODE_NON) {
		if (c_pTcRef->real.insp.nMode < INSPECTMODE_VIEW) {
			oDuc.duc.b.inspExec = TRUE;
			if (c_pTcRef->real.du.nTerm & DUTERM_REPEAT)	oDuc.duc.b.inspRepeat = TRUE;
			if (c_pTcRef->real.du.nTerm & DUTERM_SKIP)		oDuc.duc.b.inspSkip = TRUE;

		}
		else	oDuc.duc.b.inspView = TRUE;
	}
	oDuc.duc.b.logtohard = (c_pTcRef->real.nProperty & PROPERTY_USEEXTERNALMEM) ? FALSE : TRUE;
	//oDuc.duc.b.pwmout = (c_pTcRef->real.op.nState & OPERATESTATE_WITHPWM) ? TRUE : FALSE;
	//if (!(c_pTcRef->real.nProperty & PROPERTY_ATLABORATORY))
	//	oDuc.duc.b.disdownload = TRUE;
	// Modified 2013/02/10
	oDuc.duc.b.ver264 = TRUE;
	// Modified 2013/11/02
	oDuc.duc.b.heavyRecipe = TRUE;

	// Modified 2013/03/05
	if ((c_pDoz->tcs[CAR_HEAD].real.nSlaveCtrl & SLAVETCCTRL_ENABLEOVERHAUL) &&
			c_pTcRef->real.insp.nRefMode != INSPECTMODE_NON && c_pTcRef->real.insp.wItem != 0)
		oDuc.duc.b.otrvbe = TRUE;
	// Modified 2013/03/11
	if (c_pTcRef->real.nDuty == 0 &&
			(c_pDoz->tcs[CAR_HEAD].real.nSlaveCtrl & SLAVETCCTRL_ENABLEOVERHAUL)) {	// stop after time release
		oDuc.duc.b.trialbe = TRUE;
		// Modified 2013/11/02
		//if (c_pTcRef->real.dev.wV3fRstReq != 0)	oDuc.duc.b.v3frst = TRUE;
		oDuc.wResetV3f = (c_pTcRef->real.dev.wV3fRstReq & 0x7e);
	}

	oDuc.wDoorBypass = 0;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD bypass = GetDoorBypass(n);
		WORD door = READCBYTE(n, CCDI_DOOR);
		for (UCURV m = 0; m < 8; m ++) {
			if (bypass & (1 << m))	oDuc.dwDoor[n + HEADCCBYLENGTHA] |= (TD_DBYPASS << m);
			else if (door & (1 << m))	oDuc.dwDoor[n + HEADCCBYLENGTHA] |= (TD_DON << m);
		}
		// Modified 2012/11/14
		if (!GETCBIT(n, CCDIB_DNBS))	oDuc.wDoorBypass |= (1 << n);
	}

	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)
		memcpy((PVOID)&c_nHeadBk[0], (PVOID)&oDuc.glamp.a, SIZE_HEADBK);
	else {
		memcpy((PVOID)&oDuc.glamp.a, (PVOID)&c_pDoz->tcs[CAR_HEAD].real.ducia, SIZE_HEADBK);
		//oDuc.wOpMode = wOpMode;
	}
	//	TC1									TC0
	//	master(duty = 0)					(duty = 2)
	//	CheckTcFirm[0]-> HON or HFAULT		CheckTcFirm[2]-> TON or TFAULT
	//	(duty = 2)							master(duty = 0)
	//	CheckTcFirm[2]-> HON or HFAULT		CheckTcFirm[0]-> TON

	// Modified 2012/11/21
	BYTE tci = GetTcInfo();
	if (tci & 1)	oDuc.dwTc |= 0x00010000;
	else	oDuc.dwTc |= 0x01000000;
	if (tci & 2)	oDuc.dwTc |= 0x00020000;
	else	oDuc.dwTc |= 0x02000000;
	if (tci & 4)	oDuc.dwTc |= 0x00800000;
	else	oDuc.dwTc |= 0x80000000;
	if (tci & 8)	oDuc.dwTc |= 0x00400000;
	else	oDuc.dwTc |= 0x40000000;
	//		CAR_HEAD			CAR_TAIL
	//mast	0		2			0		2
	//addr	1	3	1	3		1	3	1	3
	//dir	HeadTailTailHead	TailHeadHeadTail
	//val.	LON	RON	RON	LON		RON	LON	LON	RON
	BYTE master = (c_pDoz->tcs[CAR_HEAD].real.nHcrInfo - 1) & 2;	// 0 or 2
	BYTE addr = (c_pTcRef->real.nAddr - 1) & 2;		// 0 or 2
	if (master == 0) {
		c_dwTbh = TD_LON;	// 0x00010000
		c_dwTbt = TD_RON;	// 0x00800000
	}
	else {
		c_dwTbh = TD_RON;	// 0x00800000
		c_dwTbt = TD_LON;	// 0x00010000
	}
	//oDuc.dwTc |= c_dwTbh;
	//if (pTcx->GetBusCondition(CAR_HEADBK)) {
	//	if (master == 0)	oDuc.dwTc |= (c_dwTbh << 1);
	//	else	oDuc.dwTc |= (c_dwTbh >> 1);
	//}
	//else {
	//	if (master == 0)	oDuc.dwTc |= (c_dwTbh << (TD_SHIFTCOMF + 1));
	//	else	oDuc.dwTc |= (c_dwTbh >> (TD_SHIFTCOMF - 1));
	//}
	//if (pTcx->GetBusCondition(CAR_TAIL))	oDuc.dwTc |= c_dwTbt;
	//else	oDuc.dwTc |= (c_dwTbt << TD_SHIFTCOMF);
	//if (pTcx->GetBusCondition(CAR_TAILBK)) {
	//	if (master == 0)	oDuc.dwTc |= (c_dwTbt >> 1);
	//	else	oDuc.dwTc |= (c_dwTbt << 1);
	//}
	//else {
	//	if (master == 0)	oDuc.dwTc |= (c_dwTbt << (TD_SHIFTCOMF - 1));
	//	else	oDuc.dwTc |= (c_dwTbt << (TD_SHIFTCOMF + 1));
	//}

	UCURV res;
	// Modified 2013/03/12
	if (FBTBIT(CAR_HEAD, TCDOB_ATCON)) {
	//if (c_pTcRef->real.op.nState & OPERATESTATE_ATCON) {
		if (c_pDoz->tcs[CAR_HEAD].real.eAto.nState == DEVFLOW_ERR)
			oDuc.dwAto |= (c_dwTbh << TD_SHIFTCOMF);
		else	oDuc.dwAto |= c_dwTbh;
		if (c_pDoz->tcs[CAR_HEAD].real.eAtcA.nState == DEVFLOW_ERR &&
				c_pDoz->tcs[CAR_HEAD].real.eAtcB.nState == DEVFLOW_ERR)
			oDuc.dwAtc |= (c_dwTbh << TD_SHIFTCOMF);
		else	oDuc.dwAtc |= c_dwTbh;
	}
	if (FBTBIT(CAR_TAIL, TCDOB_ATCON)) {
		oDuc.dwAto |= (c_dwTbt << TD_SHIFTCOMF);
		oDuc.dwAtc |= (c_dwTbt << TD_SHIFTCOMF);
	}

	Conditional(&oDuc.dwCsc, c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali,
			c_pDoz->tcs[CAR_TAIL].real.acmd.wLocali, LOCALID_CSC);
	Conditional(&oDuc.dwPis, c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali,
			c_pDoz->tcs[CAR_TAIL].real.acmd.wLocali, LOCALID_PIS);
	Conditional(&oDuc.dwHtc, c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali,
			c_pDoz->tcs[CAR_TAIL].real.acmd.wLocali, LOCALID_HTC);
	Conditional(&oDuc.dwPsd, c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali,
			c_pDoz->tcs[CAR_TAIL].real.acmd.wLocali, LOCALID_PSD);
	Conditional(&oDuc.dwTrs, c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali,
			c_pDoz->tcs[CAR_TAIL].real.acmd.wLocali, LOCALID_TRS);

	// Modified 2013/11/02
	oDuc.wHcc = 0xff00;
	if (c_pDoz->tcs[CAR_HEAD].real.acmd.wLocali & (1 << LOCALID_HTC)) {
		for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
			if (!(c_pLcFirm->aHtc.fail.a & (1 << n))) {
				oDuc.wHcc &= ~(1 << (n + HEADCCBYLENGTHA + 8));
				oDuc.wHcc |= (1 << (n + HEADCCBYLENGTHA));
			}
		}
	}

	if (GETTBIT(CAR_HEAD, TCDIB_BATK))	oDuc.dwBatk |= c_dwTbh;
	if (GETTBIT(CAR_HEAD, TCDIB_BATKN))	oDuc.dwBatkn |= c_dwTbh;
	if (pTcx->GetBusCondition(CAR_TAIL)) {
		if (GETTBIT(CAR_TAIL, TCDIB_BATK))	oDuc.dwBatk |= c_dwTbt;
		if (GETTBIT(CAR_TAIL, TCDIB_BATKN))	oDuc.dwBatkn |= c_dwTbt;
	}
	else {
		oDuc.dwBatk |= (c_dwTbt << TD_SHIFTCOMF);
		oDuc.dwBatkn |= (c_dwTbt << TD_SHIFTCOMF);
	}
	if (addr == 0) {
		oDuc.wBat1 = c_pTcRef->real.bat.v;
		UCURV id = master == 0 ? CAR_TAIL : CAR_HEAD;
		WORD w = READANALOG(id, c_pTcRef->real.cond.nAichBat);
		w = w * BATVOLTAGE_LGCRANGE / BATVOLTAGE_PSCRANGE;
		oDuc.wBat0 = w;
	}
	else {
		oDuc.wBat0 = c_pTcRef->real.bat.v;
		UCURV id = master == 0 ? CAR_HEAD : CAR_TAIL;
		WORD w = READANALOG(id, c_pTcRef->real.cond.nAichBat);
		w = w * BATVOLTAGE_LGCRANGE / BATVOLTAGE_PSCRANGE;
		oDuc.wBat1 = w;
	}

	WORD wl = 1;
	//BYTE flag;
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD deves = pTcx->GetCarDevices(n);
		if (!pTcx->GetBusCondition(n + LENGTH_TC)) {
			oDuc.dwCc |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			if (deves & DEVBEEN_SIV) {
				// Modified 2013/11/02
				//flag = n == 0 ? 0 : 7;
				//oDuc.dwSiv |= (TD_DCOMF << flag);
				//oDuc.dwIvs |= (TD_DCOMF << flag);
				oDuc.dwSiv |= (TD_DCOMF << n);
				oDuc.dwIvs |= (TD_DCOMF << n);
			}
			if (deves & DEVBEEN_V3F) {
				// Modified 2013/11/02
				//if ((flag = GetV3fMap(n)) < 0xff) {
				//	oDuc.dwV3f |= (TD_DCOMF << flag);
				//	oDuc.dwHscb |= (TD_DCOMF << flag);
				//	oDuc.dwCck |= (TD_DCOMF << flag);
				//}
				oDuc.dwV3f |= (TD_DCOMF << n);
				oDuc.dwHscb |= (TD_DCOMF << n);
				oDuc.dwCck |= (TD_DCOMF << n);
			}
			oDuc.dwEcu |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			if (deves & DEVBEEN_CMSB) {
				// Modified 2013/11/02
				//if ((flag = GetCmsbMap(n)) < 0xff)
				//	oDuc.dwCmsb |= (TD_DCOMF << flag);
				oDuc.dwCmsb |= (TD_DCOMF << n);
			}
			if (deves & DEVBEEN_AUX) {
				// Modified 2013/11/02
				//if ((flag = GetAuxMap(n)) < 0xff) {
				//	oDuc.dwAcmkr |= (TD_DCOMF << flag);
				//	oDuc.dwIes |= (TD_DCOMF << flag);
				//}
				oDuc.dwAcmkr |= (TD_DCOMF << n);
				oDuc.dwIes |= (TD_DCOMF << n);
			}
			oDuc.dwDcu[n + HEADCCBYLENGTHA] = TD_DCOMFALL;
			oDuc.dwFire |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwPassenger |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwNrbd |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwCprl |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwBoucs |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwBccs1 |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwBccs2 |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwAscs1 |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwAscs2 |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwK1 |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.dwK6 |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			oDuc.wLamp1 |= (TD_WCOMF << (n + HEADCCBYLENGTHA));
			oDuc.wLamp2 |= (TD_WCOMF << (n + HEADCCBYLENGTHA));
		}
		else {
			oDuc.dwCc |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (deves & DEVBEEN_SIV) {
				// Modified 2013/11/02
				//flag = n == 0 ? 0 : 7;
				//res = pTcx->CheckSivReply(n, &c_pDoz->ccs[n].real.eSiv, FALSE);
				//if (res == FIRMERROR_ADDR)	oDuc.dwSiv |= (TD_DCOMF << flag);
				//else if (res == FIRMERROR_FAULT)	oDuc.dwSiv |= (TD_DFAULT << flag);
				//else	oDuc.dwSiv |= (TD_DON << flag);
				//if (GETCBIT(n, CCDIB_IVS))	oDuc.dwIvs |= (TD_DON << flag);
				res = pTcx->CheckSivReply(n, &c_pDoz->ccs[n].real.eSiv, FALSE);
				if (res == FIRMERROR_ADDR)	oDuc.dwSiv |= (TD_DCOMF << n);
				else if (res == FIRMERROR_FAULT)	oDuc.dwSiv |= (TD_DFAULT << n);
				else	oDuc.dwSiv |= (TD_DON << n);
				if (GETCBIT(n, CCDIB_IVS))	oDuc.dwIvs |= (TD_DON << n);
			}
			if (deves & DEVBEEN_V3F) {
				// Modified 2013/11/02
				//flag = GetV3fMap(n);
				//if (flag < 0xff) {
				//	res = pTcx->CheckV3fReply(n, &c_pDoz->ccs[n].real.eV3f, FALSE);
				//	if (res == FIRMERROR_ADDR)	oDuc.dwV3f |= (TD_DCOMF << flag);
				//	else if (res == FIRMERROR_FAULT)	oDuc.dwV3f |= (TD_DFAULT << flag);
				//	else {
				//		if (c_pDoz->ccs[n].real.eV3f.d.st.st.req.b.acoreq)	oDuc.dwV3f |= (TD_DCUTOUT << flag);
				//		else	oDuc.dwV3f |= (TD_DON << flag);
				//		if (c_pDoz->ccs[n].real.eV3f.d.st.st.sig.b.hscb)		oDuc.dwHscb |= (TD_DON << flag);
				//		if (c_pDoz->ccs[n].real.eV3f.d.st.st.sig.b.cck)		oDuc.dwCck |= (TD_DON << flag);
				//	}
				//}
				res = pTcx->CheckV3fReply(n, &c_pDoz->ccs[n].real.eV3f, FALSE);
				if (res == FIRMERROR_ADDR)	oDuc.dwV3f |= (TD_DCOMF << n);
				else if (res == FIRMERROR_FAULT)	oDuc.dwV3f |= (TD_DFAULT << n);
				else {
					if (c_pDoz->ccs[n].real.eV3f.d.st.st.req.b.acoreq)	oDuc.dwV3f |= (TD_DCUTOUT << n);
					else	oDuc.dwV3f |= (TD_DON << n);
					if (c_pDoz->ccs[n].real.eV3f.d.st.st.sig.b.hscb)		oDuc.dwHscb |= (TD_DON << n);
					if (c_pDoz->ccs[n].real.eV3f.d.st.st.sig.b.cck)		oDuc.dwCck |= (TD_DON << n);
				}
			}
			res = pTcx->CheckEcuReply(n, &c_pDoz->ccs[n].real.aEcu, FALSE);
			if (res == FIRMERROR_ADDR)	oDuc.dwEcu |= (TD_DCOMF << (n + HEADCCBYLENGTHA));
			else if (res == FIRMERROR_FAULT)	oDuc.dwEcu |= (TD_DFAULT << (n + HEADCCBYLENGTHA));
			else	oDuc.dwEcu |= (TD_DON << (n + HEADCCBYLENGTHA));
			// Modified 2012/12/05
			//else {
			//	if (GETCBIT(n, CCDIB_BHEKN))	oDuc.dwEcu |= (TD_DCUTOUT << (n + tc));
			//	else	oDuc.dwEcu |= (TD_DON << (n + tc));
			//}
			if (deves & DEVBEEN_CMSB) {
				// Modified 2013/11/02
				//flag = GetCmsbMap(n);
				//if (flag < 0xff) {
				//	res = pTcx->CheckCmsbReply(n, &c_pDoz->ccs[n].real.aCmsb, FALSE);
				//	if (res == FIRMERROR_ADDR)	oDuc.dwCmsb |= (TD_DCOMF << flag);
				//	else if (res == FIRMERROR_FAULT)	oDuc.dwCmsb |= (TD_DFAULT << flag);
				//	else {
				//		if (c_pDoz->ccs[n].real.aCmsb.state.b.bypass)	oDuc.dwCmsb |= (TD_DBYPASS << flag);
				//		else if (c_pDoz->ccs[n].real.aCmsb.state.b.invon)	oDuc.dwCmsb |= (TD_DCUTOUT << flag);
				//		else	oDuc.dwCmsb |= (TD_DON << flag);
				//	}
				//}
				res = pTcx->CheckCmsbReply(n, &c_pDoz->ccs[n].real.aCmsb, FALSE);
				if (res == FIRMERROR_ADDR)	oDuc.dwCmsb |= (TD_DCOMF << n);
				else if (res == FIRMERROR_FAULT)	oDuc.dwCmsb |= (TD_DFAULT << n);
				else {
					if (c_pDoz->ccs[n].real.aCmsb.state.b.bypass)	oDuc.dwCmsb |= (TD_DBYPASS << n);
					else if (c_pDoz->ccs[n].real.aCmsb.state.b.invon)	oDuc.dwCmsb |= (TD_DCUTOUT << n);
					else	oDuc.dwCmsb |= (TD_DON << n);
				}
			}
			if (deves & DEVBEEN_AUX) {
				// Modified 2013/11/02
				//if ((flag = GetAuxMap(n)) < 0xff) {
				//	if (GETCBIT(n, CCDIB_ACMKR))
				//		oDuc.dwAcmkr |= (TD_DON << flag);
				//	if (GETCBIT(n, CCDIB_IESPE))
				//		oDuc.dwIes |= (TD_DFAULT << flag);
				//	else if (GETCBIT(n, CCDIB_IESPS))
				//		oDuc.dwIes |= (TD_DON << flag);
				//}
				if (GETCBIT(n, CCDIB_ACMKR))
					oDuc.dwAcmkr |= (TD_DON << n);
				if (GETCBIT(n, CCDIB_IESPE))
					oDuc.dwIes |= (TD_DFAULT << n);
				else if (GETCBIT(n, CCDIB_IESPS))
					oDuc.dwIes |= (TD_DON << n);
			}
			for (UCURV m = 0; m < 4; m ++) {
				//if (c_pDoz->ccs[n].real.aDcu[0].nDcua[m] == (m + 1)) {
				if (c_pDoz->ccs[n].real.aDcu[0][m].nDcua != 0) {
					if (c_pDoz->ccs[n].real.aDcu[0][m].test.b.dcus) {
						if (c_pDoz->ccs[n].real.aDcu[0][m].state.b.diso)
							oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DBYPASS << m);
						//else if (GETCBIT(n, CCDIB_DSL1 + m))
						else
							oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DON << m);
					}
					else	oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DFAULT << m);
				}
				else	oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DCOMF << m);
			}
			for (UCURV m = 0; m < 4; m ++) {
				//if (c_pDoz->ccs[n].real.aDcu[1][m].nDcua == (m + 5)) {
				if (c_pDoz->ccs[n].real.aDcu[1][m].nDcua != 0) {
					if (c_pDoz->ccs[n].real.aDcu[1][m].test.b.dcus) {
						if (c_pDoz->ccs[n].real.aDcu[1][m].state.b.diso)
							oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DBYPASS << (m + 4));
						//else if (GETCBIT(n, CCDIB_DSR1 + m))
						else
							oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DON << (m + 4));
					}
					else	oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DFAULT << (m + 4));
				}
				else	oDuc.dwDcu[n + HEADCCBYLENGTHA] |= (TD_DCOMF << (m + 4));
			}
			if (c_pTcRef->real.ca.fire.wRep & wl)
				oDuc.dwFire |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.ca.emer.wRep & wl)
				oDuc.dwPassenger |= (TD_DON << (n + HEADCCBYLENGTHA));
			if ((c_pTcRef->real.dev.wNrbi & wl) || (c_pTcRef->real.dev.wNrbc & wl))
				oDuc.dwNrbd |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wCprl & wl)
				oDuc.dwCprl |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wBoucs & wl)
				oDuc.dwBoucs |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wFailBccs1 & wl)
				oDuc.dwBccs1 |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wFailBccs2 & wl)
				oDuc.dwBccs2 |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wAscs1 & wl)
				oDuc.dwAscs1 |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wAscs2 & wl)
				oDuc.dwAscs2 |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wK1 & wl)
				oDuc.dwK1 |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (c_pTcRef->real.dev.wK6 & wl)
				oDuc.dwK6 |= (TD_DON << (n + HEADCCBYLENGTHA));
			if (GETCBIT(n, CCDIB_LK1))	oDuc.wLamp1 |= (wl << HEADCCBYLENGTHA);
			if (GETCBIT(n, CCDIB_LK2))	oDuc.wLamp2 |= (wl << HEADCCBYLENGTHA);
		}
		wl <<= 1;
	}
	// Modified 2012/11/29
	oDuc.dwDcu[c_pTcRef->real.cf.nLength - 1] = XchAtZero(oDuc.dwDcu[c_pTcRef->real.cf.nLength - 1]);

	oDuc.pform.b.bypass = (GETTBIT(CAR_HEAD, TCDIB_ADSBS)) ? TRUE : FALSE;
	oDuc.pform.b.open = c_pLcFirm->aPsd.info.b.psdo;
	oDuc.pform.b.close = c_pLcFirm->aPsd.info.b.psdc;
	oDuc.pform.b.alarm = c_pLcFirm->aPsd.info.b.wpsdf;
	oDuc.pform.b.error = c_pLcFirm->aPsd.info.b.psdf;

	switch(c_pTcRef->real.door.nMode) {
	case DOORMODE_AOAC :	oDuc.wDoorMode = 1;	break;
	case DOORMODE_AOMC :	oDuc.wDoorMode = 2;	break;
	case DOORMODE_MOMC :	oDuc.wDoorMode = 3;	break;
	default :				oDuc.wDoorMode = 0;	break;
	}

	// Modified 2013/11/02
	//for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
	//	if (n == __AUXA || n == __AUXB) {
	//		WORD pan = 4;
	//		if (GETCBIT(n, CCDIB_PANPS2))	pan |= 2;
	//		if (GETCBIT(n, CCDIB_PANPS1))	pan |= 1;
	//		oDuc.wPattern[n] = pan;
	//	}
	//	else	oDuc.wPattern[n] = c_wPattern[n];
	//}
	BYTE type, rpf, rpr, dpf, dpr;
	switch (c_pTcRef->real.cf.nLength) {
	case 4 :	type = 2;	rpf = 1;	dpf = 3;	rpr = dpr = 0xff;		break;
	case 6 :	type = 1;	rpf = 1;	dpf = 2;	rpr = 3;	dpr = 4;	break;
	default :	type = 0;	rpf = dpf = 1;	rpr = dpr = 5;					break;
	}
	for (UCURV n = 0; n < MAX_TRAIN_LENGTH; n ++)
		oDuc.wPattern[n] = c_wPattern[type][n];
	WORD pan = 4;
	if (GETCBIT(rpf, CCDIB_PANPS2))	pan |= 2;
	if (GETCBIT(rpf, CCDIB_PANPS1))	pan |= 1;
	oDuc.wPattern[dpf] = pan;
	if (rpr < 0xff) {
		pan = 4;
		if (GETCBIT(rpr, CCDIB_PANPS2))	pan |= 2;
		if (GETCBIT(rpr, CCDIB_PANPS1))	pan |= 1;
		oDuc.wPattern[dpr] = pan;
	}

	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)
		memcpy((PVOID)&c_nNormalBk[0], (PVOID)&oDuc.pform.a, SIZE_NORMALBK);
	else	memcpy((PVOID)&oDuc.pform.a, (PVOID)&c_pDoz->tcs[CAR_HEAD].real.ducib, SIZE_NORMALBK);

	oDuc.nCSum = CalcSum(sizeof(DUOPERATEINFO));
	oDuc.nCr = '\r';
	oDuc.nLf = '\n';
}

void CDuDoc::TcBusDraw(BYTE state, BYTE flag, WORD node, WORD bus)
{
	if (state & flag) {	// live?
		if (state & (flag << 4)) {
			mDuc.wResvNode |= node;
			mDuc.wResvBus |= bus;
		}
		else {
			mDuc.wMainNode |= node;
			mDuc.wMainBus |= bus;
		}
	}
}

void CDuDoc::DevMonit()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUMONIT;

	BYTE master = (c_pDoz->tcs[CAR_HEAD].real.nHcrInfo - 1) & 2;	// 0 or 2
	// Modified 2012/11/21
	mDuc.wMainNode = mDuc.wResvNode = mDuc.wMainBus = mDuc.wResvBus = 0;
	BYTE tci = GetTcInfo();
	TcBusDraw(tci, 1, 1, 1);
	TcBusDraw(tci, 2, 2, 1);
	TcBusDraw(tci, 4, 0x400, 0x100);
	TcBusDraw(tci, 8, 0x800, 0x100);
	WORD net = master == 0 ? 0 : 0x1ff;
	for (UCURV n = 0; n < 8; n ++) {
		if (master == 0)	net |= (1 << n);
		else	net &= ~(1 << n);
		if (IsValidCc(n)) {
			if (GETLWORD(c_pDoz->ccs[n].real.wAddr) & 0x8000) {
				mDuc.wResvNode |= (1 << (n + 2));
				mDuc.wResvBus |= net;
			}
			else {
				mDuc.wMainNode |= (1 << (n + 2));
				mDuc.wMainBus |= net;
			}
		}
	}

	if (master == 0) {	// TC1
		if (mDuc.wMainBus & 0x100)	mDuc.wMainBus = 0x1ff;
		if (mDuc.wResvBus & 0x100)	mDuc.wResvBus = 0x1ff;
		if ((mDuc.wMainBus & 1) && ((mDuc.wMainNode & 3) == 0)) {
			mDuc.wMainNode |= (mDuc.wResvNode & 3);
			if (mDuc.wMainNode & 1)	mDuc.wMainNode &= 0xfffd;
		}
		if ((mDuc.wResvBus & 1) && ((mDuc.wResvNode & 3) == 0)) {
			mDuc.wResvNode |= (mDuc.wMainNode & 3);
			if (mDuc.wResvNode & 1)	mDuc.wResvNode &= 0xfffd;
		}
	}
	else {	// TC0
		if (mDuc.wMainBus & 1)	mDuc.wMainBus = 0x1ff;
		if (mDuc.wResvBus & 1)	mDuc.wResvBus = 0x1ff;
		if ((mDuc.wMainBus & 0x100) && ((mDuc.wMainNode & 0xc00) == 0)) {
			mDuc.wMainNode |= (mDuc.wResvNode & 0xc00);
			if (mDuc.wMainNode & 0x400)	mDuc.wMainNode &= 0xf7ff;
		}
		if ((mDuc.wResvBus & 0x100) && ((mDuc.wResvNode & 0xc00) == 0)) {
			mDuc.wResvNode |= (mDuc.wMainNode & 0xc00);
			if (mDuc.wResvNode & 0x400)	mDuc.wResvNode &= 0xf7ff;
		}
	}

	PDUMBDEV pDumb = pTcx->GetDumb();
	memcpy((PVOID)&mDuc.dumb, (PVOID)pDumb, sizeof(DUMBDEV));
	if (master != 0) {
		Exchange(&mDuc.dumb.wCsc[0], &mDuc.dumb.wCsc[1]);
		Exchange(&mDuc.dumb.wPis[0], &mDuc.dumb.wPis[1]);
		Exchange(&mDuc.dumb.wHtc[0], &mDuc.dumb.wHtc[1]);
		Exchange(&mDuc.dumb.wTrs[0], &mDuc.dumb.wTrs[1]);
		Exchange(&mDuc.dumb.wPsd[0], &mDuc.dumb.wPsd[1]);
	}
	for (UCURV n = 0; n < CAR_MAX; n ++) {		// @@@@@ need check length @@@@@
		if (n < CAR_CC) {
			mDuc.dwPanelNo[n ^ master] = ((DWORD)c_pDoz->tcs[n].real.nThumbWheel[0] << 16) |
								((DWORD)c_pDoz->tcs[n].real.nThumbWheel[1] << 8) |
								(DWORD)c_pDoz->tcs[n].real.nThumbWheel[2];
			mDuc.wTcVer[n ^ master] = HEXTODEC(c_pDoz->tcs[n].real.wVersion);
		}
		else {
			mDuc.dwPanelNo[n] = ((DWORD)c_pDoz->ccs[ONLYCC(n)].real.nThumbWheel[0] << 16) |
								((DWORD)c_pDoz->ccs[ONLYCC(n)].real.nThumbWheel[1] << 8) |
								(DWORD)c_pDoz->ccs[ONLYCC(n)].real.nThumbWheel[2];
			mDuc.wCcVer[ONLYCC(n)] = HEXTODEC(c_pDoz->ccs[ONLYCC(n)].real.wVersion);
		}
	}

	mDuc.nCSum = CalcSum(sizeof(DUMONITINFO));
	mDuc.nCr = '\r';
	mDuc.nLf = '\n';
}

void CDuDoc::StatusA()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUSTATUSA;
	UCURV n, vi, si;
	sDuc.wSiv = TB4_HC | TB4_RC;
	for (n = vi = si = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD dev = pTcx->GetCarDevices(n);
		if (dev & DEVBEEN_V3F) {
			sDuc.wFc[vi] = pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nFc, 0, 250, (WORD)0, (WORD)2000);
			sDuc.wTrqRef[vi] = pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nTrqRef, 0, 250, (WORD)0, (WORD)2000);
			sDuc.wIq[vi] = pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nIq, 0, 250, (WORD)0, (WORD)2000);
			sDuc.tbe[vi] = (SHORT)pTcx->ConvS(GETLWORD(c_pDoz->ccs[n].real.eV3f.d.st.st.vTbeL), -1024, 1024, -100, 100);
			sDuc.wVl[vi] = pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nVl, 0, 250, (WORD)0, (WORD)100);
			sDuc.wIm[vi] = pTcx->Conv(c_pDoz->ccs[n].real.eV3f.d.st.st.nIm, 0, 250, (WORD)0, (WORD)1000);

			sDuc.wBed[vi] = pTcx->Conv(c_pDoz->ccs[n].real.aEcu.d.st.nBed, 0, 255, (WORD)0, (WORD)102);
			sDuc.wBea[vi] = pTcx->Conv(c_pDoz->ccs[n].real.aEcu.d.st.nBea, 0, 255, (WORD)0, (WORD)102);
			++ vi;
		}
		if (dev & DEVBEEN_SIV) {
			sDuc.wEd[si] = pTcx->Conv(c_pDoz->ccs[n].real.eSiv.d.st.st.nEd, 0, 250, (WORD)0, (WORD)3000);
			sDuc.wEo[si] = pTcx->Conv(c_pDoz->ccs[n].real.eSiv.d.st.st.nEo, 0, 250, (WORD)0, (WORD)750);
			sDuc.wIo[si] = pTcx->Conv(c_pDoz->ccs[n].real.eSiv.d.st.st.nIo, 0, 250, (WORD)0, (WORD)1250);
			sDuc.wOf[si] = pTcx->Conv(c_pDoz->ccs[n].real.eSiv.d.st.st.nOf, 0, 250, (WORD)0, (WORD)1250);
			if (si == 0 && (c_pTcRef->real.dev.wFailSiv & (1 << __SIVA)))
				sDuc.wSiv |= TB4_HF;
			else if (si == 1 && (c_pTcRef->real.dev.wFailSiv & (1 << __SIVB)))
				sDuc.wSiv |= TB4_RF;
			++ si;
		}
	}
	if (_ISOCC(__ESK) && GETCBIT(__ESK, CCDIB_ESKC))	sDuc.wExt = 1;
	sDuc.wLamp1 = sDuc.wLamp2 = 0;
	WORD wl, wh;
	// Modified 2013/11/02
	for (n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		// decimal point 2
		sDuc.wAsp[n + HEADCCBYLENGTHA] = (WORD)(pTcx->GetRealAsp(n) * 100.f);
		sDuc.wBcp[n + HEADCCBYLENGTHA] = (WORD)(pTcx->GetRealBcp(n) * 100.f);

		wl = (WORD)(1 << (n + HEADCCBYLENGTHA));
		wh = (WORD)(0x100 << (n + HEADCCBYLENGTHA));
		if (GETCBIT(n, CCDIB_LK1))	sDuc.wLamp1 |= wl;
		if (GETCBIT(n, CCDIB_LK2))	sDuc.wLamp2 |= wl;
		if (!pTcx->GetBusCondition(n + LENGTH_TC)) {
			sDuc.wLamp1 |= wh;
			sDuc.wLamp2 |= wh;
		}
	}
	// Modified 2012/09/06
	// Modified 2012/03/05 ... start
	//if (c_pTcRef->real.op.nRescueCtrl != RESCUECTRL_NON) {
	//	switch (c_pTcRef->real.op.nRescueCtrl) {
	//	case RESCUECTRL_PASSIVETRAIN :
	//		sDuc.wRescue = DUKEY_RESCUETRAIN;
	//		break;
	//	case RESCUECTRL_ACTIVECAR :
	//		sDuc.wRescue = DUKEY_RESCUECAR;
	//		break;
	//	case RESCUECTRL_ACTIVEUNDERGO :
	//	case RESCUECTRL_PASSIVEUNDERGO :
	//		sDuc.wRescue = DUKEY_RESCUEUNDERGO;
	//		break;
	//	case RESCUECTRL_READYEXIT :
	//		sDuc.wRescue = DUKEY_RESCUEEXIT;
	//		c_pTcRef->real.op.nRescueCtrl = RESCUECTRL_NON;
	//		break;
	//	default :
	//		break;
	//	}
	//}

	// Modified 2013/11/02
	//BYTE duty = OWN_SPACEA & 2;						// 0 or 2
	//BYTE addr = (c_pTcRef->real.nAddr - 1) & 2;		// 0 or 2
	//BYTE x = duty != addr ? 2 : 0;
	//for (n = 0; n < 4; n ++) {
	//	sDuc.wMcDrv[n ^ x] =
	//			pTcx->Conv(READANALOG(n, c_pTcRef->real.cond.nAichDsv), 0, 4095, (WORD)0, (WORD)500);
	//}
	//if (x)	x = 1;
	//sDuc.wBat[0 ^ x] = pTcx->Conv(READANALOG(CAR_HEAD, c_pTcRef->real.cond.nAichBat), 0, 4095, (WORD)0, (WORD)500);
	//sDuc.wBat[2 ^ x] = pTcx->Conv(READANALOG(CAR_HEAD, c_pTcRef->real.cond.nAichBat), 0,
	//							BATVOLTAGE_PSCRANGE, (WORD)0, (WORD)BATVOLTAGE_LGCRANGE);
	//sDuc.wBat[1 ^ x] = pTcx->Conv(READANALOG(CAR_TAIL, c_pTcRef->real.cond.nAichBat), 0, 4095, (WORD)0, (WORD)500);
	//sDuc.wBat[3 ^ x] = pTcx->Conv(READANALOG(CAR_TAIL, c_pTcRef->real.cond.nAichBat), 0,
	//							BATVOLTAGE_PSCRANGE, (WORD)0, (WORD)BATVOLTAGE_LGCRANGE);
	for (n = 0; n < LENGTH_TC; n ++) {
		SHORT tcid = pTcx->GetPscIDByAddr(n);
		if (tcid >= 0) {
			sDuc.wMcDrv[tcid] = pTcx->Conv(READANALOG(n, c_pTcRef->real.cond.nAichDsv), 0, 4095, (WORD)0, (WORD)500);
			if (tcid == 0 || tcid == 2)
				sDuc.wBat[tcid] = pTcx->Conv(READANALOG(n, c_pTcRef->real.cond.nAichBat), 0, 4095, (WORD)0, (WORD)500);
			else	sDuc.wBat[tcid] = pTcx->Conv(READANALOG(n - 1, c_pTcRef->real.cond.nAichBat), 0, BATVOLTAGE_PSCRANGE, (WORD)0, (WORD)BATVOLTAGE_LGCRANGE);
		}
	}

	for (UCURV n = 0; n < 40; n ++)
		sDuc.wMsg[n] = MAKEWORD(c_nOsMsg[n * 2], c_nOsMsg[n * 2 + 1]);

	sDuc.nCSum = CalcSum(sizeof(DUSTATUSINFOA));
	sDuc.nCr = '\r';
	sDuc.nLf = '\n';
}

void CDuDoc::StatusB()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUSTATUSB;

	//BYTE duty = OWN_SPACEA & 2;						// 0 or 2
	//BYTE addr = (c_pTcRef->real.nAddr - 1) & 2;		// 0 or 2
	//BYTE x = duty != addr ? 2 : 0;
	for (UCURV n = 0; n < LENGTH_TC; n ++) {
		SHORT tcid = pTcx->GetPscIDByAddr(n);
		if (tcid >= 0) {
			for (UCURV m = 0; m < (SIZE_TCDI / 2); m ++)
				bDuc.wTcDib[tcid][m] =
						MAKEWORD(READTBYTE(n, m * 2 + 1), READTBYTE(n, m * 2));
			for (UCURV m = 0; m < (SIZE_TCDO / 2); m ++)
				bDuc.wTcDob[tcid][m] =
						// Modified 2012/11/29
						//MAKEWORD(FEEDTBYTE(n, m * 2 + 1), FEEDTBYTE(n, m * 2));
						MAKEWORD(c_pDoz->tcs[n].real.nRealOutput[m * 2 + 1], c_pDoz->tcs[n].real.nRealOutput[m * 2]);
		}
	}
	bDuc.wTcDob[0][3] = bDuc.wTcDob[1][3] =
	bDuc.wTcDob[2][3] = bDuc.wTcDob[3][3] = 0;

	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		for (BYTE m = 0; m < 3; m ++) {
			if (n == (c_pTcRef->real.cf.nLength - 1) && m == 2)
				bDuc.wCcDib[n][m] = MAKEWORD(READCBYTE(n, m * 2 + 1), WrenchByte(READCBYTE(n, m * 2)));
			else	bDuc.wCcDib[n][m] = MAKEWORD(READCBYTE(n, m * 2 + 1), READCBYTE(n, m * 2));
			if (m < 2)
				bDuc.wCcDob[n][m] = MAKEWORD(FEEDCBYTE(n, m * 2 + 1), FEEDCBYTE(n, m * 2));
			else	bDuc.wCcDob[n][2] = 0;
		}
	}

	// Modified 2013/11/02
	//WORD* pV;
	//switch (c_pTcRef->real.cf.nLength) {
	//case 4 :	pV = &bDuc.wPsgRate[2];	break;
	//case 6 :	pV = &bDuc.wPsgRate[1];	break;
	//default :	pV = &bDuc.wPsgRate[0];	break;
	//}
	WORD* pV = &bDuc.wPsgRate[HEADCCBYLENGTHA];

	double sumRate, sumWeight;
	sumRate = sumWeight = 0;
	for (BYTE n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		double rate = pTcx->GetPassengerWeightRate(n);
		*pV = (WORD)(rate * 10.f);	// decimal 1 pt
		sumRate += rate;
		double weight = rate / 10.f;	//(rate / 100.f) * 10.f;	// 10t
		*(pV + 9) = (WORD)(weight * 10.f);	// decimal 1 pt
		sumWeight += weight;
		++ pV;
	}
	bDuc.wPsgRate[8] = (WORD)(sumRate * 10.f / c_pTcRef->real.cf.nLength);	// decimal 1 pt
	bDuc.wPsgWeight[8] = (WORD)(sumWeight * 10.f / c_pTcRef->real.cf.nLength);	// decimal 1 pt
	for (UCURV n = 0; n < 4; n ++) {
		if (_ISOCC(c_pTcRef->real.cid.oc.nV3fI[n])) {
			for (UCURV m = 0; m < 4; m ++) {
				bDuc.wSpeed[n][m] = (WORD)c_pDoz->ccs[c_pTcRef->real.cid.oc.nV3fI[n]].real.eV3f.d.st.dwSpd[m];
			}
		}
	}
	bDuc.wRelease = c_pTcRef->real.cond.wReleaseTimer;

	PTRIALINFO pTrial = pTcx->GetTrial();
	bDuc.wPtime = pTrial->pwr.t.wCurrent;
	bDuc.wPdist = pTrial->pwr.d.wCurrent;
	bDuc.wBtime = pTrial->brk.t.wCurrent;
	bDuc.wBdist = pTrial->brk.d.wCurrent;
	bDuc.wAccel = pTrial->vAccel;
	if (bDuc.wAccel & 0x8000) {	// - sign
		bDuc.wAccelGraph = 0;
		bDuc.wDecelGraph = bDuc.wAccel;
	}
	else {	// + sign
		bDuc.wAccelGraph = bDuc.wAccel;
		bDuc.wDecelGraph = 0;
	}
	bDuc.wDist = (WORD)(pTrial->dwDist & 0xfffff);
	// Modified 2013/11/02
	// Modified 2013/03/11
	//bDuc.state.a = 0;
	//if (c_pTcRef->real.scLog.dwTag > 0 &&
	//		(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) &&
	//		(c_pTcRef->real.nState & STATE_BEENUSB) &&
	//		// Modified 2013/03/04
	//		c_pTcRef->real.mcDrv.bValid &&
	//		c_pTcRef->real.mcDrv.nIndex == MCDSV_NPOS)
	//	bDuc.state.b.enDownload = 1;
	bDuc.state.a = 0;
	if (c_pTcRef->real.mcDrv.bValid && c_pTcRef->real.mcDrv.nIndex == MCDSV_NPOS) {
		if (c_pTcRef->real.scLog.dwTag > 0 &&
				(c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) &&
				(c_pTcRef->real.nState & STATE_BEENUSB))
			bDuc.state.b.enDownload = 1;
	}
	//else	bDuc.state.b.disoverhaul = TRUE;

	PATOAINFO paAto = pTcx->GetAto(FALSE);
	if (paAto != NULL) {
		bDuc.ato.wPsn = (WORD)paAto->d.st.nPsn;
		bDuc.ato.wNsn = (WORD)paAto->d.st.nNsn;
		bDuc.ato.wDsn = (WORD)paAto->d.st.nDsn;
		bDuc.ato.vTbeB = paAto->d.st.vTbeB;
		bDuc.ato.vPwbB = paAto->d.st.vPwbB;
		bDuc.ato.st.a = (WORD)(paAto->d.st.psm.a & 0x3f);
		bDuc.ato.st.a |= ((WORD)paAto->d.st.info.a << 8) & 0xf000;
		bDuc.ato.st.b.stb = paAto->d.st.state.b.stb;
		bDuc.ato.st.b.idft = paAto->d.st.state.b.idft;
		bDuc.ato.st.b.ator = paAto->d.st.state.b.ator;
		bDuc.ato.st.b.pssmf = paAto->d.st.fail.b.pssmf;
		//bDuc.ato.st.a = (WORD)paAto->d.st.psm.a & 0x3f;
		//bDuc.ato.st.a |= (WORD)((paAto->d.st.state.a & 0x3f) << 8);
		//bDuc.ato.st.b.pssmf = paAto->d.st.fail.b.pssmf;
		//bDuc.ato.st.b.itaf = paAto->d.st.fail.b.itaf;
		//bDuc.ato.st.b.hf = paAto->d.st.arm.b.hf;
		//bDuc.ato.st.b.tcf = paAto->d.st.arm.b.tcf;
	}
	else	memset((PVOID)&bDuc.ato, 0, sizeof(DUATOSTATUS));

	PATCAINFO paAtc = pTcx->GetActiveAtc(FALSE);
	if (paAtc != NULL) {
		bDuc.atc.wSpeed = (WORD)(c_pTcRef->real.speed.dwAtcs / 1000);	// to km
		bDuc.atc.sta.a = (WORD)(paAtc->d.st.fail.a & 0x3c);
		bDuc.atc.sta.a |= ((WORD)paAtc->d.st.pos.a << 9) & 0x3c00;
		bDuc.atc.sta.b.dchf = paAtc->d.st.err.b.dchf;
		bDuc.atc.sta.b.mce = paAtc->d.st.err.b.mce;
		bDuc.atc.sta.b.edls = paAtc->d.st.sph.b.edls;
		bDuc.atc.sta.b.ahbr = paAtc->d.st.req.b.ahbr;
		bDuc.atc.sta.b.dbp = paAtc->d.st.req.b.dbp;
		bDuc.atc.sta.b.fsbr = paAtc->d.st.req.b.fsbr;
		bDuc.atc.sta.b.edrs = paAtc->d.st.sph.b.edrs;
		bDuc.atc.stb.a = (WORD)paAtc->d.st.mode.a;
		bDuc.atc.stb.a |= ((WORD)paAtc->d.st.swi.a << 8) & 0xf00;
		bDuc.atc.wheel = (WORD)WHEELBYATC(paAtc);
		//bDuc.atc.sta.a = (WORD)(paAtc->d.st.err.a & 0x37);
		//bDuc.atc.sta.a |= (WORD)((paAtc->d.st.fail.a & 0x3c) << 8);
		//bDuc.atc.sta.b.adp = paAtc->d.st.mode.b.adp;
		//bDuc.atc.sta.b.odr = paAtc->d.st.mode.b.odr;
		//bDuc.atc.sta.b.odl = paAtc->d.st.mode.b.odl;
		//bDuc.atc.sta.b.fsbr = paAtc->d.st.req.b.fsbr;
		//bDuc.atc.sta.b.dbp = paAtc->d.st.req.b.dbp;
		//bDuc.atc.sta.b.ahbr = paAtc->d.st.req.b.ahbr;
		//bDuc.atc.stb.a = (WORD)(paAtc->d.st.tdws.a & 0xf);
		//bDuc.atc.stb.a |= (WORD)((paAtc->d.st.swi.a & 0xf) << 4);
		//bDuc.atc.stb.a |= (WORD)((paAtc->d.st.pos.a & 0x3e) << 8);
		//bDuc.atc.stb.b.edrs = paAtc->d.st.sph.b.edrs;
		//bDuc.atc.stb.b.edls = paAtc->d.st.sph.b.edls;
	}
	else	memset((PVOID)&bDuc.atc, 0, sizeof(DUATCSTATUS));

	bDuc.nCSum = CalcSum(sizeof(DUSTATUSINFOB));
	bDuc.nCr = '\r';
	bDuc.nLf = '\n';
}

void CDuDoc::CommMonit()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	memset((PVOID)&cDuc, 0, sizeof(DUCOMMMONIT));
	c_pDucInfo->srDuc.wTargetAddr = ADDR_COMMMONIT;

	if (c_pTcRef->real.op.nState & OPERATESTATE_ATCON) {
		BYTE atci = c_pTcRef->real.cond.nActiveAtc == (LOCALID_ATCA + 1) ? 0 : 1;
		memcpy((PVOID)&cDuc.ratc, (PVOID)&c_pLcFirm->rAtc[atci], SIZE_ATCSR);
		memcpy((PVOID)&cDuc.aatc, (PVOID)&c_pLcFirm->aAtc[atci], SIZE_ATCSA);
		memcpy((PVOID)&cDuc.rato, (PVOID)&c_pLcFirm->rAto, SIZE_ATOSR);
		memcpy((PVOID)&cDuc.aato, (PVOID)&c_pLcFirm->aAto, SIZE_ATOSA);
	}
	else {
		memset((PVOID)&cDuc.ratc, 0, SIZE_ATCSR);
		memset((PVOID)&cDuc.aatc, 0, SIZE_ATCSA);
		memset((PVOID)&cDuc.rato, 0, SIZE_ATOSR);
		memset((PVOID)&cDuc.aato, 0, SIZE_ATOSA);
	}
	BYTE cid = GetRealCid(pTcx->GetSelectedCid(), FALSE);
	memcpy((PVOID)&cDuc.rsiv, (PVOID)&c_pDoz->ccs[cid].real.rSiv, SIZE_SIVSR);
	memcpy((PVOID)&cDuc.asiv, (PVOID)&c_pDoz->ccs[cid].real.eSiv, SIZE_SIVSA);
	memcpy((PVOID)&cDuc.rv3f, (PVOID)&c_pDoz->ccs[cid].real.rV3f, SIZE_V3FSR);
	memcpy((PVOID)&cDuc.av3f, (PVOID)&c_pDoz->ccs[cid].real.eV3f, SIZE_V3FSA);
	memcpy((PVOID)&cDuc.recu, (PVOID)&c_pDoz->ccs[cid].real.rEcu, SIZE_ECUSR);
	memcpy((PVOID)&cDuc.aecu, (PVOID)&c_pDoz->ccs[cid].real.aEcu, SIZE_ECUSA);
	memcpy((PVOID)&cDuc.rcmsb, (PVOID)&c_pDoz->ccs[cid].real.rCmsb, sizeof(CMSBRINFO));
	memcpy((PVOID)&cDuc.acmsb, (PVOID)&c_pDoz->ccs[cid].real.aCmsb, sizeof(CMSBAINFO));

	//BYTE* p = &cDuc.ratc[0];
	//for (UCURV n = 0; n < (sizeof(DUCOMMMONIT) - 6); n ++)
	//	*p ++ = (BYTE)(n & 0xff);
	//
	cDuc.wSortLamp = 1 << cid;
	cDuc.nCSum = CalcSum(sizeof(DUCOMMMONIT));
	cDuc.nCr = '\r';
	cDuc.nLf = '\n';
}

void CDuDoc::DirectoryByDay()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	memset((PVOID)&lDuc, 0, sizeof(DUSELITEM));
	c_pDucInfo->srDuc.wTargetAddr = ADDR_SELITEM;

	lDuc.wLength = pTcx->GetFilesByDayLength();
	if (lDuc.wLength > SIZE_FILESBYDAY)	lDuc.wLength = SIZE_FILESBYDAY;
	DWORD* pV = pTcx->GetFilesByDay();
	for (WORD w = 0; w < lDuc.wLength; w ++) {
		DWORD dw;
		if (pV != NULL)	dw = *pV ++;
		else	dw = 0;
		lDuc.wItem[w][0] = (WORD)(dw / 10000);	// year
		dw %= 10000;
		lDuc.wItem[w][1] = (WORD)(dw / 100);	// month
		lDuc.wItem[w][2] = (WORD)(dw % 100);	// day
	}
	if (c_pTcRef->real.nAddr < 3) {
		lDuc.wMine = 1;
		lDuc.wOppo = 0;
	}
	else {
		lDuc.wMine = 0;
		lDuc.wOppo = 1;
	}
	//if (c_pTcRef->real.es.nTargetAddr == 2 || c_pTcRef->real.es.nTargetAddr == 4)
	//	lDuc.wSub = 2;
	if (c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty] == 2 ||
			c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty] == 4)
		lDuc.wSub = 2;
	else	lDuc.wSub = 1;
	lDuc.wSide = (c_pTcRef->real.es.nTargetDuty & 2) == (OWN_SPACEA & 2) ? 5 : 2;

	lDuc.nCSum = CalcSum(sizeof(DUSELITEM));
	lDuc.nCr = '\r';
	lDuc.nLf = '\n';
}

void CDuDoc::OpenPage(WORD addr)
{
	c_pDucInfo->srDuc.wTargetAddr = ADDR_PAGE;
	c_pDucInfo->srDuc.j.sp.page = addr;
	c_pDucInfo->srDuc.j.sp.nCSum = CalcSum(sizeof(DUSETPAGEINFO));
	c_pDucInfo->srDuc.j.sp.nCr = '\r';
	c_pDucInfo->srDuc.j.sp.nLf = '\n';
}

void CDuDoc::OpenNormalPage()
{
	OpenPage(1001);
}

void CDuDoc::OpenOverhaulPage()
{
	OpenPage(2001);
}

void CDuDoc::OpenInspectPage()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	OpenPage(pTcx->GetInspectPage());
}

void CDuDoc::OpenRescuePage()
{
	OpenPage(2901);
}

void CDuDoc::OpenTimeSetPage()
{
	OpenPage(9451);
}

void CDuDoc::Inspect()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PDUINSPECTINFO p = pTcx->GetInspectInfo();
	if (p == NULL)	return;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUINSPECT;
	memcpy((PVOID)&iDuc, (PVOID)p, sizeof(DUINSPECTINFO));
	iDuc.nCSum = CalcSum(sizeof(DUINSPECTINFO));
	iDuc.nCr = '\r';
	iDuc.nLf = '\n';
}

void CDuDoc::Enviro()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUENV;
	PENVARCHEXTENSION pEnv = pTcx->GetEnv();
	eDuc.dwVar[0] = (DWORD)(pEnv->real.entire.elem.dbDist / 1000.f);
	eDuc.dwVar[1] = (DWORD)(pEnv->real.entire.elem.dbTime / 3600.f);
	eDuc.dwVar[2] = (DWORD)(pEnv->real.entire.elem.dbV3fPower / 1000.f);
	eDuc.dwVar[3] = (DWORD)(pEnv->real.entire.elem.dbRevival / 1000.f);
	eDuc.dwVar[4] = (DWORD)(pEnv->real.entire.elem.dbSivPower / 1000.f);
	for (UCURV n = 0; n < SIZE_WHEELCAR; n ++) {
		for (UCURV m = 0; m < SIZE_WHEELAXEL; m ++) {
			eDuc.wDia[n][m] = pTcx->ToBcd(pEnv->real.w[n][m]);
		}
	}

	eDuc.wAtc = c_wDevTraceLength[TRACEID_ATC][0];
	eDuc.wAto = c_wDevTraceLength[TRACEID_ATO][0];
	UCURV siv, v3f, ecu;
	siv = v3f = ecu = 0;
	// Modified 2013/11/02
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD deves = pTcx->GetCarDevices(n);
		if (deves & DEVBEEN_SIV)	eDuc.wSiv[siv ++] = c_wDevTraceLength[TRACEID_SIV][n];
		if (deves & DEVBEEN_V3F)	eDuc.wV3f[v3f ++] = c_wDevTraceLength[TRACEID_V3F][n];
		if (deves & DEVBEEN_ECU)	eDuc.wEcu[HEADCCBYLENGTHA + ecu ++] = c_wDevTraceLength[TRACEID_ECU][n];
	}

	eDuc.nCSum = CalcSum(sizeof(DUENVINFO));
	eDuc.nCr = '\r';
	eDuc.nLf = '\n';
}

void CDuDoc::TraceView()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_DUTRACEINFO;
	tDuc.wAtc = c_wDevTraceLength[TRACEID_ATC][0];
	tDuc.wAtcProgress = c_wDevTraceProgress[TRACEID_ATC][0];
	tDuc.wAto = c_wDevTraceLength[TRACEID_ATO][0];
	tDuc.wAtoProgress = c_wDevTraceProgress[TRACEID_ATO][0];
	UCURV siv, v3f, ecu;
	siv = v3f = ecu = 0;
	// Modified 2013/11/02
	for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
		WORD deves = pTcx->GetCarDevices(n);
		if (deves & DEVBEEN_SIV) {
			tDuc.wSiv[siv] = c_wDevTraceLength[TRACEID_SIV][n];
			tDuc.wSivProgress[siv ++] = c_wDevTraceProgress[TRACEID_SIV][n];
		}
		if (deves & DEVBEEN_V3F) {
			tDuc.wV3f[v3f] = c_wDevTraceLength[TRACEID_V3F][n];
			tDuc.wV3fProgress[v3f ++] = c_wDevTraceProgress[TRACEID_V3F][n];
		}
		if (deves & DEVBEEN_ECU) {
			tDuc.wEcu[HEADCCBYLENGTHA + ecu] = c_wDevTraceLength[TRACEID_ECU][n];
			tDuc.wEcuProgress[HEADCCBYLENGTHA + ecu ++] = c_wDevTraceProgress[TRACEID_ECU][n];
		}
	}

	tDuc.nCSum = CalcSum(sizeof(DUTRACEINFO));
	tDuc.nCr = '\r';
	tDuc.nLf = '\n';
}

void CDuDoc::AlarmList()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pDucInfo->srDuc.wTargetAddr = ADDR_ALARMLIST;
	PARM_ARCH pArm = pTcx->GetAlarmList();
	// Modified 2013/11/02
	//for (UCURV n = 0; n < ALARMLIST_LINE; n ++) {
	//	aDuc.arm[n].wOccurDate = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_mon) << 8) |
	//								(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_day);
	//	aDuc.arm[n].wOccurTime = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_hour) << 8) |
	//								(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_min);
	//	aDuc.arm[n].wOccurSec = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_sec) << 8) |
	//								(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.d_sec);
	//	aDuc.arm[n].wEraseTime = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.d_hour) << 8) |
	//								(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.d_min);
	//	TuningCode((WORD)(pArm + n)->b.code, (WORD)(pArm + n)->b.cid, &aDuc.arm[n].fc);
	//}
	memset((PVOID)&aDuc, 0, sizeof(DUALARMLIST));
	for (UCURV n = 0; n < ALARMLIST_LINES; n ++) {
		aDuc.wOccurDate[n] = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_mon) << 8) |
									(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_day);
		aDuc.wOccurTime[n] = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_hour) << 8) |
									(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_min);
		aDuc.wOccurEraseSec[n] = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.u_sec) << 8) |
									(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.d_sec);
		aDuc.wEraseTime[n] = ((WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.d_hour) << 8) |
									(WORD)pTcx->ToBcd((BYTE)(pArm + n)->b.d_min);
		aDuc.wCarID[n] = (WORD)(pArm + n)->b.cid;
		if ((pArm + n)->b.code > 500) {
			aDuc.wReal[n] = (pArm + n)->b.code;
			aDuc.wHigh[n] = (pArm + n)->b.code - 500;
		}
		else	aDuc.wLow[n] = (pArm + n)->b.code;
	}
	aDuc.wCurrentPage = c_pTcRef->real.ai.wCurrentPage + 1;
	aDuc.wMaxPages = c_pTcRef->real.ai.wMaxPages;
	// Modified 2013/03/11
	//if (c_pTcRef->real.ai.wCurrentFile == ALARMFILE_CURRENT)
	//	aDuc.wCurrentFile = c_pTcRef->real.ai.wMaxFiles + 1;
	//else	aDuc.wCurrentFile = c_pTcRef->real.ai.wCurrentFile;
	if (c_pTcRef->real.ai.wCurrentFile == ALARMFILE_CURRENT)
		aDuc.wCurrentFile = 1;
	else	aDuc.wCurrentFile = c_pTcRef->real.ai.wCurrentFile + 1;
	// Modified 2013/01/28
	aDuc.wMaxFiles = c_pTcRef->real.ai.wMaxFiles + 1;
	pTcx->ArmNameParse(&aDuc.wYearSec, &aDuc.wDate, &aDuc.wTime);

	BYTE cid = GetRealCid(pTcx->GetSelectedCid(), TRUE);
	aDuc.wSortLamp = 1 << cid;

	aDuc.nCSum = CalcSum(sizeof(DUALARMLIST));
	aDuc.nCr = '\r';
	aDuc.nLf = '\n';
}

void CDuDoc::HeadPile()
{
	memcpy((PVOID)&c_pDucInfo->srDuc.nEsc, (PVOID)&c_nHeadPile[0], SIZE_HEADPILE);
}

void CDuDoc::TailPile()
{
	memcpy((PVOID)&c_pDucInfo->srDuc.nEsc, (PVOID)&c_nTailPile[0], SIZE_TAILPILE);
}

void CDuDoc::GetEnviro()
{
	Setting(ADDR_ENV, LENGTH_ENV);
}

void CDuDoc::GetWheel()
{
	Setting(ADDR_WHEEL, LENGTH_WHEEL);
}

void CDuDoc::GetInspectItem()
{
	Setting(ADDR_INSPECTITEM, LENGTH_INSPECTITEM);
}

void CDuDoc::GetTraceItem()
{
	Setting(ADDR_TRACEITEM, LENGTH_TRACEITEM);
}

void CDuDoc::Setting(WORD wStart, WORD wLength)
{
	c_pDucInfo->srDuc.wTargetAddr = wStart;
	c_pDucInfo->srDuc.wSendLength = wLength;
	c_pDucInfo->srDuc.j.rc.nCSum = CalcSum(sizeof(DUREADCMDINFO), FALSE);
	c_pDucInfo->srDuc.j.rc.nCr = '\r';
	c_pDucInfo->srDuc.j.rc.nLf = '\n';
}

void CDuDoc::Conditional(DWORD* pDw, WORD head, WORD tail, BYTE id)
{
	if (!(head & (1 << id)))	*pDw |= (c_dwTbh << TD_SHIFTCOMF);
	else	*pDw |= c_dwTbh;
	if (!(tail & (1 << id)))	*pDw |= (c_dwTbt << TD_SHIFTCOMF);
	else	*pDw |= c_dwTbt;
}

void CDuDoc::Format(UCURV page)
{
	memset((PVOID)c_pDucInfo, 0, sizeof(SRDUCINFO));
	c_pDucInfo->srDuc.nEsc = 0x1b;
	c_pDucInfo->srDuc.nFlow = page < DUPAGE_SETENVNTIME ? 'W' : 'R';
	switch (page) {
	case DUPAGE_OPERATE :		Operate();			break;
	case DUPAGE_MONIT :			DevMonit();			break;
	case DUPAGE_STATUSA :		StatusA();			break;
	case DUPAGE_STATUSB :		StatusB();			break;
	case DUPAGE_COMMMONIT :		CommMonit();		break;
	case DUPAGE_DIRBYDAY :		DirectoryByDay();	break;
	case DUPAGE_OPENNORMAL :	OpenNormalPage();	break;
	case DUPAGE_OPENOVERHAUL :	OpenOverhaulPage();	break;
	case DUPAGE_OPENINSPECT :	OpenInspectPage();	break;
	case DUPAGE_OPENTIMESET :	OpenTimeSetPage();	break;
	case DUPAGE_INSPECT :		Inspect();			break;
	case DUPAGE_ENV :
	case DUPAGE_ENVED :			Enviro();			break;
	case DUPAGE_TRACE :			TraceView();		break;
	case DUPAGE_ARMLIST :		AlarmList();		break;
	case DUPAGE_RESCUE :		OpenRescuePage();	break;
	case DUPAGE_HEADPILE :		HeadPile();			break;
	case DUPAGE_TAILPILE :		TailPile();			break;
	case DUPAGE_SETENVNTIME :
	case DUPAGE_SETTIMEONLY :
	case DUPAGE_SETTRAINNO :	GetEnviro();		break;
	case DUPAGE_SETWHEEL :		GetWheel();			break;
	case DUPAGE_INSPECTITEM :	GetInspectItem();	break;
	case DUPAGE_TRACEITEM :		GetTraceItem();		break;
	default :	break;
	}
	// Modified 2013/11/02
	if (c_pTcRef->real.scArm.dwScrollIndex != 0 && c_pTcRef->real.scArm.wScrollTimer == 0)
		c_pTcRef->real.scArm.dwScrollIndex = 0;
}

void CDuDoc::AlarmScroll()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	DWORD length = pTcx->GetAlarmLength();
	if (c_pTcRef->real.scArm.dwScrollIndex < length) {
		++ c_pTcRef->real.scArm.dwScrollIndex;
		pTcx->InterTrigger(&c_pTcRef->real.scArm.wScrollTimer, TIME_SCROLL);
	}
	else {
		c_pTcRef->real.scArm.dwScrollIndex = 0;
		pTcx->InterTrigger(&c_pTcRef->real.scArm.wScrollTimer, 0);
	}
}

PVOID CDuDoc::GetSendInfo()
{
	return ((PVOID)&c_pDucInfo->srDuc.nEsc);
}

WORD CDuDoc::GetSendLength(UCURV page)
{
	WORD length = 0;
	switch (page) {
	case DUPAGE_OPERATE :
		length = sizeof(DUOPERATEINFO) + 6;
		break;
	case DUPAGE_MONIT :
		length = sizeof(DUMONITINFO) + 6;
		break;
	case DUPAGE_STATUSA :
		length = sizeof(DUSTATUSINFOA) + 6;
		break;
	case DUPAGE_STATUSB :
		length = sizeof(DUSTATUSINFOB) + 6;
		break;
	case DUPAGE_COMMMONIT :
		length = sizeof(DUCOMMMONIT) + 6;
		break;
	case DUPAGE_DIRBYDAY :
		length = sizeof(DUSELITEM) + 6;
		break;
	case DUPAGE_INSPECT :
		length = sizeof(DUINSPECTINFO) + 6;
		break;
	case DUPAGE_ENV :
	case DUPAGE_ENVED :
		length = sizeof(DUENVINFO) + 6;
		break;
	case DUPAGE_TRACE :
		length = sizeof(DUTRACEINFO) + 6;
		break;
	case DUPAGE_ARMLIST :
		length = sizeof(DUALARMLIST) + 6;
		break;
	case DUPAGE_HEADPILE :
		length = SIZE_HEADPILE;
		break;
	case DUPAGE_TAILPILE :
		length = SIZE_TAILPILE;
		break;
	case DUPAGE_OPENNORMAL :
	case DUPAGE_OPENOVERHAUL :
	case DUPAGE_OPENINSPECT :
	case DUPAGE_OPENTIMESET :
	case DUPAGE_RESCUE :
		length = sizeof(DUSETPAGEINFO) + 6;
		break;
	case DUPAGE_SETENVNTIME :
	case DUPAGE_SETTIMEONLY :
	case DUPAGE_SETTRAINNO :
	case DUPAGE_SETWHEEL :
	case DUPAGE_INSPECTITEM :
	case DUPAGE_TRACEITEM :
		length = sizeof(DUREADCMDINFO) + 6;
		break;
	default :
		length = 0;
		break;
	}
	return length;
}

WORD CDuDoc::GetRecvLength(UCURV page)
{
	switch (page) {
	case DUPAGE_SETENVNTIME :
	case DUPAGE_SETTIMEONLY :
	case DUPAGE_SETTRAINNO :
		return (LENGTH_ENV + 4);
		break;
	case DUPAGE_SETWHEEL :
		return (LENGTH_WHEEL + 4);
		break;
	case DUPAGE_INSPECTITEM :
		return (LENGTH_INSPECTITEM + 4);
		break;
	case DUPAGE_TRACEITEM :
		return (LENGTH_TRACEITEM + 4);
		break;
	default :
		return DEFAULT_DUCRECV;
		break;
	}
	return DEFAULT_DUCRECV;
}

void CDuDoc::SetTraceLength(UCURV di, UCURV ci, WORD length)
{
	if (di < TRACEID_MAX && ci < MAX_DEV_LENGTH) {
		c_wDevTraceLength[di][ci] = length;
		MSGLOG("[DUC]Set trace Length %d %d = %d\n", di, ci, length);
	}
}

void CDuDoc::SetTraceProgress(UCURV di, UCURV ci, WORD progress)
{
	if (di < TRACEID_MAX && ci < MAX_DEV_LENGTH)
		c_wDevTraceProgress[di][ci] = progress;
}

void CDuDoc::AppendHead()
{
	memcpy((PVOID)&c_pDoz->tcs[CAR_HEAD].real.ducia, (PVOID)&c_nHeadBk[0], SIZE_HEADBK);
	memcpy((PVOID)&c_pDoz->tcs[CAR_HEAD].real.ducib, (PVOID)&c_nNormalBk[0], SIZE_NORMALBK);
}

void CDuDoc::ExitFromDownload()
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PBUCKET pBucket = pTcx->GetBucket();
	if (c_pTcRef->real.nState & STATE_DOWNLOADING) {
		pBucket->wState |= BEILSTATE_CMDEXIT;
		pTcx->GetChaser()->TraceInitial();
		c_pTcRef->real.nState &= ~(STATE_DOWNLOADING | STATE_PROGRESSING);
		c_pTcRef->real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
		Hangup(FILEMSG_BEGIN + FILEERROR_EXITCMD);
	}
	if (c_pDoz->tcs[OWN_SPACEA].real.nIssue & ISSUE_DOWNLOAD) {
		if (c_pTcRef->real.es.nDownloadMethod == OWN_SPACEA) {
			c_pDoz->tcs[OWN_SPACEA].real.nIssue &= ~ISSUE_DOWNLOAD;
			c_pTcRef->real.nState &= ~STATE_PROGRESSING;
			Hangup(FILEMSG_BEGIN + FILEERROR_EXITCMD);
			MSGLOG("[TC]Cancel active download.\r\n");
		}
	}
	if (c_pTcRef->real.nActiveBus & INHIBITBUS_B) {
		c_pDoz->tcs[OWN_SPACEA].real.nIssue |= ISSUE_KILLDOWNLOAD;
		MSGLOG("[TC]Cancel download.\r\n");
	}
}

void CDuDoc::OsMsg(BYTE* p)
{
	BOOL copy = TRUE;
	BYTE c;
	for (UCURV n = 0; n < SIZE_OSMSG; n ++) {
		if (copy) {
			c = *p ++;
			if (c == 0 || c == '\r' || c == '\n')	copy = FALSE;
		}
		c_nOsMsg[n] = copy ? c : 0x20;
	}
	printf("[OSMSG] %s\n", p);
}

BOOL CDuDoc::DevToTrace(UCURV di, WORD* pwTrace)
{
	if (_SHIFTV(di) && !(*pwTrace & _SHIFTV(di)) &&
			(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL)) {
		*pwTrace |= _SHIFTV(di);
		return TRUE;
	}
	return FALSE;
}

BOOL CDuDoc::DirectorySelected(BOOL bCmd)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PBUCKET pBucket = pTcx->GetBucket();

	if (pBucket != NULL) {
		WORD id = (WORD)(c_pTcRef->real.du.nSel - 1);
		if (id < SIZE_FILESBYDAY) {
			DWORD date = pTcx->GetSelectedByDay(id);
			date %= 1000000l;
			pBucket->src.dt.year = (BYTE)(date / 10000);
			date %= 10000;
			pBucket->src.dt.month = (BYTE)(date / 100);
			pBucket->src.dt.day = (BYTE)(date % 100);
			if (pBucket->src.dt.month != 0 && pBucket->src.dt.day != 0) {
				MSGLOG("[DUC]Start downloading for item%d.\r\n", id);
				pTcx->Lock();
				RESET_BEILACKA(pBucket);
				pTcx->BeilTimeTrigger();
				if (bCmd)	pTcx->BeilCmd(BEILCMD_SELTOUSB);
				else {
					pTcx->BeilCmd(BEILCMD_SELTODELETE);
					c_pTcRef->real.scLog.nMainStep = BEILSTEP_SELDELETE;
					pTcx->InterTrigger(&c_pTcRef->real.scLog.wMainTimer, TIME_BEILWAIT);
				}
				pTcx->Unlock();
				return TRUE;
			}
			else	MSGLOG("[DUC]ERROR:select item error(%d-%02d/%02d/%02d).\r\n",
							id, pBucket->src.dt.year, pBucket->src.dt.month, pBucket->src.dt.day);
		}
		else	MSGLOG("[DUC]ERROR:select item over range(%d).\r\n", id);
	}
	return FALSE;
}

// Modified 2013/11/02
//void CDuDoc::SetEntireInfo(BYTE* p)
void CDuDoc::SetEntireInfo(BYTE* p, BOOL bEnv)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PENVARCHEXTENSION pEnv = pTcx->GetEnv();

	p += 2;		// remove 0x1b-0x41
	for (UCURV n = 0; n < 6; n ++) {
		DWORD dw = 0;
		for (UCURV m = 0; m < 4; m ++) {
			dw <<= 8;
			dw |= *p ++;
		}
		if (bEnv) {
			switch (n) {
			case 0 :	// 1200-1201
				pEnv->real.entire.elem.dbDist = (double)dw * 1000.f;
				break;
			case 1 :	// 1202-1203
				pEnv->real.entire.elem.dbTime = (double)dw * 3600.f;
				break;
			case 2 :	// 1204-1205
				pEnv->real.entire.elem.dbV3fPower = (double)dw * 1000.f;
				break;
			case 3 :	// 1206-1207
				pEnv->real.entire.elem.dbRevival = (double)dw * 1000.f;
				break;
			case 4 :	// 1208-1209
				pEnv->real.entire.elem.dbSivPower = (double)dw * 1000.f;
				break;
			default :	// 1210-1211, skip wNull
				break;
			}
		}
	}
	if (bEnv) {
		MSGLOG("[TC]Cause information change.\r\n");
		pTcx->SaveEnv(5);		// by du key(DUKEY_SETENVTIME, DUKEY_SETTIMEONLY),cause setup from duc
	}
	else	MSGLOG("[TC]Cause time change.\r\n");

	WORD year = pTcx->ToHex((BYTE)*p ++);	// 1212
	year *= 100;
	year += pTcx->ToHex((BYTE)*p ++);
	++ p;
	BYTE mon = pTcx->ToHex((BYTE)*p ++);
	++ p;
	BYTE day = pTcx->ToHex((BYTE)*p ++);
	++ p;
	BYTE hour = pTcx->ToHex((BYTE)*p ++);
	++ p;
	BYTE min = pTcx->ToHex((BYTE)*p ++);
	pTcx->TimeSet(year, mon, day, hour, min);
	c_pDoz->tcs[OWN_SPACEA].real.nIssue |= ISSUE_TIMERSET;	// time set command by user
	pTcx->InterTrigger(&c_pTcRef->real.cond.wTimerSetWait, WAIT_TIMERSET);
	SETTBIT(OWN_SPACEA, TCDOB_TSET);		// by du key
	c_pTcRef->real.wDevTimerSet = COUNT_DEVTIMERSET;	// time set command to CC's device
}

void CDuDoc::SetTrainNo(BYTE* p)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PENVARCHEXTENSION pEnv = pTcx->GetEnv();

	p += 2;		// remove 0x1b-0x41
	p += 22;	// byte, 1211
	pEnv->real.entire.elem.wTrainNo = c_pTcRef->real.cond.wTrainNo = (WORD)*((WORD*)p);
	MSGLOG("[TC]Cause train no. change A.\r\n");
	pTcx->SaveEnv(6);		// by du key(DUKEY_SETTRAINNO),cause setup from duc
}

void CDuDoc::SetWheelInfo(BYTE* p)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PENVARCHEXTENSION pEnv = pTcx->GetEnv();

	p += 2;		// remove 0x1b-0x41
	// Modified 2013/11/02
	//for (UCURV n = 0; n < SIZE_WHEELCAR; n ++) {
	//	for (UCURV m = 0; m < SIZE_WHEELAXEL; m ++) {
	//		WORD w = (WORD)pTcx->ToHex((BYTE)*p ++);
	//		w *= 100;
	//		w += (WORD)pTcx->ToHex((BYTE)*p ++);
	//		pEnv->real.w[n][m] = w;
	//	}
	//}
	UCURV motorCar;
	switch (c_pTcRef->real.cf.nLength) {
	case 4 :	motorCar = 2;	break;
	case 6 :	motorCar = 3;	break;
	default :	motorCar = 4;	break;
	}
	UCURV car;
	for (car = 0; car < motorCar; car ++) {
		for (UCURV axel = 0; axel < SIZE_WHEELAXEL; axel ++) {
			WORD w = (WORD)pTcx->ToHex((BYTE)*p ++);
			w *= 100;
			w += (WORD)pTcx->ToHex((BYTE)*p ++);
			pEnv->real.w[car][axel] = w;
		}
	}
	for ( ; car < SIZE_WHEELCAR; car ++) {
		for (UCURV axel = 0; axel < SIZE_WHEELAXEL; axel ++)
			pEnv->real.w[car][axel] = 0;
	}
	// Modified 2013/11/02
	//pTcx->GetScene()->CheckEnv(pEnv, sizeof(ENVARCHEXTENSION), TRUE);
	pTcx->GetScene()->CheckEnv(pEnv, TRUE);
	MSGLOG("[TC]Cause wheel change.\r\n");
	pTcx->SaveEnv(7);		// by du key(DUKEY_SETWHEEL),cause setup from duc
}

void CDuDoc::KeyAction(BYTE* p)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	PBUCKET pBucket = pTcx->GetBucket();

	if (*(p + 1) == 'I' && *(p + 2) == 1 && *(p + 4) == 3) {
		BYTE cmd = *(p + 3);
		//if (cmd >= DUKEY_KEYBEGIN && cmd <= DUKEY_KEYEND) {
		//	c_pTcRef->real.du.nSw = cmd;
		//}
		switch (cmd) {
		case DUKEY_OVERHAUL :
			// Modified 2013/03/05
			if (c_pTcRef->real.nState & STATE_DOWNLOADING) {
				pBucket->wState |= BEILSTATE_CMDEXIT;
				pTcx->GetChaser()->TraceInitial();
				c_pTcRef->real.nState &= ~(STATE_DOWNLOADING | STATE_PROGRESSING);
				c_pTcRef->real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
			}
			c_pTcRef->real.du.nDownloadItem = 0;

			if (IsNotRescue()) {
				c_pTcRef->real.du.nSetPage = DUPAGE_ENV;
				if (c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) {
					c_pTcRef->real.nState &= ~STATE_PROGRESSING;
					if (c_pTcRef->real.du.nSetPage == DUPAGE_INSPECT) {
						c_pTcRef->real.du.nSw = DUKEY_EXIT;
						c_pTcRef->real.du.nSetPage = DUPAGE_OPENOVERHAUL;
					}
					else if (c_pTcRef->real.insp.nMode != INSPECTMODE_NON) {
						c_pTcRef->real.insp.nMode = INSPECTMODE_NON;
						c_pTcRef->real.insp.nId = 0;
						pTcx->GetInsp()->ResetStep();
					}
					memset((PVOID)&c_pTcRef->real.ai, 0, sizeof(ALARMLISTINFO));
					CLocal* pLocal = pTcx->GetLocal();
					pLocal->TraceToNormalABS(LOCALID_ATCA);
					pLocal->TraceToNormalABS(LOCALID_ATCB);
					pLocal->TraceToNormalABS(LOCALID_ATO);
					c_pTcRef->real.wSivTrace = 0;
					c_pTcRef->real.wV3fTrace = 0;
					c_pTcRef->real.wEcuTrace = 0;
				}
				// Modified 2013/11/02
				//else
				if (c_pTcRef->real.nState & STATE_TRIAL) {
					c_pTcRef->real.nState &= ~STATE_TRIAL;
				}
				unlink(TRIAL_PATHT);		// perhaps from trail downloading...
			}
			c_pDoz->tcs[OWN_SPACEA].real.each.wId = TCINFORM_NON;
			Hangup(0);
			break;
		case DUKEY_INSPECT :
			if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) && IsNotRescue()) {
				if (c_pTcRef->real.du.nSetPage == DUPAGE_INSPECT)	c_pTcRef->real.du.nSw = DUKEY_EXIT;
				else if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
						(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
						c_pTcRef->real.insp.nMode == INSPECTMODE_NON &&
						(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL))
					c_pTcRef->real.du.nSetPage = DUPAGE_INSPECTITEM;	// get inspect item
			}
			break;
		case DUKEY_TRIAL :
			if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) && IsNotRescue()) {
				if (!(c_pTcRef->real.nState & STATE_TRIAL)) {
					c_pTcRef->real.nState |= STATE_TRIAL;
					c_pTcRef->real.scLog.dwTag = 0;
					PTRIALINFO pTrial = pTcx->GetTrial();
					memset((PVOID)pTrial, 0, sizeof(TRIALINFO));
					pTrial->sampleClk = ClockCycles();
					pTrial->dbBeginDist = c_pTcRef->real.intg.dbDistLive;
					c_pTcRef->real.du.nDownloadItem = cmd;	// DUKEY_TRIAL
				}
			}
			break;
		case DUKEY_RESETED :
			if (GetResetMotion() != 0) {
				ResetMotion(0);
				SETTBIT(OWN_SPACEA, TCDOB_DUHOLD);
				if (c_pTcRef->real.nState & STATE_NEEDTIMESET)
					c_pTcRef->real.du.nSetPage = DUPAGE_OPENTIMESET;
				else	c_pTcRef->real.du.nSetPage = DUPAGE_OPENNORMAL;
				c_pTcRef->real.nState &= ~STATE_NEEDTIMESET;
			}
			break;
		// Modified 2013/11/02
		//case DUKEY_SETENV :
		//case DUKEY_SETTIME :
		//	if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
		//			(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
		//			(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
		//			IsNotRescue())
		//		c_pTcRef->real.du.nSetPage = DUPAGE_SETENV;	// get enviroment item
		//	break;
		case DUKEY_SETENVNTIME :
			if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
					(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
					(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					IsNotRescue())
				c_pTcRef->real.du.nSetPage = DUPAGE_SETENVNTIME;
			break;
		case DUKEY_SETTIMEONLY :
			if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
					(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
					(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					IsNotRescue())
				c_pTcRef->real.du.nSetPage = DUPAGE_SETTIMEONLY;
			break;
		// Modified 2013/02/05
		case DUKEY_SETTRAINNO :
			if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
					(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
					(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					IsNotRescue())
				c_pTcRef->real.du.nSetPage = DUPAGE_SETTRAINNO;
			break;
		case DUKEY_SETWHEEL :
			if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
					(c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
					(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					IsNotRescue())
				c_pTcRef->real.du.nSetPage = DUPAGE_SETWHEEL;
			break;
		case DUKEY_DELETE :
			if ((c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					c_pTcRef->real.du.nDownloadItem == DUKEY_DIRBYDAY && c_pTcRef->real.es.nTargetDuty == OWN_SPACEA) {
				if (c_pTcRef->real.du.nSel > 0 && c_pTcRef->real.du.nSel <= SIZE_FILESBYDAY) {
					if (c_pTcRef->real.du.wDoubleCounter != 0) {
						if (DirectorySelected(FALSE))	c_pTcRef->real.nProperty |= PROPERTY_LOGMASKATDOWNLOAD;
					}
					else	pTcx->InterTrigger(&c_pTcRef->real.du.wDoubleCounter, COUNT_DUDELETE);
				}
			}
			break;
		case DUKEY_DOWNLOAD :
			//if ((c_pTcRef->real.nState & STATE_BEENUSB) &&
			//		!(c_pTcRef->real.nState & STATE_DOWNLOADING) && IsNotRescue() && pBucket != NULL) {
			if (!(c_pTcRef->real.nState & STATE_DOWNLOADING) && IsNotRescue() && pBucket != NULL) {
				if ((c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
						c_pTcRef->real.cond.wReleaseTimer == TIME_RELEASE) {
					if (c_pTcRef->real.du.nDownloadItem == DUKEY_DIRBYDAY) {
						if (c_pTcRef->real.es.nTargetDuty == OWN_SPACEA) {
							if ((c_pTcRef->real.nState & STATE_BEENUSB) &&
									c_pTcRef->real.du.nSel > 0 && c_pTcRef->real.du.nSel <= SIZE_FILESBYDAY) {
								pTcx->CampUsb();
								if (DirectorySelected(TRUE)) {
									c_pTcRef->real.es.nDownloadMethod = DOWNLOADMETHOD_SELF;
									c_pTcRef->real.nState |= STATE_DOWNLOADING;
								}
							}
						}
						else if (c_pTcRef->real.es.nTargetDuty < LENGTH_TC) {
							// !!!!! very important - reserve bus out !!!!!
							WORD sel = (WORD)(c_pTcRef->real.du.nSel - 1);
							WORD total = pTcx->GetFilesByDayLength();
							if (sel < total) {
								if (c_pTcRef->real.nState & STATE_BEENUSB) {	// self usb
									if (c_pTcRef->real.es.nDownloadStep == DOWNLOADSTEP_NON) {
										c_pTcRef->real.es.nDownloadMethod = DOWNLOADMETHOD_OPPO;
										c_pTcRef->real.nState |= STATE_DOWNLOADING;
										c_pTcRef->real.es.nDownloadStep = DOWNLOADSTEP_READYENTRYBUS;
										c_pDoz->tcs[OWN_SPACEA].real.each.wId = TCINFORM_DIR;
										c_pDoz->tcs[OWN_SPACEA].real.each.info.dir.nDuty = c_pTcRef->real.es.nTargetDuty;	// = OWN_SPACEA < 2 ? 2 : 0;
										c_pTcRef->real.es.dwSelectedDay = pTcx->GetSelectedByDay(sel);
									}
									else	MSGLOG("[DUCK]ERROR:download condition duty=%d sel=%d total=%d confirm=%02X.\r\n",
											c_pTcRef->real.es.nTargetDuty, sel, total, c_pTcRef->real.es.nDownloadStep);
								}
								else if (c_pTcRef->real.es.nTargetDuty < LENGTH_TC &&
										c_pDoz->tcs[c_pTcRef->real.es.nTargetDuty].real.nIssue & ISSUE_BEENUSB) {
									DWORD* pV = pTcx->GetFilesByDay();
									DWORD dw = *(pV + sel);
									//WORD year = dw / 10000;
									//dw %= 10000;
									//BYTE month = (BYTE)(dw / 100);
									//BYTE day = (BYTE)(dw % 100);
									//MSGLOG("[DUCK]remote download duty=%d %d/%02d/%02d.\r\n", c_pTcRef->real.es.nTargetDuty, year, month, day);
									c_pDoz->tcs[OWN_SPACEA].real.nIssue |= ISSUE_DOWNLOAD;
									c_pDoz->tcs[OWN_SPACEA].real.each.wId = TCINFORM_DOWNLOAD;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.nDuty = c_pTcRef->real.es.nTargetDuty;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.nTotal = total;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.wYear = dw / 10000;
									dw %= 10000;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.nMonth = dw / 100;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.nDay = dw % 100;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.nAck = FILEERROR_NON;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.iRespond = FILEERROR_NON;
									c_pDoz->tcs[OWN_SPACEA].real.each.info.dl.wProgress = 0;
									c_pTcRef->real.es.nDownloadMethod = OWN_SPACEA;
									c_pTcRef->real.es.nRespCounter = 0;
									c_pTcRef->real.nState |= STATE_PROGRESSING;
									MSGLOG("[TC]Active download command from %d to %d.\r\n",
											c_pTcRef->real.es.nDownloadMethod, c_pTcRef->real.es.nTargetDuty);
								}
								else	MSGLOG("[DUCK]ERROR:not found usb duty=%d sel=%d total=%d confirm=%02X.\r\n",
										c_pTcRef->real.es.nTargetDuty, sel, total, c_pTcRef->real.es.nDownloadStep);
							}
							else	MSGLOG("[DUCK]ERROR:download condition duty=%d sel=%d total=%d.\r\n",
									c_pTcRef->real.es.nTargetDuty, sel, total);
						}
					}
					else if ((c_pTcRef->real.nState & STATE_BEENUSB) &&
							c_pTcRef->real.du.nDownloadItem == DUKEY_ALARMCOLLECTION) {
						c_pTcRef->real.es.nDownloadMethod = DOWNLOADMETHOD_SELF;
						c_pTcRef->real.nState |= STATE_DOWNLOADING;
						RESET_BEILACKA(pBucket);
						pTcx->CampUsb();
						pTcx->BeilCmd(BEILCMD_LOGBOOKTOUSB);		// not need respond from beil
					}
					else if ((c_pTcRef->real.nState & STATE_BEENUSB) &&
							c_pTcRef->real.du.nDownloadItem == DUKEY_DEVTRACEDL) {
						if ((c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&
								(c_pTcRef->real.nObligation & OBLIGATION_HEAD)) {
							c_pTcRef->real.du.nSetPage = DUPAGE_TRACEITEM;
						}
					}
					else if ((c_pTcRef->real.nState & STATE_BEENUSB) &&
							c_pTcRef->real.du.nDownloadItem == DUKEY_INSPECTDL) {
						CScene* pScene = pTcx->GetScene();
						if (pScene->SaveInspect(pTcx->GetInspectSpec()) >= 0) {
							c_pTcRef->real.es.nDownloadMethod = DOWNLOADMETHOD_SELF;
							c_pTcRef->real.nState |= STATE_DOWNLOADING;
							RESET_BEILACKA(pBucket);
							pTcx->CampUsb();
							pTcx->BeilCmd(BEILCMD_INSPECTTOUSB);
						}
					}
				}
				if ((c_pTcRef->real.nState & STATE_BEENUSB) &&
						c_pTcRef->real.du.nDownloadItem == DUKEY_TRIAL) {
					// Appended 2013/11/02
					c_pTcRef->real.es.nDownloadMethod = DOWNLOADMETHOD_SELF;
					c_pTcRef->real.nState |= STATE_DOWNLOADING;
					RESET_BEILACKA(pBucket);
					pTcx->CampUsb();
					pTcx->BeilCmd(BEILCMD_TRIALTOUSB);
				}
				if (c_pTcRef->real.nState & STATE_DOWNLOADING) {
					c_pTcRef->real.nProperty |= PROPERTY_LOGMASKATDOWNLOAD;
					Hangup(FILEMSG_BEGIN + FILEWORK_WORKING);
				}
			}
			break;
		case DUKEY_DELETEARCHIVES :
			if (pBucket != NULL)	pTcx->BeilCmd(BEILCMD_DELETEARCHIVES);
			break;
		case DUKEY_EXITFROMDOWNLOAD :
			// Modified 2013/03/11
			ExitFromDownload();
			break;
		case DUKEY_V3FRESET :
			// Modified 2013/03/12
			{
				WORD req = c_pTcRef->real.dev.wV3fRstReq;
				WORD cmd = 0;
				// Modified 2013/11/02
				//for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++)
				//	if (req & (1 << n))	cmd |= (1 << n);
				//c_pTcRef->real.dev.wV3fRstCmd = cmd;
				for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
					if (req & (1 << n))	cmd |= (1 << n);
					SHOT(n, 359);
				}
				c_pTcRef->real.dev.wV3fRstCmd = cmd;
			}
			break;
		case DUKEY_COMMMONIT :
			break;

		// Modified 2013/11/02
		case DUKEY_ALARMSCROLL :
			AlarmScroll();
			//{
			//	DWORD length = pTcx->GetAlarmLength();
			//	if (c_pTcRef->real.scArm.dwScrollIndex < length) {
			//		++ c_pTcRef->real.scArm.dwScrollIndex;
			//		pTcx->InterTrigger(&c_pTcRef->real.scArm.wScrollTimer, TIME_SCROLL);
			//	}
			//	else {
			//		c_pTcRef->real.scArm.dwScrollIndex = 0;
			//		pTcx->InterTrigger(&c_pTcRef->real.scArm.wScrollTimer, 0);
			//	}
			//}
			break;

		case DUKEY_ALARMLIST :
			if (c_pTcRef->real.ai.nListID == 0) {
				c_pTcRef->real.ai.nListID = ALARMLIST_PRESENT;
				c_pTcRef->real.du.nTerm &= ~DUTERM_FEED;
				c_pTcRef->real.du.nTiePage = 0;
				c_pTcRef->real.ai.wCurrentPage = 0;
				pTcx->SetSelectedCid(8);	// all
				pTcx->OpenAlarm(c_pTcRef->real.ai.wCurrentPage);
			}
			break;
		case DUKEY_ALARMCOLLECTION :
			// Modified 2013/02/05
			//if (c_pTcRef->real.ai.nListID == 0 && pBucket != NULL &&
			//		(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL)) {
			if (c_pTcRef->real.ai.nListID == 0 && pBucket != NULL) {
				RESET_BEILACKA(pBucket);
				pTcx->BeilTimeTrigger();
				c_pTcRef->real.nProperty |= PROPERTY_LOGMASKATDOWNLOAD;
				pTcx->BeilCmd(BEILCMD_MAKEDIRECTORYLIST);			// need respond from beil
				c_pTcRef->real.scLog.nMainStep = BEILSTEP_MAKEDIRECTORYLIST;
				pTcx->InterTrigger(&c_pTcRef->real.scLog.wMainTimer, TIME_BEILWAIT / 5);
			}
			break;
		case DUKEY_DEVTRACEDL :
			if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
					(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					IsNotRescue())
				c_pTcRef->real.du.nDownloadItem = cmd;	// DUKEY_DEVTRACEDL
			break;
		case DUKEY_INSPECTDL :
			if ((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&
					c_pTcRef->real.insp.nMode == INSPECTMODE_NON &&
					(c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					IsNotRescue()) {
				if (c_pTcRef->real.insp.nRefMode > INSPECTMODE_NON &&
						c_pTcRef->real.insp.nRefMode < INSPECTMODE_VIEW &&
						c_pTcRef->real.insp.wItem != 0) {
					c_pTcRef->real.insp.nMode = INSPECTMODE_VIEW;
					pTcx->GetInsp()->ResetStep();
					WORD page = 0xffff;
					for (c_pTcRef->real.insp.nId = 0; ; ++ c_pTcRef->real.insp.nId) {
						page = pTcx->GetInspectPage();
						if (page != 0xffff)	break;
					}
					if (page != 0 && page != 0xffff) {
						c_pTcRef->real.du.nDownloadItem = cmd;	// DUKEY_INSPECTDL
						c_pTcRef->real.du.nTiePage = 0;
						c_pTcRef->real.du.nTerm |= DUTERM_FEED;
					}
					else	c_pTcRef->real.insp.nMode = INSPECTMODE_NON;
				}
			}
			break;
		case DUKEY_DIRBYDAY :
			if ((c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					!(c_pDoz->tcs[OWN_SPACEA].real.nIssue & ISSUE_DIRECTORYREQUEST)) {
				pTcx->Lock();
				c_pTcRef->real.es.nDirectoryState = DIRECTORYSTATE_SELF;
				c_pTcRef->real.es.nTargetDuty = OWN_SPACEA;
				//c_pTcRef->real.es.nTargetAddr = c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty];
				pTcx->DirectoryList();		// self directory
				pTcx->Unlock();
			}
			break;
		case DUKEY_DIRSELF :
			if ((c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					!(c_pTcRef->real.nState & STATE_DOWNLOADING) &&
					(c_pTcRef->real.es.nDirectoryState != DIRECTORYSTATE_SELF) &&
					!(c_pDoz->tcs[OWN_SPACEA].real.nIssue & ISSUE_DIRECTORYREQUEST)) {
				pTcx->Lock();
				c_pTcRef->real.es.nDirectoryState = DIRECTORYSTATE_SELF;
				c_pTcRef->real.es.nTargetDuty = OWN_SPACEA;
				//c_pTcRef->real.es.nTargetAddr = c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty];
				pTcx->DirectoryList();		// self directory
				pTcx->Unlock();
			}
			break;
		case DUKEY_DIROPPO :
			if ((c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					!(c_pTcRef->real.nState & STATE_DOWNLOADING) &&
					(c_pTcRef->real.es.nDirectoryState != DIRECTORYSTATE_OPPO) &&
					!(c_pDoz->tcs[OWN_SPACEA].real.nIssue & ISSUE_DIRECTORYREQUEST)) {
				pTcx->Lock();
				c_pTcRef->real.es.nDirectoryState = DIRECTORYSTATE_OPPO;
				c_pDoz->tcs[OWN_SPACEA].real.each.wId = TCINFORM_DIR;
				c_pDoz->tcs[OWN_SPACEA].real.each.info.dir.nDuty =
					c_pTcRef->real.es.nTargetDuty = OWN_SPACEA < 2 ? 2 : 0;
				//c_pTcRef->real.es.nTargetAddr = c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty];
				c_pDoz->tcs[OWN_SPACEA].real.nIssue |= ISSUE_DIRECTORYREQUEST;
				c_pTcRef->real.es.nCounter = COUNT_WAITOPPODIR;
				pTcx->OppoDirTimeTrigger();
				pTcx->Unlock();
			}
			break;
		case DUKEY_DIRSUB :
			if ((c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) &&
					!(c_pTcRef->real.nState & STATE_DOWNLOADING) &&
					!(c_pDoz->tcs[OWN_SPACEA].real.nIssue & ISSUE_DIRECTORYREQUEST)) {
				BYTE duty = c_pTcRef->real.es.nTargetDuty;
				duty ^= 1;
				if (duty == OWN_SPACEA) {	// self
					pTcx->Lock();
					c_pTcRef->real.es.nDirectoryState = DIRECTORYSTATE_SELF;
					c_pTcRef->real.es.nTargetDuty = duty;
					//c_pTcRef->real.es.nTargetAddr = c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty];
					pTcx->DirectoryList();		// self directory
					pTcx->Unlock();
				}
				else {
					// OWN_SPACE	0		2(^2=0)
					// duty			1		3(^2=1)	-> DIRECTORYSTATE_SELFSUB
					//				2		0(^2=2)	-> DIRECTORYSTATE_OPPO
					//				3		1(^2=3)	-> DIRECTORYSTATE_OPPOSUB
					BYTE cmd = duty ^ (OWN_SPACEA & 2);	// o or 2
					if (c_pTcRef->real.cid.nTcAddr[duty] != 0 && cmd > DIRECTORYSTATE_SELF && cmd <= DIRECTORYSTATE_OPPOSUB) {
						pTcx->Lock();
						c_pTcRef->real.es.nDirectoryState = cmd;
						c_pDoz->tcs[OWN_SPACEA].real.each.wId = TCINFORM_DIR;
						c_pDoz->tcs[OWN_SPACEA].real.each.info.dir.nDuty =
							c_pTcRef->real.es.nTargetDuty = duty;
						//c_pTcRef->real.es.nTargetAddr = c_pTcRef->real.cid.nTcAddr[c_pTcRef->real.es.nTargetDuty];
						c_pDoz->tcs[OWN_SPACEA].real.nIssue |= ISSUE_DIRECTORYREQUEST;
						c_pTcRef->real.es.nCounter = COUNT_WAITOPPODIR;
						pTcx->OppoDirTimeTrigger();
						pTcx->Unlock();
					}
				}
			}
			break;
		case DUKEY_LEFT :
		case DUKEY_RIGHT :
			break;
		case DUKEY_UP :
			if (!(c_pTcRef->real.ai.nProtect & PROTECT_PAGE)) {
				if (c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT) {
					if (c_pTcRef->real.ai.wCurrentPage > 0) {
						pTcx->LockArm();
						-- c_pTcRef->real.ai.wCurrentPage;
						c_pTcRef->real.du.nTerm |= DUTERM_FEED;
						pTcx->UnlockArm();
					}
				}
				else if (c_pTcRef->real.ai.nListID == ALARMLIST_COLLECTION) {
					if (c_pTcRef->real.ai.wCurrentFile == ALARMFILE_CURRENT) {
						// Modified 2013/03/12
						if (c_pTcRef->real.ai.wCurrentPage > 0) {
							-- c_pTcRef->real.ai.wCurrentPage;
							if (pTcx->GetScene()->LoadCurAlarmBlock(c_pTcRef->real.ai.wCurrentPage, c_pTcRef->real.ai.wMaxPages))
								pTcx->OpenAlarmPageToDu();	// by DUKEY_UP
						}
					}
					else {
						if (c_pTcRef->real.ai.wMaxFiles > 0) {
							if (c_pTcRef->real.ai.wCurrentPage > 0) {
								-- c_pTcRef->real.ai.wCurrentPage;
								if (pTcx->GetScene()->LoadAlarmBlock(c_pTcRef->real.ai.wCurrentPage, c_pTcRef->real.ai.wMaxPages))
									pTcx->OpenAlarmPageToDu();	// by DUKEY_UP
							}
							// Modified 2013/02/26
							//else if (c_pTcRef->real.ai.wCurrentFile < c_pTcRef->real.ai.wMaxFiles) {
							//	pTcx->SetSelectedCid(8);
							//	pTcx->OpenAlarmList(c_pTcRef->real.ai.wCurrentFile + 1);
							//}
							//else	pTcx->MakeCurAlarmList();
							else if (c_pTcRef->real.ai.wCurrentFile > 1) {
								pTcx->SetSelectedCid(8);
								pTcx->OpenAlarmList(c_pTcRef->real.ai.wCurrentFile - 1);
							}
							else	pTcx->MakeCurAlarmList();
						}
					}
				}
				else if (c_pTcRef->real.insp.nMode > INSPECTMODE_NON) {
					if (c_pTcRef->real.insp.nMode < INSPECTMODE_VIEW)
						c_pTcRef->real.du.nSw = cmd;
					else {
						if (c_pTcRef->real.insp.wItem != 0) {
							if (c_pTcRef->real.insp.nId > 0) {
								BYTE idbk = c_pTcRef->real.insp.nId;
								WORD page = 0xffff;
								do {
									-- c_pTcRef->real.insp.nId;
									page = pTcx->GetInspectPage();
								} while (page == 0xffff && c_pTcRef->real.insp.nId > 0);
								if (page == 0xffff || page == 0)
									c_pTcRef->real.insp.nId = idbk;
								else	c_pTcRef->real.du.nTerm |= DUTERM_FEED;
							}
						}
					}
				}
			}
			break;
		case DUKEY_DOWN :
			if (!(c_pTcRef->real.ai.nProtect & PROTECT_PAGE)) {
				if (c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT) {
					if ((c_pTcRef->real.ai.wCurrentPage + 1) < c_pTcRef->real.ai.wMaxPages) {
						pTcx->LockArm();
						++ c_pTcRef->real.ai.wCurrentPage;
						c_pTcRef->real.du.nTerm |= DUTERM_FEED;
						pTcx->UnlockArm();
					}
				}
				else if (c_pTcRef->real.ai.nListID == ALARMLIST_COLLECTION) {
					if (c_pTcRef->real.ai.wCurrentFile == ALARMFILE_CURRENT) {
						if ((c_pTcRef->real.ai.wCurrentPage + 1) < c_pTcRef->real.ai.wMaxPages) {
							++ c_pTcRef->real.ai.wCurrentPage;
							if (pTcx->GetScene()->LoadCurAlarmBlock(c_pTcRef->real.ai.wCurrentPage, c_pTcRef->real.ai.wMaxPages))
								pTcx->OpenAlarmPageToDu();	// by DUKEY_DOWN(present)
						}
						else if (c_pTcRef->real.ai.wMaxFiles > 0) {
							pTcx->SetSelectedCid(8);
							// Modified 2013/02/27
							//pTcx->OpenAlarmList(c_pTcRef->real.ai.wMaxFiles);
							pTcx->OpenAlarmList(1);
						}
					}
					else {
						if (c_pTcRef->real.ai.wMaxFiles > 0) {
							if ((c_pTcRef->real.ai.wCurrentPage + 1) < c_pTcRef->real.ai.wMaxPages) {
								++ c_pTcRef->real.ai.wCurrentPage;
								if (pTcx->GetScene()->LoadAlarmBlock(c_pTcRef->real.ai.wCurrentPage, c_pTcRef->real.ai.wMaxPages))
									pTcx->OpenAlarmPageToDu();	// by DUKEY_DOWN(collection)
							}
							// Modified 2013/02/27
							//else if (c_pTcRef->real.ai.wCurrentFile > 1) {
							//	pTcx->SetSelectedCid(8);
							//	pTcx->OpenAlarmList(c_pTcRef->real.ai.wCurrentFile - 1);
							//}
							else if (c_pTcRef->real.ai.wCurrentFile < c_pTcRef->real.ai.wMaxFiles) {
								pTcx->SetSelectedCid(8);
								pTcx->OpenAlarmList(c_pTcRef->real.ai.wCurrentFile + 1);
							}
						}
					}
				}
				else if (c_pTcRef->real.insp.nMode > INSPECTMODE_NON) {
					if (c_pTcRef->real.insp.nMode < INSPECTMODE_VIEW)
						c_pTcRef->real.du.nSw = cmd;
					else {
						if (c_pTcRef->real.insp.wItem != 0) {
							BYTE idbk = c_pTcRef->real.insp.nId;
							WORD page = 0xffff;
							do {
								++ c_pTcRef->real.insp.nId;
								c_pTcRef->real.insp.nNull = c_pTcRef->real.insp.nId;
								page = pTcx->GetInspectPage();
							} while (page == 0xffff);
							if (page == 0xffff || page == 0)
								c_pTcRef->real.insp.nId = idbk;
							else	c_pTcRef->real.du.nTerm |= DUTERM_FEED;
						}
					}
				}
			}
			break;
		case DUKEY_OK :
		case DUKEY_NG :
		case DUKEY_EXIT :
		case DUKEY_ESC :
			if (cmd == DUKEY_ESC || cmd == DUKEY_EXIT)	Hangup(0);
			// Modified 2012/11/12 ... begin
			if (c_pTcRef->real.insp.nMode != INSPECTMODE_NON) {
				if (c_pTcRef->real.insp.nMode == INSPECTMODE_VIEW) {
					c_pTcRef->real.insp.nMode = INSPECTMODE_NON;
					c_pTcRef->real.insp.nId = c_pTcRef->real.insp.nStep = 0;
					c_pTcRef->real.du.nTiePage = 0;
					c_pTcRef->real.du.nSetPage = DUPAGE_OPENOVERHAUL;
				}
				else {
					// ... end
					c_pTcRef->real.du.nSw = cmd;
					if (cmd == DUKEY_EXIT || cmd == DUKEY_ESC)
						c_pTcRef->real.du.nSetPage =
							c_pTcRef->real.insp.nMode == INSPECTMODE_PDT ? DUPAGE_OPENNORMAL : DUPAGE_OPENOVERHAUL;
				}
			}
			break;
		case DUKEY_FACK :
			// Modified 2012/12/25
			//if (c_pTcRef->real.scArm.nState & ARMSTATE_BUZZON)
			//	pTcx->InterTrigger(&c_pTcRef->real.scArm.wBuzzTimer, 0);
			if (c_pTcRef->real.insp.nMode == INSPECTMODE_NON || c_pTcRef->real.insp.nMode > INSPECTMODE_EACHDEV) {
				c_pTcRef->real.scArm.nState &= ~ARMSTATE_LAMPON;	// by DUKEY_FACK
				// Modified 2013/11/02
				if (c_pTcRef->real.scArm.wSevereCode != 0)
					pTcx->Hide(c_pTcRef->real.scArm.nSevereID, c_pTcRef->real.scArm.wSevereCode);
				else	AlarmScroll();
			}
			break;
		case DUKEY_ABSOVERHAUL :
			if (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF)
				pTcx->InterTrigger(&c_pTcRef->real.cond.wReleaseTimer, TIME_RELEASE);
			break;
		// Appended 2012/03/29 ... begin
		//case DUKEY_PWMOUT :
		//	if ((c_pTcRef->real.op.nMode == OPMODE_EMERGENCY || c_pTcRef->real.op.nMode == OPMODE_MANUAL) && IsNotRescue())
		//		c_pTcRef->real.op.nState ^= OPERATESTATE_WITHPWM;
		//	break;
		// ... end
		case DUKEY_RESCUETRAIN :
		case DUKEY_RESCUECAR :
			// Modified 2012/11/29
			//if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY || c_pTcRef->real.op.nMode == OPMODE_MANUAL) {
			if (c_pTcRef->real.op.nMode == OPMODE_EMERGENCY || c_pTcRef->real.op.nMode == OPMODE_MANUAL ||
					c_pTcRef->real.op.nMode == OPMODE_YARD) {
				if (IsNotRescue()) {
				//if (c_pTcRef->real.op.nRescueMode == RESCUEMODE_NON) {
					if (c_pTcRef->real.speed.nZv & SPEEDDETECT_ZVF) {
						if (cmd == DUKEY_RESCUECAR) {
							if (GETTBIT(CAR_HEAD, TCDIB_ROS)) {
								if (GETTBIT(CAR_TAIL, TCDIB_ROS)) {
									c_pTcRef->real.op.nRescueMode = RESCUEMODE_PASSIVEUNDERGO;
									// Modified 2012/11/29
									pTcx->AbsoluteEmergency();
									SHOT(SCENE_ID_HEADCAR, 64);
									MSGLOG("[DUCK]Rescue passive undergo mode.\r\n");
								}
								else {
									c_pTcRef->real.op.nRescueMode = RESCUEMODE_PASSIVEDRIVE;
									SHOT(SCENE_ID_HEADCAR, 63);
									// Modified 2012/11/29
									pTcx->AbsoluteEmergency();
									MSGLOG("[DUCK]Rescue passive drive mode.\r\n");
								}
							}
							else if (GETTBIT(CAR_TAIL, TCDIB_ROS)) {
								c_pTcRef->real.op.nRescueMode = RESCUEMODE_ACTIVEUNDERGO;
								// Modified 2012/11/29
								pTcx->AbsoluteEmergency();
								SHOT(SCENE_ID_HEADCAR, 64);
								MSGLOG("[DUCK]Rescue active undergo mode.\r\n");
							}
							else {
								c_pTcRef->real.op.nRescueMode = RESCUEMODE_ACTIVEDRIVE;
								// Modified 2012/11/29
								pTcx->AbsoluteEmergency();
								SHOT(SCENE_ID_HEADCAR, 63);
								MSGLOG("[DUCK]Rescue active drive mode.\r\n");
							}
						}
						else {
							c_pTcRef->real.op.nRescueMode = RESCUEMODE_PASSIVETRAIN;
							SETTBIT(OWN_SPACEA, TCDOB_ROLR);
							// Modified 2012/11/29
							pTcx->AbsoluteEmergency();
							SHOT(SCENE_ID_HEADCAR, 62);
							MSGLOG("[DUCK]Rescue passive train mode.\r\n");
						}
						memset((PVOID)&c_pTcRef->real.rescPwr, 0, sizeof(INSTRUCTA));	// !!! very important
						pTcx->GetArtop()->ReleaseAll();
						CUT(SCENE_ID_HEADCAR, 691);
						SHOT(SCENE_ID_HEADCAR, 61);
					}
					else	SHOT(SCENE_ID_HEADCAR, 591);
				}
			}
			break;
		case DUKEY_RESCUEEXIT :
			if (IsRescue()) {
				c_pTcRef->real.op.nRescueMode = RESCUEMODE_NON;
				pTcx->RecoverAllBrake();
				// Modified 2013/03/25
				//CLRTBIT(OWN_SPACEA, TCDOB_ROLR);
				//CLRTBIT(OWN_SPACEA, TCDOB_ROPR);
				//CLRTBIT(OWN_SPACEA, TCDOB_ROBR);
				pTcx->RemoveRescueOutput();
				CUTS(SCENE_ID_HEADCAR, 61, 70);
				CUTS(SCENE_ID_HEADCAR, 591, 593);
				SHOT(SCENE_ID_HEADCAR, 66);
			}
			c_pTcRef->real.du.nSetPage = DUPAGE_OPENNORMAL;
			break;
		case DUKEY_ALARMSORT1 :
		case DUKEY_ALARMSORT2 :
		case DUKEY_ALARMSORT3 :
		case DUKEY_ALARMSORT4 :
		case DUKEY_ALARMSORT5 :
		case DUKEY_ALARMSORT6 :
		case DUKEY_ALARMSORT7 :
		case DUKEY_ALARMSORT0 :
		case DUKEY_ALARMSORTALL :
			{
				BYTE cid = cmd - DUKEY_ALARMSORT1;		// 0:head, 7:tail, 8:all
				if (cid < c_pTcRef->real.cf.nLength) {
					if (++ cid >= c_pTcRef->real.cf.nLength)	cid = 0;		// 1:head, 0:tail, 8:all
				}
				if (pTcx->GetSelectedCid() != cid) {
					pTcx->SetSelectedCid(cid);
					if (c_pTcRef->real.ai.nListID == ALARMLIST_PRESENT) {
						c_pTcRef->real.du.nTerm &= ~DUTERM_FEED;
						c_pTcRef->real.du.nTiePage = 0;
						c_pTcRef->real.ai.wCurrentPage = 0;
						pTcx->OpenAlarm(c_pTcRef->real.ai.wCurrentPage);
					}
				}
			}
			break;
		default :
			if (cmd == DUKEY_GENERAL || cmd == DUKEY_STATUS)	// && c_pTcRef->real.du.nSetPage == DUPAGE_COMMMONIT)
				c_pTcRef->real.du.nSetPage = c_pTcRef->real.du.nCurPage = DUPAGE_OPERATE;
			else if ((cmd >= DUKEY_SELITEM1 && cmd <= DUKEY_SELITEMMAX) &&
					c_pTcRef->real.du.nDownloadItem == DUKEY_DIRBYDAY)
				c_pTcRef->real.du.nSel = cmd - (DUKEY_SELITEM1 - 1);	// from 1
			memset((PVOID)&c_pTcRef->real.ai, 0, sizeof(ALARMLISTINFO));
			break;
		}
	}
	else if (*(p + 1) == 'A' && (c_pTcRef->real.nState & STATE_ENABLEOVERHAUL) && IsNotRescue()) {
		switch (c_pTcRef->real.du.nSetPage) {
		// Modified 2013/11/02
		//case DUPAGE_SETENV :
		//case DUPAGE_SETTIME :
		//	SetEntireInfo(p);
		//	c_pTcRef->real.du.nSetPage = DUPAGE_OPENOVERHAUL;
		//	break;
		case DUPAGE_SETENVNTIME :
			SetEntireInfo(p, TRUE);
			c_pTcRef->real.du.nSetPage = DUPAGE_OPENOVERHAUL;
			break;
		case DUPAGE_SETTIMEONLY :
			SetEntireInfo(p, FALSE);
			c_pTcRef->real.du.nSetPage = DUPAGE_OPENNORMAL;
			break;
		case DUPAGE_SETTRAINNO :
			SetTrainNo(p);
			c_pTcRef->real.du.nSetPage = DUPAGE_OPERATE;	// keep current page
			break;
		case DUPAGE_SETWHEEL :
			SetWheelInfo(p);
			c_pTcRef->real.du.nSetPage = DUPAGE_OPENOVERHAUL;
			break;
		case DUPAGE_INSPECTITEM :
			{
				BYTE nMode = pTcx->GetDoor()->BitIndex(*(p + 7) & 0xf, TRUE);
				if (nMode > INSPECTMODE_NON && nMode < INSPECTMODE_VIEW) {
					SHOT(SCENE_ID_HEADCAR, nMode + 150);
					c_pTcRef->real.insp.nMode = c_pTcRef->real.insp.nRefMode = nMode;
					c_pTcRef->real.insp.wItem = MAKEWORD(*(p + 2), *(p + 3));
					pTcx->GetInsp()->ResetStep();
					Hangup(0);
					c_pTcRef->real.du.nSetPage = DUPAGE_INSPECT;
					c_pTcRef->real.du.nRoll = 0xff;
				}
			}
			break;
		case DUPAGE_TRACEITEM :
			c_pTcRef->real.wTraceDev = MAKEWORD(*(p + 2), *(p + 3));
			if (c_pTcRef->real.wTraceDev != 0 && !(c_pTcRef->real.nState & STATE_DOWNLOADING)) {
				MSGLOG("[DUCK]Trace device = %04X\r\n", c_pTcRef->real.wTraceDev);
				CLocal* pLocal = pTcx->GetLocal();
				if (c_pTcRef->real.wTraceDev & 1) {
					if (c_pTcRef->real.cond.nActiveAtc == (LOCALID_ATCA + 1))	pLocal->ToTrace(LOCALID_ATCA);
					else if (c_pTcRef->real.cond.nActiveAtc == (LOCALID_ATCB + 1))	pLocal->ToTrace(LOCALID_ATCB);
				}
				if (c_pTcRef->real.wTraceDev & 2)	pLocal->ToTrace(LOCALID_ATO);
				if ((c_pTcRef->real.wTraceDev & 4) && _ISOCC(__SIVA))
					DevToTrace(__SIVA, &c_pTcRef->real.wSivTrace);
				if ((c_pTcRef->real.wTraceDev & 8) && _ISOCC(__SIVB))
					DevToTrace(__SIVB, &c_pTcRef->real.wSivTrace);
				if ((c_pTcRef->real.wTraceDev & 0x10) && _ISOCC(__V3FA))
					DevToTrace(__V3FA, &c_pTcRef->real.wV3fTrace);
				if ((c_pTcRef->real.wTraceDev & 0x20) && _ISOCC(__V3FB))
					DevToTrace(__V3FB, &c_pTcRef->real.wV3fTrace);
				if ((c_pTcRef->real.wTraceDev & 0x40) && _ISOCC(__V3FC))
					DevToTrace(__V3FC, &c_pTcRef->real.wV3fTrace);
				if ((c_pTcRef->real.wTraceDev & 0x80) && _ISOCC(__V3FD))
					DevToTrace(__V3FD, &c_pTcRef->real.wV3fTrace);
				// Modified 2013/11/02
				for (UCURV n = 0; n < c_pTcRef->real.cf.nLength; n ++) {
					if (c_pTcRef->real.wTraceDev & (0x100 << (n + HEADCCBYLENGTHA)))
						DevToTrace(n, &c_pTcRef->real.wEcuTrace);
				}
				c_pTcRef->real.du.nSetPage = DUPAGE_TRACE;
				c_pTcRef->real.du.nRoll = 0xff;
				RESET_BEILACKA(pBucket);
				c_pTcRef->real.es.nDownloadMethod = DOWNLOADMETHOD_SELF;
				c_pTcRef->real.nState |= STATE_DOWNLOADING;
				c_pTcRef->real.nProperty |= PROPERTY_LOGMASKATDOWNLOAD;
				Hangup(FILEMSG_BEGIN + FILEWORK_WORKING);
			}
			else	c_pTcRef->real.du.nSetPage = 0;
			break;
		default :
			break;
		}
	}
}
