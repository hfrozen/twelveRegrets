/*
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include <sys/neutrino.h>
#include <inttypes.h>
#include <math.h>

#include <sys/iofunc.h>
#include <sys/dispatch.h>

#include "CTcx.h"

//#if (sizeof(TCREFERENCEREAL) >= 0sizeof(TCREFERENCE))
//#error	("Error : size of TCREFERENCEREAL.\r");
//#endif

//#if (sizeof(TCFIRM) != sizeof(TCMATE))
//#error	("Error : TCMATE size different to TCFIRM.\n");
//#endif

#define	BUSFAULTLAMPON()\
	do {\
		c_panel.SetLamp(0x40);\
		c_tcRef.real.cycle.wPeriodOverLampOnTimer = 1;\
	} while (0)

#define	BUSFAULTLAMPOFF()\
	do {\
		c_panel.ClrLamp(0x40);\
	} while (0)

const WORD CTcx::c_wTrainCompose[3][12] = {
		{	0xff,	FSBTERM_FAULTV3FN,		FSBTERM_FAULTECUN,		FSBTERM_FAULTBCCN	},
		{	0x3f,	FSBTERM_FAULTV3FN - 1,	FSBTERM_FAULTECUN - 1,	FSBTERM_FAULTBCCN - 1	},
		{	0x0f,	FSBTERM_FAULTV3FN - 1,	FSBTERM_FAULTECUN - 1,	FSBTERM_FAULTBCCN - 1	}
};

const double CTcx::c_dbAspBase[] = {
		2.06, 1.96, 2.01, 1.69,
		1.69, 1.96, 2.01, 2.06,
		0.0,  0.0,  0.0,  0.0,
		0.0,  0.0,  0.0,  0.0
};

const BYTE CTcx::c_nAtcSample[2][ATCSPEED_UNITMAX] = {
		//Modified 2011/10/20 cause sampling time is 100ms
		{  1,  2,  4,  5, 10, 20 },
		//{ 20, 10,  5,  4,  2,  1 }
		{ 10, 5,  4,  2,  1,  1 }
};

const BYTE CTcx::c_nDuPages[4][5] = {
		{ DUPAGE_OPERATE, DUPAGE_MONIT, DUPAGE_STATUSA, DUPAGE_STATUSB, DUPAGE_COMMMONIT },
		{ DUPAGE_OPERATE, DUPAGE_OPENINSPECT, DUPAGE_INSPECT, DUPAGE_OPENINSPECT, DUPAGE_INSPECT },
		{ DUPAGE_OPERATE, DUPAGE_TRACE, DUPAGE_TRACE, DUPAGE_TRACE, DUPAGE_TRACE },
		{ 0, 0, 0, 0, 0 }
};

const BYTE CTcx::c_nJointOut[8] = {
/*00*/	(1 << (TCDOB_BCID - 0)),
/*08*/	0,
/*16*/	0,
/*24*/	(1 << (TCDOB_HCR - 24)) | (1 << (TCDOB_TCR - 24)) | (1 << (TCDOB_ZVR - 24)),
/*32*/	0,
/*40*/	(1 << (TCDOB_ATCON - 40)),
		0,
		0
};

const WORD CTcx::c_wLamps[SIZE_LAMP] = {
		(1 << (TCDOB_EMML - 0)) | (1 << (TCDOB_YAML - 0)) | (1 << (TCDOB_MAML - 0)) | (1 << (TCDOB_AUML - 0)) |
		(1 << (TCDOB_DRML - 0)) | (1 << (TCDOB_FACKLP - 0)) | (1 << (TCDOB_AUML1 - 0)) |
		(1 << (TCDOB_PAAK - 0)) | (1 << (TCDOB_DCL - 0)) | (1 << (TCDOB_DAOACL - 0)) | (1 << (TCDOB_DAOMCL - 0)) |
		(1 << (TCDOB_DMOMCL - 0)),

		(1 << (TCDOB_DPLP - 16)) |
		(1 << (TCDOB_EPANDNLP - 16)) | (1 << (TCDOB_ESLP - 16)) | (1 << (TCDOB_EBLP - 16)) | (1 << (TCDOB_PBLP - 16)) |
		(1 << (TCDOB_SBLP - 16)) | (1 << (TCDOB_CPRLP - 16)) | (1 << (TCDOB_EBCOSLP - 16)),
};

void CTcx::Interval(PVOID pVoid)
{
	CTcx* pTcx = (CTcx*)pVoid;
	// Modified 2012/10/23
	//if (pTcx->c_tcRef.real.nDuty == 0) {
	if (pTcx->c_tcRef.real.cycle.wState & TCSTATE_XCHMASTER) {
		if (-- pTcx->c_tcRef.real.cycle.wXch == 0)
			pTcx->c_tcRef.real.cycle.wState &= ~TCSTATE_XCHMASTER;
		//else {
		//	if ((pTcx->c_tcRef.real.cycle.wXch % 10) == 0)
		//		pTcx->ReportBus();
		//}
	}
	if (pTcx->c_tcRef.real.nDuty == 0) {
		if (pTcx->c_tcRef.real.cycle.wCur < 100)
			++ pTcx->c_tcRef.real.cycle.wCur;
		if (pTcx->c_tcRef.real.cycle.wCur >= pTcx->c_tcRef.real.cycle.wClose &&
				(pTcx->c_tcRef.real.cycle.wState & TCSTATE_FRAMEEND) &&
				!(pTcx->c_tcRef.real.cycle.wState & TCSTATE_XCHMASTER)) {
			pTcx->c_tcRef.real.cycle.wCur = 0;
			if (pTcx->c_bLocalSync) {
				pTcx->c_nLocalTimer = 0;
				pTcx->c_bLocalSync = FALSE;
			}
			pTcx->c_tcRef.real.cycle.wState &= ~TCSTATE_FRAMEEND;
			if (pTcx->PeriodicSnap() != 0)
				pTcx->c_tcRef.real.cycle.wState |= TCSTATE_ERROR;
		}
	}
	pTcx->SysTimer();
}

CTcx::CTcx()
{
	c_pTcRef = &c_tcRef;
	c_pLcFirm = &c_lcFirm;
	c_pDoz = &c_dozMain;

	memset((PVOID)&c_tcRef, 0, sizeof(TCREFERENCE));
	memset((PVOID)&c_dumb, 0, sizeof(DUMBDEV));
	memset((PVOID)&c_lcFirm, 0, sizeof(LCFIRM));
	memset((PVOID)&c_dozMain, 0, sizeof(TCDOZ));
	memset((PVOID)&c_dozAux, 0, sizeof(TCDOZ));
	memset((PVOID)&c_ducInfo, 0, sizeof(DUCINFO));

	c_mail = -1;
	c_bMfb = FALSE;
	memset((PVOID)&c_tbTime, 0, sizeof(TBTIME));
	memset((PVOID)&c_devTime, 0, sizeof(DEVTIME));
	memset((PVOID)&c_env, 0, sizeof(ENVARCHEXTENSION));
	c_pTimer = NULL;
	c_pLocal = NULL;
	c_busA.wState = 0;
	c_busA.pLine = NULL;
	c_busB.wState = 0;
	c_busB.pLine = NULL;
	c_hShm = -1;
	c_pBucket = NULL;
	c_iWatch = -1;
	c_dwRecvOrderID = 0;
	c_bLocalSync = TRUE;
	c_nLocalTimer = 0;
	c_nHoldSec = 0xff;
	c_vV3fTbe = 0;
	c_nUsbScanCounter = 0;
	c_nSelectedCid = 1;
	c_nDownloadPulse = 0;

	memset((PVOID)&c_pTimeTables[0], 0, sizeof(WORD*) * TIMETABLES_MAX);
	memset((PVOID)&c_ain, 0, sizeof(AINPARCH) * SIZE_AIN);
	memset((PVOID)&c_din, 0, sizeof(DINPARCH) * SIZE_DIN);
	memset((PVOID)&c_df, 0, sizeof(DISTANCEFAIR));
	memset((PVOID)&c_wFlashT, 0, sizeof(WORD) * FLASH_MAX);
	memset((PVOID)&c_wFlashO, 0, sizeof(WORD) * CCDOB_MAX);
	memset((PVOID)&c_nAccpes, 0, sizeof(BYTE) * SIZE_ATCSPEEDMAX);
	memset((PVOID)&c_nEachDevError, 0, SIZE_EACHDEV);

	c_tcRef.real.bm.wBuf = c_tcRef.real.bm.wState = 0xfffe;
	c_tcRef.real.cycle.wClose = CYCLECLOSE_TBUS;
	c_tcRef.real.cycle.wTime = CYCLETIME_TBUS;
	c_tcRef.real.cycle.wCtrlDisable = TIME_CTRLDISABLE;

	c_tcRef.real.speed.nAtcCur = 0;

	for (UCURV n = 0; n < SIZE_WHEELCAR; n ++) {
		for (UCURV m = 0; m < SIZE_WHEELAXEL; m ++) {
			c_env.real.w[n][m] = DEFAULT_WHEELDIA;
		}
	}
	c_syncCur = c_syncMax = 0;
	c_syncMin = 1000;
	c_busTime = -1;
	c_dwUsbSize = 0;
	c_dwUsbFree = 0;
}

CTcx::~CTcx()
{
	Destroy();
}

//===== Initialize ====================================================================
void CTcx::SysTimer()
{
	RECTIME(TTIME_TIMERBEGIN);
	Lock();
	c_panel.Interval(); // very important

	if (!(c_tcRef.real.nProperty & PROPERTY_DISABLESWWATCHDOG)) {
		if (c_iWatch > -1)	MsgSendPulse(c_iWatch, getprio(0), _PULSE_CODE_MINAVAIL, 0);
	}

	for (UCURV n = 0; c_pTimeTables[n] != NULL; n ++) {
		if (n >= TIMETABLES_MAX)	break;
		WORD* pT = c_pTimeTables[n];
		if (*pT != 0)	-- *pT;
	}
	if ((c_tcRef.real.intg.nDistPulse & DISTPULSE_PUT)
			&& c_tcRef.real.intg.wDistTimer == 0) {
		c_tcRef.real.intg.nDistPulse &= ~DISTPULSE_PUT;
		CLRTBIT(OWN_SPACE, TCDOB_DCALS);
	}

	// scan that all control of train to relax...
	if ((c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF) &&
			c_tcRef.real.cond.wReleaseTimer > 0 &&
			c_tcRef.real.cond.wReleaseTimer < TIME_RELEASE)
		++ c_tcRef.real.cond.wReleaseTimer;

	// scan deadman control at manual
	if ((c_tcRef.real.nObligation & OBLIGATION_ACTIVE) && (c_tcRef.real.nObligation & OBLIGATION_HEAD) &&
			// Modified 2013/05/08
			(c_tcRef.real.op.nMode == OPMODE_EMERGENCY || c_tcRef.real.op.nMode == OPMODE_MANUAL || c_tcRef.real.op.nMode == OPMODE_YARD) &&
			// Modified 2012/03/06 ... begin
			!IsRescueTrain() && !IsRescuePassiveDrive() && !IsRescuePassiveUndergo()) {
			// ... end
		if (!GETTBIT(OWN_SPACE, TCDIB_DADS)) {
			if (c_tcRef.real.mcDrv.bValid && (c_tcRef.real.mcDrv.nIndex > MCDSV_NPOS ||
					(c_tcRef.real.mcDrv.nIndex == MCDSV_NPOS && !(c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF)))) {
				++ c_tcRef.real.dead.wTimer;
				if (c_tcRef.real.dead.wTimer >= TIME_DEADMANFAULT) {
					c_tcRef.real.dead.nActive = DEADMAN_FSB;
					SHOTB(SCENE_ID_HEADCAR, 129);
					CUTB(SCENE_ID_HEADCAR, 130);
				}
				else if (c_tcRef.real.dead.wTimer >= TIME_DEADMANWARNING) {
					c_tcRef.real.dead.nActive = DEADMAN_POWERMASK;
					SHOTB(SCENE_ID_HEADCAR, 130);
				}
			}
			// Modified 2012/10/09
			else if (c_tcRef.real.mcDrv.bValid &&
					c_tcRef.real.mcDrv.nIndex < MCDSV_NPOS && c_tcRef.real.dead.nActive == DEADMAN_POWERMASK) {
				c_tcRef.real.dead.nActive = 0;
				CUTB(SCENE_ID_HEADCAR, 130);
				c_tcRef.real.dead.wTimer = 0;
			}
		}
		else {
			if (c_tcRef.real.dead.nActive != DEADMAN_FSB) {
				c_tcRef.real.dead.nActive = 0;
				CUTB(SCENE_ID_HEADCAR, 130);
			}
			c_tcRef.real.dead.wTimer = 0;
		}
	}

	// link to local device
	if (++ c_nLocalTimer >= 100)
		c_nLocalTimer = 0;
	if ((c_tcRef.real.cycle.wState & TCSTATE_LOCALING)) {
			// && !(c_tcRef.real.nState & STATE_BEENUSB)) {
		if ((c_nLocalTimer % 5) == 2 && (c_tcRef.real.op.nState & OPERATESTATE_ATCON)) {
				// FBTBIT(OWN_SPACE, TCDOB_ATCON)) {
			if ((c_tcRef.real.cycle.wState & TCSTATE_LOCALPACK) || c_tcRef.real.speed.nLocalCycle == 0)
				c_pack.PackLocal();
			// normally Package() excute PackLocal() at PeriodicSnap(). so nLocalCycle has 2
			// however Package() be stoped. nLocalCycle is reduced to 0.
			// nLocalCycle is 0, so excute PackLocal()
			c_pLocal->Send50ms();
			if (c_tcRef.real.speed.nLocalCycle > 0)
				-- c_tcRef.real.speed.nLocalCycle;
		}
		if ((c_nLocalTimer % 10) == 1) {
			if (c_tcRef.real.cycle.wState & TCSTATE_LOCALPACK)
				c_pack.PackLocal();
			c_pLocal->Send100ms();
		}
		if ((c_nLocalTimer % 20) == 3) {
			if (c_tcRef.real.cycle.wState & TCSTATE_LOCALPACK)
				c_pack.PackLocal();
			c_pLocal->Send200ms();
		}
	}

	// check bus fault lamp
	if (c_tcRef.real.cycle.wPeriodOverLampOnTimer > 0 &&
			-- c_tcRef.real.cycle.wPeriodOverLampOnTimer == 0) {
		BUSFAULTLAMPOFF();
	}

	// check liu master condition
	if (OWN_SPACE > 0 && (c_tcRef.real.nProperty & PROPERTY_ENABLEBUS)) {
		if (++ c_tcRef.real.cycle.wPeriodOverTime >= CYCLEOVER_TBUSPERIOD) {
			c_tcRef.real.cycle.wPeriodOverTime = 0;
			//MSGLOG("[CAUTION]EnableIntr by timeout.\r\n");		// ?????
			c_busA.pLine->EnableIntr(TRUE);
			c_busB.pLine->EnableIntr(TRUE);
			BUSFAULTLAMPON();
			switch (OWN_SPACE) {
			case 0 :				// at master, ignore
				break;
			case 1 :				// at master, backup
				if (++ c_tcRef.real.cycle.wMasterRecoveryCounter >= COUNT_MASTERRECOVERY) {
					if (c_panel.GetPartner())
						c_tcRef.real.cycle.wMasterRecoveryCounter = COUNT_MASTERRECOVERY;
					else {
						if (!(c_tcRef.real.nProperty & PROPERTY_DISABLEBACKUP)) {
							// Change to Master main
							c_tcRef.real.cycle.wMasterRecoveryCounter = 0;
							c_tcRef.real.hcr.nMasterID = c_tcRef.real.hcr.nBuf = c_tcRef.real.nAddr;
							HcrInfo(c_tcRef.real.hcr.nMasterID, c_dozMain.tcs[OWN_SPACE].real.nHcrInfo, 4);
							c_tcRef.real.cycle.wXch = 0x7fff;
							MSGLOG("[TC]Bus recover by backup\n");
							// Not found master, so recovery bus master
							c_path.Setup(c_tcRef.real.hcr.nMasterID);	// by backup at head
							SHOTB(SCENE_ID_HEADCAR, 12);
							SETTBIT(CAR_HEAD, TCDOB_RESVL);
							c_panel.ClrLamp(4);		// capture du's link at bus master
							c_tcRef.real.cycle.wXch = 0;
							c_tcRef.real.cycle.wState &= ~TCSTATE_XCHMASTER;
							c_tcRef.real.cycle.wState |= TCSTATE_FRAMEEND;
							//c_tcRef.real.cycle.wState |= TCSTATE_LOCALING;
						}
					}
				}
				break;
			default :
				if (!(c_din[0].v & ((1 << TCDIB_F) | (1 << TCDIB_R)))) {
					HalfwayNeutral(TRUE);
					OutputD();
				}
				INCBYTE(c_tcRef.real.revolt.nCondition);
				if (c_tcRef.real.revolt.nCondition >= CYCLE_REVOLTCONDITION) {
					c_tcRef.real.revolt.nCondition = 0;
					if (c_din[0].v & ((1 << TCDIB_F) | (1 << TCDIB_R))) {
						MSGLOG("[REVOLT]master change! at %d.\r\n", OWN_SPACE);
						if (c_tcRef.real.nObligation & OBLIGATION_MAIN)
							c_path.Setup(c_tcRef.real.nAddr | HCR_NOACTIVE);
						else	c_path.Setup((c_tcRef.real.nAddr - 1) | HCR_NOACTIVE);
					}
				}
				break;
			}
			//c_spy.ReportBus(&c_tcRef, c_pLcFirm, &c_dozMain, &c_ducInfo.srDuc);
		}
	}

	// output lamp flashing
	WORD w;
	for (w = 0; w < FLASH_MAX; w ++) {
		if (c_wFlashT[w] > 0 && -- c_wFlashT[w] == 0) {
			c_wFlashT[w] = TIME_FLASH;
			if (FBTBIT(OWN_SPACE, w))	CLRTBIT(OWN_SPACE, w);
			else	SETTBIT(OWN_SPACE, w);
			//TOGTBIT(OWN_SPACE, CAR_HEAD, w);
		}
	}
	for (w = 0; w < CCDOB_MAX; w ++) {
		if (c_wFlashO[w] > 0 && -- c_wFlashO[w] == 0) {
			c_wFlashO[w] = TIME_FLASH;
			for (BYTE n = 0; n < c_tcRef.real.cf.nLength; n ++)
				TOGOBIT(OWN_SPACE, n, w);
		}
	}

	if (c_bMfb) {
		// analog input
		for (UCURV n = 0; n < SIZE_AIN; n ++) {
			if (!c_probe.Geta(n, &w)) {
				c_ain[n].dwSum += (DWORD) w;
				if (++ c_ain[n].wLeng >= 10) {
					c_ain[n].v = (WORD) (c_ain[n].dwSum / 10);
					c_ain[n].dwSum = 0;
					c_ain[n].wLeng = 0;
				}
			}
		}
		// dizital input
		for (UCURV n = 0; n < SIZE_DIN; n ++) {
			if (!c_probe.Getd(n, &w)) {
				if (c_din[n].nDeb != 0 && -- c_din[n].nDeb == 0)
					c_din[n].v = c_din[n].wCur;
				else if (c_din[n].wCur != w) {
					c_din[n].wCur = w;
					c_din[n].nDeb = DEBOUNCE_INP;
				}
			}
		}
	}

	// set & sync. each devices
	if ((c_tcRef.real.nObligation & OBLIGATION_ACTIVE) &&
			c_tcRef.real.cycle.wCtrlDisable > 0) {
		-- c_tcRef.real.cycle.wCtrlDisable;
		if (c_tcRef.real.cycle.wCtrlDisable == 0 &&
				(c_tcRef.real.nObligation & OBLIGATION_HEAD) &&
				(c_tcRef.real.nObligation & OBLIGATION_MAIN)) {		// only main
			c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_TIMERSET;	// time set command by power on
			InterTrigger(&c_tcRef.real.cond.wTimerSetWait, WAIT_TIMERSET);
			SETTBIT(OWN_SPACE, TCDOB_TSET);			// by bootup
			c_tcRef.real.wDevTimerSet = COUNT_DEVTIMERSET;	// time set command to CC's device
		}
	}
	if (c_tbTime.sec != c_nHoldSec) {
		c_nHoldSec = c_tbTime.sec;
		//if ((c_tcRef.real.nObligation & OBLIGATION_TOP) == OBLIGATION_TOP) {	// only head-liu1 upgrade env. file
		if ((c_tcRef.real.nObligation & OBLIGATION_ACTIVE) &&
				(c_tcRef.real.nObligation & OBLIGATION_HEAD) &&
				(c_tcRef.real.nObligation & OBLIGATION_MAIN)) {	// only head-liu1 upgrade env. file
			if (c_tcRef.real.nState & STATE_VALIDENVF) {
				c_env.real.entire.elem.dbTime += 1.0f;
				c_tcRef.real.nState |= STATE_SAVEENVF;
			//if (++ c_env.real.entire.elem.wTime >= 3600) {
			//	c_env.real.entire.elem.wTime = 0;
			//	++ c_env.real.entire.elem.dwTime;
			}
		}
	}

	if (c_pBucket != NULL)	ScanBucketStep();

	if (c_tcRef.real.es.nDownloadStep == (DOWNLOADSTEP_DOWNLOADING | DOWNLOADSTEP_WAITRESPOND) &&
			c_tcRef.real.es.wSyncTimer == 0) {	// wait respond
		if (!SendPert(PERTMSG_AGAIN)) {		// to slave
			Hangup(FILEMSG_BEGIN + FILEERROR_FAILOPPOSITE);
			c_tcRef.real.nState &= ~(STATE_PROGRESSING | STATE_DOWNLOADING);
		}
	}
	if ((c_tcRef.real.nActiveBus & INHIBITBUS_B) && c_tcRef.real.es.wBusBMonitor == 0) {
		c_tcRef.real.nActiveBus &= ~INHIBITBUS_B;
		c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_KILLDOWNLOAD;
		c_tcRef.real.es.nDownloadStep = DOWNLOADSTEP_NON;
		c_tcRef.real.nState &= ~(STATE_PROGRESSING | STATE_DOWNLOADING);
		//c_tcRef.real.es.nMasterAddr = 0;
		c_tcRef.real.es.nMasterDuty = 0xff;
		MSGLOG("[TC]Exit download bus by bus empty.\r\n");
	}

	if (-- c_tcRef.real.cycle.wReportTime == 0) {
		c_tcRef.real.cycle.wReportTime = TIME_REPORT;
		ReportBus();
	}

	Unlock();
	RECTIME(TTIME_TIMEREND);
}

