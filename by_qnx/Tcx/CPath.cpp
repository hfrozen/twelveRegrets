/*
 * CPath.cpp
 *
 *  Created on: 2011. 2. 16
 *      Author: Che
 */

#include "CPath.h"
#include "CTcx.h"

CPath::CPath()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_pLcFirm = NULL;
	c_pDoz = NULL;
	// c_bTakeHcr = FALSE;
}

CPath::~CPath()
{
}

// Modified 2012/12/25
//const WORD CPath::c_wTdir[16][2] = {
//		{ TDIR_UNKNOWN,		133 },	// 0000	: N-N
//		{ TDIR_FIXCONTINUE,	131 },	// 0001	: F-N	: G
//		{ TDIR_FIXCONTINUE,	131 },	// 0010 : R-N	: G
//		{ TDIR_CLASH,		134 },	// 0011	: X-N
//		{ TDIR_TURNOVER,	132 },	// 0100 : N-F	: G
//		{ TDIR_CLASH,		138 },	// 0101 : F-F
//		{ TDIR_CLASH,		137 },	// 0110	: R-F
//		{ TDIR_CLASH,		134 },	// 0111 : X-F
//		{ TDIR_TURNOVER,	132 },	// 1000	: N-R	: G
//		{ TDIR_CLASH,		136 },	// 1001	: F-R
//		{ TDIR_CLASH,		139 },	// 1010	: R-R
//		{ TDIR_CLASH,		134 },	// 1011	: X-R
//		{ TDIR_CLASH,		135 },	// 1100	: N-X
//		{ TDIR_CLASH,		135 },	// 1101	: F-X
//		{ TDIR_CLASH,		135 },	// 1110	: R-X
//		{ TDIR_CLASH,		134 }	// 1111	: X-X
//};
const WORD CPath::c_wTdir[16][2] = {
		{ TDIR_UNKNOWN,		133 },	// 0000	: N-N
		{ TDIR_FIXCONTINUE,	131 },	// 0001	: F-N	: G
		{ TDIR_FIXCONTINUE,	131 },	// 0010 : R-N	: G
		{ TDIR_FIXCONTINUE,	134 },	// 0011	: X-N

		{ TDIR_TURNOVER,	132 },	// 0100 : N-F	: G
		{ TDIR_CLASH,		138 },	// 0101 : F-F
		{ TDIR_CLASH,		137 },	// 0110	: R-F
		{ TDIR_FIXCONTINUE,	134 },	// 0111 : X-F

		{ TDIR_TURNOVER,	132 },	// 1000	: N-R	: G
		{ TDIR_CLASH,		136 },	// 1001	: F-R
		{ TDIR_CLASH,		139 },	// 1010	: R-R
		{ TDIR_FIXCONTINUE,	134 },	// 1011	: X-R

		{ TDIR_CLASH,		135 },	// 1100	: N-X	// ????? need more flag !!!!!
		{ TDIR_CLASH,		135 },	// 1101	: F-X
		{ TDIR_CLASH,		135 },	// 1110	: R-X
		{ TDIR_FIXCONTINUE,	134 }	// 1111	: X-X

};


void CPath::InitialModule(PVOID pVoid)
{
	c_pParent = pVoid;
	CTcx* pTcx = (CTcx*)c_pParent;
	c_pTcRef = pTcx->GetTcRef();
	c_pLcFirm = pTcx->GetLcFirm();
	c_pDoz = pTcx->GetTcDoz();
}