BOOL CTcx::TakingOff()
{
	pthread_mutex_init(&c_mutex, NULL);
	pthread_mutex_init(&c_mutexSig, NULL);
	pthread_mutex_init(&c_mutexArm, NULL);

	c_tcRef.real.cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	MSGLOG("[INIT]%f cycles/sec.\r\n", c_tcRef.real.cps);

	if (!(c_tcRef.real.nProperty & PROPERTY_DISABLESWWATCHDOG)) {
		//PROCSTEP(10);
		c_iWatch = name_open("Stall_func", 0);
		if (c_iWatch < 0) {
			c_tcRef.real.nProperty |= PROPERTY_DISABLESWWATCHDOG;
			MSGLOG("[INIT]ERROR:Can not open stall.\r\n");
		}
		else	MSGLOG("[INIT]stall OK.\r\n");
	}

	if (!Seed())	return FALSE;

	int pid;
	PROCSTEP(1);
	MSGLOG("[INIT]Find mq\n");
	if ((pid = c_panel.FindExec((PSZ) "nand/bin/mq")) < 0) {
		if ((pid = c_panel.FindExec((PSZ) "proc/boot/mq")) < 0) {
			PROCALARM(1);
			MSGLOG("[INIT]ERROR:Can not found mq process.\r\n");
			return FALSE;
		}
	}
	if ((c_mail = CREATEMQ(NAME_TCMQ)) < 0) {
		PROCALARM(1);
		MSGLOG("[INIT]ERROR:Can not create message queue(%s).\r\n", strerror(errno));
		return FALSE;
	}

	c_pack.InitialModule(this);
	c_pack.FormatDevTime(&c_devTime);
	MSGLOG("[INIT]Launch at %04x/%02x/%02x %02x:%02x:%02x.\r\n",
			c_tbTime.year, c_tbTime.month, c_tbTime.day, c_tbTime.hour, c_tbTime.minute, c_tbTime.sec);

	PROCSTEP(2);
	MSGLOG("[INIT]Capture panel\n");
	int res;
	if ((res = c_panel.Capture(&c_dwThumbWheel)) != FROMPANELFAULT_NON) {
		MSGLOG("[INIT]ERROR:Can not capture from panel(%d)!\r\n", res);
		PROCALARM(2);
		return FALSE;
	}
	WORD n;
	switch (c_tcRef.real.cf.nLength) {
	case 4 :	n = 2;	break;
	case 6 :	n = 1;	break;
	default :	n = 0;	break;
	}

	BYTE si, vi, ai, ci, pi, ei;
	si = vi = ai = ci = pi = ei = 0;
	memset((PVOID)&c_tcRef.real.cid.oc, 0xff, sizeof(DEVESID));

	for (UCURV m = 0; m < MAX_TRAIN_LENGTH; m ++) {
		WORD deves = GetCarDevices(m);
		if (deves & DEVBEEN_SIV) {
			if (si < LENGTH_SIV)	c_tcRef.real.cid.oc.nSivI[si] = m;
			++ si;
		}
		if (deves & DEVBEEN_V3F) {
			if (vi < LENGTH_V3F)	c_tcRef.real.cid.oc.nV3fI[vi] = m;
			++ vi;
		}
		if (deves & DEVBEEN_AUX) {
			if (ai < LENGTH_AUX)	c_tcRef.real.cid.oc.nAuxI[ai] = m;
			++ ai;
		}
		if (deves & DEVBEEN_CMSB) {
			if (ci < LENGTH_CMSB)	c_tcRef.real.cid.oc.nCmsbI[ci] = m;
			++ ci;
		}
		if (deves & DEVBEEN_PWM) {
			if (pi < LENGTH_PWM)	c_tcRef.real.cid.oc.nPwmI = m;
			++ pi;
		}
		if (deves & DEVBEEN_ESK) {
			if (ei < LENGTH_ESK)	c_tcRef.real.cid.oc.nEskI = m;
			++ ei;
		}
	}
	c_tcRef.real.cid.wAll = c_wTrainCompose[n][0];
	c_tcRef.real.cid.fsbTerm.nV3f = (BYTE)c_wTrainCompose[n][1];
	c_tcRef.real.cid.fsbTerm.nEcu = (BYTE)c_wTrainCompose[n][2];
	c_tcRef.real.cid.fsbTerm.nBcc = (BYTE)c_wTrainCompose[n][3];
	MSGLOG("[INIT]Siv has %d, V3f has %d, Aux has %d, Cmsb has %d, Pwm has %d, Esk has %d\r\n", si, vi, ai, ci, pi, ei);

	PROCSTEP(3);
	MSGLOG("[INIT]Init mbf\n");
	if ((res = init_mfb()) != 0) {
		MSGLOG("[INIT]ERROR:Can not initial mfb driver!(%s)\r\n", res);
		PROCALARM(3);
		return FALSE;
	}
	c_bMfb = TRUE;

	PROCSTEP(4);
	MSGLOG("[INIT]Start probe open.\n");
	if (c_probe.Open() != 0) {
		MSGLOG("[INIT]ERROR:Can not initial io driver!\r\n");
		PROCALARM(4);
		return FALSE;
	}
	WORD w = 0;
	for (UCURV n = 0; n < SIZE_DOUTEX; n++)
		c_probe.Putd(n, &w);

	if ((res = c_probe.Getd(3, &w)) != 0) {
		MSGLOG("[INIT]ERROR:Can not read liu index.\r\n");
		PROCALARM(4);
		return FALSE;
	}
	MSGLOG("[INIT]Capture liu index %02X.\r\n", w);

	if ((w & 0xa0) == 0xa0) {
		MSGLOG("[INIT]ERROR:Operate by opposite LIU.\r\n");
		PROCALARM(12);
		return FALSE;
	}
	if ((res = c_panel.Junction(w)) != 0) {
		MSGLOG("[INIT]ERROR:Can not capture direction(%d).\r\n", res);
		PROCALARM(4);
		return FALSE;
	}
	MSGLOG("[INIT]Duty is %d, Obligation is %d.\r\n", OWN_SPACE,
			c_tcRef.real.nObligation);
	if (OWN_SPACE == 0 || OWN_SPACE == 2)
		c_panel.ClrLamp(4);
	else	c_panel.SetLamp(4);

	PROCSTEP(5);
	KILL(c_pLocal);
	c_pLocal = new CLocal(this, c_mail);
	c_pLocal->InitialModule();
	//c_pLocal->SetPrioL(10);
	if ((res = c_pLocal->InitDev()) != LOCALFAULT_NON) {
		MSGLOG("[INIT]ERROR:Can not initial local devices(%d).\r\n", res);
		KILL(c_pLocal);
		PROCALARM(5);
		return FALSE;
	}
	for (UCURV n = 0; n < LOCALID_MAX; n ++)
		c_tcRef.real.nLocalDataType[n] = DATATYPE_NORMAL;
	//if (OWN_SPACE == 0 && !GETTBIT(OWN_SPACE, TCDIB_RESVL))

	PROCSTEP(6);
	MSGLOG("[INIT]Bus setup.\r\n");
	if (BusSetup() != BUSFAIL_NON) {
		PROCALARM(6);
		return FALSE;
	}
	c_tcRef.real.nProperty |= PROPERTY_ENABLEBUS;

	PROCSTEP(7);
	c_scene.InitialModule(this);
	c_scene.ResetAlarmArch();
	c_scene.ResetDrvArch();
	c_spy.SetOwner(this);
	if (OpenPost()) {
		MSGLOG("[INIT]Open post.\r\n");
		c_spy.Nameplate();
	}
	else	MSGLOG("[INIT]ERROR:Can not open post.\r\n");

	PROCSTEP(8);
	if (OpenBeil()) {
		MSGLOG("[INIT]Open Beil.\r\n");
		//MSGLOG("[INIT] :Open Beil addr is %08X.\r\n", c_pBucket);
	}
	else {
		MSGLOG("[INIT]ERROR:Can not open beil.\r\n");
		PROCALARM(8);
		return FALSE;
	}

	PROCSTEP(9);
	if (c_pBucket != NULL && (c_pBucket->wState & BEILSTATE_BEENMEM)) {
		c_tcRef.real.nProperty |= PROPERTY_USEEXTERNALMEM;
		MSGLOG("[INIT]Logbook to external memory.\n");
	}
	else {
		if (c_tcRef.real.nProperty & PROPERTY_ATLABORATORY) {
			c_tcRef.real.nProperty &= ~PROPERTY_USEEXTERNALMEM;
			MSGLOG("[INIT]Logbook to hard disk.\r\n");
		}
		else {
			MSGLOG("[INIT]ERROR:Can not found external memory.\r\n");
			PROCALARM(9);
			return FALSE;
		}
	}
	TimeRegister(&c_tcRef.real.hcr.wMainWarningInhibitTimer);
	TimeRegister(&c_tcRef.real.hcr.wAidWarningInhibitTimer);
	TimeRegister(&c_tcRef.real.op.wWaitAtcValid);
	TimeRegister(&c_tcRef.real.op.wWaitAtcMode);
	TimeRegister(&c_tcRef.real.op.wWaitSignalAutoA);
	TimeRegister(&c_tcRef.real.op.wWaitSignalAutoB);
	TimeRegister(&c_tcRef.real.op.wWaitAtcOn);
	TimeRegister(&c_tcRef.real.op.wLightMonTimer);
	TimeRegister(&c_tcRef.real.op.wCmsbTimer[0]);
	TimeRegister(&c_tcRef.real.op.wCmsbTimer[1]);
	//TimeRegister(&c_tcRef.real.op.wRescueTimer);
	TimeRegister(&c_tcRef.real.power.wTimer);
	TimeRegister(&c_tcRef.real.door.wTimerL);
	TimeRegister(&c_tcRef.real.door.wTimerR);
	//TimeRegister(&c_tcRef.real.door.wOpenRescanTimer);
	TimeRegister(&c_tcRef.real.door.wInhibitAlarmTimer);
	TimeRegister(&c_tcRef.real.ca.wPaapbTimer);
	TimeRegister(&c_tcRef.real.intg.wDistTimer);
	TimeRegister(&c_tcRef.real.scLog.wMainTimer);
	TimeRegister(&c_tcRef.real.scLog.wPeriodTimer);
	TimeRegister(&c_tcRef.real.cond.wTimerSetWait);
	TimeRegister(&c_tcRef.real.cond.wEnvSetWait);
	TimeRegister(&c_tcRef.real.es.wSyncTimer);
	TimeRegister(&c_tcRef.real.es.wBusBMonitor);
	TimeRegister(&c_tcRef.real.du.wDoubleCounter);
	TimeRegister(&c_tcRef.real.wTerminalBackupTimer);
	c_tcRef.real.cond.wReleaseTimer = TIME_RELEASE;

	c_path.InitialModule(this);
	c_look.InitialModule(this);
	//c_pack.InitialModule(this);
	c_artop.InitialModule(this);
	c_panto.InitialModule(this);
	c_ext.InitialModule(this);
	c_door.InitialModule(this);
	c_insp.InitialModule(this);
	c_chaser.InitialModule(this);
	c_doc.InitialModule(this);
	c_mate.InitialModule(this);

	PROCSTEP(8);
	c_panel.SetTimer(TRUE);
	if (GETTBIT(OWN_SPACE, TCDIB_RESVL)) {
		Destroy();
		return FALSE;
	}

	return TRUE;
}

BOOL CTcx::Seed()
{
	if (!c_panel.Initial(&c_tcRef)) {
		MSGLOG("[INIT]ERROR:Can not initial gpio.\r\n");
		return FALSE;
	}
	c_panel.SetLamp(4);	// first, disable du's sending

	if (!Trigger()) {
		MSGLOG("[INIT]ERROR:Timer failure!\r\n");
		return FALSE;
	}
	return TRUE;
}

BOOL CTcx::Trigger()
{
	if (c_pTimer != NULL) {
		c_pTimer->Release();
		delete c_pTimer;
	}
	c_pTimer = new CTimer(this);
	c_pTimer->SetOwner(this);
	c_pTimer->SetOwnerName((PSZ) "CTcx");
	c_pTimer->Initial(PRIORITY_TIMER, Interval, 0, TRUE);
	int res = c_pTimer->Trigger(TIME_INTERVAL);
	return (res == 0 ? TRUE : FALSE);
}

UCURV CTcx::BusSetup()
{
	BYTE buf[20];
	memset((PVOID) buf, 0, 20);

	if (c_busA.pLine != NULL || c_busB.pLine != NULL)
	strcpy((char*)buf, "[BUS]");
	if (buf[0] != NULL) {
		MSGLOG("%sbus reconfiguration - ", buf);
	}

	KILL(c_busA.pLine);
	c_busA.wState = 0;
	KILL(c_busB.pLine);
	c_busB.wState = 0;

	if (buf[0] != NULL)	MSGLOG("main - ");
	BYTE res = BUSFAIL_NON;
	c_busA.pLine = new CBusTrx(this, c_mail, (PSZ) "CBusMain");
	c_busA.pLine->SetOwnerName((PSZ) "CBusMain");
	c_busA.pLine->SetCycleTime(c_tcRef.real.cycle.wTime);
	//c_busA.pLine->SetPrio(19);
	//c_busA.pLine->SetCps(c_tcRef.real.cps);
	if ((res = c_busA.pLine->Open(MF_MANCHA, OWN_SPACE,
			c_tcRef.real.nAddr, c_tcRef.real.cf.nLength, TRUE)) != PORTERROR_NON) {
		KILL(c_busA.pLine);
		c_busA.wState = 0;
		MSGLOG("[INIT]ERROR:Can not initial main bus(%d).\r\n", res);
		res |= BUSFAIL_MAIN;
	}
	//else	c_busA.wState |= BUSSTATE_ENABLE;
	else	c_tcRef.real.cycle.nBusState[0] |= BUSSTATE_ENABLE;

	if (buf[0] != NULL)	MSGLOG("reserve.\r\n");
	c_busB.pLine = new CBusTrx(this, c_mail, (PSZ) "CBusResv");
	c_busB.pLine->SetOwnerName((PSZ) "CBusResv");
	c_busB.pLine->SetCycleTime(c_tcRef.real.cycle.wTime);
	//c_busB.pLine->SetPrio(19);
	//c_busB.pLine->SetCps(c_tcRef.real.cps);
	if ((res = c_busB.pLine->Open(MF_MANCHB, OWN_SPACE,
			c_tcRef.real.nAddr, c_tcRef.real.cf.nLength, FALSE)) != PORTERROR_NON) {
		KILL(c_busB.pLine);
		c_busB.wState = 0;
		MSGLOG("[INIT]ERROR:Can not initial reserve bus(%d).\r\n", res);
		res |= BUSFAIL_RESV;
	}
	//else	c_busB.wState |= BUSSTATE_ENABLE;
	else	c_tcRef.real.cycle.nBusState[1] |= BUSSTATE_ENABLE;
	MSGLOG("[TC]Cycle time is %dms.\r\n", c_tcRef.real.cycle.wTime);
	return res;
}

BOOL CTcx::OpenPost()
{
	g_mqPost = mq_open(NAME_SERV, O_WRONLY);
	if (IsValid(g_mqPost)) {
		g_post.dwLength = (DWORD) this;
		g_post.wID = MSG_CONTC;
		mq_send(g_mqPost, (char*) &g_post, sizeof(SMSG), NULL); //PRIORITY_POST);
		return TRUE;
	}
	else	MSGLOG("[TC]ERROR:Can not open post queue(%s).\r\n", strerror(errno));
	g_mqPost = -1;
	return FALSE;
}

BOOL CTcx::OpenBeil()
{
	g_mqBeil = mq_open(NAME_BEIL, O_WRONLY);
	if (IsValid(g_mqBeil)) {
		if ((c_hShm = shm_open(NAME_BUCKET, O_RDWR, 0777)) < 0) {
			MSGLOG("[TC]ERROR:Can not open shared memory(%s).\r\n", strerror(errno));
			mq_close(g_mqBeil);
			g_mqBeil = -1;
			return FALSE;
		}
		c_pBucket = (PBUCKET)mmap(NULL, sizeof(WORD), PROT_READ | PROT_WRITE, MAP_SHARED, c_hShm, 0);
		if (c_pBucket == NULL) {
			MSGLOG("[TC]ERROR:Can not access to shared memory(%s).\r\n", strerror(errno));
			close(c_hShm);
			c_hShm = -1;
			mq_close(g_mqBeil);
			g_mqBeil = -1;
			return FALSE;
		}
		return TRUE;
	}
	else	MSGLOG("[TC]ERROR:Can not open beil queue(%s).\r\n", strerror(errno));
	g_mqBeil = -1;
	return FALSE;
}

//====== Communication ===================================================================
void CTcx::Opinion(BYTE id)
{
	-- id;
	if (id < LENGTH_TC)	INCBYTE(c_tcRef.real.revolt.nCandidate[id]);
}

void CTcx::OutputD()
{
	WORD w;
	for (UCURV n = 0; n < SIZE_DOUT; n ++) {
		w = MAKEWORD(FEEDTBYTE(OWN_SPACE, n * 2 + 1), FEEDTBYTE(OWN_SPACE, n * 2));
		if (OWN_SPACE == 2) {
			if (n == 1) {
				WORD v = (WORD)(FEEDTBYTE(CAR_HEAD, n * 2) & 0xb9);	// keep EBCOSLP, SBLP, PBLP, EBLP, EPANDNLP
				w |= v;
			}
		}
		else if (OWN_SPACE == 1 || OWN_SPACE == 3) {
			WORD mask = (WORD)MAKEWORD(c_nJointOut[n * 2 + 1], c_nJointOut[n * 2]);
			w &= mask;
		}
		// Modified 2012/11/12
		// Modified 2012/10/09
		//if (c_tcRef.real.insp.nMode > INSPECTMODE_NON && c_tcRef.real.insp.nMode < INSPECTMODE_VIEW &&
		//		c_tcRef.real.insp.nAbsLamp != 0 && n < SIZE_LAMP)
		if (c_tcRef.real.insp.nAbsLamp != 0 && n < SIZE_LAMP)
			w |= c_wLamps[n];
		c_probe.Putd(n, &w);
		// Modified 2012/11/29
		c_dozMain.tcs[OWN_SPACE].real.nRealOutput[n * 2] = LOWBYTE(w);
		c_dozMain.tcs[OWN_SPACE].real.nRealOutput[n * 2 + 1] = HIGHBYTE(w);
	}
	w = (c_tcRef.real.op.nState & OPERATESTATE_ATCON) ? 0x8000 : 0;	// output extention to MFB
	c_probe.Putd(3, &w);
}

void CTcx::HalfwayNeutral(BOOL cmd)
{
	if (cmd) {
		CLRTBIT(OWN_SPACE, TCDOB_HCR);	// no effect
		SETTBIT(OWN_SPACE, TCDOB_TCR);
		SETTBIT(OWN_SPACE, TCDOB_ZVR);	// no effect
		c_artop.ForceHoldBrake(FALSE);
		c_tcRef.real.ms.wBrake &= ~MSTATE_HOLDINGBRAKE;
	}
	else {
		CLRTBIT(OWN_SPACE, TCDOB_TCR);
	}
}

BOOL CTcx::CheckBusState()
{
	BOOL res = TRUE;
	//if (OWN_SPACE > 1)	return res;
	// Modified 2012/01/11 ... begin
	//BYTE tail = GetHeadCarID() ^ 7;
	BYTE head = GetHeadCarID();		// 0->TC1, other->TC0
	// ... end
	c_tcRef.real.bm.nTcs = 0;
	for (UCURV n = CAR_HEADBK; n < (c_tcRef.real.cf.nLength + LENGTH_TC); n ++) {
		WORD flag = 1 << n;
		if (!IsValidFirm(n)) {
			if (n >= LENGTH_TC) {
				CountDumb(&c_dumb.wBus[n]);
				CountEachDevError(n);
				c_tcRef.real.bm.nWakeup[ONLYCC(n)] = 0;
			}
			if (c_tcRef.real.bm.nDeb[n] != 0 && -- c_tcRef.real.bm.nDeb[n] == 0)
				c_tcRef.real.bm.wState |= flag; // bus fault;
			else {
				c_tcRef.real.bm.wBuf |= flag;
				if (!(c_tcRef.real.bm.wState & flag)) {
					if (n > 0 && n < LENGTH_TC) {
						memcpy((PVOID)&c_dozMain.tcs[n], (PVOID)&c_dozBkup.tcs[n], sizeof(TCFIRM));
						c_dozMain.tcs[n].real.dwOrderID = c_dozMain.tcs[CAR_HEAD].real.dwOrderID;
					}
					else {
						memcpy((PVOID)&c_dozMain.ccs[ONLYCC(n)], (PVOID)&c_dozBkup.ccs[ONLYCC(n)], sizeof(CCFIRM));
						c_dozMain.ccs[ONLYCC(n)].real.dwOrderID = c_dozMain.tcs[CAR_HEAD].real.dwOrderID;
					}
				}
			}
			if (c_tcRef.real.bm.wState & flag) {
				// Modified 2012/11/30
				//if ((c_tcRef.real.nObligation & OBLIGATION_TOP) == OBLIGATION_TOP) {
				//if ((c_tcRef.real.nObligation & OBLIGATION_CTRL) == OBLIGATION_CTRL) {
				if ((c_tcRef.real.nObligation & OBLIGATION_ACTIVE) && (c_tcRef.real.nObligation & OBLIGATION_HEAD)) {	// main & reserve
					if (n < LENGTH_TC) {
						WORD wMsg = GetPscIDByBus(n, head) + 31;
						ShotMainDev(wMsg < 33 ? 0 : 7, wMsg);		// !!!!!
					}
					else	ShotMainDev(ONLYCC(n), 35);		// !!!!!
				}
				if (n >= LENGTH_TC)
					c_tcRef.real.door.nBackup |= (1 << (ONLYCC(n) ^ 1));
			}
			res = FALSE;
		}
		else {
			c_tcRef.real.bm.wBuf &= ~flag;
			c_tcRef.real.bm.wState &= ~flag;
			c_tcRef.real.bm.nDeb[n] = DEBOUNCE_BUSMONIT;
			// Modified 2012/11/21
			if (n < LENGTH_TC) {
				SHORT tcid = GetPscIDByAddr(n);
				if (tcid >= 0) {
					c_tcRef.real.bm.nTcs |= (1 << tcid);
					// Modified 2012/11/30
					//if ((c_tcRef.real.nObligation & OBLIGATION_TOP) == OBLIGATION_TOP) {
					//if ((c_tcRef.real.nObligation & OBLIGATION_CTRL) == OBLIGATION_CTRL) {
					if ((c_tcRef.real.nObligation & OBLIGATION_ACTIVE) && (c_tcRef.real.nObligation & OBLIGATION_HEAD)) {	// main & reserve
						WORD wMsg = (WORD)(tcid + 31);
						CUTB(wMsg < 33 ? 0 : 7, wMsg);
					}
				}
			}
			else {
				c_tcRef.real.door.nBackup &= ~(1 << (ONLYCC(n) ^ 1));
				CUTB(ONLYCC(n), 35);
				if (c_tcRef.real.bm.nWakeup[ONLYCC(n)] < COUNT_CCWAKEUP)	++ c_tcRef.real.bm.nWakeup[ONLYCC(n)];
			}
		}
	}
	SHORT tcid = GetPscIDByAddr(0);
	if (tcid >= 0)	c_tcRef.real.bm.nTcs |= (1 << GetPscIDByAddr(0));
	for (UCURV n = 0; n < LENGTH_TC; n ++) {
		if (!(c_tcRef.real.bm.nTcs & (1 << n))) {
			CountDumb(&c_dumb.wBus[n]);
			CountEachDevError(n);
		}
	}
	return res;
}

BOOL CTcx::IsValidFirm(UCURV id)
{
	if (id < LENGTH_TC)	return IsValidTcFirm(id);
	return IsValidCcFirm(id - LENGTH_TC);
}

BOOL CTcx::IsValidTcFirm(UCURV ti)
{
	WORD add = GETLWORD(c_dozMain.tcs[ti].real.wAddr);
	if ((add & 0x7f00) == BUSADDR_HIGH && //(add & 0xff) == (ti + 1) &&
			c_dozMain.tcs[CAR_HEAD].real.dwOrderID == c_dozMain.tcs[ti].real.dwOrderID)
		return TRUE;
	return FALSE;
}

BOOL CTcx::IsValidCcFirm(UCURV ci)
{
	WORD add = GETLWORD(c_dozMain.ccs[ci].real.wAddr);
	if ((add & 0x7f00) == BUSADDR_HIGH && (add & 0xff) == (ci + CAR_CC + 1) &&
			c_dozMain.tcs[CAR_HEAD].real.dwOrderID == c_dozMain.ccs[ci].real.dwOrderID)
		return TRUE;
	return FALSE;
}

BOOL CTcx::GetBusCondition(UCURV id)
{
	if (c_tcRef.real.bm.wState & (1 << id))	return FALSE;
	return TRUE;
}

BOOL CTcx::GetRealBusCondition(UCURV id)
{
	if (c_tcRef.real.bm.wBuf & (1 << id))	return FALSE;
	return TRUE;
}

void CTcx::BusTrace(UCURV id, BOOL line)
{
	if (line)	c_tcRef.real.bm.wMain |= (1 << id);
	else	c_tcRef.real.bm.wResv |= (1 << id);
}

void CTcx::ClearBusTrace()
{
	c_tcRef.real.bm.wMain = c_tcRef.real.bm.wResv = 0;
}

BOOL CTcx::GetLocalCondition(UCURV uLocalID)
{
	// CAUTION- atc,ato be referenced to wResult, service device be referenced to wValidate
	if (c_tcRef.real.lm.wResult & (1 << uLocalID))	return TRUE;
	return FALSE;
}

void CTcx::ScanBucketStep()
{
	// capture beil's message
	if (c_tcRef.real.scLog.nPeriodStep == BEILSTEP_APPENDDRV) {
		if (c_pBucket->nPeriodAck == BEILACK_WORKEND || c_tcRef.real.scLog.wPeriodTimer == 0) {
			c_tcRef.real.scLog.nPeriodStep = 0;
			c_tcRef.real.scLog.wPeriodTimer = 0;
			if (c_pBucket->nPeriodAck == BEILACK_WORKEND) {
				uint64_t fe = ClockCycles();
				double sec = (double)(fe - c_drvTriggerTime) / (double)c_tcRef.real.cps;
				c_tcRef.real.cycle.wDrvSaveTime = (WORD)(sec * 1000.f);
				if (c_tcRef.real.cycle.wDrvSaveMaxTime < c_tcRef.real.cycle.wDrvSaveTime)
					c_tcRef.real.cycle.wDrvSaveMaxTime = c_tcRef.real.cycle.wDrvSaveTime;
				if (c_pBucket->iRespond == FILEWORK_END)	CUTBS(SCENE_ID_HEADCAR, 25, 28);
				else {
					WORD alarm = (WORD)(42 + c_pBucket->iRespond);
					EDITB(SCENE_ID_HEADCAR, 25, 28, alarm);
				}
			}
			else {
				EDITB(SCENE_ID_HEADCAR, 25, 27, 28);
				MSGLOG("[TC]ERROR:Append drv data timeout.\r\n");
			}
			// Modified 2013/11/02
			if (c_tcRef.real.nCurtains & CURTAIN_NEGATEBCID) {
				c_tcRef.real.nCurtains &= ~CURTAIN_WRITEENV;
				if ((c_tcRef.real.nCurtains & CURTAIN_CONDITION) == 0) {
					c_tcRef.real.nCurtains = 0;
					CLRTBIT(OWN_SPACE, TCDOB_BCID);
				}
			}
		}
	}
	else if (c_tcRef.real.scLog.nPeriodStep != BEILSTEP_NON) {
		MSGLOG("[TC]ERROR:Unknown command at driver data save.(%d)", c_tcRef.real.scLog.nPeriodStep);
		c_tcRef.real.scLog.nPeriodStep = BEILSTEP_NON;
	}

	if (c_tcRef.real.scLog.nMainStep != BEILSTEP_NON) {
		if (c_pBucket->nMainAck == BEILACK_WORKEND || c_tcRef.real.scLog.wMainTimer == 0) {
			BOOL bMakeDir = FALSE;
			BYTE step = c_tcRef.real.scLog.nMainStep;
			c_tcRef.real.scLog.nMainStep = BEILSTEP_NON;
			InterTrigger(&c_tcRef.real.scLog.wMainTimer, 0);
			switch (step) {
			case BEILSTEP_SAVELOG :
				if (c_pBucket->nMainAck == BEILACK_WORKEND)
					BeilTimeRecorder("[TC]Save. result is");
				else	MSGLOG("[TC]ERROR:Save. timeout\r\n");
				c_scene.ResetDrvArch();
				c_scene.ResetAlarmArch();
				//memset((PVOID)&c_tcRef.real.loginTime, 0, sizeof(DEVTIME));
				c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATFILESAVE;
				// Modified 2013/11/02
				//if (c_tcRef.real.scArm.nState & ARMSTATE_NEGATEBCID) {
				//	CLRTBIT(OWN_SPACE, TCDOB_BCID);		// power off
				//}
				if (c_tcRef.real.nCurtains & CURTAIN_NEGATEBCID) {
					c_tcRef.real.nCurtains &= ~CURTAIN_WRITELOG;
					if ((c_tcRef.real.nCurtains & CURTAIN_CONDITION) == 0) {
						c_tcRef.real.nCurtains = 0;
						CLRTBIT(OWN_SPACE, TCDOB_BCID);
					}
				}
				break;
			case BEILSTEP_MAKEDIRECTORYLIST :
				c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
				if (c_pBucket->nMainAck == BEILACK_WORKEND) {
					BeilTimeRecorder("[TC]Make directory list. result is");
					if ((c_tcRef.real.ai.wMaxFiles = c_scene.GetAlarmFileMaxIndex()) > 0) {
						c_nSelectedCid = 8;
						// Modified 2012/10/10
						MakeCurAlarmList();
						//OpenAlarmList(c_tcRef.real.ai.wMaxFiles);	// c_tcRef.real.scLog.nMainStep = BEILSTEP_LOADALARM;
						c_tcRef.real.du.nDownloadItem = DUKEY_ALARMCOLLECTION;
						c_tcRef.real.ai.nListID = ALARMLIST_COLLECTION;
					}
				}
				else	MSGLOG("[TC]ERROR:Make directory list. timeout\r\n");
				break;
			case BEILSTEP_LOADALARM :
				if (c_pBucket->nMainAck == BEILACK_WORKEND) {
					BeilTimeRecorder("[TC]Load alarm. result is");
					c_tcRef.real.ai.wMaxPages = c_scene.GetAlarmFileMaxLine();
					c_tcRef.real.ai.wCurrentPage = 0;
					c_scene.LoadAlarmBlock(c_tcRef.real.ai.wCurrentPage, c_tcRef.real.ai.wMaxPages);
					OpenAlarmPageToDu();	// by Tcx::OpenAlarmList()-DUKEY_UP DUKEY_DOWN
				}
				else	MSGLOG("[TC]ERROR:Load alarm. timeout\r\n");
				break;
			case BEILSTEP_SORTBYDAY :
				c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
				if (c_pBucket->nMainAck == BEILACK_WORKEND) {
					BeilTimeRecorder("[TC]Make directory list. result is");
					c_scene.SortByDay();
					WORD length = GetFilesByDayLength();
					if (c_tcRef.real.es.nTargetDuty == OWN_SPACE) {
						// Modified 2013/03/12
						//if (length > 0) {
						//	c_tcRef.real.du.nSetPage = DUPAGE_DIRBYDAY;
						//	c_tcRef.real.du.nDownloadItem = DUKEY_DIRBYDAY;		// for download
						//}
						//else	MSGLOG("[TC]ERROR:Not found sort data.\r\n");
						if (length == 0)	MSGLOG("[TC]ERROR:Not found sort data.\r\n");
						c_tcRef.real.du.nSetPage = DUPAGE_DIRBYDAY;
						c_tcRef.real.du.nDownloadItem = DUKEY_DIRBYDAY;		// for download
					}
					else {
						c_dozMain.tcs[OWN_SPACE].real.each.wId = TCINFORM_DIR;
						c_dozMain.tcs[OWN_SPACE].real.each.info.dir.nLength = (BYTE)length;
						c_dozMain.tcs[OWN_SPACE].real.each.info.dir.nDuty = OWN_SPACE;
						DWORD* pDw = GetFilesByDay();
						memcpy((PVOID)&c_dozMain.tcs[OWN_SPACE].real.each.info.dir.dwItem[0], (PVOID)pDw, sizeof(DWORD) * SIZE_FILESBYDAY);
						c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_DIRECTORYREQUEST;
					}
				}
				else	MSGLOG("[TC]ERROR:Make directory list. timeout.\r\n");
				break;
			case BEILSTEP_SELDELETE :
				c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
				if (c_pBucket->nMainAck == BEILACK_WORKEND)
					BeilTimeRecorder("[TC]Deleted");
				else	MSGLOG("[TC]ERROR:Delete timeout.\r\n");
				bMakeDir = TRUE;
				break;
			case BEILSTEP_SECTQUANTITY :
				if (c_pBucket->nMainAck == BEILACK_WORKEND) {
					c_tcRef.real.es.dwTotalSize = c_pBucket->tar.s.size;
					c_mate.ContinueDirectoryToPert(TRUE);
				}
				else	c_mate.ContinueDirectoryToPert(FALSE);
				break;
			case BEILSTEP_READSECT :
				if (c_pBucket->nMainAck == BEILACK_WORKEND)	c_mate.ContinueFileToPert(TRUE);
				else {
					INCWORD(c_tcRef.real.es.wReadError);
					c_mate.ContinueFileToPert(FALSE);
				}
				break;
			case BEILSTEP_WRITESECT :
				if (c_pBucket->nMainAck == BEILACK_WORKEND) {
					if (c_pBucket->iRespond < FILEERROR_NON) {
						MSGLOG("[TC]ERROR:error code %d.\r\n", c_pBucket->iRespond);
						Hangup(FILEMSG_BEGIN + c_pBucket->iRespond);
						c_mate.ActiveClose();
					}
					else	c_mate.ContinueFileFromPert(TRUE);
				}
				else {
					INCWORD(c_tcRef.real.es.wWriteError);
					c_mate.ContinueFileFromPert(FALSE);
				}
				break;
			default :
				MSGLOG("[TC]ERROR:Unknown command to beil.(%d)", c_tcRef.real.scLog.nMainStep);
				c_tcRef.real.scLog.nMainStep = BEILSTEP_NON;
				break;
			}

			if (bMakeDir) {
				c_tcRef.real.es.nTargetDuty = OWN_SPACE;
				DirectoryList();	// cause file delete
			}
		}
	}
}

void CTcx::SearchBroadcastAddr(BYTE* pBca, BYTE sa)
{
	if (*pBca < 0xff)	return;
	if (sa < 3)	*pBca = 3;
	else	*pBca = 1;
	if (!GetBusCondition(CAR_HEADBK) && FBTBIT(CAR_HEAD, TCDOB_RESVL))	++ *pBca;
}

void CTcx::CalcAtcSpeed()
{
	PATCAINFO paAtc = GetActiveAtc(TRUE);
	c_tcRef.real.speed.nCurPulse = (paAtc != NULL && paAtc->nFlow == LOCALFLOW_NORMAL) ? paAtc->d.st.nAccp : 0;
	c_nAccpes[c_tcRef.real.speed.nAtcCur] = c_tcRef.real.speed.nCurPulse;
	if (++ c_tcRef.real.speed.nAtcCur >= c_nAtcSample[0][ATCSPEED_UNIT])
		c_tcRef.real.speed.nAtcCur = 0;
	WORD sum = 0;
	for (UCURV n = 0; n < c_nAtcSample[0][ATCSPEED_UNIT]; n ++)
		sum += c_nAccpes[n];
	BYTE wi;
	paAtc = GetActiveAtc(FALSE);
	if (paAtc != NULL && paAtc->nFlow == LOCALFLOW_NORMAL)
		wi = c_tcRef.real.cond.nAtcWi = paAtc->d.st.tdws.a & 0xf;
	else	wi = c_tcRef.real.cond.nAtcWi;
	double v = (double)sum * (double)c_nAtcSample[1][ATCSPEED_UNIT] * (((double)860 - (double)(wi * 5)) / 1000.f) * // 860mm -> 0.86m
						(double)M_PI * 3600.f / 200.f;
	c_tcRef.real.speed.dwAtcs = (DWORD)v;
}

void CTcx::ScanNuetralAtDriverless(BOOL bStop)
{
	if (c_tcRef.real.nObligation & OBLIGATION_HEAD) {
		if (bStop && c_tcRef.real.op.nMode == OPMODE_DRIVERLESSREADY) {
			if (c_tcRef.real.op.nAutoStep != DRIVERLESSSTEP_WAITATOR) {
				if (GETTBIT(OWN_SPACE, TCDIB_C) && !GETTBIT(OWN_SPACE, TCDIB_F) && !GETTBIT(OWN_SPACE, TCDIB_R)) {
					if (c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITNUETRAL) {
						c_tcRef.real.op.nAutoStep &= ~DRIVERLESSSTEP_WAITNUETRAL;
						CLRTBIT(OWN_SPACE, TCDOB_HCR);
					}
				}
				else {
					if (!(c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITNUETRAL))
						c_tcRef.real.op.nAutoStep |= DRIVERLESSSTEP_WAITNUETRAL;
					if (GETTBIT(OWN_SPACE, TCDIB_F) || GETTBIT(OWN_SPACE, TCDIB_R))
						SETTBIT(OWN_SPACE, TCDOB_HCR);
				}
			}
		}
		else if (!bStop || c_tcRef.real.op.nMode == OPMODE_DRIVERLESS) {
			if (GETTBIT(OWN_SPACE, TCDIB_P) || GETTBIT(OWN_SPACE, TCDIB_B) || GETTBIT(OWN_SPACE, TCDIB_F) || GETTBIT(OWN_SPACE, TCDIB_R)) {
				MSGLOG("[TC]to DriverlessReady,\r\n");
				c_look.ToDriverlessReady();
			}
		}
	}
}

void CTcx::ControlByMainDev()
{
	PATCAINFO paAtc = GetActiveAtc(TRUE);
	// Modified 2012/11/12 ... begin
	if (paAtc != NULL)	c_tcRef.real.insp.nAbsLamp = paAtc->d.st.req.b.lampt;
	// ... end
	PATOAINFO paAto = GetAto(TRUE);
	// Modified 2011/10/13
	// if paAtc is NULL then system down
	if (paAtc != NULL && paAto != NULL &&
			// not need this line, because can not inspect in auto mode
			paAtc->nFlow == LOCALFLOW_NORMAL && paAto->nFlow == LOCALFLOW_NORMAL &&
			CheckAtcReply(paAtc) == FIRMERROR_NON && CheckAtoReply(paAto) == FIRMERROR_NON) {
		if (c_tcRef.real.op.nMode == OPMODE_DRIVERLESSREADY) {
			if (c_tcRef.real.nObligation & OBLIGATION_HEAD) {
				if (c_tcRef.real.op.nAutoStep == DRIVERLESSSTEP_WAITATOR) {
					if (paAto->d.st.state.b.ator) {
						if (c_tcRef.real.op.nState & OPERATESTATE_INHIBITATO) {
							c_tcRef.real.op.nState &= ~OPERATESTATE_INHIBITATO;
							c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_HOLDINHIBIT;
							InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_ATOINHIBIT);
						}
						else {
							c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_INIT;
						}
						c_tcRef.real.op.nAutoDeb = DEBOUNCE_PSM6;		// psm6 deb
						//c_tcRef.real.op.nState &= ~OPERATESTATE_DETECTPSM6;
						c_tcRef.real.op.nState |= OPERATESTATE_DETECTPSM6;	// ????? !!!!!
						c_tcRef.real.op.nMode = OPMODE_DRIVERLESS;
						CUTB(SCENE_ID_HEADCAR, 678);
					}
					else if (c_tcRef.real.op.wWaitSignalAutoB == 0) {
						SHOTB(SCENE_ID_HEADCAR, 678);
						InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATOREADY);
					}
				}
				else {
					if ((paAtc->d.st.mode.a & (ATCMODE_VALID | ATCMODE_ONLY)) == (ATCMODE_VALID | ATCMODE_DM)) {
						if (c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITMODE) {
							c_tcRef.real.op.nAutoStep &= ~DRIVERLESSSTEP_WAITMODE;
							c_tcRef.real.atoCmd.nCmd |= INSTCMD_ACTIVE;		// ?????
							CUTB(SCENE_ID_HEADCAR, 613);
							InterTrigger(&c_tcRef.real.op.wWaitSignalAutoA, 0);
						}
					}
					else if (!(c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITMODE)) {
						c_tcRef.real.op.nAutoStep |= DRIVERLESSSTEP_WAITMODE;
						if (c_tcRef.real.op.wWaitSignalAutoA == 0) {
							c_tcRef.real.atoCmd.nCmd &= ~INSTCMD_ACTIVE;	// ?????
							ShotMainDev(SCENE_ID_HEADCAR, 613);
							InterTrigger(&c_tcRef.real.op.wWaitSignalAutoA, TIME_WAITATCDMBIT);
						}
					}

					if (paAtc->d.st.req.b.kup) {
						if ((c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITKUP) &&
								-- c_tcRef.real.op.nAutoDeb == 0) {		// kup deb
							c_tcRef.real.op.nAutoStep &= ~DRIVERLESSSTEP_WAITKUP;
							//c_tcRef.real.op.nState &= ~OPERATESTATE_DETECTPSM6;
							c_tcRef.real.op.nState |= OPERATESTATE_DETECTPSM6;
							ToFlash(TCDOB_DRML, FALSE);
							SETTBIT(OWN_SPACE, TCDOB_HCR);
							SETTBIT(OWN_SPACE, TCDOB_DRML);
							c_tcRef.real.atoCmd.nCmd |= INSTCMD_INHIBITA;
							SHOTB(SCENE_ID_HEADCAR, 671);
							if (c_tcRef.real.op.nState & OPERATESTATE_INHIBITATO)
								SHOTB(SCENE_ID_HEADCAR, 682);
							SHOTB(SCENE_ID_HEADCAR, 141);
							CUTB(SCENE_ID_HEADCAR, 673);
						}
						InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATCKUP);
						CUTB(SCENE_ID_HEADCAR, 672);
					}
					//else if (!(c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITKUP)) {
					else {
						if (c_tcRef.real.op.wWaitSignalAutoB == 0) {
							SHOTB(SCENE_ID_HEADCAR, 672);
							InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATCKUP);
						}
						if (!(c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_WAITKUP)) {
							c_tcRef.real.op.nAutoStep |= DRIVERLESSSTEP_WAITKUP;
							ToFlash(TCDOB_DRML, TRUE);
							CLRTBIT(OWN_SPACE, TCDOB_HCR);
						}
						c_tcRef.real.op.nAutoDeb = DEBOUNCE_KUP;	// kup deb
						c_tcRef.real.atoCmd.nCmd &= ~INSTCMD_INHIBITA;
					}

					if (!(c_tcRef.real.op.nAutoStep & DRIVERLESSSTEP_INTRO)) {
						c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_WAITATOR;
						InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATOREADY);
					}
				}
			}
		}
		else if (c_tcRef.real.op.nMode == OPMODE_DRIVERLESS) {
			if (c_tcRef.real.op.nAutoStep == DRIVERLESSSTEP_HOLDINHIBIT) {
				if (c_tcRef.real.op.wWaitSignalAutoB == 0) {
					c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_INIT;
					c_tcRef.real.atoCmd.nCmd &= ~INSTCMD_INHIBITA;
				}
			}
			else {
				if (paAtc->d.st.req.b.kup) {
					c_tcRef.real.atoCmd.nCmd |= INSTCMD_INHIBITA;
					if (c_dozMain.tcs[CAR_TAIL].real.nDrvlCmd & DRVLCMD_KUP)
						SHOTB(SCENE_ID_HEADCAR, 675);
					else	CUTB(SCENE_ID_HEADCAR, 675);
				}
				else {
					c_tcRef.real.atoCmd.nCmd &= ~INSTCMD_INHIBITA;
					CUTB(SCENE_ID_HEADCAR, 675);
				}
			}

			if (c_tcRef.real.op.nState & OPERATESTATE_DETECTPSM6) {
				if (paAtc->d.st.req.b.kdn && !paAtc->d.st.req.b.kup) {
					if (-- c_tcRef.real.op.nAutoDeb == 0) {	// kdn deb
						c_tcRef.real.op.nMode = OPMODE_DRIVERLESSEXIT;
						c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_INIT;
						c_tcRef.real.op.nKdnState |= KDNSTATE_SETTLE;
						c_tcRef.real.op.nState &= ~OPERATESTATE_DETECTPSM6;
						//InterTrigger(&c_tcRef.real.op.wWaitSignalAutoA, TIME_WAITATCDMBIT);
						//InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATCKUP);
						//CLRTBIT(OWN_SPACE, TCDOB_DPLP);
						//CLRTBIT(OWN_SPACE, TCDOB_HCR);
						SHOTB(SCENE_ID_HEADCAR, 673);
						CUTB(SCENE_ID_HEADCAR, 671);
					}
				}
				else	c_tcRef.real.op.nAutoDeb = DEBOUNCE_KDN;	// kdn deb
			}
			else {
				if (paAto->d.st.psm.b.f6) {
					if (-- c_tcRef.real.op.nAutoDeb == 0) {	// psm6 deb
						c_tcRef.real.op.nState |= OPERATESTATE_DETECTPSM6;
						c_tcRef.real.op.nAutoDeb = DEBOUNCE_KDN;	// kdn deb
					}
				}
				else	c_tcRef.real.op.nAutoDeb = DEBOUNCE_PSM6;	// psm6 deb
			}
		}
		else {
			if (c_tcRef.real.op.nMode == OPMODE_AUTOREADY ||
					c_tcRef.real.op.nMode == OPMODE_AUTO) {
				switch (c_tcRef.real.op.nAutoStep) {
				case AUTOSTEP_WAITATCCMD :
					if (paAtc->d.st.mode.b.amv) {
						BYTE mode = paAtc->d.st.mode.a & ATCMODE_ONLY;
						if (mode == ATCMODE_AM) {
							c_tcRef.real.op.nAutoStep = AUTOSTEP_WAITATOR;
							InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATOREADY);
							c_tcRef.real.op.nAutoStep = AUTOSTEP_WAITATOR;
							CUTB(SCENE_ID_HEADCAR, 692);
						}
					}
					else if (c_tcRef.real.op.wWaitSignalAutoB == 0) {
						SHOTB(SCENE_ID_HEADCAR, 692);
						InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATCCMD);
					}
					break;
				case AUTOSTEP_WAITATOR :
					if (paAto->d.st.state.b.ator) {
						c_look.OpModeLamp(TCDOM_AMLP | TCDOM_AMLP1, LP_ON);
						InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, 0);
						if (c_tcRef.real.op.nMode == OPMODE_AUTOREADY)
							c_tcRef.real.op.nMode = OPMODE_AUTO;
						c_tcRef.real.op.nAutoStep = AUTOSTEP_SCANADP;
						CUTB(SCENE_ID_HEADCAR, 678);
					}
					else if (c_tcRef.real.op.wWaitSignalAutoB == 0) {
						SHOTB(SCENE_ID_HEADCAR, 678);
						InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATOREADY);
					}
					break;
				case AUTOSTEP_SCANADP :
					// Modified 2011/12/21 ... begin
					//if (!paAtc->d.st.req.b.dbp && paAtc->d.st.mode.b.adp &&
					//		GETTBIT(OWN_SPACE, TCDIB_ADSCR)) {
					if (paAtc->d.st.mode.b.adp && GETTBIT(OWN_SPACE, TCDIB_ADSCR)) {
						SETTBIT(OWN_SPACE, TCDOB_DPLP);
						c_tcRef.real.op.nAutoStep = AUTOSTEP_SCANDBP;
					}
					// ... end
					break;
				case AUTOSTEP_SCANDBP :
					// Modified 2011/12/21 ... begin
					//if (!paAtc->d.st.mode.b.adp && paAtc->d.st.req.b.dbp) {
					if (!paAtc->d.st.mode.b.adp) {
						CLRTBIT(OWN_SPACE, TCDOB_DPLP);
						c_tcRef.real.op.nAutoStep = AUTOSTEP_SCANADP;
					}
					// ... end
					break;
				default:
					break;
				}
			}
			if (paAtc->d.st.req.b.kup)	SHOTB(SCENE_ID_HEADCAR, 674);
			else	CUTB(SCENE_ID_HEADCAR, 674);
		}
	}
}

PATCAINFO CTcx::GetActiveAtc(BOOL real)
{
	if (c_tcRef.real.cond.nActiveAtc == (LOCALID_ATCA + 1)) {
		if (real || c_lcFirm.aAtc[0].nFlow == LOCALFLOW_NORMAL)
			return (&c_lcFirm.aAtc[0]);
		else	return c_pLocal->GetAtca();
	}
	else if (c_tcRef.real.cond.nActiveAtc == (LOCALID_ATCB + 1)) {
		if (real || c_lcFirm.aAtc[1].nFlow == LOCALFLOW_NORMAL)
			return (&c_lcFirm.aAtc[1]);
		else	return c_pLocal->GetAtcb();
	}
	return NULL;
}

PATOAINFO CTcx::GetAto(BOOL real, BOOL type)
{
	if (real || c_lcFirm.aAto.nFlow == LOCALFLOW_NORMAL)
		return (&c_lcFirm.aAto);
	else	return c_pLocal->GetAto(type);
}

void CTcx::LocalErrorToBus()
{
	BYTE n = OWN_SPACE < 2 ? 0 : 1;
	// Modified 2013/02/05
	c_dozMain.tcs[OWN_SPACE].real.eAtcA.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_ATCA)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.eAtcA.wCount = c_dumb.wAtc[0];
	c_dozMain.tcs[OWN_SPACE].real.eAtcB.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_ATCB)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.eAtcB.wCount = c_dumb.wAtc[1];
	c_dozMain.tcs[OWN_SPACE].real.eAto.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_ATO)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.eAto.wCount = c_dumb.wAto;
	c_dozMain.tcs[OWN_SPACE].real.eCsc.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_CSC)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.eCsc.wCount = c_dumb.wCsc[n];
	c_dozMain.tcs[OWN_SPACE].real.ePis.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_PIS)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.ePis.wCount = c_dumb.wPis[n];
	c_dozMain.tcs[OWN_SPACE].real.eTrs.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_TRS)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.eTrs.wCount = c_dumb.wTrs[n];
	c_dozMain.tcs[OWN_SPACE].real.eHtc.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_HTC)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.eHtc.wCount = c_dumb.wHtc[n];
	c_dozMain.tcs[OWN_SPACE].real.ePsd.nState = (c_tcRef.real.lm.wValidate & (1 << LOCALID_PSD)) ? 0 : DEVFLOW_ERR;
	c_dozMain.tcs[OWN_SPACE].real.ePsd.wCount = c_dumb.wPsd[n];
}

void CTcx::TipPartner(BYTE addr, BOOL stat)
{
	if (addr < 1 || addr > 4)	return;
	WORD wMsg;
	if (!stat) {
		switch (addr) {
		case 1 :	wMsg = 37;	break;	// found at TC1-LIU1
		case 2 :	wMsg = 36;	break;	// found at TC1-LIU2
		case 3 :	wMsg = 39;	break;	// found at TC0-LIU1
		default :	wMsg = 38;	break;	// found at TC0-LIU2
		}
		SHOTB(wMsg < 38 ? 0 : 7, wMsg);
	}
	else {
		if (addr < 3)	CUTBS(0, 36, 37);
		else	CUTBS(7, 38, 39);
	}
}