void CPath::AtcPowerCtrl(BOOL cmd)
{
	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->Lock();
	if (cmd) {
		c_pTcRef->real.op.nState |= OPERATESTATE_ATCON;
		SETTBIT(OWN_SPACEA, TCDOB_ATCON);
		pTcx->InterTrigger(&c_pTcRef->real.hcr.wMainWarningInhibitTimer, TIME_MAINWARNINGINHIBIT);
		c_pTcRef->real.hcr.nMasterID &= HCR_ONLYID;		// ???
		//pTcx->HcrInfo(c_pTcRef->real.hcr.nMasterID, c_pDoz->tcs[OWN_SPACEA].real.nHcrInfo, 1);	// ?????
	}
	else {
		c_pTcRef->real.op.nState &= ~OPERATESTATE_ATCON;
		CLRTBIT(OWN_SPACEA, TCDOB_ATCON);
		c_pTcRef->real.cond.nActiveAtc = 0;			// !!!!! inportant
		pTcx->InterTrigger(&c_pTcRef->real.hcr.wMainWarningInhibitTimer, 0);
		memset((PVOID)&c_pLcFirm->rAtc[0], 0, sizeof(ATCRINFO));
		memset((PVOID)&c_pLcFirm->aAtc[0], 0, sizeof(ATCAINFO));
		memset((PVOID)&c_pLcFirm->rAtc[1], 0, sizeof(ATCRINFO));
		memset((PVOID)&c_pLcFirm->aAtc[1], 0, sizeof(ATCAINFO));
		memset((PVOID)&c_pLcFirm->rAto, 0, sizeof(ATORINFO));
		memset((PVOID)&c_pLcFirm->aAto, 0, sizeof(ATOAINFO));
	}
	pTcx->Unlock();
}

void CPath::CopyTcFrame(BYTE nSrcID, BYTE nDestID)
{
	if (nSrcID == nDestID)	return;
	memcpy((PVOID)&c_pDoz->tcs[nSrcID].real.wAddr, (PVOID)&c_pDoz->tcs[nDestID].real.wAddr, sizeof(TCFIRM));
}