UCURV CTcx::CheckSivReply(UCURV nCcid, PSIVEINFO peSiv, BOOL log)
{
	if (!GETCBIT(nCcid, CCDIB_SIVK)) {
		int id = nCcid < 2 ? 0 : 1;
		if (log && CheckSivAddr(peSiv) && c_tcRef.real.dev.nSivkmon[id] >= DEVMON_SIVK)
			SHOTB(nCcid, 264);
		CUTB(nCcid, 235);
	}
	else if (log)	CUTB(nCcid, 264);
	if (!GETCBIT(nCcid, CCDIB_IVS) || GETCBIT(nCcid, CCDIB_IVF) ||
			GETCBIT(nCcid, CCDIB_SIVFR)) {
		if (log)	SHOTB(nCcid, 231);
		if (GETCBIT(nCcid, CCDIB_SIVFR) && log)
			SHOTB(nCcid, 265);
		return FIRMERROR_FAULT;
	}
	else if (log) {
		CUTB(nCcid, 231);
		CUTB(nCcid, 265);
		CUTB(nCcid, 235);
	}
	if (!CheckSivAddr(peSiv)) {
		if (log && IsWakeupB(nCcid))	ShotMainDev(nCcid, 245);		// !!!!!
			//SHOTB(nCcid, 245);
		return FIRMERROR_ADDR;
	}
	else if (log)	CUTB(nCcid, 245);
	return FIRMERROR_NON;
}

UCURV CTcx::CheckV3fReply(UCURV nCcid, PV3FEINFO peV3f, BOOL log)
{
	if (!GETCBIT(nCcid, CCDIB_DCK)) {
		if (log)	SHOTB(nCcid, 336);
		return FIRMERROR_FAULT;
	}
	if (log)	CUTB(nCcid, 336);
	if (!CheckV3fAddr(peV3f)) {
		if (log)	ShotMainDev(nCcid, 334);		// !!!!!
			// SHOTB(nCcid, 334);
		return FIRMERROR_ADDR;
	}
	if (log)	CUTB(nCcid, 334);

	if (peV3f->nFlow == DEVFLOW_TRACEA)	return FIRMERROR_NON;
	if (peV3f->d.st.st.ack.b.dck) {
		if (log)	SHOTB(nCcid, 327);
		return FIRMERROR_FAULT;
	}
	if (log)	CUTB(nCcid, 327);
	return FIRMERROR_NON;
}

UCURV CTcx::CheckEcuReply(UCURV nCcid, PECUAINFO paEcu, BOOL log)
{
	if (!GETCBIT(nCcid, CCDIB_ECUF)) {
		if (log)	SHOTB(nCcid, 442);
		return FIRMERROR_FAULT;
	}
	if (log)	CUTB(nCcid, 442);
	if (!CheckEcuAddr(paEcu)) {
		if (log)	ShotMainDev(nCcid, 454);		// !!!!!
			// SHOTB(nCcid, 454);
		return FIRMERROR_ADDR;
	}
	if (log)	CUTB(nCcid, 454);
	return FIRMERROR_NON;
}

UCURV CTcx::CheckCmsbReply(UCURV nCcid, PCMSBAINFO paCmsb, BOOL log)
{
	if (GETCBIT(nCcid, CCDIB_CMIF)) {
		if (log)	SHOTB(nCcid, 501);
		return FIRMERROR_FAULT;
	}
	if (log)	CUTB(nCcid, 501);
	if (!CheckCmsbAddr(paCmsb)) {
		if (log)	ShotMainDev(nCcid, 510);		// !!!!!
			// SHOTB(nCcid, 510);
		return FIRMERROR_ADDR;
	}
	if (log)	CUTB(nCcid, 510);
	return FIRMERROR_NON;
}

BOOL CTcx::CheckDcuReply(PDCUAINFO paDcu, BYTE id, BYTE cid)
{
	if (!CheckDcuAddr(paDcu, id, cid))	return FALSE;
	return TRUE;
}

BOOL CTcx::CheckSivAddr(PSIVEINFO peSiv)
{
	if (GETLWORD(peSiv->wAddr) != DEVADD_SIV)	return FALSE;
	return TRUE;
}

BOOL CTcx::CheckV3fAddr(PV3FEINFO peV3f)
{
	if (GETLWORD(peV3f->wAddr) != DEVADD_V3F)	return FALSE;
	return TRUE;
}

BOOL CTcx::CheckEcuAddr(PECUAINFO paEcu)
{
	if (GETLWORD(paEcu->wAddr) != DEVADD_ECU)	return FALSE;
	return TRUE;
}

BOOL CTcx::CheckCmsbAddr(PCMSBAINFO paCmsb)
{
	// Modified 2012/12/25
	//if (GETLWORD(paCmsb->wAddr) != DEVADD_CMSB)	return FALSE;
	//return TRUE;
	if (paCmsb->nStx != DEVFLOW_STX || GETLWORD(paCmsb->wAddr) != DEVADD_CMSB)	return FALSE;
	return TRUE;
}

BOOL CTcx::CheckDcuAddr(PDCUAINFO paDcu, BYTE id, BYTE cid)
{
	// Modified 2012/12/25
	//if (GETLWORD(paDcu->wAddr) != DEVADD_DCU)	return FALSE;
	//return TRUE;
	if (cid == (c_tcRef.real.cf.nLength - 1))	id = 9 - id;	// Xch at zero
	if (paDcu->nStx != DEVFLOW_STX || GETLWORD(paDcu->wAddr) != DEVADD_DCU || paDcu->nDcua != id)	return FALSE;
	if (c_pLocal->CheckBcc((BYTE*)paDcu, sizeof(DCUAINFO)) != 0)	return FALSE;
	return TRUE;
}

void CTcx::ResetDuc()
{
	c_doc.ResetMotion(2);
}

void CTcx::FormatDuc()
{
	if (c_tcRef.real.du.nSetPage != 0) {
		if (c_tcRef.real.du.nSetPage == DUPAGE_INSPECT ||
				c_tcRef.real.du.nSetPage == DUPAGE_TRACE) {
			BYTE id = c_tcRef.real.du.nSetPage == DUPAGE_INSPECT ? 1 : 2;
			if (++ c_tcRef.real.du.nRoll > 4)	c_tcRef.real.du.nRoll = 0;
			c_tcRef.real.du.nCurPage = c_nDuPages[id][c_tcRef.real.du.nRoll];
		}
		else {
			c_tcRef.real.du.nCurPage = c_tcRef.real.du.nSetPage;
			if (c_tcRef.real.du.nSetPage == DUPAGE_OPENNORMAL ||
					c_tcRef.real.du.nSetPage == DUPAGE_OPENOVERHAUL ||
					c_tcRef.real.du.nSetPage == DUPAGE_OPENTIMESET ||
					c_tcRef.real.du.nSetPage == DUPAGE_RESCUE ||
					c_tcRef.real.du.nSetPage == DUPAGE_DIRBYDAY ||
					c_tcRef.real.du.nSetPage == DUPAGE_HEADPILE ||
					c_tcRef.real.du.nSetPage == DUPAGE_TAILPILE ||
					c_tcRef.real.du.nSetPage == DUPAGE_ENV)
				c_tcRef.real.du.nSetPage = DUPAGE_OPERATE;
		}
	}
	else {
		if (c_tcRef.real.du.nCurPage == DUPAGE_ARMLIST) {
		}
		else {
			if (++ c_tcRef.real.du.nCurPage >= DUPAGE_CIRCLEEND)
				c_tcRef.real.du.nCurPage = DUPAGE_OPERATE;
			else {
				if (c_tcRef.real.du.nTerm & DUTERM_FEED) {
					if (c_tcRef.real.ai.nListID == ALARMLIST_PRESENT || c_tcRef.real.insp.nMode == INSPECTMODE_VIEW) {
						if (c_tcRef.real.du.nCurPage == (c_tcRef.real.du.nTiePage + 1)) {
							LockArm();
							c_tcRef.real.du.nTerm &= ~DUTERM_FEED;
							UnlockArm();
							if (c_tcRef.real.ai.nListID == ALARMLIST_PRESENT) {
								c_tcRef.real.ai.wMaxPages = c_scene.OpenAlarm(c_tcRef.real.ai.wCurrentPage, c_nSelectedCid);
								//c_tcRef.real.du.nCurPage = DUPAGE_ARMLIST;
								OpenAlarmPageToDu();
							}
							else if (c_tcRef.real.insp.nMode == INSPECTMODE_VIEW) {
								if (c_tcRef.real.insp.nStep == 0) {
									c_tcRef.real.du.nCurPage = DUPAGE_INSPECT;
									c_tcRef.real.insp.nStep = 1;
									c_tcRef.real.du.nTerm |= DUTERM_FEED;
								}
								else	c_tcRef.real.du.nCurPage = DUPAGE_OPENINSPECT;
							}
							if (++ c_tcRef.real.du.nTiePage >= 4)	c_tcRef.real.du.nTiePage = 0;
						}
					}
				}
			}
		}
	}
	c_doc.Format((WORD)c_tcRef.real.du.nCurPage);
}

//void CTcx::LatterDuc()
//{
//	if (c_tcRef.real.du.nCurPage == DUPAGE_ARMLIST) {
//		if (++ c_tcRef.real.du.nAlarmPageCount >= COUNT_ALARMPAGE)
//			c_tcRef.real.du.nCurPage = c_tcRef.real.du.nBkupPage;
//	}
//}
//
void CTcx::TimeSet(WORD year, BYTE mon, BYTE day, BYTE hour, BYTE min)
{
	if (mon > 0 && mon < 13 && day > 0 && day < 32 && hour < 24 && min < 60) {
		MSGLOG("[TC]%04d-%02d-%02d %02d:%02d.\n", year, mon, day, hour, min);
		struct tm st;
		st.tm_year = year - 1900;
		st.tm_mon = mon - 1;
		st.tm_mday = day;
		st.tm_hour = hour;
		st.tm_min = min;
		st.tm_sec = 0;
		st.tm_isdst = 0;

		time_t t = mktime(&st);
		if (t == (time_t)-1)
			MSGLOG("[TC]FAIL:Set time at mktime.\n");
		else {
			struct timespec ts;
			ts.tv_sec = t;
			ts.tv_nsec = 0;
			int res;
			if ((res = clock_settime(CLOCK_REALTIME, &ts)) < 0)
				MSGLOG("[TC]FAIL:Set time(%s).\n", strerror(errno));
			else	BeilCmd(BEILCMD_RTC);		// not need respond from beil
		}
	}
	CUTB(SCENE_ID_HEADCAR, 49);
}

void CTcx::OpenAlarm(WORD index)
{
	c_tcRef.real.ai.wMaxPages = c_scene.OpenAlarm(index, c_nSelectedCid);
	OpenAlarmPageToDu();	// by OpenAlarm()-DUKEY_ALARMLIST DUKEY_ALARMSORTX
}

void CTcx::MakeCurAlarmList()
{
	DEVTIME dt;
	memcpy((PVOID)&dt, (PVOID)&c_devTime, sizeof(DEVTIME));
	sprintf((char*)c_armTimeText, "D%02x%02x%02x%02x%02x%02x",
			dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.sec);
	WORD wLine = (WORD)PickAlarm();
	c_tcRef.real.ai.wCurrentFile = ALARMFILE_CURRENT;
	c_tcRef.real.ai.wMaxPages = wLine / ALARMLIST_LINES;
	if ((wLine % ALARMLIST_LINES) > 0)	++ c_tcRef.real.ai.wMaxPages;
	c_tcRef.real.ai.wCurrentPage = 0;
	c_scene.LoadCurAlarmBlock(c_tcRef.real.ai.wCurrentPage, c_tcRef.real.ai.wMaxPages);
	OpenAlarmPageToDu();	// by MakeCurAlarmList()-DUKEY_UP DUKEY_DOWN(BEILSTEP_MAKEDIRECTORYLIST)
}

void CTcx::OpenAlarmList(WORD index)
{
	if (c_pBucket != NULL) {
		RESET_BEILACK();
		c_tcRef.real.ai.wCurrentFile = c_pBucket->wParam1 = index;
		c_scene.GetAlarmTimeByIndex(index, c_armTimeText);
		MSGLOG("[TC]Open alarm %d(%s).\r\n", index, c_armTimeText);
		c_beilTriggerTime = ClockCycles();
		BeilCmd(BEILCMD_LOADALARM);
		c_tcRef.real.scLog.nMainStep = BEILSTEP_LOADALARM;
		InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT / 5);
	}
}

void CTcx::OpenAlarmPageToDu()
{
	c_tcRef.real.ai.nProtect |= PROTECT_PAGE;
	if (c_tcRef.real.du.nCurPage != DUPAGE_ARMLIST)
		c_tcRef.real.du.nBkupPage = c_tcRef.real.du.nCurPage;
	c_tcRef.real.du.nCurPage = DUPAGE_ARMLIST;
}

void CTcx::SetTimeByTb()
{
	WORD year  = ToHex((WORD)c_tbTime.year);
	BYTE month = ToHex((BYTE)c_tbTime.month);
	BYTE day   = ToHex((BYTE)c_tbTime.day);
	BYTE hour  = ToHex((BYTE)c_tbTime.hour);
	BYTE min   = ToHex((BYTE)c_tbTime.minute);
	TimeSet(year, month, day, hour, min);
}

//===== Train Control ====================================================================
BYTE CTcx::GetPscIDByBus(UCURV id, BYTE head)
{
	if (id >= LENGTH_TC)	return 3;	// ?????

	if (id == 1 || id == 3) {
		BYTE ti = id == 1 ? 0 : 2;	// liu1 pos.
		BYTE add = c_dozMain.tcs[ti].real.nRealAddr;
		switch (add) {
		case 1 :	return 1;	break;	// TC1-LIU1 address -> TC1-LIU2 fail
		case 2 :	return 0;	break;	// TC1-LIU2 address -> TC1-LIU1 fail
		case 3 :	return 3;	break;	// TC0-LIU1 address -> TC0-LIU2 fail
		case 4 :	return 2;	break;	// TC0-LIU2 address -> TC0-LIU1 fail
		default :
			// n	head		code
			// 1	!=0(TC1)	TC1-LIU2
			// 1	=0(TC0)		TC0-LIU2
			// 3	!=0(TC1)	TC0-LIU2
			// 3	=0(TC0)		TC1-LIU2
			if ((id == 1 && head != 0) || (id == 3 && head == 0))	return 3;
			else	return 1;
			break;
		}
	}
	else {
		if (head != 0)	id ^= 2;
		return id;	// 0 or 2 -> TCX->LIU1 fail
	}
}

SHORT CTcx::GetPscIDByAddr(UCURV id)
{
	if (id >= LENGTH_TC)	return -1;

	BYTE add = c_dozMain.tcs[id].real.nRealAddr;
	if (add >= 1 && add <= 4)	return ((int)-- add);
	else	return -1;
}

WORD CTcx::GetCarDevices(WORD carID)
{
	CARINFO car;
	car.nLength = c_tcRef.real.cf.nLength;
	if (carID >= MAX_TRAIN_LENGTH)	car.nID = MAX_TRAIN_LENGTH;
	else	car.nID = carID;
	return (c_panel.GetCcDev(&car));
}

BYTE CTcx::GetV3fQuantity()
{
	CARINFO car;
	car.nLength = c_tcRef.real.cf.nLength;
	car.nID = 0;
	return (c_panel.GetV3fQuantity(&car));
}

BYTE CTcx::GetEcuQuantity()
{
	CARINFO car;
	car.nLength = c_tcRef.real.cf.nLength;
	car.nID = 0;
	return (c_panel.GetEcuQuantity(&car));
}

BYTE CTcx::GetTrainDir(BOOL bSide)
{
	WORD side = bSide ? CAR_HEAD : CAR_TAIL;
	BYTE dir = 0;
	if (GETTBIT(side, TCDIB_F))	dir |= 1;
	if (GETTBIT(side, TCDIB_R))	dir |= 2;
	//MSGLOG("[TC]side%d direction is %d.\r\n", bSide, dir);
	return dir;
}

double CTcx::GetRealAsp(UCURV cid)
{
	double pw = 0.f;
	if (CheckEcuReply(cid, &c_dozMain.ccs[cid].real.aEcu, FALSE) == FIRMERROR_NON)
		pw = Conv(c_pDoz->ccs[cid].real.aEcu.d.st.nAsp, 0, 255, 0.51f, 5.71f);
	return pw;
}

double CTcx::GetRealAcp(UCURV cid)
{
	double pw = 0.f;
	if (CheckEcuReply(cid, &c_dozMain.ccs[cid].real.aEcu, FALSE) == FIRMERROR_NON)
		pw = Conv(c_pDoz->ccs[cid].real.aEcu.d.st.nAcp, 0, 255, 0.f, 7.8f);	// 0 ~ 7.8
	return pw;
}

double CTcx::GetRealBcp(UCURV cid)
{
	double pw = 0.f;
	if (CheckEcuReply(cid, &c_dozMain.ccs[cid].real.aEcu, FALSE) == FIRMERROR_NON)
		pw = Conv(c_pDoz->ccs[cid].real.aEcu.d.st.nBcp, 0, 255, 0.f, 7.8f);	// 0 ~ 7.8
	return pw;
}

double CTcx::GetPassengerWeightRate(UCURV cid)
{
	double pw = GetRealAsp(cid);
	if (pw > c_dbAspBase[cid]) {
		pw = (pw - c_dbAspBase[cid]) / 1.91f * 200.f;
		// Modified 2013/05/08
		if (pw > 200.f)	pw = 200.f;
	}
	else	pw = 0.f;
	return pw;
}

BYTE CTcx::GetHeadCarID()
{
	BYTE carid = c_tcRef.real.hcr.nMasterID & HCR_ONLYID;
	if (carid < 3)	return 0;	// head car
	return 7;
}

void CTcx::ReadProbe()
{
	//RECTIME(TTIME_INPUTOPEN);
	Lock();
	for (UCURV n = 0; n < SIZE_TCAI; n ++)
		c_dozMain.tcs[OWN_SPACE].real.vInput[n] = c_ain[n].v;
	for (UCURV n = 0; n < SIZE_DIN; n ++) {
		c_dozMain.tcs[OWN_SPACE].real.nInput[n * 2] = LOWBYTE(c_din[n].v);
		c_dozMain.tcs[OWN_SPACE].real.nInput[n * 2 + 1] = HIGHBYTE(c_din[n].v);
	}
	Unlock();
	//RECTIME(TTIME_INPUTCLOSE);
}

// Modified 2013/11/02
//void CTcx::Powering(BOOL bP, BOOL bB, BYTE nDir, _TV_ vTbeB)
void CTcx::Powering(BOOL bP, BOOL bB, BYTE nDir, _TV_ vTbeB, BOOL bMask248)
{
	// very important, TC0 has reverse direction about TC1
	// nDir = 1 -> forward, 2 -> reverse
	if (c_tcRef.real.nAddr > 2 && nDir != 0)	nDir ^= 3;
	c_tcRef.real.power.inst.b.f =
	c_dozMain.tcs[OWN_SPACE].real.cV3f.inst.b.f = nDir == 1 ? TRUE : FALSE;
	c_tcRef.real.power.inst.b.r =
	c_dozMain.tcs[OWN_SPACE].real.cV3f.inst.b.r = nDir == 2 ? TRUE : FALSE;
	c_tcRef.real.power.inst.b.p =
	c_dozMain.tcs[OWN_SPACE].real.cV3f.inst.b.p =
	c_dozMain.tcs[OWN_SPACE].real.cEcu.inst.b.p = bP;
	c_tcRef.real.power.inst.b.b =
	c_dozMain.tcs[OWN_SPACE].real.cV3f.inst.b.b =
	c_dozMain.tcs[OWN_SPACE].real.cEcu.inst.b.b = bB;
	c_tcRef.real.power.vTbeL =
	c_dozMain.tcs[OWN_SPACE].real.cV3f.vTbeL =
	c_dozMain.tcs[OWN_SPACE].real.cEcu.vTbeL = GETLWORD(vTbeB);
	// for alarm 51
	if (bP) {
		if (!(c_tcRef.real.power.nCond & COND_POWERSTART)) {
			InterTrigger(&c_tcRef.real.power.wTimer, TIME_POWERMONIT);
			c_tcRef.real.power.nCond |= COND_POWERSTART;
		}
		// Modified 2013/11/02
		// Modified 2013/01/11
		//if (!GETCBIT(__AUXA, CCDIB_PANPS1) && !GETCBIT(__AUXA, CCDIB_PANPS2) &&
		//		!GETCBIT(__AUXB, CCDIB_PANPS1) && !GETCBIT(__AUXB, CCDIB_PANPS2))
		if (!bMask248) {
			if (c_tcRef.real.ptflow.nPos != (FRONT_PANPS1 | FRONT_PANPS2 | REAR_PANPS1 | REAR_PANPS2))
				SHOTB(SCENE_ID_HEADCAR, 248);
			else	CUTB(SCENE_ID_HEADCAR, 248);
		}
	}
	else {
		InterTrigger(&c_tcRef.real.power.wTimer, 0);
		c_tcRef.real.power.nCond &= ~COND_POWERSTART;
		CUTB(SCENE_ID_HEADCAR, 248);
	}
}

void CTcx::PoweringMask()
{
	//Powering(FALSE, TRUE, TRUE, (_TV_)0);
	c_tcRef.real.power.inst.b.p =
		c_dozMain.tcs[OWN_SPACE].real.cV3f.inst.b.p =
			c_dozMain.tcs[OWN_SPACE].real.cEcu.inst.b.p = FALSE;
	c_tcRef.real.power.vTbeL =
		c_dozMain.tcs[OWN_SPACE].real.cV3f.vTbeL =
			c_dozMain.tcs[OWN_SPACE].real.cEcu.vTbeL = 0;
}

SHORT CTcx::TbeToPercent(_TV_ v)
{
	BOOL sign;
	WORD max;
	if (v < 0) {
		sign = TRUE;
		max = TEBEMAX_BRAKE;
		v = -v;
	}
	else {
		sign = FALSE;
		max = TEBEMAX_POWER;
	}
	SHORT per = v * 100 / max;
	if (sign)	per = -per;
	return per;
}

void CTcx::RecoverAllBrake()
{
	c_artop.RecoverAll();
}

void CTcx::LcdCtrl(BOOL side, BOOL cmd)
{
	WORD lcd = side ? CCDOB_LRR2 : CCDOB_LRR1;
	BOOL ctrl = cmd ? FALSE : TRUE; // if set output so LCD to off
	ObsCtrl(ctrl, lcd, c_tcRef.real.cid.wAll);
}

void CTcx::AnnalsMrp(BOOL state, BYTE nCid, WORD wMsg)
{
	if (state)	ShotMrp(nCid, wMsg);
	else	Cut(nCid, wMsg, wMsg);
}

void CTcx::Annals(BOOL state, BYTE nCid, WORD wMsg)
{
	if (state)	Shot(nCid, wMsg);
	else	Cut(nCid, wMsg, wMsg);
}

void CTcx::ShotMrp(BYTE nCid, WORD wMsg)
{
	if (c_tcRef.real.ptflow.nState & PS_ENABLEALARM)
		c_scene.Shot(nCid, wMsg, &c_devTime);
}

void CTcx::ShotMainDev(BYTE nCid, WORD wMsg)
{
	// Modified 2012/12/05
	//if (IsAtcWarn())	c_scene.Shot(nCid, wMsg, &c_devTime);
	if (IsPermitMainB())	c_scene.Shot(nCid, wMsg, &c_devTime);
}

void CTcx::ShotAidDev(BYTE nCid, WORD wMsg)
{
	if (IsPermitAidB())	c_scene.Shot(nCid, wMsg, &c_devTime);
}

// Modified 2012/12/10
void CTcx::Primary()
{
	c_door.Shut();
	c_tcRef.real.door.nMode = DOORMODE_UNKNOWN;
	// Modified 2013/01/03
	c_tcRef.real.door.nCuri = c_tcRef.real.door.nDeb = 0;

	memset((PVOID)&c_tcRef.real.revolt, 0, sizeof(REVOLT));
	memset((PVOID)&c_dumb, 0, sizeof(DUMBDEV));
	c_scene.ShutBuzz();
	if (c_tcRef.real.nObligation & OBLIGATION_MAIN)	c_panel.SetMain(TRUE);
	else	c_panel.SetMain(FALSE);

	// Modified 2013/03/11
	c_tcRef.real.du.nDownloadItem = c_tcRef.real.du.nSel = 0;
	c_tcRef.real.es.nMasterDuty = c_tcRef.real.es.nTargetDuty = 0xff;
	c_tcRef.real.nState &= ~(STATE_DOWNLOADING | STATE_PROGRESSING);
}
//void CTcx::PanelActive()
//{
//	if (c_tcRef.real.nObligation & OBLIGATION_MAIN)		c_panel.SetMain(TRUE);
//	else	c_panel.SetMain(FALSE);
//}