UCURV CPath::Control()
{
	UCURV res = TDIRRES_NON;
	BYTE tdir = TDIR_UNKNOWN;
	WORD scene = 0;
	// first, scan atc respond
	CTcx* pTcx = (CTcx*)c_pParent;
	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD) {
		if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY ||
				c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
				c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) {
			if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT)	tdir = TDIR_TURNOVERD;
			else	tdir = TDIR_FIXCONTINUE;
		}
		else {
			// excute by head car, cause this is bus manager
			BYTE index;
			if (pTcx->GetBusCondition(CAR_TAIL)) {
				index = pTcx->GetTrainDir(TRUE);		// Get head F/R
				BYTE tail = pTcx->GetTrainDir(FALSE);	// Get tail F/R
				index |= (tail << 2);
				tdir = c_wTdir[index][0];
				scene = c_wTdir[index][1];
			}
			else {
				tdir = pTcx->GetTrainDir(TRUE) > 0 ? TDIR_NOTOPPOFR : TDIR_NOTOPPON;
				//tdir = TDIR_NOTOPPO;
				scene = 140;
			}
		}

		if (c_pTcRef->real.dir.nDir != tdir) {
			MSGLOG("[PATH]Start debounce for dir.(%d to %d)\r\n", c_pTcRef->real.dir.nDir, tdir);		// ?????
			c_pTcRef->real.dir.nDir = tdir;
			c_pTcRef->real.dir.nDeb = DEBOUNCE_TDIR;
			c_pTcRef->real.dir.nDirVerify = 0xfe;
			c_pTcRef->real.hcr.nState |= HCRSTATE_DEBOUNCING;
		}
		else if (c_pTcRef->real.dir.nDeb != 0 && -- c_pTcRef->real.dir.nDeb == 0) {
			c_pTcRef->real.hcr.nState &= ~HCRSTATE_DEBOUNCING;
			MSGLOG("[PATH]Change dir to %d[%d].\r\n", c_pTcRef->real.dir.nDir, c_pTcRef->real.nObligation);	// ?????
			if (c_pTcRef->real.nObligation & OBLIGATION_MAIN) {	// only head-main
				// only master...
				switch (c_pTcRef->real.dir.nDir) {
				case TDIR_FIXCONTINUE :
					c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr;
					break;
				case TDIR_TURNOVER :	// exchange bus master
					c_pTcRef->real.hcr.nMasterID =
						(BYTE)(GETLWORD(c_pDoz->tcs[CAR_TAIL].real.wAddr) & 0xff);
					break;
				case TDIR_TURNOVERD :	// exchange bus master in driverless
					c_pTcRef->real.hcr.nMasterID =
						(BYTE)(GETLWORD(c_pDoz->tcs[CAR_TAIL].real.wAddr) & 0xff);
					c_pTcRef->real.cycle.wState |= TCSTATE_XCHDRVLESS;
					break;
				case TDIR_CLASH :
					// Modified 2012/12/25
					//c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr | HCR_NOACTIVE;
					c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr;
					break;
				case TDIR_NOTOPPOFR :
					// Modified 2012/12/25
					//c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr | HCR_NOACTIVE;
					c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr;
					break;
				case TDIR_NOTOPPON :
					// Modified 2013/01/28
					c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr | HCR_NOACTIVE | HCR_DIFFERENT;
					break;
				default :
					c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.nAddr | HCR_NOACTIVE;
					break;
				}
				//pTcx->HcrInfo(c_pTcRef->real.hcr.nMasterID, c_pDoz->tcs[OWN_SPACEA].real.nHcrInfo, 2);	// ?????
			}
			if (scene == 131 || scene == 132)	CUTS(SCENE_ID_HEADCAR, 134, 140);
			if (scene > 0) {
				SHOT(SCENE_ID_HEADCAR, scene);
				SHOT(SCENE_ID_HEADCAR, scene);
			}
			c_pTcRef->real.dir.nDirVerify = c_pTcRef->real.dir.nDir;
		}
		else	c_pTcRef->real.dir.nDirVerify = 0xfd;
	}
	else	c_pTcRef->real.dir.nDirVerify = 0xff;

	if (c_pTcRef->real.hcr.nState & HCRSTATE_DEBOUNCING)	return TDIRRES_DEBOUNCING;

	if (c_pDoz->tcs[CAR_HEAD].real.nHcrInfo == 0)	SHOT(SCENE_ID_HEADCAR, 17);
	//if (c_pTcRef->real.hcr.nBuf != c_pDoz->tcs[CAR_HEAD].real.nHcrInfo) {
	if (c_pTcRef->real.hcr.nBuf != c_pDoz->tcs[CAR_HEAD].real.nHcrInfo &&
			c_pDoz->tcs[CAR_HEAD].real.nHcrInfo != 0) {		// !!!상당히 중요한 줄인데. 이것이 없으면 버그가 생기더라.. 겨우 생각났다.
		MSGLOG("[PATH]Different master id, buf is %02x bus is %02x, dir is %d.\r\n",
				c_pTcRef->real.hcr.nBuf, c_pDoz->tcs[CAR_HEAD].real.nHcrInfo, c_pTcRef->real.dir.nDir);
		c_pTcRef->real.hcr.nBuf = c_pDoz->tcs[CAR_HEAD].real.nHcrInfo;
		c_pTcRef->real.hcr.nDeb = DEBOUNCE_EXCHANGEHCRSMALL;
		res = TDIRRES_EXCHANGE;
	}
	else if (c_pTcRef->real.hcr.nDeb != 0 && -- c_pTcRef->real.hcr.nDeb == 0) {
		c_pTcRef->real.hcr.nMasterID = c_pTcRef->real.hcr.nBuf;
		//pTcx->HcrInfo(c_pTcRef->real.hcr.nMasterID, c_pDoz->tcs[OWN_SPACEA].real.nHcrInfo, 3);	// ?????
		if (c_pDoz->tcs[CAR_HEAD].real.nHint & HINT_DELAYATC)
			c_pTcRef->real.op.nState |= OPERATESTATE_TRIGGERATCON;
		else	c_pTcRef->real.op.nState &= ~OPERATESTATE_TRIGGERATCON;
		MSGLOG("[PATH]Bus recover by 0\n");
		res = Setup(c_pTcRef->real.hcr.nMasterID);		// by hcr changing
		SHOT(SCENE_ID_HEADCAR, 11);
		if (c_pTcRef->real.op.nKdnState & KDNSTATE_SETTLE)
			c_pTcRef->real.op.nKdnState &= ~KDNSTATE_SETTLE;
	}
	return res;
}

UCURV CPath::Setup(BYTE nMaster)
{
	BOOL bNoActiveHcr = (nMaster & HCR_NOACTIVE) ? TRUE : FALSE;
	nMaster &= HCR_ONLYID;
	if (nMaster > LENGTH_TC)	return	TDIRRES_UNKNOWNID;

	CTcx* pTcx = (CTcx*)c_pParent;
	pTcx->LocalSync();
	DWORD dwIndex = c_pTcRef->real.scArm.dwIndex;		// for SAVE ALARM

	MSGLOG("[PATHSETUP]Master id is %X.\r\n", nMaster);

	if (nMaster == 0) {
		// from ?????
		// not found head car, not change dir only remove active part
		if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) {
			c_pTcRef->real.nObligation &= ~OBLIGATION_ACTIVE;
			MSGLOG("[PATH]Lost active obligation.\r\n");	// neet log, time
		}
		//c_pTcRef->real.scArm.nTimeSet = 0;
		CLRTBIT(OWN_SPACEA, TCDOB_HCR);
		CLRTBIT(OWN_SPACEA, TCDOB_TCR);
		AtcPowerCtrl(FALSE);
		pTcx->InterTrigger(&c_pTcRef->real.hcr.wAidWarningInhibitTimer, 0);
		if (c_pTcRef->real.nLocalDataType[LOCALID_ATO] == DATATYPE_NORMAL) {
			c_pTcRef->real.atoCmd.nCmd &= ~(INSTCMD_CABIN | INSTCMD_ACTIVE);
			c_pTcRef->real.atoCmd.nCmd |= INSTCMD_INHIBIT;
		}
		SHOT(SCENE_ID_HEADCAR, 681);
		ClearInform();
		c_pTcRef->real.cycle.wCtrlDisable = TIME_CTRLDISABLE;
		c_pTcRef->real.cycle.nLetterCycle = CYCLE_LETTER_NORMAL;
		SetInform();
		MSGLOG("[PATH]Any has active head.\r\n");
		return TDIRRES_EXCHANGE0;
	}

	UCURV result = TDIRRES_NON;
	if ((c_pTcRef->real.nAddr < 3 && nMaster < 3) ||
			(c_pTcRef->real.nAddr > 2 && nMaster > 2)) {	// HEAD
		pTcx->Lock();
		BYTE duty = c_pTcRef->real.nAddr == nMaster ? 0 : 1;
		if (duty != c_pTcRef->real.nDuty)
			CopyTcFrame(duty, c_pTcRef->real.nDuty);	// what this line do...?????
		c_pTcRef->real.nDuty = duty;
		// Modified 2013/02/18
		pTcx->InterTrigger(&c_pTcRef->real.wTerminalBackupTimer,
					(c_pTcRef->real.nObligation & OBLIGATION_HEAD) ? 0 : TIME_TERMINALBACKUP);

		c_pTcRef->real.nObligation = OBLIGATION_HEAD;
		if (c_pTcRef->real.nDuty == 0) {
			c_pTcRef->real.nObligation |= OBLIGATION_MAIN;
			// Modified 2012/10/23
			//c_pTcRef->real.cycle.wXch = CYCLE_XCHMASTER;
			c_pTcRef->real.cycle.wXch = c_pTcRef->real.nAddr < 3 ? CYCLE_XCHMASTER1 : CYCLE_XCHMASTER0;
			c_pTcRef->real.cycle.wState |= (TCSTATE_FRAMEEND | TCSTATE_XCHMASTER);	// by hcr changing
			//c_pTcRef->real.scArm.nTimeSet = 0;
		}
		//else	c_pTcRef->real.scArm.nTimeSet = TIMESET_BEGIN;
		pTcx->Unlock();
		CLRTBIT(OWN_SPACEA, TCDOB_HCR);
		CLRTBIT(OWN_SPACEA, TCDOB_TCR);
		AtcPowerCtrl(FALSE);
		pTcx->InterTrigger(&c_pTcRef->real.hcr.wAidWarningInhibitTimer, TIME_AIDWARNINGINHIBIT);
		if (c_pTcRef->real.nLocalDataType[LOCALID_ATO] == DATATYPE_NORMAL) {
			c_pTcRef->real.atoCmd.nCmd &= ~INSTCMD_INHIBIT;
			c_pTcRef->real.atoCmd.nCmd |= INSTCMD_CABIN;
			SHOT(SCENE_ID_HEADCAR, 677);
		}
		SHOT(SCENE_ID_HEADCAR, 680);

		BOOL bDrvless = (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY ||
					c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
					c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) ? TRUE : FALSE;

		if (bDrvless) {
			pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcMode, TIME_WAITATCMODE);
			pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcValid, TIME_WAITATCVALID);
		}

		if (!bNoActiveHcr) {
			// !!!!!WARNING TIME_WAITATCON should not be less than bus exchange time !!!!!
			c_pTcRef->real.nObligation |= OBLIGATION_ACTIVE;
			if (bDrvless) {
				c_pTcRef->real.op.nMode = OPMODE_DRIVERLESSREADY;
				if (c_pTcRef->real.nLocalDataType[LOCALID_ATO] == DATATYPE_NORMAL)
					c_pTcRef->real.atoCmd.nCmd |= INSTCMD_ACTIVE;
				pTcx->EntryDriverless(TRUE);
				SHOT(SCENE_ID_HEADCAR, 679);
			}
			else {
				SETTBIT(OWN_SPACEA, TCDOB_HCR);
				c_pTcRef->real.cycle.wCtrlDisable = TIME_CTRLDISABLE;
				SHOT(SCENE_ID_HEADCAR, 141);
			}

			if (c_pTcRef->real.op.nState & OPERATESTATE_TRIGGERATCON) {
				pTcx->Lock();
				c_pTcRef->real.op.nState &= ~OPERATESTATE_TRIGGERATCON;
				c_pTcRef->real.op.nState |= OPERATESTATE_READYATCON;
				pTcx->InterTrigger(&c_pTcRef->real.op.wWaitAtcOn, TIME_WAITATCON);
				pTcx->Unlock();
			}
			else	AtcPowerCtrl(TRUE);

			// Modified 2013/11/02
			//if (!(c_pTcRef->real.nProperty & PROPERTY_USEEXTERNALMEM))
			SETTBIT(OWN_SPACEA, TCDOB_BCID);	// Absolute on
			pTcx->ClearDistance();
		}
		else if (c_pTcRef->real.dir.nDir == TDIR_NOTOPPON)	pTcx->HalfwayNeutral(TRUE);

		ClearInform();
		result = TDIRRES_EXCHANGETOHEAD;
		if (c_pTcRef->real.nDuty == 0) {
			pTcx->ResetDuc();
			if (!bNoActiveHcr) {
				pTcx->LoadEnv();		// by master
				result = TDIRRES_EXCHANGETOHEADWITHLOADENV;
			}
			else {
				MSGLOG("[TC]Cause non hcr head.\r\n");
				pTcx->SaveEnv(1);	// by master with non-hcr
			}
			PENVARCHEXTENSION pEnv = pTcx->GetEnv();
			c_pTcRef->real.cond.wTrainNo = pEnv->real.entire.elem.wTrainNo;
			MSGLOG("[PATH]Capture master main %d.\r\n", c_pTcRef->real.nObligation);
		}
		else	MSGLOG("[PATH]Capture master reserve %d.\r\n", c_pTcRef->real.nObligation);

		// c_bTakeHcr = TRUE;
		c_pTcRef->real.cycle.nLetterCycle = CYCLE_LETTER_HCR;
		pTcx->AuxCompReset();
		c_pTcRef->real.du.nSetPage = DUPAGE_HEADPILE;
	}
	else {	// TAIL
		BYTE duty = c_pTcRef->real.nDuty | 2;
		if (duty != c_pTcRef->real.nDuty)	CopyTcFrame(duty, c_pTcRef->real.nDuty);
		c_pTcRef->real.nDuty = duty;
		c_pTcRef->real.cycle.wCtrlDisable = TIME_CTRLDISABLE;
		c_pTcRef->real.nObligation &= ~(OBLIGATION_HEAD | OBLIGATION_ACTIVE);
		CLRTBIT(OWN_SPACEA, TCDOB_HCR);
		CLRTBIT(OWN_SPACEA, TCDOB_TCR);

		AtcPowerCtrl(FALSE);
		pTcx->InterTrigger(&c_pTcRef->real.hcr.wAidWarningInhibitTimer, 0);
		SHOT(SCENE_ID_HEADCAR, 681);

		if (!bNoActiveHcr) {
			c_pTcRef->real.nObligation |= OBLIGATION_ACTIVE;
			if (c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSREADY ||
					c_pTcRef->real.op.nMode == OPMODE_DRIVERLESS ||
					c_pTcRef->real.op.nMode == OPMODE_DRIVERLESSEXIT) {
				c_pTcRef->real.op.nMode = OPMODE_DRIVERLESSREADY;
				c_pTcRef->real.op.nCabm = 0;
				// !!!!!Very important. maintenance ATC communication at tail!!!!!
				c_pTcRef->real.op.nAutoStep = DRIVERLESSSTEP_INIT;
			}
			// 2012/06/13 ... skip, TCR allways follow HCR at CTcx receive routine
			//else	SETTBIT(OWN_SPACEA, TCDOB_TCR);
		}

		if (c_pTcRef->real.nObligation & OBLIGATION_MAIN)	pTcx->ResetDuc();
		// if OBLIGATION_MAIN flag be removes at LIU1, LIU2 !!!!!
		//c_pTcRef->real.scArm.nTimeSet = TIMESET_BEGIN;
		ClearInform();
		MSGLOG("[PATH]Capture slave ");
		if (c_pTcRef->real.nDuty == 2)	MSGLOG(" main %d.\r\n", c_pTcRef->real.nObligation);
		else	MSGLOG(" reserve %d.\r\n", c_pTcRef->real.nObligation);
		c_pTcRef->real.cycle.nLetterCycle = CYCLE_LETTER_NORMAL;
		c_pTcRef->real.du.nSetPage = DUPAGE_TAILPILE;
		result = TDIRRES_EXCHANGETOTAIL;
	}

	MSGLOG("[PATH]Duty is %d.\r\n", c_pTcRef->real.nDuty);
	// Modified 2012/09/27
	//pTcx->ShutDoor();

	//pTcx->PanelActive();
	//UCURV res = pTcx->BusSetup();
	//pTcx->ClearDumb();
	// Modified 2012/12/05
	//pTcx->ShutBuzz();
	// Modified 2012/12/10
	pTcx->Primary();
	UCURV res = pTcx->BusSetup();

	SetInform();
	//if (nMaster > 0)	SHOT(SCENE_ID_HEADCAR, 141);

	c_pTcRef->real.cycle.wState &= ~TCSTATE_XCHDRVLESS;
	c_pDoz->tcs[OWN_SPACEA].real.nHint &= ~HINT_DELAYATC;

	if (/*c_bTakeHcr && */!(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE)) {
		MSGLOG("[TC]Cause non active head.\r\n");
		pTcx->SaveEnv(2);		// by master with non-hcr
		pTcx->SaveArchives(dwIndex);
		// c_bTakeHcr = FALSE;
		// Modified 2012/05/24 ... begin
		//if (!(c_pTcRef->real.nProperty & PROPERTY_USEEXTERNALMEM))
		//	c_pTcRef->real.scArm.nState |= ARMSTATE_NEGATEBCID;
		// ... end
		// Modified 2013/11/02
		c_pTcRef->real.nCurtains = (CURTAIN_NEGATEBCID | CURTAIN_WRITEENV | CURTAIN_WRITELOG);
	}
	// Modified 2012/11/14
	pTcx->PrimaryPanto();

	if (res == BUSFAIL_NON)	return result;
	return res + TDIRRES_BUSERROR;
}

void CPath::ClearInform()
{
	CLRTBIT(OWN_SPACEA, TCDOB_ACT);
	CLRTBIT(OWN_SPACEA, TCDOB_HEAD);
	CLRTBIT(OWN_SPACEA, TCDOB_DUHOLD);
}

void CPath::SetInform()
{
	if (c_pTcRef->real.nObligation & OBLIGATION_HEAD)
		SETTBIT(OWN_SPACEA, TCDOB_HEAD);
	if (c_pTcRef->real.nObligation & OBLIGATION_ACTIVE)
		SETTBIT(OWN_SPACEA, TCDOB_ACT);
}