// Appended 2013/11/02
void CTcx::PrintEnv(UCURV from, BOOL rw)
{
	printf("\r\n");
	//MSGLOG("    ------ ENV. INFOMATION at READ ------\r\n");
	if (rw)
		MSGLOG("    ------ ENV. INFOMATION at READ ------\r\n");
	else	MSGLOG("    ------ ENV. INFOMATION at WRITE%d-----\r\n", from);
	MSGLOG("    | Total running distance | %8u |\r\n", (DWORD)c_env.real.entire.elem.dbDist);
	MSGLOG("    | Total running time     | %8u |\r\n", (DWORD)c_env.real.entire.elem.dbTime);
	MSGLOG("    | Siv power consumption  | %8u |\r\n", (DWORD)c_env.real.entire.elem.dbSivPower);
	MSGLOG("    | Power comsumption      | %8u |\r\n", (DWORD)c_env.real.entire.elem.dbV3fPower);
	MSGLOG("    | Regenerative power     | %8u |\r\n", (DWORD)c_env.real.entire.elem.dbRevival);
	MSGLOG("    | Train no.              | %8u |\r\n", (DWORD)c_env.real.entire.elem.wTrainNo);
	MSGLOG("    | Last time     - 20%02x/%02x/%02x %02x:%02x:%02x\r\n",
			c_env.real.entire.elem.devTime.year, c_env.real.entire.elem.devTime.month, c_env.real.entire.elem.devTime.day,
			c_env.real.entire.elem.devTime.hour, c_env.real.entire.elem.devTime.minute, c_env.real.entire.elem.devTime.sec);
	MSGLOG("    -------------------------------------\r\n");
}

// Modified 2013/03/25
void CTcx::RemoveRescueOutput()
{
	CLRTBIT(OWN_SPACE, TCDOB_ROLR);
	CLRTBIT(OWN_SPACE, TCDOB_ROPR);
	CLRTBIT(OWN_SPACE, TCDOB_ROBR);
	CLRNBIT(OWN_SPACE, NESTED_ROLR);
	CLRNBIT(OWN_SPACE, NESTED_ROPR);
	CLRNBIT(OWN_SPACE, NESTED_ROBR);
}

void CTcx::EntryDriverless(BOOL mode)
{
	InterTrigger(&c_tcRef.real.op.wWaitSignalAutoA, TIME_WAITATCDMBIT);
	InterTrigger(&c_tcRef.real.op.wWaitSignalAutoB, TIME_WAITATCKUP);
	CLRTBIT(OWN_SPACE, TCDOB_DPLP);	// mode xch
	if (mode)	CLRTBIT(OWN_SPACE, TCDOB_HCR);
	ToFlash(TCDOB_DRML, TRUE);
	if (GETTBIT(OWN_SPACE, TCDIB_C) && !GETTBIT(OWN_SPACE, TCDIB_F) && !GETTBIT(OWN_SPACE, TCDIB_R))
		c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_INTROB;
	else	c_tcRef.real.op.nAutoStep = DRIVERLESSSTEP_INTRO;
	c_tcRef.real.op.nAutoDeb = DEBOUNCE_KUP;		// kup deb
}

void CTcx::ClearDistance()
{
	c_tcRef.real.intg.dbDist = 0.f;			// distance between stations
	c_tcRef.real.intg.dbDistLive = 0.f;		// total distance at HCR
	c_tcRef.real.intg.dbSivPowerLive = 0.f;	// total power at HCR
	c_tcRef.real.intg.dbV3fPowerLive = 0.f;
}

void CTcx::ObsCtrl(BOOL bState, UCURV uID, WORD wCarMap)
{
	for (UCURV n = 0; n < LENGTH_CC; n++) { // c_tcRef.real.cf.nLength
		if (wCarMap & (1 << n)) {
			if (bState)	SETOBIT(OWN_SPACE, n, uID);
			else	CLROBIT(OWN_SPACE, n, uID);
		}
	}
}

void CTcx::ToFlash(WORD to, BOOL cmd)
{
	if (to >= FLASH_MAX)	return;
	if (cmd)	SETTBIT(OWN_SPACE, to);
	else	CLRTBIT(OWN_SPACE, to);
	c_wFlashT[to] = cmd ? TIME_FLASH : 0;
}

void CTcx::CoFlash(WORD co, BOOL cmd)
{
	if (co >= CCDOB_MAX)	return;
	ObsCtrl(cmd, co, c_tcRef.real.cid.wAll);
	c_wFlashO[co] = cmd ? TIME_FLASH : 0;
}

//===== Files ====================================================================
// Not used SaveArchives()
void CTcx::SaveArchives()	// not used
{
	if (!(c_tcRef.real.nProperty & PROPERTY_LOGMASKATFILESAVE) && c_pBucket != NULL) {
		MSGLOG("[TC]Start saving...\r\n");
		c_scene.Cancel(&c_devTime);
		c_tcRef.real.nProperty |= PROPERTY_LOGMASKATFILESAVE;
		DEVTIME dt;
		memcpy((PVOID)&dt, (PVOID)&c_devTime, sizeof(DEVTIME));
		c_pBucket->src.dt.year = ToHex((BYTE)dt.year);
		c_pBucket->src.dt.month = ToHex((BYTE)dt.month);
		c_pBucket->src.dt.day = ToHex((BYTE)dt.day);
		c_pBucket->src.dt.hour = ToHex((BYTE)dt.hour);
		c_pBucket->src.dt.minute = ToHex((BYTE)dt.minute);
		c_pBucket->src.dt.sec = ToHex((BYTE)dt.sec);
		PARM_ARCH pArm = c_scene.GetAlarmArch();
		DWORD arms = c_tcRef.real.scArm.dwIndex;	//c_scene.GetAlarmArchSize();
		SHORT res = c_scene.SaveAlarm(pArm, arms, ARM_PATHT);
		if (res != FILEERROR_NON)
			MSGLOG("[TC]ERROR:Alarm save(%d).\r\n", res);
		Lock();
		RESET_BEILACK();
		c_beilTriggerTime = ClockCycles();
		BeilCmd(BEILCMD_LOGBOOKTOMEM);			// need respond from beil
		c_tcRef.real.scLog.nMainStep = BEILSTEP_SAVELOG;
		InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT);
		Unlock();
	}
}

void CTcx::SaveArchives(DWORD dwArmID)
{
	if (!(c_tcRef.real.nProperty & PROPERTY_LOGMASKATFILESAVE) && c_pBucket != NULL) {
		MSGLOG("[TC]Start saving...\r\n");
		c_scene.Cancel(dwArmID, &c_devTime);
		c_tcRef.real.nProperty |= PROPERTY_LOGMASKATFILESAVE;
		DEVTIME dt;
		memcpy((PVOID)&dt, (PVOID)&c_devTime, sizeof(DEVTIME));
		c_pBucket->src.dt.year = ToHex((BYTE)dt.year);
		c_pBucket->src.dt.month = ToHex((BYTE)dt.month);
		c_pBucket->src.dt.day = ToHex((BYTE)dt.day);
		c_pBucket->src.dt.hour = ToHex((BYTE)dt.hour);
		c_pBucket->src.dt.minute = ToHex((BYTE)dt.minute);
		c_pBucket->src.dt.sec = ToHex((BYTE)dt.sec);
		PARM_ARCH pArm = c_scene.GetAlarmArch();
		SHORT res = c_scene.SaveAlarm(pArm, dwArmID, ARM_PATHT);
		if (res != FILEERROR_NON)
			MSGLOG("[TC]ERROR:Alarm save(%d).\r\n", res);
		Lock();
		RESET_BEILACK();
		c_beilTriggerTime = ClockCycles();
		BeilCmd(BEILCMD_LOGBOOKTOMEM);			// need respond from beil
		c_tcRef.real.scLog.nMainStep = BEILSTEP_SAVELOG;
		InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT);
		Unlock();
	}
}

SHORT CTcx::SaveTrace(UCURV di, UCURV ci, BYTE* pTrace, DWORD dwLength)
{
	return c_scene.SaveTrace(di, ci, pTrace, dwLength);
}

SHORT CTcx::SaveEnv(UCURV from)
{
	SHORT res = 0;
	if (OWN_SPACE != 0 || (c_tcRef.real.nState & STATE_SAVEENVF)) {
		memcpy((PVOID)&c_env.real.entire.elem.devTime.year, (PVOID)&c_devTime.year, sizeof(DEVTIME));
		res = c_scene.SaveEnvToTemp(&c_env);
		if (res >= 0) {
			//c_tcRef.real.scArm.nState |= ARMSTATE_SAVEENV;
			//c_pBucket->ack = BEILACK_NON;
			// Modified 2013/01/09
			//BeilCmd(BEILCMD_WRITEENV);		// not need respond from beil
			c_pBucket->wTicker |= BEILTICKER_ENV;
		}
		c_tcRef.real.nState &= ~STATE_SAVEENVF;
		c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_ENVIROMENTSET;		// master's cmd or slave's respond
		if (OWN_SPACE == 0)	InterTrigger(&c_tcRef.real.cond.wEnvSetWait, WAIT_ENVSET);
		SETTBIT(OWN_SPACE, TCDOB_ESET);		// by save
		PrintEnv(from, FALSE);
	}
	return res;
}

void CTcx::LoadEnv()
{
	c_scene.LoadEnv(&c_env);
	c_tcRef.real.nState |= STATE_VALIDENVF;
	c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_ENVIROMENTSET;		// master's cmd
	InterTrigger(&c_tcRef.real.cond.wEnvSetWait, WAIT_ENVSET);
	SETTBIT(OWN_SPACE, TCDOB_ESET);		// by load
	PrintEnv(0, TRUE);
}

void CTcx::SaveDrv(PDRV_ARCH pDrv)
{
	if (c_pBucket != NULL) {
		c_drvTriggerTime = ClockCycles();
		c_pBucket->nPeriodAck = 0;
		memcpy((PVOID)&c_pBucket->drv, (PVOID)pDrv, sizeof(DRV_ARCH));
		c_pBucket->wTicker |= BEILTICKER_DRV;
		BeilCmd(BEILCMD_APPENDDRV);
		c_tcRef.real.scLog.nPeriodStep = BEILSTEP_APPENDDRV;
		InterTrigger(&c_tcRef.real.scLog.wPeriodTimer, TIME_BEILWAIT / 5);
	}
}

DWORD CTcx::PickAlarm()
{
	PARM_ARCH pArm = c_scene.GetAlarmArch();
	Lock();
	DWORD dwIndex = c_tcRef.real.scArm.dwIndex;
	Unlock();
	DWORD dwSize = dwIndex * sizeof(ARM_ARCH);
	BYTE* pCopied = (BYTE*)malloc(dwSize);
	if (pCopied != NULL) {
		memcpy((PVOID)pCopied, (PVOID)pArm, dwSize);
		dwSize = c_scene.PickAlarm((PARM_ARCH)pCopied, dwIndex, ARM_PATHCT);
		free(pCopied);
		return dwSize;
	}
	MSGLOG("[TC]ERROR:malloc %ldbytes.\r\n", dwSize);
	return 0;
}

void CTcx::DisjointTime(DWORD day, DWORD sec)
{
	c_pBucket->src.dt.year = (BYTE)(day / 10000);
	day %= 10000;
	c_pBucket->src.dt.month = (BYTE)(day / 100);
	c_pBucket->src.dt.day = (BYTE)(day % 100);
	c_pBucket->src.dt.hour = (BYTE)(sec / 10000);
	sec %= 10000;
	c_pBucket->src.dt.minute = (BYTE)(sec / 100);
	c_pBucket->src.dt.sec = (BYTE)(sec % 100);
}

void CTcx::SectQuantity(DWORD dwDay)
{
	if (c_pBucket != NULL) {
		DisjointTime(dwDay, 0);
		Lock();
		c_beilTriggerTime = ClockCycles();
		BeilCmd(BEILCMD_SECTQUANTITY);
		c_tcRef.real.scLog.nMainStep = BEILSTEP_SECTQUANTITY;
		InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT / 5);
		Unlock();
	}
	else {
		MSGLOG("[TC]ERROR:at sect quantity.\r\n");
		c_mate.PassiveClose();
	}
}

// Passive
void CTcx::ReadSect(BYTE nDirectoryID, BYTE nFileID, DWORD dwOffset)
{
	if (c_pBucket != NULL && nDirectoryID < GetFilesUnderDayLength()) {
		c_tcRef.real.es.dwSelectedTime = GetSelectedUnderDay((WORD)nDirectoryID);
		DisjointTime(c_tcRef.real.es.dwSelectedDay, c_tcRef.real.es.dwSelectedTime);
		c_pBucket->wFileID = (WORD)nFileID;
		c_pBucket->dwOffset = dwOffset;
		Lock();
		RESET_BEILACK();
		c_beilTriggerTime = ClockCycles();
		BeilCmd(BEILCMD_READSECT);			// need respond from beil
		c_tcRef.real.scLog.nMainStep = BEILSTEP_READSECT;
		InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT / 5);
		Unlock();
	}
	else {
		MSGLOG("[TC]ERROR:at read sect %d of %d.\r\n", nDirectoryID, GetFilesUnderDayLength());
		c_mate.PassiveClose();
	}
}

// Active
void CTcx::WriteSect(BYTE nDirectoryID, BYTE nFileID, DWORD dwOffset)
{
	if (c_pBucket != NULL && nDirectoryID < GetFilesUnderDayLength()) {
		c_tcRef.real.es.dwSelectedTime = GetSelectedUnderDay((WORD)nDirectoryID);
		DisjointTime(c_tcRef.real.es.dwSelectedDay, c_tcRef.real.es.dwSelectedTime);
		c_pBucket->wFileID = (WORD)nFileID;
		c_pBucket->dwOffset = dwOffset;
		Lock();
		RESET_BEILACK();
		//if (c_tcRef.real.es.nTargetAddr < 3)	c_pBucket->wState &= ~BEILSTATE_TC1;	// invert at beil
		if (c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nTargetDuty] < 3)
			c_pBucket->wState &= ~BEILSTATE_TC1;	// invert at beil
		else	c_pBucket->wState |= BEILSTATE_TC1;
		//if (c_tcRef.real.es.nTargetAddr == 2 || c_tcRef.real.es.nTargetAddr == 4)	c_pBucket->wState |= BEILSTATE_LIU2;
		if (c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nTargetDuty] == 2 ||
				c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nTargetDuty] == 4)
			c_pBucket->wState |= BEILSTATE_LIU2;
		else	c_pBucket->wState &= ~BEILSTATE_LIU2;
		c_beilTriggerTime = ClockCycles();
		BeilCmd(BEILCMD_WRITESECT);			// need respond from beil
		c_tcRef.real.scLog.nMainStep = BEILSTEP_WRITESECT;
		InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT / 5);
		Unlock();
	}
	else {
		MSGLOG("[TC]ERROR:at write sect. %d of %d.\r\n", nDirectoryID, GetFilesUnderDayLength());
		c_mate.ActiveClose();
	}
}

void CTcx::CopyEachDevError(BYTE* p)
{
	Lock();
	memcpy((PVOID)p, (PVOID)&c_nEachDevError, SIZE_EACHDEV);
	memset((PVOID)&c_nEachDevError, 0, SIZE_EACHDEV);
	Unlock();
}

void CTcx::BeilTimeRecorder(const char* pMsg)
{
	uint64_t fe = ClockCycles();
	double sec = (double)(fe - c_beilTriggerTime) / (double)c_tcRef.real.cps;
	MSGLOG("%s %d (%dms).\r\n", pMsg, c_pBucket->iRespond, (WORD)(sec * 1000.f));
}

void CTcx::DirectoryList()
{
	RESET_BEILACK();
	c_beilTriggerTime = ClockCycles();
	c_tcRef.real.nProperty |= PROPERTY_LOGMASKATDOWNLOAD;
	BeilCmd(BEILCMD_MAKEDIRECTORYLIST);
	c_tcRef.real.scLog.nMainStep = BEILSTEP_SORTBYDAY;
	InterTrigger(&c_tcRef.real.scLog.wMainTimer, TIME_BEILWAIT / 5);
}

void CTcx::ScanUsb()
{
	if (c_pBucket != NULL) {
		// c_pBucket->ack = BEILACK_NON;
		BeilCmd(BEILCMD_SEARCHUSB);		// not need respond from beil
		if (c_pBucket->wState & BEILSTATE_BEENUSB)
			c_tcRef.real.nState |= STATE_BEENUSB;
		else	c_tcRef.real.nState &= ~STATE_BEENUSB;
	}
}

void CTcx::CampUsb()
{
	if (c_pBucket != NULL) {
		if (c_tcRef.real.nAddr < 3)	c_pBucket->wState |= BEILSTATE_TC1;
		else	c_pBucket->wState &= ~BEILSTATE_TC1;
		if (c_tcRef.real.nAddr == 2 || c_tcRef.real.nAddr == 4)	c_pBucket->wState |= BEILSTATE_LIU2;
		else	c_pBucket->wState &= ~BEILSTATE_LIU2;
	}
}

// Modified 2013/03/04
void CTcx::BeilCmd(WORD cmd)
{
	if (cmd == BEILCMD_SELTOUSB || cmd == BEILCMD_INSPECTTOUSB || cmd == BEILCMD_TRACETOUSB ||
			cmd == BEILCMD_TRIALTOUSB || cmd == BEILCMD_WRITESECT/* || cmd == BEILCMD_APPENDDRV*/) {
		DWORD dw = GetThumbWheel();
		c_pBucket->drv.wFno = MAKEWORD((BYTE)((dw >> 8) & 0xff), (BYTE)(dw & 0xff));
	}
	CmdLine(cmd);
}

WORD CTcx::GetBucketProgress()
{
	switch (c_tcRef.real.es.nDownloadMethod) {
	case DOWNLOADMETHOD_SELF :
		return c_pBucket->wProgress;
		break;
	case DOWNLOADMETHOD_OPPO :
		return c_mate.GetProgress();
		break;
	default :
		return c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wProgress;
		break;
	}
	//if (c_tcRef.real.es.nDownloadStep == DOWNLOADSTEP_NON)
	//	return c_pBucket->wProgress;
	//else return c_mate.GetProgress();
}

//===== Miscellaneous ====================================================================
void CTcx::PutDistanceFair(WORD pulse, double sp)
{
	Lock();
	c_df.pulse += (DWORD)pulse;
	c_df.speed += sp;
	++ c_df.cycle;
	//c_tcRef.dummy[0] = HIGHBYTE(c_df.cycle);
	//c_tcRef.dummy[1] = LOWBYTE(c_df.cycle);
	Unlock();
}

void CTcx::GetDistanceFair(DWORD* pPulse, double* pSp)
{
	Lock();
	if (c_df.cycle > 0) {
		*pPulse = c_df.pulse / (DWORD)c_df.cycle;
		*pSp = c_df.speed / (double)c_df.cycle;
	}
	else {
		*pPulse = 0;
		*pSp = 0.f;
	}
	memset((PVOID)&c_df, 0, sizeof(DISTANCEFAIR));
	Unlock();
}

BOOL CTcx::TimeRegister(WORD* pTimer)
{
	UCURV n;
	for (n = 0; n < TIMETABLES_MAX; n ++) {
		if (c_pTimeTables[n] == NULL)
			break;
	}
	if (n < TIMETABLES_MAX) {
		c_pTimeTables[n] = pTimer;
		// Inserted 2012/01/16
		c_tcRef.real.wInterTimerLength = n;
		return TRUE;
	}
	return FALSE;
}

void CTcx::InterTrigger(WORD* pTimer, WORD t)
{
	*pTimer = t;
}

void CTcx::GetTbTime(PTBTIME pTt)
{
	memcpy((PVOID) pTt, (PVOID) &c_tbTime, sizeof(TBTIME));
}

void CTcx::RecordTime(UCURV n)
{
	RECTIME(n);
}

void CTcx::CalcTimes()
{
	RECTIME(TTIME_CYCLEEND);
	if (c_tcRef.real.cycle.wState & TCSTATE_MSRTIME)
		c_spy.ReportTime(c_tcRef.real.cps, c_opTimes);
	c_opTimes[TTIME_BEGIN].cur = c_opTimes[TTIME_CYCLEEND].cur;
	c_tcRef.real.cycle.wState |= TCSTATE_MSRTIME;
}

double CTcx::TimeToSec(PDEVTIME pDt)
{
	double db = (double)ToHex(pDt->year) * (400.f * 86400.f) +
				((double)ToHex(pDt->month) - 1.f) * (31.f * 86400.f) +
				((double)ToHex(pDt->day) - 1.f) * 86400.f +
				(double)ToHex(pDt->hour) * 3600.f +
				(double)ToHex(pDt->minute) * 60.f +
				(double)ToHex(pDt->sec);
	return db;
}

void CTcx::LongToByte(BYTE* p, long v)
{
	*p ++ = (BYTE)((v >> 24) & 0xff);
	*p ++ = (BYTE)((v >> 16) & 0xff);
	*p ++ = (BYTE)((v >> 8) & 0xff);
	*p = (BYTE)(v & 0xff);
}

int CTcx::ConvS(WORD psc, int pscMin, int pscMax, int logMin, int logMax)
{
	WORD wAbs;
	BOOL bSign;
	if (psc & 0x8000) {
		wAbs = 0 - psc;
		bSign = TRUE;
	}
	else {
		wAbs = psc;
		bSign = FALSE;
	}
	int cpsc = (int)wAbs;
	if (bSign)	cpsc = 0 - cpsc;
	return ((SHORT)((logMax - logMin) * (cpsc - pscMin) / (pscMax - pscMin) + logMin));
}

WORD CTcx::Conv(WORD wPsc, WORD wPscMin, WORD wPscMax, WORD wLogMin, WORD wLogMax)
{
	return ((wLogMax - wLogMin) * (wPsc - wPscMin) / (wPscMax - wPscMin) + wLogMin);
}

double CTcx::Conv(WORD wPsc, WORD wPscMin, WORD wPscMax, double min, double max)
{
	return (double)((max - min) * (double)(wPsc - wPscMin) / (double)(wPscMax - wPscMin) + min);
}

DWORD CTcx::ToBcd(DWORD dw)
{
	dw %= 100000000;
	return (((ToBcd(WORD(dw / 10000))) << 16) | ToBcd((WORD)(dw % 10000)));
}

WORD CTcx::ToBcd(WORD w)
{
	w %= 10000;
	return (((ToBcd((BYTE)(w / 100))) << 8) | ToBcd((BYTE)(w % 100)));
}

BYTE CTcx::ToBcd(BYTE n)
{
	n %= 100;
	return (((n / 10) << 4) | (n % 10));
}

WORD CTcx::ToBcd(BYTE* p, UCURV length)
{
	WORD w = 0;
	while (length > 0) {
		w <<= 4;
		w |= (WORD)(*p - '0');
		++ p;
		-- length;
	}
	return w;
}

WORD CTcx::ToHex(WORD bcd)
{
	WORD w = ToHex((BYTE)(bcd >> 8));
	w *= 100;
	w += ToHex((BYTE)(bcd & 0xff));
	return w;
}

BYTE CTcx::ToHex(BYTE bcd)
{
	BYTE hex = ((bcd >> 4) & 0xf) * 10;
	hex += (bcd & 0xf);
	return hex;
}

DWORD CTcx::DevideBy10(DWORD dw)
{
	DWORD quotient = dw / 10;
	DWORD remainder = dw % 10;
	if (remainder >= 5)	++ quotient;
	return quotient;
}

double CTcx::DevideBy10(double db)
{
	double quotient = db / 10.f;
	double remainder = fmod(fabs(db), 10.f);
	if (remainder >= 5.f) {
		if (db > 0.f)	quotient += 1.0f;
		else if (db < 0.f)	quotient -= 1.0f;
	}
	return quotient;
}

BYTE CTcx::GetBitQuantity(WORD w, BOOL bState)
{
	BYTE sum = 0;
	WORD bitw = 1;
	for (UCURV n = 0; n < 16; n ++) {
		if (bState && (w & bitw))	++ sum;
		else if (!bState && !(w & bitw))	++ sum;
		bitw <<= 1;
	}
	return sum;
}

void CTcx::ArmNameParse(WORD* pYearSec, WORD* pDate, WORD* pTime)
{
	WORD year = ToBcd(&c_armTimeText[1], 2);
	WORD mon = ToBcd(&c_armTimeText[3], 2);
	WORD day = ToBcd(&c_armTimeText[5], 2);
	WORD hour = ToBcd(&c_armTimeText[7], 2);
	WORD min = ToBcd(&c_armTimeText[9], 2);
	WORD sec = ToBcd(&c_armTimeText[11], 2);
	*pYearSec = (year << 8) | sec;
	*pDate = (mon << 8) | day;
	*pTime = (hour << 8) | min;
}

//=========================================================================
// bus master shell execute this code
int CTcx::PeriodicSnap()
{
	memcpy((PVOID)&c_dozBkup, (PVOID)&c_dozMain, sizeof(TCDOZ));
	CalcTimes();
	LocalErrorToBus();
	c_pack.Package();
	uint64_t bt = ClockCycles();
	if (c_busTime > 0) {
		double sec = (double)(bt - c_busTime) / (double)c_tcRef.real.cps;
		c_tcRef.real.bm.wTime = (WORD)(sec * 1000.f);
		c_busTime = bt;
	}
	RECTIME(TTIME_TCOPEN);
	int res = 0;
	c_dozMain.tcs[CAR_HEAD].real.nHcrInfo = c_tcRef.real.hcr.nMasterID;
	//HcrInfo(c_tcRef.real.hcr.nMasterID, c_dozMain.tcs[OWN_SPACE].real.nHcrInfo, 5);	// ?????
	if (c_dozMain.tcs[CAR_HEAD].real.nHcrInfo == 0) {	// ?????
		MSGLOG("[TC]CAUTION:HCR INFO A.\r\n");
	}
	if (c_tcRef.real.cycle.wState & TCSTATE_XCHDRVLESS)
		c_dozMain.tcs[CAR_HEAD].real.nHint |= HINT_DELAYATC;
	else	c_dozMain.tcs[CAR_HEAD].real.nHint &= ~HINT_DELAYATC;
	c_doc.AppendHead();
	DWORD dwOrderID = 0;
	//if (c_busA.wState & BUSSTATE_ENABLE) {
	if (c_tcRef.real.cycle.nBusState[0] & BUSSTATE_ENABLE) {
		c_busA.pLine->ClearFirmWithoutZero();
		res = c_busA.pLine->Send((BYTE*)&c_dozMain.tcs[CAR_HEAD], dwOrderID);
		dwOrderID = c_busA.pLine->GetOrderID();
		//c_busA.wState |= BUSSTATE_WAIT;
		c_tcRef.real.cycle.nBusState[0] |= BUSSTATE_WAIT;
		BusTrace(OWN_SPACE, TRUE);
	}
	//if ((c_busB.wState & BUSSTATE_ENABLE) && !(c_tcRef.real.nActiveBus & INHIBITBUS_B)) {
	if ((c_tcRef.real.cycle.nBusState[1] & BUSSTATE_ENABLE) && !(c_tcRef.real.nActiveBus & INHIBITBUS_B)) {
		c_busB.pLine->ClearFirmWithoutZero();
		res = c_busB.pLine->Send((BYTE*)&c_dozMain.tcs[CAR_HEAD], dwOrderID);
		//c_busB.wState |= BUSSTATE_WAIT;
		c_tcRef.real.cycle.nBusState[1] |= BUSSTATE_WAIT;
		BusTrace(OWN_SPACE, FALSE);
	}
	RECTIME(TTIME_TCOPENED);
	return res;
}

BOOL CTcx::SendPert(BYTE cmd)
{
	if (cmd != PERTMSG_AGAIN)	c_tcRef.real.es.nRespCounter = 0;
	else if (++ c_tcRef.real.es.nRespCounter >= COUNT_COMMFAULT) {	// always active
		//if (c_tcRef.real.es.nMasterAddr == c_tcRef.real.nAddr)	c_mate.ActiveClose();
		if (c_tcRef.real.es.nMasterDuty < LENGTH_TC &&
				c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nMasterDuty] == c_tcRef.real.nAddr)
			c_mate.ActiveClose();
		else	c_mate.PassiveClose();
		MSGLOG("[TC]ERROR:End download from bus cause timeout.\r\n");
		return FALSE;
	}
	c_busB.pLine->Dispatch((WORD)(cmd == PERTMSG_ACK ?
			c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nMasterDuty] :
			c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nTargetDuty]),
			(PVOID)&c_pertSend);
	Lock();
	if (cmd != PERTMSG_ACK) {
		c_tcRef.real.es.nDownloadStep |= DOWNLOADSTEP_WAITRESPOND;
		InterTrigger(&c_tcRef.real.es.wSyncTimer, TIME_WAITBUSBRESPOND);
	}
	else {
		c_tcRef.real.es.nDownloadStep &= ~DOWNLOADSTEP_WAITRESPOND;
		InterTrigger(&c_tcRef.real.es.wSyncTimer, 0);
	}
	Unlock();
	return TRUE;
}

BOOL CTcx::Run(int aich, int bkdis, int sdis)
{
	MSGLOG("\r\n\r\nTcx v%.2f\r\nRebuild %ld\r\n", VERSION_TCX, REBUILD_TCX);
	c_tcRef.real.nProperty = 0;
	if (aich > 0) {
		c_tcRef.real.nProperty |= PROPERTY_ATLABORATORY;
		c_tcRef.real.cond.nAichDsv = TCAI_CH0;
		c_tcRef.real.cond.nAichBat = TCAI_CH1;
	}
	else {
		c_tcRef.real.nProperty &= ~PROPERTY_ATLABORATORY;
		c_tcRef.real.cond.nAichDsv = TCAI_CH2;
		c_tcRef.real.cond.nAichBat = TCAI_CH0;
	}
	if (bkdis > 0) {
		c_tcRef.real.nProperty |= PROPERTY_DISABLEBACKUP;
		MSGLOG("[INIT]Backup disable mode.\n");
	}
	if (sdis > 0) {
		c_tcRef.real.nProperty |= PROPERTY_DISABLESWWATCHDOG;
		MSGLOG("[INIT]Watchdog disable mode.\n");
	}

	if (!TakingOff()) {
		if (c_bMfb)	fin_mfb();
		return FALSE;
	}

	printf("\r\n");
	MSGLOG("    --------- BASIC INFOMATION ----------\r\n");
	MSGLOG("    | Enviroment data size   | %8u |\r\n", sizeof(ENVINFOEXTENSION));
	MSGLOG("    | Alarm arch. size       | %8u |\r\n", sizeof(ARM_ARCH));
	MSGLOG("    | Operate arch. size     | %8u |\r\n", sizeof(DRV_ARCH));
	MSGLOG("    | Trial arch. size       | %8u |\r\n", sizeof(TRIAL_ARCH));
	MSGLOG("    | Bucket size            | %8u |\r\n", sizeof(BUCKET));
	MSGLOG("    | Debug report length    | %8u |\r\n", c_spy.GetReportLength());
	MSGLOG("    | Inspect data length    | %8u |\r\n", sizeof(INSPECTSPEC));
	MSGLOG("    | ATC trace data length  | %8u |\r\n", sizeof(ATCSTABLE));
	MSGLOG("    | ATO trace data length  | %8u |\r\n", sizeof(ATOSTABLE));
	MSGLOG("    | SIV trace data length  | %8u |\r\n", sizeof(SIVSTABLE));
	MSGLOG("    | VVVf trace data length | %8u |\r\n", sizeof(V3FSTABLE));
	MSGLOG("    | ECU trace data length  | %8u |\r\n", sizeof(ECUSTABLE));
	MSGLOG("    | DU comm. monit length  | %8u |\r\n", sizeof(DUCOMMMONIT));
	MSGLOG("    | TCPERT length          | %8u |\r\n", sizeof(TCPERT));
	MSGLOG("    | MATE TEXT length       | %8u |\r\n", SIZE_MATETEXT);
	MSGLOG("    -------------------------------------\r\n");

	memset((PVOID) &c_opTimes[0], 0, sizeof(c_opTimes));
	c_tcRef.real.cycle.wXch = c_tcRef.real.nAddr < 3 ? CYCLE_XCHMASTER1 : CYCLE_XCHMASTER0;
	c_tcRef.real.hcr.nBuf = 0xff;
	c_tcRef.real.cycle.wState |= (TCSTATE_XCHMASTER | TCSTATE_FRAMEEND | TCSTATE_LOCALING | TCSTATE_LOCALPACK);
	return TRUE;
}

void CTcx::Destroy()
{
	if (c_pTimer != NULL) {
		c_pTimer->Release();
		KILL(c_pTimer);
	}
	if (c_bMfb) {
		fin_mfb();
		c_bMfb = FALSE;
	}
	if (g_mqPost > -1) {
		mq_close(g_mqPost);
		g_mqPost = -1;
	}
	if (g_mqBeil > -1) {
		mq_close(g_mqBeil);
		g_mqBeil = -1;
	}
	if (c_hShm > -1) {
		close(c_hShm);
		c_hShm = -1;
		c_pBucket = NULL;
	}
	if (c_iWatch > -1) {
		name_close(c_iWatch);
		c_iWatch = -1;
	}
	KILL(c_pLocal);
	KILL(c_busA.pLine);
	KILL(c_busB.pLine);
}

void CTcx::HcrInfo(BYTE nMasterID, BYTE nHcrInfo, WORD at)
{
	if (nMasterID == 0 || nHcrInfo == 0) {
		MSGLOG("[CAUTION]MasterID is %d, HcrInfo is %d, at %d.\r\n", nMasterID, nHcrInfo, at);
		if (nMasterID == 0)	SHOTB(SCENE_ID_HEADCAR, 160 + at);
		else	SHOTB(SCENE_ID_HEADCAR, 170 + at);
	}
}

//===== Receive & Mail ====================================================================
BOOL CTcx::ReceiveFrameZero(BOOL bCh)
{
	DWORD dwRecvOrderID;
	TCFIRM tcf;
	if (bCh) {
		//c_busA.wState |= BUSSTATE_WAIT;
		c_tcRef.real.cycle.nBusState[0] |= BUSSTATE_WAIT;
		c_busA.pLine->CombineZero((BYTE*)&tcf);
		//c_dozMain.tcs[OWN_SPACE].dummy[0] |= 1;
		dwRecvOrderID = c_busA.pLine->GetOrderID();
		if (c_dwRecvOrderID == dwRecvOrderID)	return FALSE;
	}
	else {
		//c_busB.wState |= BUSSTATE_WAIT;
		c_tcRef.real.cycle.nBusState[1] |= BUSSTATE_WAIT;
		c_busB.pLine->CombineZero((BYTE*)&tcf);
		tcf.real.wAddr |= 0x80;
		//c_dozMain.tcs[OWN_SPACE].dummy[0] |= 0x10;
		dwRecvOrderID = c_busB.pLine->GetOrderID();
		if (c_dwRecvOrderID == dwRecvOrderID)	return FALSE;
	}
	memcpy((PVOID)&c_dozMain.tcs[CAR_HEAD], (PVOID)&tcf, sizeof(TCFIRM));
	c_dwRecvOrderID = dwRecvOrderID;
	c_tcRef.real.nActiveBus &= ~ACTIVEBUS_ONLY;
	c_tcRef.real.nActiveBus |= bCh ? ACTIVEBUS_A : ACTIVEBUS_B;
	//c_dozMain.tcs[OWN_SPACE].dummy[0] |= bCh ? 2 : 0x20;
	return TRUE;
}

void CTcx::ReceiveAll()
{
	c_busA.pLine->Combine((BYTE*)&c_dozMain);
	if (!(c_tcRef.real.nActiveBus & INHIBITBUS_B))
		c_busB.pLine->Combine((BYTE*)&c_dozAux);
	else	memset((PVOID)&c_dozAux, 0, sizeof(TCDOZ));
	if (OWN_SPACE != 0) {	// only slave
		if (c_dozMain.tcs[CAR_HEAD].real.dwOrderID == 0 ||
				c_dozMain.tcs[CAR_HEAD].real.dwOrderID != c_dwRecvOrderID) {
			memcpy((PVOID)&c_dozMain.tcs[CAR_HEAD], (PVOID)&c_dozAux.tcs[CAR_HEAD], sizeof(TCFIRM));
			c_dozMain.tcs[CAR_HEAD].real.wAddr |= 0x80;
		}
	}
	for (UCURV n = CAR_HEADBK; n < CAR_TCFRONTORG; n ++) {
		if (!IsValidTcFirm(n)) {
			memcpy((PVOID)&c_dozMain.tcs[n], (PVOID)&c_dozAux.tcs[n], sizeof(TCFIRM));
			if (IsValidTcFirm(n))	c_dozMain.tcs[n].real.wAddr |= 0x80;
		}
	}
	for (UCURV n = 0; n < c_tcRef.real.cf.nLength; n ++) {
		if (!IsValidCcFirm(n)) {
			memcpy((PVOID)&c_dozMain.ccs[n], (PVOID)&c_dozAux.ccs[n], sizeof(CCFIRM));
			if (IsValidCcFirm(n))	c_dozMain.ccs[n].real.wAddr |= 0x80;
		}
	}
}

void CTcx::MaAccept(DWORD dwParam)
{
	c_spy.Nameplate();
}

void CTcx::MaSendBus(DWORD dwParam)
{
	BOOL bCh = (BOOL)dwParam;
	if (OWN_SPACE != 0) { // only slave
		RECTIME(TTIME_BUSSEND);
		CBusTrx* pBus = bCh ? c_busA.pLine : c_busB.pLine;
		c_dozMain.tcs[OWN_SPACE].real.wAddr = GETLWORD(c_tcRef.real.nAddr | BUSADDR_HIGH);
		c_dozMain.tcs[OWN_SPACE].real.nFlowNo = OWN_SPACE + 1;
		c_dozMain.tcs[OWN_SPACE].real.nHcrInfo = c_dozMain.tcs[CAR_HEAD].real.nHcrInfo;
		//HcrInfo(c_tcRef.real.hcr.nMasterID, c_dozMain.tcs[OWN_SPACE].real.nHcrInfo, 7);	// ?????
		c_dozMain.tcs[OWN_SPACE].real.dwOrderID = pBus->GetOrderID();
		BusTrace(OWN_SPACE, bCh);
		pBus->Send((PVOID)&c_dozMain.tcs[OWN_SPACE]);
		if ((bCh && (c_tcRef.real.nActiveBus & ACTIVEBUS_ONLY) == ACTIVEBUS_A) ||
				(!bCh && (c_tcRef.real.nActiveBus & ACTIVEBUS_ONLY) == ACTIVEBUS_B))
			memcpy((PVOID)&c_dozAux.tcs[OWN_SPACE], (PVOID)&c_dozMain.tcs[OWN_SPACE], sizeof(TCFIRM));
		RECTIME(TTIME_BUSSENDED);
	}
	else {
		// for time trigger by c_port..uWatchTime
		if (dwParam != 0)	c_busA.pLine->Send(NULL, 0);
		else	c_busB.pLine->Send(NULL, 0);
	}
}

void CTcx::MaReceiveBus(DWORD dwParam)
{
	WORD state = HIGHWORD(dwParam);
	BOOL bCh = BOOL(LOWWORD(dwParam));
	if (state == RECEIVE_FRAMEZERO) {
		if (c_tcRef.real.cycle.wState & TCSTATE_XCHMASTER) {
			// valid at power on - measuring at 2012/06/04
			if (!ReceiveFrameZero(bCh))	return;
			// Modified 2012/10/23
			//c_tcRef.real.cycle.wXch = CYCLE_XCHMASTER;
			c_tcRef.real.cycle.wXch = c_tcRef.real.nAddr < 3 ? CYCLE_XCHMASTER1 : CYCLE_XCHMASTER0;
			if (c_tcRef.real.hcr.nBuf != c_dozMain.tcs[CAR_HEAD].real.nHcrInfo) {
				c_tcRef.real.hcr.nBuf = c_dozMain.tcs[CAR_HEAD].real.nHcrInfo;
				//HcrInfo(c_tcRef.real.hcr.nMasterID, c_dozMain.tcs[OWN_SPACE].real.nHcrInfo, 8);	// ?????
				c_tcRef.real.hcr.nDeb = DEBOUNCE_EXCHANGEHCRSMALL;
			}
			else if (c_tcRef.real.hcr.nDeb != 0 && --c_tcRef.real.hcr.nDeb == 0) {
				c_tcRef.real.hcr.nMasterID = c_tcRef.real.hcr.nBuf;
				//HcrInfo(c_tcRef.real.hcr.nMasterID, c_dozMain.tcs[OWN_SPACE].real.nHcrInfo, 6);	// ?????
				c_tcRef.real.cycle.wXch = 0x7fff;
				// master is tc0, so change to bus slave
				MSGLOG("[TC]Reconfig from frame0 by %d.\r\n", c_tcRef.real.hcr.nMasterID);
				c_path.Setup(c_tcRef.real.hcr.nMasterID);	// by master changing
				SHOTB(SCENE_ID_HEADCAR, 13);
				Lock();
				c_tcRef.real.cycle.wXch = 0;
				c_tcRef.real.cycle.wState &= ~TCSTATE_XCHMASTER;
				Unlock();
			}
		}
		else if (OWN_SPACE != 0) { // only slave
			Lock();
			c_tcRef.real.cycle.wPeriodOverTime = 0;
			c_tcRef.real.cycle.wMasterRecoveryCounter = 0;
			Unlock();
			RECTIME(TTIME_TCOPEN);
			if (!ReceiveFrameZero(bCh))	return;
			if (c_bLocalSync) {
				c_nLocalTimer = 0;
				c_bLocalSync = FALSE;
			}
			RECTIME(TTIME_TCOPENED);
			CalcTimes();
			memcpy((PVOID) &c_tbTime, (PVOID) &c_dozMain.tcs[CAR_HEAD].real.time, sizeof(TBTIME));
			if (c_dozMain.tcs[CAR_HEAD].real.nIssue & ISSUE_ENVIROMENTSET) {
				if (!(c_dozMain.tcs[OWN_SPACE].real.nIssue & ISSUE_ENVIROMENTSET) &&
						c_dozMain.tcs[CAR_HEAD].real.each.wId == TCINFORM_ENV) {
					memcpy((PVOID)&c_env, (PVOID)&c_dozMain.tcs[CAR_HEAD].real.each.info.env, sizeof(ENVARCHEXTENSION));
					// Modified 2012/11/30
					c_tcRef.real.cond.wTrainNo = c_env.real.entire.elem.wTrainNo;
					MSGLOG("[TC]Cause save command from tcfirm.\r\n");
					SaveEnv(4);		// by slave cause train no. changing
				}
			}
			else {
				c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_ENVIROMENTSET;
				CLRTBIT(OWN_SPACE, TCDOB_ESET);
			}

			if (c_dozMain.tcs[CAR_HEAD].real.nIssue & ISSUE_TIMERSET) {	// time set at slave ?
				if (!(c_dozMain.tcs[OWN_SPACE].real.nIssue & ISSUE_TIMERSET) &&
						c_dozMain.tcs[CAR_HEAD].real.time.sec == 0) {
					c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_TIMERSET;	// respond of slave
					SETTBIT(OWN_SPACE, TCDOB_TSET);			// at slave
					memcpy((PVOID)&c_tbTime, (PVOID)&c_dozMain.tcs[CAR_HEAD].real.time, sizeof(TBTIME));
					SetTimeByTb();	// time set at slave
				}
			}
			else {
				c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_TIMERSET;
				CLRTBIT(OWN_SPACE, TCDOB_TSET);
			}

			//if (!(c_tcRef.real.nObligation & OBLIGATION_MAIN)) {
			//	memcpy((PVOID) &c_dozMain.tcs[OWN_SPACE].real.rAtc,
			//			(PVOID) &c_dozMain.tcs[OWN_SPACE - 1].real.rAtc, sizeof(ATCRINFO));
			//	memcpy((PVOID) &c_dozMain.tcs[OWN_SPACE].real.rAto,
			//			(PVOID) &c_dozMain.tcs[OWN_SPACE - 1].real.rAto, sizeof(ATORINFO));
			//	memcpy((PVOID) &c_dozMain.tcs[OWN_SPACE].real.rCmm,
			//			(PVOID) &c_dozMain.tcs[OWN_SPACE - 1].real.rCmm, sizeof(CMMRINFO));
			//	//????? =============== NEVER EXCUTE CODE ===================== ?????
			//	/*memcpy((PVOID) &c_lcFirm.aAtc[0],
			//			(PVOID) &c_dozMain.tcs[OWN_SPACE - 1].real.aAtc, sizeof(ATCAINFO));
			//	memcpy((PVOID) &c_lcFirm.aAtc[1],
			//			(PVOID) &c_dozMain.tcs[OWN_SPACE - 1].real.aAtc, sizeof(ATCAINFO));
			//	memcpy((PVOID) &c_lcFirm.aAto,
			//			(PVOID) &c_dozMain.tcs[OWN_SPACE - 1].real.aAto, sizeof(ATOAINFO));*/
			//	for (UCURV n = 0; n < 4; n ++)
			//		SendMail(MA_RECEIVELOCAL, this, (long) n);
			//	//????? =============== NEVER EXCUTE CODE ===================== ?????
			//}
			// 1st. at slave receive frame0, dumb count -> frame
			// 2nd. send local, if receive respond, clear dumb count
			// 3nd. at master receive all frame, tail frame -> dumb count
			// ??????
			LocalErrorToBus();
			c_pack.Package();
			c_tcRef.real.cycle.wState |= TCSTATE_FRAMEOPEN;
		}
	}
	else {
		if (c_tcRef.real.cycle.wState & TCSTATE_XCHMASTER)	return;
		//if (bCh)	c_busA.wState &= ~BUSSTATE_WAIT;
		if (bCh)	c_tcRef.real.cycle.nBusState[0] &= ~BUSSTATE_WAIT;
		else {
			//c_busB.wState &= ~BUSSTATE_WAIT;
			c_tcRef.real.cycle.nBusState[1] &= ~BUSSTATE_WAIT;
			if (c_tcRef.real.nActiveBus & INHIBITBUS_B) {
				c_tcRef.real.nActiveBus &= ~INHIBITBUS_B;
				c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_KILLDOWNLOAD;
				c_tcRef.real.es.nDownloadStep = DOWNLOADSTEP_NON;
				//c_tcRef.real.es.nMasterAddr = 0;
				c_tcRef.real.es.nMasterDuty = 0xff;
				c_tcRef.real.nState &= ~(STATE_PROGRESSING | STATE_DOWNLOADING);
				MSGLOG("[TC]Bus B come back by normal bus.\r\n");
			}
		}
		//if ((c_busA.wState & BUSSTATE_WAIT) || (c_busB.wState & BUSSTATE_WAIT))	return;
		if ((c_tcRef.real.cycle.nBusState[0] & BUSSTATE_WAIT) || (c_tcRef.real.cycle.nBusState[1] & BUSSTATE_WAIT))	return;
		ReceiveAll();
		// Modified 2013/01/28
		c_tcRef.real.revolt.nCondition = 0;

		if (!CheckBusState()) { // excute by head's main-reserve
			if (OWN_SPACE == 0) {
				c_busA.pLine->EnableIntr(TRUE);
				c_busB.pLine->EnableIntr(TRUE);
				INCBYTE(c_tcRef.real.nBusFailCounter[0]);
				BUSFAULTLAMPON();
			}
		}
		RECTIME(TTIME_TCCLOSE);
		// Modified 2013/02/20
		for (UCURV n = CAR_HEAD; n < LENGTH_TC; n ++)
			c_tcRef.real.cid.nTcAddr[n] = c_dozMain.tcs[n].real.nRealAddr;

		InterTrigger(&c_tcRef.real.cycle.wReportTime, TIME_REPORT);
		//c_spy.ReportBus(&c_tcRef, c_pLcFirm, &c_dozMain, &c_ducInfo.srDuc);
		ReportBus();
		RECTIME(TTIME_PROCBEGIN);
		if (c_tcRef.real.cond.wReleaseTimer == TIME_RELEASE)
			c_tcRef.real.nState |= STATE_ENABLEOVERHAUL;		// train is completely stopped state !!!
		else	c_tcRef.real.nState &= ~STATE_ENABLEOVERHAUL;
		ReadProbe();
		if (c_tcRef.real.nState & STATE_ENABLEOVERHAUL) {
			if (c_chaser.Link()) {
				RESET_BEILACK();
				CampUsb();
				BeilCmd(BEILCMD_TRACETOUSB);
			}
		}
		UCURV res = c_path.Control();
		// Modified 2012/01/11 ... begin
		if (res == TDIRRES_EXCHANGETOHEADWITHLOADENV) {
			double now = TimeToSec(&c_devTime);
			double before = TimeToSec(&c_env.real.entire.elem.devTime);
			if (now < before) {
				MSGLOG("[ENV]ERROR:Time setting is wrong!!! now %02X-%02X-%02X %02X:%02X:%02X before %02X-%02X-%02X %02X:%02X:%02X\r\n",
						c_devTime.year, c_devTime.month, c_devTime.day, c_devTime.hour, c_devTime.minute, c_devTime.sec,
						c_env.real.entire.elem.devTime.year, c_env.real.entire.elem.devTime.month, c_env.real.entire.elem.devTime.day,
						c_env.real.entire.elem.devTime.hour, c_env.real.entire.elem.devTime.minute, c_env.real.entire.elem.devTime.sec);
				SHOTB(SCENE_ID_HEADCAR, 49);
				c_tcRef.real.nState |= STATE_NEEDTIMESET;
			}
		}
		// ... end
		if (c_tcRef.real.cycle.wCtrlDisable == 0) {
			c_tcRef.real.dev.nState = 0;
			// Modified 2012/11/29
			//if (c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF)	c_look.ModeCtrl();
			if ((c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF) && IsNotRescue())	c_look.ModeCtrl();
			//else {
			//	if (c_tcRef.real.op.nMode == OPMODE_DRIVERLESS)	ScanNuetralAtDriverless(FALSE);
			//}
			c_dozMain.tcs[OWN_SPACE].real.pwm.period = c_dozMain.tcs[OWN_SPACE].real.pwm.high = 0;
			c_dozMain.tcs[OWN_SPACE].real.pwm.nMode = 0;
			c_look.Check();
			if (c_tcRef.real.op.nState & OPERATESTATE_READYATCON) {
				if (c_tcRef.real.op.wWaitAtcOn == 0) {
					c_tcRef.real.op.nState &= ~OPERATESTATE_READYATCON;
					c_path.AtcPowerCtrl(TRUE);
					c_tcRef.real.op.nState |= OPERATESTATE_INHIBITATO;
				}
			}
			// Modified 2012/11/30
			//if ((c_tcRef.real.nObligation & OBLIGATION_TOP) == OBLIGATION_TOP ||
			//if ((c_tcRef.real.nObligation & OBLIGATION_CTRL) == OBLIGATION_CTRL ||
			if (((c_tcRef.real.nObligation & OBLIGATION_ACTIVE) && (c_tcRef.real.nObligation & OBLIGATION_HEAD)) ||	// main & reserve
					(c_tcRef.real.cycle.wState & TCSTATE_FRAMEOPEN)) {	// tail
				c_artop.Drive();
				c_panto.Drive();
				// Inserted 2012/03/06 ... begin
				if (IsNotRescue() || IsRescuePassiveDrive() || IsRescueActiveDrive())	c_ext.Check();
				c_door.Control();
				if (!(c_tcRef.real.nObligation & OBLIGATION_HEAD)) {	// at tail
					c_dumb.wAtc[0] = c_dozMain.tcs[CAR_HEAD].real.eAtcA.wCount;
					c_dumb.wAtc[1] = c_dozMain.tcs[CAR_HEAD].real.eAtcB.wCount;
					c_dumb.wAto = c_dozMain.tcs[CAR_HEAD].real.eAto.wCount;
				}
				BYTE n = OWN_SPACE < 2 ? 1 : 0;
				BYTE car = OWN_SPACE < 2 ? CAR_TAIL : CAR_HEAD;
				if (c_dozMain.tcs[car].real.eCsc.nState == DEVFLOW_ERR)
					c_dumb.wCsc[n] = c_dozMain.tcs[car].real.eCsc.wCount;
				if (c_dozMain.tcs[car].real.ePis.nState == DEVFLOW_ERR)
					c_dumb.wPis[n] = c_dozMain.tcs[car].real.ePis.wCount;
				if (c_dozMain.tcs[car].real.eTrs.nState == DEVFLOW_ERR)
					c_dumb.wTrs[n] = c_dozMain.tcs[car].real.eTrs.wCount;
				if (c_dozMain.tcs[car].real.eHtc.nState == DEVFLOW_ERR)
					c_dumb.wHtc[n] = c_dozMain.tcs[car].real.eHtc.wCount;
				if (c_dozMain.tcs[car].real.ePsd.nState == DEVFLOW_ERR)
					c_dumb.wPsd[n] = c_dozMain.tcs[car].real.ePsd.wCount;
				c_tcRef.real.cycle.wState &= ~TCSTATE_FRAMEOPEN;
			}
			if (OWN_SPACE == 0) {
				if (c_dozMain.tcs[OWN_SPACE].real.nIssue & ISSUE_TIMERSET) {
					// at head, time setting of all slave has been completed, this bit clear
					if ((!GetBusCondition(CAR_HEADBK) ||
							(c_dozMain.tcs[CAR_HEADBK].real.nIssue & ISSUE_TIMERSET)) &&
							(!GetBusCondition(CAR_TAIL) ||
							(c_dozMain.tcs[CAR_TAIL].real.nIssue & ISSUE_TIMERSET)) &&
							(!GetBusCondition(CAR_TAILBK) ||
							(c_dozMain.tcs[CAR_TAILBK].real.nIssue & ISSUE_TIMERSET))) {
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_TIMERSET;
						CLRTBIT(OWN_SPACE, TCDOB_TSET);
						CUTBS(SCENE_ID_HEADCAR, 41, 44);
					}
					else if (c_tcRef.real.cond.wTimerSetWait == 0) {
						SHORT tcid;
						if (GetBusCondition(CAR_HEADBK) && !(c_dozMain.tcs[CAR_HEADBK].real.nIssue & ISSUE_TIMERSET)) {
							tcid = GetPscIDByAddr(CAR_HEADBK);
							if (tcid >= 0)	SHOTB(tcid < 2 ? 0 : 7, tcid + 41);
						}
						if (GetBusCondition(CAR_TAIL) && !(c_dozMain.tcs[CAR_TAIL].real.nIssue & ISSUE_TIMERSET)) {
							tcid = GetPscIDByAddr(CAR_TAIL);
							if (tcid >= 0)	SHOTB(tcid < 2 ? 0 : 7, tcid + 41);
						}
						if (GetBusCondition(CAR_TAILBK) && !(c_dozMain.tcs[CAR_TAILBK].real.nIssue & ISSUE_TIMERSET)) {
							tcid = GetPscIDByAddr(CAR_TAILBK);
							if (tcid >= 0)	SHOTB(tcid < 2 ? 0 : 7, tcid + 41);
						}
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_TIMERSET;
						CLRTBIT(OWN_SPACE, TCDOB_TSET);
					}
				}
				if (c_dozMain.tcs[OWN_SPACE].real.nIssue & ISSUE_ENVIROMENTSET) {
					if ((!GetBusCondition(CAR_HEADBK) ||
							(c_dozMain.tcs[CAR_HEADBK].real.nIssue & ISSUE_ENVIROMENTSET)) &&
							(!GetBusCondition(CAR_TAIL) ||
							(c_dozMain.tcs[CAR_TAIL].real.nIssue & ISSUE_ENVIROMENTSET)) &&
							(!GetBusCondition(CAR_TAILBK) ||
							(c_dozMain.tcs[CAR_TAILBK].real.nIssue & ISSUE_ENVIROMENTSET))) {
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_ENVIROMENTSET;
						CLRTBIT(OWN_SPACE, TCDOB_ESET);
						CUTBS(SCENE_ID_HEADCAR, 45, 48);
					}
					else if (c_tcRef.real.cond.wEnvSetWait == 0) {
						SHORT tcid;
						if (GetBusCondition(CAR_HEADBK) && !(c_dozMain.tcs[CAR_HEADBK].real.nIssue & ISSUE_ENVIROMENTSET)) {
							tcid = GetPscIDByAddr(CAR_HEADBK);
							if (tcid >= 0)	SHOTB(tcid < 2 ? 0 : 7, tcid + 45);
						}
						if (GetBusCondition(CAR_TAIL) && !(c_dozMain.tcs[CAR_TAIL].real.nIssue & ISSUE_ENVIROMENTSET)) {
							tcid = GetPscIDByAddr(CAR_TAIL);
							if (tcid >= 0)	SHOTB(tcid < 2 ? 0 : 7, tcid + 45);
						}
						if (GetBusCondition(CAR_TAILBK) && !(c_dozMain.tcs[CAR_TAILBK].real.nIssue & ISSUE_ENVIROMENTSET)) {
							tcid = GetPscIDByAddr(CAR_TAILBK);
							if (tcid >= 0)	SHOTB(tcid < 2 ? 0 : 7, tcid + 45);
						}
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_ENVIROMENTSET;
						CLRTBIT(OWN_SPACE, TCDOB_ESET);
					}
				}
				if (c_tcRef.real.insp.nMode > INSPECTMODE_NON && c_tcRef.real.insp.nMode < INSPECTMODE_VIEW) {
					if (c_insp.Check()) {
						if (c_tcRef.real.insp.nMode == INSPECTMODE_PDT)	c_tcRef.real.du.nSetPage = DUPAGE_OPENNORMAL;
						else	c_tcRef.real.du.nSetPage = DUPAGE_OPENOVERHAUL;
						c_tcRef.real.insp.nMode = INSPECTMODE_NON;
						CUTB(SCENE_ID_HEADCAR, 155);
					}
				}
			}
		}
		RECTIME(TTIME_PROCEND);

		// at tail
		if (OWN_SPACE >= 2) {
			// for output // 2012/06/13 ...TCR allways follow HCR
			if (FBTBIT(CAR_HEAD, TCDOB_HCR))	SETTBIT(OWN_SPACE, TCDOB_TCR);
			else	CLRTBIT(OWN_SPACE, TCDOB_TCR);
			if (FBTBIT(CAR_HEAD, TCDOB_ZVR))	SETTBIT(OWN_SPACE, TCDOB_ZVR);
			else	CLRTBIT(OWN_SPACE, TCDOB_ZVR);
			if (FBTBIT(CAR_HEAD, TCDOB_DCALS))	SETTBIT(OWN_SPACE, TCDOB_DCALS);
			else	CLRTBIT(OWN_SPACE, TCDOB_DCALS);
			if (c_dozMain.tcs[CAR_HEAD].real.pwm.nMode == PWMMODE_OUTPUT) {
				if (GETNBIT(CAR_HEAD, NESTED_ROLR))	SETTBIT(OWN_SPACE, TCDOB_ROLR);
				else	CLRTBIT(OWN_SPACE, TCDOB_ROLR);
				if (GETNBIT(CAR_HEAD, NESTED_ROPR))	SETTBIT(OWN_SPACE, TCDOB_ROPR);
				else	CLRTBIT(OWN_SPACE, TCDOB_ROPR);
				if (GETNBIT(CAR_HEAD, NESTED_ROBR))	SETTBIT(OWN_SPACE, TCDOB_ROBR);
				else	CLRTBIT(OWN_SPACE, TCDOB_ROBR);
			}
			// Modified 2013/02/18
			memcpy((PVOID)&c_tcRef.real.nTerminalCodeBackup, (PVOID)&c_dozMain.tcs[CAR_HEADBK].real.rCmm.nPrst, SIZE_VALIDTERMINALCODE);
		}
		//OutputD();
		WORD w;
		for (UCURV n = 0; n < SIZE_DOUT; n ++) {
			w = MAKEWORD(FEEDTBYTE(OWN_SPACE, n * 2 + 1), FEEDTBYTE(OWN_SPACE, n * 2));
			if (OWN_SPACE == 2) {
				if (n == 1) {
					WORD v = (WORD)(FEEDTBYTE(CAR_HEAD, n * 2) & 0xb9);	// keep EBCOSLP, SBLP, PBLP, EBLP, EPANDNLP
					w |= v;
				}
			}
			else if (OWN_SPACE == 1 || OWN_SPACE == 3) {
				WORD mask = (WORD)MAKEWORD(c_nJointOut[n * 2 + 1], c_nJointOut[n * 2]);
				w &= mask;
			}
			// Modified 2012/11/12
			// Modified 2012/10/09
			//if (c_tcRef.real.insp.nMode > INSPECTMODE_NON && c_tcRef.real.insp.nMode < INSPECTMODE_VIEW &&
			//		c_tcRef.real.insp.nAbsLamp != 0 && n < SIZE_LAMP)
			if (c_tcRef.real.insp.nAbsLamp != 0 && n < SIZE_LAMP)
				w |= c_wLamps[n];
			c_probe.Putd(n, &w);
			// Modified 2012/11/29
			c_dozMain.tcs[OWN_SPACE].real.nRealOutput[n * 2] = LOWBYTE(w);
			c_dozMain.tcs[OWN_SPACE].real.nRealOutput[n * 2 + 1] = HIGHBYTE(w);
		}
		w = (c_tcRef.real.op.nState & OPERATESTATE_ATCON) ? 0x8000 : 0;	// output extention to MFB
		c_probe.Putd(3, &w);

		if (++ c_tcRef.real.cycle.nLetterCount >= c_tcRef.real.cycle.nLetterCycle) {
			c_tcRef.real.cycle.nLetterCount = 0;
			c_scene.Letter(&c_devTime);
		}
		// Modified 2013/11/02
		//if (c_tcRef.real.nState & STATE_TRIAL) {
		//	_TV_ tbe = GETLWORD(c_tcRef.real.power.vTbeL);
		//	// Modified 2013/02/05
		//	//if (c_tcRef.real.op.nMode == OPMODE_EMERGENCY)	c_trial.dwSpeed = c_tcRef.real.speed.dwV3fs;
		//	//else	c_trial.dwSpeed = c_tcRef.real.speed.dwAtcs;
		//	if (c_tcRef.real.op.nMode != OPMODE_EMERGENCY && (c_tcRef.real.nProperty & PROPERTY_ATLABORATORY))
		//		c_trial.dwSpeed = c_tcRef.real.speed.dwAtcs;
		//	else	c_trial.dwSpeed = c_tcRef.real.speed.dwV3fs;	// m/h
		//	c_trial.dwSample[c_trial.wSampleIndex ++] = c_trial.dwSpeed;
		//	c_trial.dwDist = (DWORD)(c_tcRef.real.intg.dbDistLive - c_trial.dbBeginDist);
		//	uint64_t clk = ClockCycles();
		//	double sec = (double)(clk - c_trial.sampleClk) / (double)c_tcRef.real.cps;
		//	if (sec > 0.45f) {
		//		if (c_trial.wSampleIndex > 0) {
		//			DWORD avr = 0;
		//			WORD length;
		//			for (length = 0; length < c_trial.wSampleIndex; length ++)
		//				avr += c_trial.dwSample[length];
		//			avr /= (DWORD)length;
		//			c_trial.vAccel =
		//				(WORD)DevideBy10(((double)avr - (double)c_trial.dwSampleSpeed) * (1.f / sec));	// ex)123 -> 1.23Km/s2
		//			c_trial.dwSampleSpeed = avr;
		//		}
		//		else {
		//			c_trial.dwSampleSpeed = 0;
		//			c_trial.vAccel = 0;
		//		}
		//		c_trial.wSampleIndex = 0;
		//		c_trial.sampleClk = clk;
		//	}
		//	if (c_trial.pwr.s == 0) {
		//		if (tbe > 0) {
		//			c_trial.pwr.s = 1;
		//			c_trial.pwr.t.begin = clk;
		//			c_trial.pwr.d.dwBegin = c_trial.dwDist;
		//			Hangup(0);
		//		}
		//	}
		//	else {
		//		sec = (double)(clk - c_trial.pwr.t.begin) / (double)c_tcRef.real.cps;
		//		c_trial.pwr.t.wCurrent = (WORD)DevideBy10((DWORD)(sec * 100.f));	// ex) 12 -> 1.2sec
		//		c_trial.pwr.d.wCurrent = (WORD)(c_trial.dwDist - c_trial.pwr.d.dwBegin);
		//		if (tbe < 1)	c_trial.pwr.s = 0;
		//	}
		//	if (c_trial.brk.s == 0) {
		//		if (tbe < 0 && !(c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF)) {
		//			c_trial.brk.s = 1;
		//			c_trial.brk.t.begin = clk;
		//			c_trial.brk.d.dwBegin = c_trial.dwDist;
		//			Hangup(0);
		//		}
		//	}
		//	else {
		//		sec = (double)(clk - c_trial.brk.t.begin) / (double)c_tcRef.real.cps;
		//		c_trial.brk.t.wCurrent = (WORD)DevideBy10((DWORD)(sec * 100.f));
		//		c_trial.brk.d.wCurrent = (WORD)(c_trial.dwDist - c_trial.brk.d.dwBegin);
		//		if (c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF)	c_trial.brk.s = 0;
		//	}
		//	if (c_trial.dwSpeed > 0 || GETLWORD(c_tcRef.real.power.vTbeL) != 0)	c_scene.Tag(&c_devTime);
		//}

		if (c_tcRef.real.nState & STATE_TRIAL) {
			_TV_ tbe = GETLWORD(c_tcRef.real.power.vTbeL);
			if (c_tcRef.real.op.nMode != OPMODE_EMERGENCY && (c_tcRef.real.nProperty & PROPERTY_ATLABORATORY))
				c_trial.dwSpeed = c_tcRef.real.speed.dwAtcs;
			else	c_trial.dwSpeed = c_tcRef.real.speed.dwV3fs;	// m/h
			c_trial.dwSample[c_trial.wSampleIndex ++] = c_trial.dwSpeed;

			if (c_trial.wSampleIndex > (WORD)c_tcRef.real.cycle.nLetterCycle) {
				c_trial.dwDist = (DWORD)(c_tcRef.real.intg.dbDistLive - c_trial.dbBeginDist);
				uint64_t clk = ClockCycles();
				double sec = (double)(clk - c_trial.sampleClk) / (double)c_tcRef.real.cps;
				DWORD avr = 0;
				WORD length;
				for (length = 0; length < (WORD)c_tcRef.real.cycle.nLetterCycle; length ++)
					avr += c_trial.dwSample[length];
				avr /= (DWORD)length;
				c_trial.vAccel =
					(WORD)DevideBy10(((double)avr - (double)c_trial.dwSampleSpeed) / sec);	// ex)123 -> 1.23Km/s2
				c_trial.dwSampleSpeed = avr;
				c_trial.wSampleIndex = 0;
				c_trial.sampleClk = clk;

				if (c_trial.pwr.s == 0) {
					if (tbe > 0) {
						c_trial.pwr.s = 1;
						c_trial.pwr.t.begin = clk;
						c_trial.pwr.d.dwBegin = c_trial.dwDist;
						Hangup(0);
					}
				}
				else {
					sec = (double)(clk - c_trial.pwr.t.begin) / (double)c_tcRef.real.cps;
					c_trial.pwr.t.wCurrent = (WORD)DevideBy10((DWORD)(sec * 100.f));	// ex) 12 -> 1.2sec
					c_trial.pwr.d.wCurrent = (WORD)(c_trial.dwDist - c_trial.pwr.d.dwBegin);
					if (tbe < 1)	c_trial.pwr.s = 0;
				}

				if (c_trial.brk.s == 0) {
					if (tbe < 0 && !(c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF)) {
						c_trial.brk.s = 1;
						c_trial.brk.t.begin = clk;
						c_trial.brk.d.dwBegin = c_trial.dwDist;
						Hangup(0);
					}
				}
				else {
					sec = (double)(clk - c_trial.brk.t.begin) / (double)c_tcRef.real.cps;
					c_trial.brk.t.wCurrent = (WORD)DevideBy10((DWORD)(sec * 100.f));
					c_trial.brk.d.wCurrent = (WORD)(c_trial.dwDist - c_trial.brk.d.dwBegin);
					if (c_tcRef.real.speed.nZv & SPEEDDETECT_ZVF)	c_trial.brk.s = 0;
				}
				if (c_trial.dwSpeed > 0 || GETLWORD(c_tcRef.real.power.vTbeL) != 0)	c_scene.Tag(&c_devTime);
			}
		}

		// Modified 12/11/12 ... begin
		//if (OWN_SPACE == 0 || OWN_SPACE == 2) {
		//if (c_panel.GetPartner())	c_tcRef.real.nProperty |= PROPERTY_PARTNER;
		//else	c_tcRef.real.nProperty &= ~PROPERTY_PARTNER;
		//BYTE head = GetHeadCarID();		// 0->TC1, other->TC0
		//WORD wMsg = head == 0 ? 36 : 38;
		//if (c_tcRef.real.nAddr != 1 && c_tcRef.real.nAddr != 3)	++ wMsg;
		//if (c_tcRef.real.nProperty & PROPERTY_PARTNER)	CUTB(SCENE_ID_HEADCAR, wMsg);
		//else	SHOTB(SCENE_ID_HEADCAR, wMsg);

		if (OWN_SPACE == 0 || OWN_SPACE == 2) {
			// Modified 12/11/12 ... begin
			TipPartner(c_dozMain.tcs[0].real.nRealAddr, (c_dozMain.tcs[0].real.nHint & HINT_PARTNER) ? TRUE : FALSE);
			TipPartner(c_dozMain.tcs[2].real.nRealAddr, (c_dozMain.tcs[2].real.nHint & HINT_PARTNER) ? TRUE : FALSE);
			// ... end
			FormatDuc();
			c_pLocal->SendDu();
			if (c_tcRef.real.ai.nProtect & PROTECT_PAGE) {
				c_tcRef.real.ai.nProtect &= ~PROTECT_PAGE;
				c_tcRef.real.du.nCurPage = c_tcRef.real.du.nBkupPage;
			}
			//LatterDuc();
			// Modified 2013/02/26
			//if (c_pBucket != NULL && (c_tcRef.real.nState & STATE_ENABLEOVERHAUL)) {
			//	if ((c_tcRef.real.nState & STATE_DOWNLOADING) && (c_tcRef.real.es.nDownloadStep == DOWNLOADSTEP_NON)) {
			//		if (c_pBucket->nMainAck != BEILACK_WORKEND)
			//			c_tcRef.real.nState |= STATE_PROGRESSING;
			//		else {
			//			c_tcRef.real.nState &= ~(STATE_DOWNLOADING | STATE_PROGRESSING);
			//			c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
			//			Hangup(FILEMSG_BEGIN + c_pBucket->iRespond);
			//		}
			//	}
			//	else if (++ c_nUsbScanCounter >= COUNT_USBSCAN) {
			//		c_nUsbScanCounter = 0;
			//		ScanUsb();
			//	}
			//}
			//else	c_tcRef.real.nState &= ~STATE_BEENUSB;
		}
		else if (OWN_SPACE == 3) {
			if (!c_panel.GetPartner() && !GetRealBusCondition(CAR_TAIL) &&
					!(c_tcRef.real.nProperty & PROPERTY_DISABLEBACKUP)) {
				// Change slave, reserve to main;
				OWN_SPACE = 2;
				c_tcRef.real.nObligation |= OBLIGATION_MAIN;
				c_path.Setup(c_dozMain.tcs[CAR_HEAD].real.nHcrInfo);	// by backup at tail
				SETTBIT(OWN_SPACE, TCDOB_RESVL);
				c_panel.ClrLamp(4);		// capture du's link at bus slave
			}
		}
		// Modified 2013/02/26
		if (c_pBucket != NULL && (c_tcRef.real.nState & STATE_ENABLEOVERHAUL)) {
			if ((c_tcRef.real.nState & STATE_DOWNLOADING) && c_tcRef.real.es.nDownloadMethod == DOWNLOADMETHOD_SELF) {	//(c_tcRef.real.es.nDownloadStep == DOWNLOADSTEP_NON)) {
				if (c_pBucket->nMainAck != BEILACK_WORKEND)
					c_tcRef.real.nState |= STATE_PROGRESSING;
				else {
					c_tcRef.real.nState &= ~(STATE_DOWNLOADING | STATE_PROGRESSING);
					c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
					// Modified 2013/03/04
					//Hangup(FILEMSG_BEGIN + c_pBucket->iRespond);
				}
				//c_tcRef.dummy[0] = HIGHBYTE(c_pBucket->iRespond);
				//c_tcRef.dummy[1] = LOWBYTE(c_pBucket->iRespond);
				Hangup(FILEMSG_BEGIN + c_pBucket->iRespond);
			}
			else if (++ c_nUsbScanCounter >= COUNT_USBSCAN) {
				c_nUsbScanCounter = 0;
				ScanUsb();
			}
		}
		else	c_tcRef.real.nState &= ~STATE_BEENUSB;

		// Modified 2012/12/25
		if (c_tcRef.real.scArm.nEmergency > 0) {
			SETTBIT(OWN_SPACE, TCDOB_DBZA);
			SETTBIT(OWN_SPACE, TCDOB_FACKLP);
		}
		else {
			if (GETTBIT(OWN_SPACE, TCDIB_FBFACK)) {
				if (!(c_tcRef.real.scArm.nState & ARMSTATE_BUZZSW)) {
					c_tcRef.real.scArm.nState |= ARMSTATE_BUZZSW;
					if (c_tcRef.real.insp.nMode == INSPECTMODE_NON || c_tcRef.real.insp.nMode > INSPECTMODE_EACHDEV) {
						c_tcRef.real.scArm.nState &= ~ARMSTATE_LAMPON;	// by TCDIB_FBFACK;
						// Modified 2013/01/22
						Hide();
					}
				}
			}
			else	c_tcRef.real.scArm.nState &= ~ARMSTATE_BUZZSW;

			if (c_tcRef.real.scArm.nState & ARMSTATE_LAMPON) {
				// Modified 2013/01/22
				//if (GetAlarmCode() > 0)	SETTBIT(OWN_SPACE, TCDOB_FACKLP);
				if (GetActiveAlarmCode() > 0)	SETTBIT(OWN_SPACE, TCDOB_FACKLP);
				else {
					c_tcRef.real.scArm.nState &= ~ARMSTATE_LAMPON;
					CLRTBIT(OWN_SPACE, TCDOB_FACKLP);
				}
			}
			else	CLRTBIT(OWN_SPACE, TCDOB_FACKLP);

			if (c_tcRef.real.scArm.nState & ARMSTATE_BUZZON) {
				if (c_tcRef.real.scArm.wBuzzTimer == 0) {
					c_tcRef.real.scArm.nState &= ~ARMSTATE_BUZZON;
					CLRTBIT(OWN_SPACE, TCDOB_DBZA);
				}
				else	SETTBIT(OWN_SPACE, TCDOB_DBZA);
			}
		}

		// Modified 2012/11/14
		if (!(c_tcRef.real.ptflow.nState & PS_ENABLEALARM)) {
			if (c_tcRef.real.ptflow.wSkipTimer == 0) {
				if (IsPantoUpB())	c_tcRef.real.ptflow.wSkipTimer = TIME_PANTOALARMSKIP;
				//else	c_tcRef.real.ptflow.wSkipTimer = 0;
			}
			else {
				if (IsPantoUpB() && -- c_tcRef.real.ptflow.wSkipTimer == 0)
					c_tcRef.real.ptflow.nState |= PS_ENABLEALARM;
			}
		}

		// Modified 2013/01/28
		if (OWN_SPACE != 0) {
			if (c_dozMain.tcs[CAR_HEAD].real.arm.b.code != 0)	c_scene.AidShot(&c_dozMain.tcs[CAR_HEAD].real.arm);
		}
		// Modified 2013/03/11
		_TV_ tbe = GETLWORD(c_dozMain.tcs[CAR_HEAD].real.cV3f.vTbeL);
		if (tbe > 0)	c_doc.ExitFromDownload();

		if (c_tcRef.real.nState & STATE_ENABLEOVERHAUL) {
			// ===== start directory by bus ==================================================================
			if ((c_dozMain.tcs[OWN_SPACE].real.nIssue & ISSUE_DIRECTORYREQUEST) &&
					c_dozMain.tcs[OWN_SPACE].real.each.wId == TCINFORM_DIR) {
				// own
				if (c_dozMain.tcs[OWN_SPACE].real.each.info.dir.nDuty == OWN_SPACE) {
					// responded to directory request of opposite
					if (!(c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.nIssue & ISSUE_DIRECTORYREQUEST)) {
						// end of request from opposite
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_DIRECTORYREQUEST;
						c_tcRef.real.es.nDirectoryState &= ~DIRECTORYSTATE_WORK;
					}
				}
				else {
					// scan directory data of opposite
					if (c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.nIssue & ISSUE_DIRECTORYREQUEST) {
						SetFilesByDay((WORD)(c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dir.nLength),
								c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dir.dwItem);
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_DIRECTORYREQUEST;

						c_tcRef.real.du.nSetPage = DUPAGE_DIRBYDAY;
						c_tcRef.real.du.nDownloadItem = DUKEY_DIRBYDAY;

						uint64_t fe = ClockCycles();
						double sec = (double)(fe - c_oppoDirTriggerTime) / (double)c_tcRef.real.cps;
						MSGLOG("[TC]Directory of opposite (%dms).\r\n", (WORD)(sec * 1000.f));
					}
					else if (-- c_tcRef.real.es.nCounter == 0) {
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_DIRECTORYREQUEST;
						c_tcRef.real.es.nDirectoryState &= ~DIRECTORYSTATE_OPPO;
						MSGLOG("[TC]ERROR:Directory of opposite timeout.\r\n");
					}
				}
			}
			else {
				// scan request from bus
				for (UCURV n = CAR_HEAD; n < LENGTH_TC; n ++) {
					if (n == OWN_SPACE)	continue;
					if ((c_dozMain.tcs[n].real.nIssue & ISSUE_DIRECTORYREQUEST) &&
							c_dozMain.tcs[n].real.each.wId == TCINFORM_DIR &&
							c_dozMain.tcs[n].real.each.info.dir.nDuty == OWN_SPACE) {
						if (!(c_tcRef.real.es.nDirectoryState & DIRECTORYSTATE_WORK)) {
							c_tcRef.real.es.nDirectoryState |= DIRECTORYSTATE_WORK;
							c_tcRef.real.es.nTargetDuty = n;	// request from n
							DirectoryList();	// cause request
						}
						break;
					}
				}
			}
			// ===== end directory by bus ====================================================================

			// ===== start download by bus ===================================================================
			BOOL bHead = (c_dozMain.tcs[CAR_HEAD].real.nIssue & ISSUE_KILLBUSREQUEST) ? TRUE : FALSE;
			BOOL bTail = (c_dozMain.tcs[CAR_TAIL].real.nIssue & ISSUE_KILLBUSREQUEST) ? TRUE : FALSE;
			if (bHead != bTail) {
				//if (c_tcRef.real.es.nMasterAddr == 0) {	// rise
				if (c_tcRef.real.es.nMasterDuty == 0xff) {	// rise
					c_tcRef.real.es.nCounter = COUNT_WAITOPPODIR;
					if (c_tcRef.real.nObligation & OBLIGATION_MAIN)
						c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_KILLBUSREQUEST;
					//if (bHead)	c_tcRef.real.es.nMasterAddr = c_tcRef.real.cid.nTcAddr[CAR_HEAD];
					//else	c_tcRef.real.es.nMasterAddr = c_tcRef.real.cid.nTcAddr[CAR_TAIL];
					if (bHead)	c_tcRef.real.es.nMasterDuty = CAR_HEAD;
					else	c_tcRef.real.es.nMasterDuty = CAR_TAIL;
					MSGLOG("[TC]Sync. bus killing cycle. requester %d.\r\n", c_tcRef.real.es.nMasterDuty);
				}
				else	c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_KILLBUSREQUEST;
			}
			else {
				if (bHead && -- c_tcRef.real.es.nCounter == 0) {
					// kill busB
					InterTrigger(&c_tcRef.real.es.wBusBMonitor, TIME_BUSBMONIT);
					c_tcRef.real.nActiveBus |= INHIBITBUS_B;
					//if (c_tcRef.real.es.nMasterAddr == c_tcRef.real.nAddr)
					if (c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nMasterDuty] == c_tcRef.real.nAddr)
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_KILLBUSREQUEST;
					c_tcRef.real.es.nDownloadStep = DOWNLOADSTEP_WAITENTRYBUS;
					c_tcRef.real.es.wReadError = c_tcRef.real.es.wWriteError = c_tcRef.real.es.wFrameError = 0;
					InterTrigger(&c_tcRef.real.es.wSyncTimer, TIME_BUSBEMPTYMONIT);
					MSGLOG("[TC]Kill bus B.\r\n");
				}
			}

			BYTE dstep = c_tcRef.real.es.nDownloadStep & DOWNLOADSTEP_ONLY;
			if (dstep != 0) {
				switch (dstep) {
				case DOWNLOADSTEP_READYENTRYBUS :
					c_tcRef.real.es.nDownloadStep = DOWNLOADSTEP_ENTRYBUS;
					if ((c_tcRef.real.bm.wResv & (1 << c_tcRef.real.es.nTargetDuty)) &&
							(((c_tcRef.real.nProperty & PROPERTY_ATLABORATORY) && (c_tcRef.real.bm.wMain & 5) == 5) ||
									(c_tcRef.real.bm.wMain & 0xffe) == 0xffe)) {
						c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_KILLBUSREQUEST;
						c_tcRef.real.es.nCounter = COUNT_WAITOPPODIR;
						Hangup(0);
						MSGLOG("[TC]Start download from bus. target is %d.\r\n", c_tcRef.real.es.nTargetDuty);
					}
					else {
						Hangup(FILEMSG_BEGIN + FILEERROR_FAILRESERVEBUS);
						MSGLOG("[TC]ERROR:main=%04X resv=%04X oppo=%02X property=%d.\r\n",
								(c_tcRef.real.bm.wMain & 0xffe), c_tcRef.real.bm.wResv, 1 << c_tcRef.real.es.nTargetDuty,
								(c_tcRef.real.nProperty & PROPERTY_ATLABORATORY));
						c_tcRef.real.nState &= ~(STATE_PROGRESSING | STATE_DOWNLOADING);
						// Modified 2013/03/11
						c_tcRef.real.es.nDownloadStep = DOWNLOADSTEP_NON;	// reset download step for next downloading
						c_tcRef.real.nState &= ~(STATE_DOWNLOADING | STATE_PROGRESSING);
					}
					break;
				case DOWNLOADSTEP_WAITENTRYBUS :
					if (c_tcRef.real.es.wSyncTimer == 0) {
						c_tcRef.real.es.nDownloadStep = DOWNLOADSTEP_DOWNLOADING;
						c_tcRef.real.nState |= STATE_PROGRESSING;
						//if (c_tcRef.real.es.nMasterAddr == c_tcRef.real.nAddr) {
						if (c_tcRef.real.cid.nTcAddr[c_tcRef.real.es.nMasterDuty] == c_tcRef.real.nAddr) {
							c_mate.RequestDir();
							Hangup(FILEMSG_BEGIN + FILEWORK_WORKING);
							MSGLOG("[TC]Start download by bus.\r\n");
						}
					}
					break;
				case DOWNLOADSTEP_READYEXITBUS :
					break;
				default :
					break;
				}
			}
			// ===== end download by bus =====================================================================

			// ===== start download command by bus ===================================================================
			if (c_dozMain.tcs[OWN_SPACE].real.nIssue & ISSUE_DOWNLOAD) {
				if (c_tcRef.real.es.nDownloadMethod == OWN_SPACE) {	// requester
					if (c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.nIssue & ISSUE_DOWNLOAD) {
						c_nDownloadPulse |= (1 << c_tcRef.real.es.nTargetDuty);
						c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nAck = c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dl.nAck;
						c_dozMain.tcs[OWN_SPACE].real.each.info.dl.iRespond = c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dl.iRespond;
						c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wProgress = c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dl.wProgress;
						Hangup(FILEMSG_BEGIN + c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dl.iRespond);
						if (c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nAck == FILEWORK_END) {
							c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_DOWNLOAD;
							c_tcRef.real.nState &= ~STATE_PROGRESSING;
							Hangup(FILEMSG_BEGIN + c_dozMain.tcs[c_tcRef.real.es.nTargetDuty].real.each.info.dl.iRespond);
							MSGLOG("[TC]Close active download.\r\n");
						}
					}
					else {
						c_nDownloadPulse &= ~(1 << c_tcRef.real.es.nTargetDuty);
						if (++ c_tcRef.real.es.nRespCounter >= COUNT_DOWNLOADSYNC) {
							c_tcRef.real.es.nRespCounter = 0;
							c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_DOWNLOAD;
							c_tcRef.real.nState &= ~STATE_PROGRESSING;
							Hangup(FILEMSG_BEGIN + FILEERROR_FAILOPPOSITE);
							MSGLOG("[TC]ERROR:Do not receive download respond of %d.\r\n", c_tcRef.real.es.nDownloadMethod);
						}
					}
				}
				else {		// be download
					if (c_dozMain.tcs[c_tcRef.real.es.nDownloadMethod].real.nIssue & ISSUE_DOWNLOAD) {
						//c_nDownloadPulse |= (1 << c_tcRef.real.es.nDownloadMethod);
						if (c_tcRef.real.nState & STATE_DOWNLOADING) {
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nAck = c_pBucket->nMainAck;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.iRespond = c_pBucket->iRespond;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wProgress = c_pBucket->wProgress;
							if (c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nAck == BEILACK_WORKEND) {
								c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wProgress = 100;
								c_tcRef.real.nState &= ~STATE_DOWNLOADING;
								c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
								MSGLOG("[TC]End of passive downloading.\r\n");
							}
						}
					}
					else {	// download cancel from requester
						//c_nDownloadPulse &= ~(1 << c_tcRef.real.es.nDownloadMethod);
						c_pBucket->wState |= BEILSTATE_CMDEXIT;
						c_tcRef.real.nState &= ~STATE_DOWNLOADING;
						c_tcRef.real.nProperty &= ~PROPERTY_LOGMASKATDOWNLOAD;
						c_dozMain.tcs[OWN_SPACE].real.nIssue &= ~ISSUE_DOWNLOAD;
						MSGLOG("[TC]Close passive download.\r\n");
					}
				}
			}
			else {
				// scan download command
				for (UCURV n = CAR_HEAD; n < LENGTH_TC; n ++) {
					if (n == OWN_SPACE)	continue;
					if (c_dozMain.tcs[n].real.each.wId == TCINFORM_DOWNLOAD &&
							c_dozMain.tcs[n].real.each.info.dl.nDuty == OWN_SPACE) {
						if ((c_dozMain.tcs[n].real.nIssue & ISSUE_DOWNLOAD) && !(c_nDownloadPulse & (1 << n))) {
							c_nDownloadPulse |= (1 << n);
							c_tcRef.real.es.nDownloadMethod = n;
							c_dozMain.tcs[OWN_SPACE].real.nIssue |= ISSUE_DOWNLOAD;
							c_dozMain.tcs[OWN_SPACE].real.each.wId = TCINFORM_DOWNLOAD;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nDuty = n;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nTotal = c_dozMain.tcs[n].real.each.info.dl.nTotal;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wYear = c_dozMain.tcs[n].real.each.info.dl.wYear;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nMonth = c_dozMain.tcs[n].real.each.info.dl.nMonth;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nDay = c_dozMain.tcs[n].real.each.info.dl.nDay;
							c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wProgress = 0;
							if (c_pBucket != NULL && !(c_tcRef.real.nState & STATE_DOWNLOADING) &&
									c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nMonth != 0 &&
									c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nDay != 0) {
								c_pBucket->src.dt.year = (BYTE)c_dozMain.tcs[OWN_SPACE].real.each.info.dl.wYear;
								c_pBucket->src.dt.month = c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nMonth;
								c_pBucket->src.dt.day = c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nDay;
								Lock();
								RESET_BEILACK();
								BeilTimeTrigger();
								BeilCmd(BEILCMD_SELTOUSB);
								Unlock();
								c_dozMain.tcs[OWN_SPACE].real.each.info.dl.nAck = FILEERROR_NON;
								c_dozMain.tcs[OWN_SPACE].real.each.info.dl.iRespond = FILEERROR_NON;
								c_tcRef.real.nState |= STATE_DOWNLOADING;
								c_tcRef.real.nProperty |= PROPERTY_LOGMASKATDOWNLOAD;
								MSGLOG("[TC]Passive downloading %02d/%02d/%02d from %d.\r\n",
										c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day, n);
							}
							else	c_dozMain.tcs[OWN_SPACE].real.each.info.dl.iRespond = FILEERROR_CANNOTOPENDRV;
						}
						else if (!(c_dozMain.tcs[n].real.nIssue & ISSUE_DOWNLOAD) && (c_nDownloadPulse & (1 << n)))
							c_nDownloadPulse &= ~(1 << n);
					}
				}
			}
			// ===== end download command by bus ===================================================================
		}

		ClearBusTrace();
		RECTIME(TTIME_TCUPDATE);
		c_tcRef.real.cycle.wState |= TCSTATE_FRAMEEND;
	}
}

void CTcx::MaReceivePert(DWORD dwParam)
{
	c_tcRef.real.es.nDownloadStep &= ~DOWNLOADSTEP_WAITRESPOND;
	InterTrigger(&c_tcRef.real.es.wBusBMonitor, TIME_BUSBMONIT);
	PTCPERT pPert = c_busB.pLine->GetPert();
	memcpy((PVOID)&c_pertTake, (PVOID)pPert, sizeof(TCPERT));
	// Modified 2013/03/04
	//c_mate.Research();
	WORD cur_add = GETLWORD(c_pertTake.wAddr);
	if ((cur_add & 0xff) == c_tcRef.real.nAddr)
		c_mate.Research();
}

void CTcx::MaReceiveFault(DWORD dwParam)
{
	WORD w = HIGHWORD(dwParam);
	BOOL bCh = BOOL(LOWWORD(dwParam));
	if (w >= SIZE_BUSFAULT)	w = SIZE_BUSFAULT - 1;
	INCBYTE(c_tcRef.real.nBusFailCounter[w]);
	if (bCh)	c_busA.pLine->EnableIntr(TRUE);
	else	c_busB.pLine->EnableIntr(TRUE);
	BUSFAULTLAMPON();
	//MSGLOG("[CAUTION]EnableIntr by rec fail.\r\n");	// ?????
}

// Modified 2013/01/09
void CTcx::MaReceiveLocal(DWORD dwParam)
{
	WORD id = LOWWORD(dwParam);
	c_tcRef.real.lm.wAck |= (1 << id);
	if (id <= LOCALID_TWC) {
		c_tcRef.real.lm.wAck |= (1 << id);
		if (id == LOCALID_ATO) {
			PATOAINFO paAto = GetAto(TRUE);
			if (paAto == NULL || CheckAtoReply(paAto) != FIRMERROR_NON) {
				ShotMainDev(SCENE_ID_HEADCAR, 616);
				if (IsPermitMainB())
					INCBYTE(c_tcRef.real.atoCmd.nAtoFaultCount);
			}
			else {
				CUTB(SCENE_ID_HEADCAR, 616);
				if (paAto->nFlow == LOCALFLOW_NORMAL)	c_pLocal->AtoAnnals(paAto);
				c_chaser.AtoTraceCtrl(paAto);
				c_tcRef.real.atoCmd.nAtoFaultCount = 0;
			}
			if (c_pBucket != NULL &&
					!(c_pBucket->wTicker & BEILTICKER_ATOA)) {
				memcpy((PVOID)&c_pBucket->ato.f.a, (PVOID)&c_lcFirm.aAto, SIZE_ATOSA);
				c_pBucket->wTicker |= BEILTICKER_ATOA;
			}
		}
		if ((c_tcRef.real.lm.wAck & ((1 << LOCALID_ATCA) | (1 << LOCALID_ATCB))) == ((1 << LOCALID_ATCA) | (1 << LOCALID_ATCB))) {
			c_tcRef.real.cond.nCurrentAtc = 0;
			// Check ATC1
			if (CheckAtcReply(&c_lcFirm.aAtcBuff[0]) == FIRMERROR_NON) {
				if (c_lcFirm.aAtcBuff[0].nFlow == LOCALFLOW_TRACE || c_lcFirm.aAtcBuff[0].d.st.req.b.oatc)
					c_tcRef.real.cond.nCurrentAtc = 1;
				if (c_lcFirm.aAtcBuff[0].nFlow == LOCALFLOW_NORMAL)
					c_pLocal->AtcAnnals(&c_lcFirm.aAtcBuff[0], TRUE);
				c_tcRef.real.atoCmd.nAtc1FaultCount = 0;
				CUTB(SCENE_ID_HEADCAR, 630);
				CUTB(SCENE_ID_HEADCAR, 689);
			}
			else {
				ShotMainDev(SCENE_ID_HEADCAR, 630);
				if (IsPermitMainB()) {
					INCBYTE(c_tcRef.real.atoCmd.nAtc1FaultCount);
					if (c_tcRef.real.atoCmd.nAtc1FaultCount >= FSBTERM_FAULTATCCOUNT)
						//ShotMainDev(SCENE_ID_HEADCAR, 689);
						SHOTB(SCENE_ID_HEADCAR, 689);
				}
			}
			// Check ATC2
			if (CheckAtcReply(&c_lcFirm.aAtcBuff[1]) == FIRMERROR_NON) {
				if (c_tcRef.real.cond.nCurrentAtc == 0 &&
						(c_lcFirm.aAtcBuff[1].nFlow == LOCALFLOW_TRACE || c_lcFirm.aAtcBuff[1].d.st.req.b.oatc))
					c_tcRef.real.cond.nCurrentAtc = 2;
				if(c_lcFirm.aAtcBuff[0].nFlow == LOCALFLOW_NORMAL)
					c_pLocal->AtcAnnals(&c_lcFirm.aAtcBuff[1], FALSE);
				c_tcRef.real.atoCmd.nAtc2FaultCount = 0;
				CUTB(SCENE_ID_HEADCAR, 631);
				CUTB(SCENE_ID_HEADCAR, 690);
			}
			else {
				ShotMainDev(SCENE_ID_HEADCAR, 631);
				if (IsPermitMainB()) {
					INCBYTE(c_tcRef.real.atoCmd.nAtc2FaultCount);
					if (c_tcRef.real.atoCmd.nAtc2FaultCount >= FSBTERM_FAULTATCCOUNT)
						//ShotMainDev(SCENE_ID_HEADCAR, 690);
						SHOTB(SCENE_ID_HEADCAR, 690);
				}
			}
			// judge ATC
			LockSig();
			memcpy((PVOID)&c_lcFirm.aAtc[0], (PVOID)&c_lcFirm.aAtcBuff[0], sizeof(ATCAINFO));
			memcpy((PVOID)&c_lcFirm.aAtc[1], (PVOID)&c_lcFirm.aAtcBuff[1], sizeof(ATCAINFO));
			PATCAINFO paAtc;
			if (c_tcRef.real.cond.nCurrentAtc == 1) {
				c_tcRef.real.cond.nActiveAtc = LOCALID_ATCA + 1;
				paAtc = &c_lcFirm.aAtc[0];
			}
			else {
				if (c_tcRef.real.cond.nCurrentAtc == 2)
					c_tcRef.real.cond.nActiveAtc = LOCALID_ATCB + 1;
				else {
					c_tcRef.real.cond.nActiveAtc = 0;
					if (c_tcRef.real.op.nMode != OPMODE_EMERGENCY)
						ShotMainDev(SCENE_ID_HEADCAR, 676);
					else	CUTB(SCENE_ID_HEADCAR, 676);
				}
				paAtc = &c_lcFirm.aAtc[1];
			}
			UnlockSig();
			if (c_tcRef.real.cond.nActiveAtc != 0) {
				// Modified 2013/01/21
				//CalcAtcSpeed();
				c_pLocal->AtcAnnals(paAtc);
				c_chaser.AtcTraceCtrl(paAtc);
			}
			if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_ATCA)) {
				memcpy((PVOID)&c_pBucket->atc.f.a, (PVOID)paAtc, SIZE_ATCSA);
				c_pBucket->wTicker |= BEILTICKER_ATCA;
			}
		}
		if ((c_tcRef.real.lm.wAck & (1 << LOCALID_ATO)) &&
				(c_tcRef.real.lm.wAck & (1 << LOCALID_ATCA)) && (c_tcRef.real.lm.wAck & (1 << LOCALID_ATCB))) {
			// Modified 2013/01/21
			CalcAtcSpeed();
			ControlByMainDev();
		}
		RECTIME(TTIME_LOCALCLOSE);
	}
	else {
		switch (id) {
		case LOCALID_CSC :
			if (HIGHNIBBLE(c_lcFirm.aCsc.nStx) == LOCALERROR_NON)
				CUTB(SCENE_ID_HEADCAR, 546);
			else	ShotAidDev(SCENE_ID_HEADCAR, 546);
			break;
		case LOCALID_PIS :
			if (HIGHNIBBLE(c_lcFirm.aPis.nStx) == LOCALERROR_NON) {
				c_look.PisAnnals(&c_lcFirm.aPis);
				CUTB(SCENE_ID_HEADCAR, 594);
			}
			else	ShotAidDev(SCENE_ID_HEADCAR, 594);
			break;
		case LOCALID_TRS :
			if (HIGHNIBBLE(c_lcFirm.aTrs.nStx) == LOCALERROR_NON) {
				c_look.TrsAnnals(&c_lcFirm.aTrs);
				CUTB(SCENE_ID_HEADCAR, 595);
			}
			else	ShotAidDev(SCENE_ID_HEADCAR, 595);
			break;
		case LOCALID_HTC :
			if (HIGHNIBBLE(c_lcFirm.aHtc.nStx) == LOCALERROR_NON) {
				c_look.HtcAnnals(&c_lcFirm.aHtc);
				CUTB(SCENE_ID_HEADCAR, 545);
			}
			else	ShotAidDev(SCENE_ID_HEADCAR, 545);
			break;
		case LOCALID_PSD :
			if (HIGHNIBBLE(c_lcFirm.aPsd.nStx) == LOCALERROR_NON) {
				c_look.PsdAnnals(&c_lcFirm.aPsd);
				CUTB(SCENE_ID_HEADCAR, 596);
			}
			else	ShotAidDev(SCENE_ID_HEADCAR, 596);
			break;
		default :
			break;
		}
	}
}

void CTcx::MaReceiveDuc(DWORD dwParam)
{
	if (!(c_tcRef.real.nObligation & OBLIGATION_MAIN))	return;	// rx from duc
	BYTE* p = (BYTE*)dwParam;
	c_doc.KeyAction(p);
}

BEGIN_MAIL(CTcx, CAnt)
	_MAIL(MA_ACCEPT, CTcx::MaAccept)
	_MAIL(MA_SENDBUS, CTcx::MaSendBus)
	_MAIL(MA_RECEIVEBUS, CTcx::MaReceiveBus)
	_MAIL(MA_RECEIVEPERT, CTcx::MaReceivePert)
	_MAIL(MA_RECEIVEFAULT, CTcx::MaReceiveFault)
	_MAIL(MA_RECEIVELOCAL, CTcx::MaReceiveLocal)
	_MAIL(MA_RECEIVEDUC, CTcx::MaReceiveDuc)
END_MAIL();
