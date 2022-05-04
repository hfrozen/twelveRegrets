/*
 * CCcx.cpp
 *
 *  Created on: 2010. 12. 23
 *      Author: Che
 */

#include <sys/neutrino.h>
#include <inttypes.h>

#include <pthread.h>
#include <math.h>
#include "CCcx.h"

const PSZ	CCcx::c_timeNames[] = {
	(PSZ)"TIME_BEGIN",
	(PSZ)"TIME_DEVSEND",
	(PSZ)"TIME_DEVSENDED",
	(PSZ)"TIME_V3FSEND",
	(PSZ)"TIME_V3FSENDED",
	(PSZ)"TIME_ECUSEND",
	(PSZ)"TIME_ECUSENDED",
	(PSZ)"TIME_DCUSEND",
	(PSZ)"TIME_DCUSENDED",
	(PSZ)"TIME_CMSBSEND",
	(PSZ)"TIME_CMSBSENDED",
	(PSZ)"TIME_OPENDEV",
	(PSZ)"TIME_CLOSEV3F",
	(PSZ)"TIME_CLOSESIV",
	(PSZ)"TIME_CLOSEECU",
	(PSZ)"TIME_CLOSEDCU",
	(PSZ)"TIME_CLOSECMS",
	(PSZ)"TIME_BUSRECVOTHER",
	(PSZ)"TIME_BUSSENDA",
	(PSZ)"TIME_BUSSENDEDA",
	(PSZ)"TIME_BUSSENDB",
	(PSZ)"TIME_BUSSENDEDB",
	(PSZ)"TIME_CYCLEEND",
	(PSZ)"TIME_TICKERBEGIN",
	(PSZ)"TIME_TICKEREND",
	(PSZ)"TIME_BUSPRESENT",
	(PSZ)"TIME_BUSPRESENTED",
	(PSZ)"TIME_MAXSHEET",
	NULL
};

void CCcx::Interval(PVOID pVoid)
{
	CCcx* pCc = (CCcx*)pVoid;
	pCc->SysTimer();
}

CCcx::CCcx()
{
	c_mail = -1;
	c_bMfb = FALSE;
	c_pTimer = NULL;
	c_pPocket = NULL;
	c_main.wState = 0;
	c_main.pLine = NULL;
	c_resv.wState = 0;
	c_resv.pLine = NULL;
	c_hShm = -1;
	c_pBucket = NULL;

	c_pDoz = &c_doz;

	memset((PVOID)&c_ccRef, 0, sizeof(CCREFERENCE));
	c_ccRef.real.car.nID =
		c_ccRef.real.car.nLength = 0xff;
	c_ccRef.real.cycle.wTime = CYCLETIME_TBUS;
	for (UCURV n = 0; n < LENGTH_AXEL; n ++) {
		c_ccRef.real.dia[n].wV = DEFAULT_WHEELDIA;
		c_ccRef.real.dia[n].nCur = DEFAULT_WHEELDIA - MIN_WHEELDIA;
		c_ccRef.real.dia[n].nDeb = 0;
		CalcSpeedBase(n);
	}
	c_ccRef.real.pwm.high = c_ccRef.real.pwm.period = 0xffff;
	c_ccRef.real.nDoorFlow = 0xff;

	memset((PVOID)&c_doz, 0, sizeof(CCDOZ));
	memset((PVOID)&c_rSiv[0], 0, sizeof(SIVRINFO));
	memset((PVOID)&c_rSiv[1], 0, sizeof(SIVRINFO));
	memset((PVOID)&c_rSiv[2], 0, sizeof(SIVRINFO));
	memset((PVOID)&c_rV3f[0], 0, sizeof(V3FRINFO));
	memset((PVOID)&c_rV3f[1], 0, sizeof(V3FRINFO));
	memset((PVOID)&c_rV3f[2], 0, sizeof(V3FRINFO));
	memset((PVOID)&c_rEcu[0], 0, sizeof(ECURINFO));
	memset((PVOID)&c_rEcu[1], 0, sizeof(ECURINFO));
	memset((PVOID)&c_rEcu[2], 0, sizeof(ECURINFO));
	memset((PVOID)&c_rCmsb[0], 0, sizeof(CMSBRINFO));
	memset((PVOID)&c_rCmsb[1], 0, sizeof(CMSBRINFO));
	memset((PVOID)&c_rDcu[0], 0, sizeof(DCURINFO));
	memset((PVOID)&c_rDcu[1], 0, sizeof(DCURINFO));
	memset((PVOID)&c_aSiv, 0, sizeof(SIVAINFO));
	memset((PVOID)&c_aV3f, 0, sizeof(V3FAINFO));
	memset((PVOID)&c_aEcu, 0, sizeof(ECUAINFO));
	memset((PVOID)&c_aDcuc, 0, sizeof(DCUAINFO) * 2);
	memset((PVOID)&c_tbTime, 0, sizeof(TBTIME));
	memset((PVOID)&c_devTime, 0, sizeof(DEVTIME));
	memset((PVOID)&c_sivch.ta, 0, sizeof(TRACEARCH));
	memset((PVOID)&c_v3fch.ta, 0, sizeof(TRACEARCH));
	memset((PVOID)&c_ecuch.ta, 0, sizeof(TRACEARCH));
	c_ccRef.real.wEskMonitor = 0;
	c_syncCur = c_syncMax = 0;
	c_syncMin = 1000;

	c_bTog = FALSE;
}

CCcx::~CCcx()
{
	Destroy();
}

BOOL CCcx::Trigger()
{
	if (c_pTimer != NULL) {
		c_pTimer->Release();
		delete c_pTimer;
	}
	c_pTimer = new CTimer(this);
	c_pTimer->SetOwner(this);
	c_pTimer->SetOwnerName((PSZ)"CCx");
	c_pTimer->Initial(PRIORITY_TIMER, Interval, 0, TRUE);
	int res = c_pTimer->Trigger(TIME_INTERVAL);
	return (res == 0 ? TRUE : FALSE);
}

BOOL CCcx::Seed()
{
	if (!c_panel.Initial()) {
		MSGLOG("[INIT]ERROR:can not initial gpio.\r\n");
		return FALSE;
	}
	if (!Trigger()) {
		MSGLOG("[INIT]ERROR:timer failure!\r\n");
		return FALSE;
	}
	return TRUE;
}

BOOL CCcx::BusSetup()
{
	KILL(c_main.pLine);
	c_main.wState = 0;
	KILL(c_resv.pLine);
	c_resv.wState = 0;

	c_main.pLine = new CBusCarx(this, c_mail, (PSZ)"CBusMain");
	c_main.pLine->SetOwnerName((PSZ)"CBusMain");
	c_main.pLine->SetCycleTime(c_ccRef.real.cycle.wTime);
	int res;
	if ((res = c_main.pLine->Open(MF_MANCHA,
			(WORD)(c_ccRef.real.car.nID + CAR_CC + 1), TRUE)) != PORTERROR_NON) {
		KILL(c_main.pLine);
		MSGLOG("[INIT]ERROR:can not initial main bus(%d).\r\n", res);
		return FALSE;
	}
	c_main.wState |= BUSSTATE_ENABLE;

	c_resv.pLine = new CBusCarx(this, c_mail, (PSZ)"CBusResv");
	c_resv.pLine->SetOwnerName((PSZ)"CBusResv");
	c_resv.pLine->SetCycleTime(c_ccRef.real.cycle.wTime);
	if ((res = c_resv.pLine->Open(MF_MANCHB,
			(WORD)(c_ccRef.real.car.nID + CAR_CC + 1), FALSE)) != PORTERROR_NON) {
		KILL(c_resv.pLine);
		MSGLOG("[INIT]ERROR:can not initial reserve bus(%d).\r\n", res);
		return FALSE;
	}
	c_resv.wState |= BUSSTATE_ENABLE;
	return TRUE;
}

BOOL CCcx::OpenPost()
{
	g_mqPost = mq_open(NAME_SERV, O_WRONLY);
	if (IsValid(g_mqPost)) {
		g_post.dwLength = (DWORD)this;
		g_post.wID = MSG_CONCC;
		mq_send(g_mqPost, (char*)&g_post, sizeof(SMSG), NULL);	// PRIORITY_POST);

		return TRUE;
	}
	else	MSGLOG("[INIT]ERROR:can not open post queue(%s).\r\n", strerror(errno));
	g_mqPost = -1;
	return FALSE;
}

BOOL CCcx::OpenBeil()
{
	g_mqBeil = mq_open(NAME_BEIL, O_WRONLY);
	if (IsValid(g_mqBeil)) {
		if ((c_hShm = shm_open(NAME_BUCKET, O_RDWR, 0777)) < 0) {
			MSGLOG("[INIT]ERROR:can not open shared memory(%s).\r\n", strerror(errno));
			mq_close(g_mqBeil);
			g_mqBeil = -1;
			return FALSE;
		}
		c_pBucket = (PBUCKET)mmap(NULL, sizeof(WORD), PROT_READ | PROT_WRITE, MAP_SHARED, c_hShm, 0);
		if (c_pBucket == NULL) {
			MSGLOG("[INIT]ERROR:can not access to shared memory(%s).\r\n", strerror(errno));
			close(c_hShm);
			c_hShm = -1;
			mq_close(g_mqBeil);
			g_mqBeil = -1;
			return FALSE;
		}
		return TRUE;
	}
	else	MSGLOG("[INIT]ERROR:can not open beil queue(%s).\r\n", strerror(errno));
	g_mqBeil = -1;
	return FALSE;
}

BOOL CCcx::TakingOff()
{
	pthread_mutex_init(&c_mutex, NULL);
	pthread_mutex_init(&c_mutexA, NULL);

	c_ccRef.real.cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	MSGLOG("[INIT]%f cycles/sec.\n", c_ccRef.real.cps);
	if (!Seed())	return FALSE;

	int pid;
	PROCSTEP(1);
	if ((pid = c_panel.FindExec((PSZ)"nand/bin/mq")) < 0) {
		if ((pid = c_panel.FindExec((PSZ)"proc/boot/mq")) < 0) {
			PROCALARM(1);
			MSGLOG("[INIT]ERROR:can not found mq process.\r\n");
			return FALSE;
		}
	}
	if ((c_mail = CREATEMQ(NAME_CCMQ)) < 0) {
		PROCALARM(1);
		MSGLOG("[INIT]ERROR:can not create message queue(%s).\r\n",
				strerror(errno));
		return FALSE;
	}

	PROCSTEP(2);
	int res;
	if ((res = c_panel.Capture(&c_ccRef.real.car)) != FROMPANELFAULT_NON) {
		MSGLOG("[INIT]ERROR:can not capture from panel(%d)!\r\n", res);
		PROCALARM(2);
		return FALSE;
	}
	c_ccRef.real.wActiveDev = c_panel.GetCcDev(&c_ccRef.real.car);
	if (c_ccRef.real.wActiveDev == 0) {
		MSGLOG("[INIT]ERROR:can not found any device(%d-%d).\r\n", c_ccRef.real.car.nID, c_ccRef.real.car.nLength);
		PROCALARM(2);
		return FALSE;
	}
	MSGLOG("[INIT]Car ID = %d Length = %d Device flags = %02X.\r\n",
			c_ccRef.real.car.nID, c_ccRef.real.car.nLength, c_ccRef.real.wActiveDev);

	PROCSTEP(3);
	if ((res = init_mfb()) != 0) {
		MSGLOG("[INIT]ERROR:can not initial mfb driver!(%s)\r\n", res);
		PROCALARM(3);
		return FALSE;
	}
	c_bMfb = TRUE;

	PROCSTEP(4);
	if (c_probe.Open() != 0) {
		MSGLOG("[INIT]ERROR:can not initial io driver!\r\n");
		PROCALARM(4);
		return FALSE;
	}
	WORD w = 0;
	for (UCURV n = 0; n < SIZE_DOUTEX; n ++)
		c_probe.Putd(n, &w);

	if (c_ccRef.real.wActiveDev & DEVBEEN_PWM) {
		if (c_pwm.Open() != 0) {
			MSGLOG("[INIT]ERROR:can not initial pwm driver!\r\n");
			PROCALARM(4);
			return FALSE;
		}
		DWORD dw = 0;
		c_pwm.Put(&dw);
	}

	PROCSTEP(5);
	KILL(c_pPocket);
	c_pPocket = new CPocket(this, c_mail);
	c_pPocket->InitialModule();
	if ((res = c_pPocket->InitDev()) != DEVFAULT_NON) {
		MSGLOG("[INIT]ERROR:can not initial devices(%d).\r\n", res);
		KILL(c_pPocket);
		PROCALARM(5);
		return FALSE;
	}

	PROCSTEP(6);
	if (!BusSetup()) {
		PROCALARM(6);
		return FALSE;
	}
	c_ccRef.real.wProperty |= PROPERTY_ENABLEBUS;

	PROCSTEP(7);
	if (OpenPost()) {
		MSGLOG("[INIT]open post.\r\n");
		BusPresentEntry();
	}
	else	MSGLOG("[INIT]ERROR:can not open post.\t\n");

	PROCSTEP(8);
	if (OpenBeil()) {
		MSGLOG("[INIT]open Beil.\r\n");
	}
	else {
		if (c_ccRef.real.wProperty & PROPERTY_ATLABORATORY) {
			MSGLOG("[INIT]ERROR:can not open Beil.\r\n");
		}
		else {
			MSGLOG("[INIT]ERROR:can not open beil.\r\n");
			PROCALARM(8);
			return FALSE;
		}
	}

	PROCSTEP(9);
	if (c_pBucket != NULL && (c_pBucket->wState & BEILSTATE_BEENMEM)) {
		c_ccRef.real.wProperty |= PROPERTY_USEEXTERNALMEM;
		MSGLOG("[INIT]logbook to external memory.\r\n");
	}
	else {
		if (c_ccRef.real.wProperty & PROPERTY_ATLABORATORY) {
			c_ccRef.real.wProperty &= ~PROPERTY_USEEXTERNALMEM;
			MSGLOG("[INIT]ignore logbook.\r\n");
		}
		else {
			MSGLOG("[INIT]ERROR:can not found external memory.\r\n");
			PROCALARM(9);
			return FALSE;
		}
	}

	PROCSTEP(8);
	c_panel.FlashLamp(0x20);

	return TRUE;
}

void CCcx::SysTimer()
{
	DevTrigger();
	c_panel.Interval();
	BusMonit();

	if (c_ccRef.real.log.nState & STATELOG_SCAN) {
		if (c_pBucket->nPeriodAck == BEILACK_WORKEND || -- c_ccRef.real.log.wTimer == 0) {
			c_ccRef.real.log.nState &= ~STATELOG_SCAN;
			c_ccRef.real.log.wTimer = 0;
			if (c_pBucket->nPeriodAck == BEILACK_WORKEND) {
				uint64_t fe = ClockCycles();
				double sec = (double)(fe - c_logTriggerTime) / (double)c_ccRef.real.cps;
				c_ccRef.real.log.wCurTime = (WORD)(sec * 1000.f);
				if (c_ccRef.real.log.wMaxTime < c_ccRef.real.log.wCurTime)
					c_ccRef.real.log.wMaxTime = c_ccRef.real.log.wCurTime;
			}
			else	MSGLOG("[FILE]ERROR:append drv.data timeout.\r\n");
		}
	}

	if (++ c_ccRef.real.log.nCounter >= CYCLE_TICKER) {
		c_ccRef.real.log.nCounter = 0;
		if (c_pBucket != NULL && (c_ccRef.real.wProperty & PROPERTY_USEEXTERNALMEM)) {	// &&
				//(c_doz.tc.real.nOutput[3] & 8)) {	// HCR
			c_pBucket->wTicker &= ~BEILTICKER_DRV;
			if (c_pBucket->wTicker != 0) {
				c_logTriggerTime = ClockCycles();
				RESET_BEILACK();
				CmdLine(BEILCMD_APPENDDRV);
				c_ccRef.real.log.nState |= STATELOG_SCAN;
				c_ccRef.real.log.wTimer = TIME_BEILWAIT / 5;
			}
		}
	}
}

void CCcx::BusMonit()
{
	if (c_ccRef.real.cycle.wOverLampOnTimer > 0) {
		if (-- c_ccRef.real.cycle.wOverLampOnTimer == 0)
			c_panel.ClrLamp(0x40);
	}
	if (++ c_ccRef.real.cycle.wOver >= CYCLEOVER_TBUSPERIOD && (c_ccRef.real.wProperty & PROPERTY_ENABLEBUS)) {
		c_ccRef.real.cycle.wOver = 0;
		c_main.pLine->EnableIntr(TRUE);
		c_resv.pLine->EnableIntr(TRUE);
		c_panel.SetLamp(0x40);
		c_ccRef.real.cycle.wOverLampOnTimer = 1;
		if (c_ccRef.real.wActiveDev & DEVBEEN_ESK) {
			if (c_ccRef.real.wEskMonitor != 0 && -- c_ccRef.real.wEskMonitor == 0) {
				CLRCOBITC(CCDOB_ESK);
				OutputD();
			}
		}
		BusPresent();
	}
}

void CCcx::CopyTime(PDEVTIME pDevt, PTBTIME pTbt)
{
	pDevt->year = (BYTE)(pTbt->year & 0xff);
	memcpy((PVOID)&pDevt->month, (PVOID)&pTbt->month, 5);
}

void CCcx::CopyOut()
{
	if (c_ccRef.real.car.nID < (c_ccRef.real.car.nLength - 1)) {
		for (UCURV n = 0; n < SIZE_CCDO; n ++)
			c_doz.cc.real.nOutput[n] = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][n];
	}
	else {
		// CC0 door
		BYTE n = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][1];
		c_doz.cc.real.nOutput[0] = (n & 0xe1) | (BitwiseOut(n) & 0x1e);
		n = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][0];
		c_doz.cc.real.nOutput[1] = (n & 0xe1) | (BitwiseOut(n) & 0x1e);
		//c_doz.cc.real.nOutput[0] = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][1];
		//c_doz.cc.real.nOutput[1] = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][0];
		c_doz.cc.real.nOutput[2] = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][2];
		c_doz.cc.real.nOutput[3] = c_doz.tc.real.nOrder[c_ccRef.real.car.nID][3];
	}
	if ((c_ccRef.real.wActiveDev & DEVBEEN_ESK) && GETCOBITC(CCDOB_ESK))
		c_ccRef.real.wEskMonitor = TIME_MONITORESK;
	else	c_ccRef.real.wEskMonitor = 0;
	OutputD();
}

void CCcx::OutputD()
{
	WORD w;
	w = MAKEWORD(c_doz.cc.real.nOutput[1], c_doz.cc.real.nOutput[0]);
	c_probe.Putd(0, &w);
	if (w != c_ccRef.real.wOut[0]) {
		c_ccRef.real.wOut[0] = w;
	}
	w = MAKEWORD(c_doz.cc.real.nOutput[3], c_doz.cc.real.nOutput[2]);

	// Modified 2012/12/25
	if (c_doz.tc.real.nSlaveCtrl & SLAVECCCTRL_INVALIDOUT)	w |= (c_ccRef.real.wKxOut & KXOUTS);
	else	c_ccRef.real.wKxOut = w & KXOUTS;
	c_probe.Putd(1, &w);
	if (w != c_ccRef.real.wOut[1]) {
		c_ccRef.real.wOut[1] = w;
	}
}

void CCcx::Header(char* p, int head)
{
	char buf[10];
	sprintf(buf, "%06d", head);
	for (UCURV n = 0; n < 6; n ++)
		*p ++ = buf[n];
}

void CCcx::CalcTimes()
{
	RECTIME(CTIME_CYCLEEND);
	if (c_ccRef.real.cycle.bOpen) {
		char* p = g_bufLarge;
		memset((PVOID)p, 0, SIZE_BUFSMALL);
		sprintf(p, "000000:VARS:TCYCLE:");
		TOEND(p);
		for (UCURV n = CTIME_DEVICESEND; n < CTIME_MAXSHEET; n ++) {
			if (c_opTimes[n].bUse) {
				uint64_t cycle = c_opTimes[n].cur - c_opTimes[CTIME_BEGIN].cur;
				double sec = (double)cycle / (double)c_ccRef.real.cps;
				sprintf(p, "%d:%.3f|", n, sec * 1000.f);
				TOEND(p);
				c_opTimes[n].bUse = FALSE;
			}
			else {
				sprintf(p, "%d:===|", n);
				TOEND(p);
			}
		}
		*p ++ = ';';
		*p ++ = '\n';
		*p = NULL;
		Header(g_bufLarge, (int)strlen(g_bufLarge) - 7);
		Message(g_bufLarge, (int)strlen(g_bufLarge));
	}
	c_opTimes[CTIME_BEGIN].cur = c_opTimes[CTIME_CYCLEEND].cur;
	c_ccRef.real.cycle.bOpen = TRUE;
}

void CCcx::BusPresent()
{
	RECTIME(CTIME_BUSPRESENT);
	char* p = g_bufLarge;
	memset((PVOID)p, 0, SIZE_BUFLARGE);
	sprintf(p, "000000:VARS:BUSMON:");
	int length = strlen("000000:VARS:BUSMON:");
	p += length;
	memcpy((PVOID)p, (PVOID)&c_ccRef, sizeof(CCREFERENCE));
	length += sizeof(CCREFERENCE);
	p += sizeof(CCREFERENCE);
	memcpy((PVOID)p, (PVOID)&c_doz.tc, sizeof(TCFIRM));
	length += sizeof(TCFIRM);
	p += sizeof(TCFIRM);
	memcpy((PVOID)p, (PVOID)&c_doz.cc, sizeof(CCFIRM));
	length += sizeof(CCFIRM);
	p += sizeof(CCFIRM);
	memcpy((PVOID)p, (PVOID)&c_rSiv[1], sizeof(SIVRINFO));
	length += sizeof(SIVRINFO);
	p += sizeof(SIVRINFO);
	memcpy((PVOID)p, (PVOID)&c_rV3f[1], sizeof(V3FRINFO));
	length += sizeof(V3FRINFO);
	p += sizeof(V3FRINFO);
	memcpy((PVOID)p, (PVOID)&c_rEcu[1], sizeof(ECURINFO));
	length += sizeof(ECURINFO);
	p += sizeof(ECURINFO);
	memcpy((PVOID)p, (PVOID)&c_rCmsb[1], sizeof(CMSBRINFO));
	length += sizeof(CMSBRINFO);
	p += sizeof(CMSBRINFO);
	memcpy((PVOID)p, (PVOID)&c_rDcu[1], sizeof(DCURINFO));
	length += sizeof(DCURINFO);
	p += sizeof(DCURINFO);
	memcpy((PVOID)p, (PVOID)&c_aSiv, sizeof(SIVAINFO));
	length += sizeof(SIVAINFO);
	p += sizeof(SIVAINFO);
	memcpy((PVOID)p, (PVOID)&c_aV3f, sizeof(V3FAINFO));
	length += sizeof(V3FAINFO);
	p += sizeof(V3FAINFO);
	memcpy((PVOID)p, (PVOID)&c_aEcu, sizeof(ECUAINFO));
	length += sizeof(ECUAINFO);
	p += sizeof(ECUAINFO);
	memcpy((PVOID)p, (PVOID)&c_aDcuc[0], sizeof(DCUAINFO));
	length += sizeof(DCUAINFO);
	p += sizeof(DCUAINFO);
	memcpy((PVOID)p, (PVOID)&c_aDcuc[1], sizeof(DCUAINFO));
	length += sizeof(DCUAINFO);
	p += sizeof(DCUAINFO);
	memcpy((PVOID)p, (PVOID)&c_pPocket->c_wPocketDeb[0], sizeof(WORD) * DEVID_MAX);
	length += (sizeof(WORD) * DEVID_MAX);
	p += (sizeof(WORD) * DEVID_MAX);
	memcpy((PVOID)p, (PVOID)&c_pPocket->c_wDoorDeb[0], sizeof(WORD) * 8);
	length += (sizeof(WORD) * 8);
	p += (sizeof(WORD) * 8);
	*p ++ = ';';
	*p = NULL;
	length += 2;

	Header(g_bufLarge, length - 7);
	Message(g_bufLarge, length);

	RECTIME(CTIME_BUSPRESENTED);
}

void CCcx::BusPresentEntry()
{
	char* p = g_bufLarge;
	memset((PVOID)p, 0, SIZE_BUFLARGE);
	strcpy(p, "000000:INIT:TCYCLE:");
	TOEND(p);
	UCURV n;
	for (n = 1; c_timeNames[n] != NULL; n ++) {
		strcpy(p, c_timeNames[n]);
		TOEND(p);
		*p ++ = '|';
	}
	*p ++ = ';';

	strcpy(p, "BUSMON:REFERENCE");
	TOEND(p);
	sprintf(p, "(%s:%d)", "CAR", sizeof(WORD) + sizeof(CARINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "CYCLE", sizeof(CCBUSCYCLE));
	TOEND(p);
	sprintf(p, "(%s:%d)", "PWM", sizeof(PWMARCH));
	TOEND(p);
	sprintf(p, "(%s:%d)", "SPT", sizeof(SPEEDTRIGGER));
	TOEND(p);
	sprintf(p, "(%s:%d)", "SPE", sizeof(SPEEDELEM));
	TOEND(p);
	sprintf(p, "(%s:%d)", "SPR", sizeof(DWORD) * LENGTH_AXEL);
	TOEND(p);
	sprintf(p, "(%s:%d)", "SPB", sizeof(double) * LENGTH_AXEL);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DIA", sizeof(WHEELDIA) * LENGTH_AXEL);
	TOEND(p);
	sprintf(p, "(%s:%d)", "SIVP", sizeof(AVERAGEPOWER));
	TOEND(p);
	sprintf(p, "(%s:%d)", "V3FP", sizeof(AVERAGEPOWER));
	TOEND(p);
	sprintf(p, "(%s:%d)", "IDC", sizeof(AVERAGEIDC));
	TOEND(p);
	sprintf(p, "(%s:%d)", "BIDC", sizeof(AVERAGEIDC));
	TOEND(p);
	sprintf(p, "(%s:%d)", "POCK", sizeof(POCKETTIMER));
	TOEND(p);
	sprintf(p, "(%s:%d)", "LOG", sizeof(LOGINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "REF", sizeof(OPREF));
	TOEND(p);
	sprintf(p, "(%s:%d)", "CPS", sizeof(float));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ROID", sizeof(DWORD));
	TOEND(p);
	sprintf(p, "(%s:%d)", "ETC", sizeof(WORD) * 7 + sizeof(BYTE) * 2);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DEB", sizeof(WORD) * (DEVID_MAX + 8));
	TOEND(p);
	sprintf(p, "(%s:%d)", "DUMMY", SIZE_CCREFERENCE - sizeof(CCREFERENCEREAL));
	TOEND(p);
	*p ++ = '|';
	strcpy(p, "TC");
	TOEND(p);
	p = c_panel.TcNamePlate(p);
	*p ++ = '|';
	strcpy(p, "CC");
	TOEND(p);
	p = c_panel.CcNamePlate(p);
	*p ++ = '|';
	strcpy(p, "V3F:ECU");
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(SIVRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(V3FRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ECURINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(CMSBRINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCURINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(SIVAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(V3FAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(ECUAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", PTYPE(DCUAINFO));
	TOEND(p);
	sprintf(p, "(%s:%d)", "PDEB", sizeof(WORD) * DEVID_MAX);
	TOEND(p);
	sprintf(p, "(%s:%d)", "DDEB", sizeof(WORD) * 8);
	TOEND(p);
	*p ++ = '|';
	*p ++ = ';';
	*p ++ = '\n';
	*p = NULL;
	Header(g_bufLarge, (int)strlen(g_bufLarge) - 7);
	Message(g_bufLarge, (int)strlen(g_bufLarge));
}

void CCcx::AbstractSiv()
{
	memset((PVOID)&c_rSiv[0], 0, sizeof(SIVRINFO));
	c_rSiv[0].wAddr = GETLWORD(DEVADD_SIV);
	WORD pos = 1 << c_ccRef.real.car.nID;
	if (c_doz.tc.real.cSiv.wTracing & pos) {
		// trace data
		if (!(c_sivch.ta.nState & TRACESTATE_LOAD)) {
			c_sivch.ta.nState |= (TRACESTATE_LOAD | TRACESTATE_LOADDEV);
			c_sivch.ta.wiTc = c_sivch.ta.wiCc = 0;
		}
		else {
			if (c_sivch.ta.wiTc < c_sivch.ta.wiCc) {
				c_doz.cc.real.eSiv.d.tr.wIndex = c_sivch.ta.wiTc;
				memcpy((PVOID)&c_doz.cc.real.eSiv.d.tr.nText[0],
						(PVOID)&c_sivch.d[c_sivch.ta.wiTc][0], 100);
				++ c_sivch.ta.wiTc;
			}
			else if (c_sivch.ta.wiCc < c_sivch.ta.wDev)
				c_doz.cc.real.eSiv.d.tr.wIndex = 0xffff;
			c_doz.cc.real.eSiv.wAddr = GETLWORD(DEVADD_SIV);
			c_doz.cc.real.eSiv.nFlow = DEVFLOW_TRACEA;
		}
	}
	else {	// normal data
		c_sivch.ta.nState &= ~(TRACESTATE_LOAD | TRACESTATE_LOADDEV);
		c_rSiv[0].nFlow = DEVFLOW_NORMALR;
		c_rSiv[0].d.st.state.b.pand = c_doz.tc.real.cSiv.wPanto & pos ? TRUE : FALSE;
		c_rSiv[0].d.st.state.b.lors = c_doz.tc.real.cSiv.wLors & pos ? TRUE : FALSE;
		c_rSiv[0].d.st.test.b.tdc = c_doz.tc.real.cSiv.wTraceClear & pos ? TRUE : FALSE;
		c_rSiv[0].d.st.test.b.sivsot = c_doz.tc.real.cSiv.wSivsot & pos ? TRUE : FALSE;
		CopyTime(&c_rSiv[0].d.st.time, &c_doz.tc.real.time);
		memcpy((PVOID)&c_rSiv[2], (PVOID)&c_rSiv[0], sizeof(SIVRINFO));	// backup normal data
	}
	LockA();
	memcpy((PVOID)&c_rSiv[1], (PVOID)&c_rSiv[0], sizeof(SIVRINFO));
	memcpy((PVOID)&c_doz.cc.real.rSiv, (PVOID)&c_rSiv[0], sizeof(SIVRINFO));
	if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_SIVR)) {
		memcpy((PVOID)&c_pBucket->siv.f.r, (PVOID)&c_rSiv[0], SIZE_SIVSR);
		c_pBucket->wTicker |= BEILTICKER_SIVR;
	}
	UnlockA();
}

void CCcx::AbstractV3f()
{
	memset((PVOID)&c_rV3f[0], 0, sizeof(V3FRINFO));
	c_rV3f[0].wAddr = GETLWORD(DEVADD_V3F);
	WORD pos = 1 << c_ccRef.real.car.nID;
	if (c_doz.tc.real.cV3f.wTracing & pos) {
		// trace data
		if (!(c_v3fch.ta.nState & TRACESTATE_LOAD)) {
			c_v3fch.ta.nState |= (TRACESTATE_LOAD | TRACESTATE_LOADDEV);
			c_v3fch.ta.wiTc = c_v3fch.ta.wiCc = 0;
		}
		else {
			if (c_v3fch.ta.wiTc < c_v3fch.ta.wiCc) {
				c_doz.cc.real.eV3f.d.tr.wIndex = c_v3fch.ta.wiTc;
				memcpy((PVOID)&c_doz.cc.real.eV3f.d.tr.nText[0],
						(PVOID)&c_v3fch.d[c_v3fch.ta.wiTc][0], 100);
				++ c_v3fch.ta.wiTc;
			}
			else if (c_v3fch.ta.wiCc < c_v3fch.ta.wDev)
				c_doz.cc.real.eV3f.d.tr.wIndex = 0xffff;
			c_doz.cc.real.eV3f.wAddr = GETLWORD(DEVADD_V3F);
			c_doz.cc.real.eV3f.nFlow = DEVFLOW_TRACEA;
		}
	}
	else {	// normal
		c_v3fch.ta.nState &= ~(TRACESTATE_LOAD | TRACESTATE_LOADDEV);
		c_rV3f[0].nFlow = DEVFLOW_NORMALR;
		c_rV3f[0].d.st.vTbeL = c_doz.tc.real.cV3f.vTbeL;
		c_rV3f[0].d.st.inst.a = c_doz.tc.real.cV3f.inst.a;
		c_rV3f[0].d.st.inst.b.msb = c_doz.tc.real.cV3f.wMsb & pos ? TRUE : FALSE;
		c_rV3f[0].d.st.inst.b.rst = c_doz.tc.real.cV3f.wRstCmd & pos ? TRUE : FALSE;
		c_rV3f[0].d.st.state.a = c_doz.tc.real.cV3f.state.a;
		c_rV3f[0].d.st.state.b.pand = c_doz.tc.real.cV3f.wPanto & pos ? TRUE : FALSE;
		c_rV3f[0].d.st.mode.a = c_doz.tc.real.cV3f.mode.a;
		c_rV3f[0].d.st.mode.b.fmcco = c_doz.tc.real.cV3f.wFmccoCmd & pos ? TRUE : FALSE;
		CopyTime(&c_rV3f[0].d.st.time, &c_doz.tc.real.time);
		c_rV3f[0].d.st.test.a = c_doz.tc.real.cV3f.test.a;
		c_rV3f[0].d.st.test.b.vfsot = c_doz.tc.real.cV3f.wVfsot & pos ? TRUE : FALSE;
		//c_rV3f[0].d.st.exec.a =
		c_rV3f[0].d.st.exec.b.tdc = c_doz.tc.real.cV3f.wTraceClear & pos ? TRUE : FALSE;
		c_rV3f[0].d.st.exec.b.tmvld = c_doz.tc.real.cV3f.wTimeValid & pos ? TRUE : FALSE;
		memcpy((PVOID)&c_rV3f[2], (PVOID)&c_rV3f[0], sizeof(V3FRINFO));
	}
	LockA();
	memcpy((PVOID)&c_rV3f[1], (PVOID)&c_rV3f[0], sizeof(V3FRINFO));
	memcpy((PVOID)&c_doz.cc.real.rV3f, (PVOID)&c_rV3f[0], sizeof(V3FRINFO));
	if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_V3FR)) {
		memcpy((PVOID)&c_pBucket->v3f.f.r, (PVOID)&c_rV3f[0], SIZE_V3FSR);
		c_pBucket->wTicker |= BEILTICKER_V3FR;
	}
	UnlockA();
}

void CCcx::AbstractEcu()
{
	memset((PVOID)&c_rEcu[0], 0, sizeof(ECURINFO));
	c_rEcu[0].wAddr = GETLWORD(DEVADD_ECU);
	c_rEcu[0].vTbeL = c_doz.tc.real.cEcu.vTbeL;
	c_rEcu[0].inst.a = c_doz.tc.real.cEcu.inst.a;
	WORD pos = 1 << c_ccRef.real.car.nID;
	c_rEcu[0].inst.b.pecuf = c_doz.tc.real.cEcu.wPecuf & pos ? TRUE : FALSE;
	c_rEcu[0].inst.b.msb = c_doz.tc.real.cEcu.wMsb & pos ? TRUE : FALSE;
	if (c_doz.tc.real.cEcu.wTracing & pos) {
		// trace
		if (!(c_ecuch.ta.nState & TRACESTATE_LOAD)) {
			c_ecuch.ta.nState |= (TRACESTATE_LOAD | TRACESTATE_LOADDEV);
			c_ecuch.ta.wiTc = c_ecuch.ta.wiCc = 0;
		}
		else {
			if (c_ecuch.ta.wiTc < c_ecuch.ta.wiCc) {
				memcpy((PVOID)&c_doz.cc.real.aEcu.d.tr.n[0],
						(PVOID)&c_ecuch.d.c[c_ecuch.ta.wiTc][0], sizeof(TAECU));
				++ c_ecuch.ta.wiTc;
			}
			else if (c_sivch.ta.wiCc < c_ecuch.ta.wDev)
				memset((PVOID)&c_doz.cc.real.aEcu.d.tr.n[0], 0, sizeof(TAECU));
			c_doz.cc.real.aEcu.wAddr = GETLWORD(DEVADD_ECU);
			c_doz.cc.real.aEcu.nFlow = ECUFLOW_TDTA;
		}
	}
	else {
		c_ecuch.ta.nState &= ~(TRACESTATE_LOAD | TRACESTATE_LOADDEV);
		c_rEcu[0].d.st.set.a = c_doz.tc.real.cEcu.set.a;
		c_rEcu[0].d.st.set.b.rqt = c_doz.tc.real.cEcu.wRqt & pos ? TRUE : FALSE;
		c_rEcu[0].d.st.set.b.cals = c_doz.tc.real.cEcu.wCals & pos ? TRUE : FALSE;
		c_rEcu[0].d.st.set.b.rtdc = c_doz.tc.real.cEcu.wTraceClear & pos ? TRUE : FALSE;
		if (c_rEcu[0].d.st.set.b.rqt)	c_rEcu[0].nFlow = ECUFLOW_TEST;
		else if (c_rEcu[0].d.st.set.b.rtdc)	c_rEcu[0].nFlow = ECUFLOW_TCLR;
		else if (c_rEcu[0].d.st.set.b.cals)	c_rEcu[0].nFlow = ECUFLOW_TSET;
		else	c_rEcu[0].nFlow = ECUFLOW_SD;
		c_rEcu[0].d.st.nAsoc = c_doz.tc.real.cEcu.nAsoc[c_ccRef.real.car.nID];
		CopyTime(&c_rEcu[0].d.st.time, &c_doz.tc.real.time);
		memcpy((PVOID)&c_rEcu[2], (PVOID)&c_rEcu[0], sizeof(ECURINFO));
	}
	LockA();
	memcpy((PVOID)&c_rEcu[1], (PVOID)&c_rEcu[0], sizeof(ECURINFO));
	memcpy((PVOID)&c_doz.cc.real.rEcu, (PVOID)&c_rEcu[0], sizeof(ECURINFO));

	if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_ECUR)) {
		memcpy((PVOID)&c_pBucket->ecu.f.r, (PVOID)&c_rEcu[0], SIZE_ECUSR);
		c_pBucket->wTicker |= BEILTICKER_ECUR;
	}
	UnlockA();
}

void CCcx::AbstractCmsb()
{
	memset((PVOID)&c_rCmsb[0], 0, sizeof(CMSBRINFO));
	c_rCmsb[0].wAddr = GETLWORD(DEVADD_CMSB);
	c_rCmsb[0].nFlow = DEVFLOW_NORMALR;
	c_rCmsb[0].nSpr1 = c_doz.tc.real.cCmsb.nSpr1;
	UCURV id = c_ccRef.real.car.nID < 3 ? 0 : 1;
	c_rCmsb[0].inst.a = c_doz.tc.real.cCmsb.inst[id].a;
	c_rCmsb[0].nCmst = c_doz.tc.real.cCmsb.nCmst[id];
	CopyTime(&c_rCmsb[0].time, &c_doz.tc.real.time);
	c_rCmsb[0].nSpr10 = c_doz.tc.real.cCmsb.nSpr10;
	LockA();
	memcpy((PVOID)&c_rCmsb[1], (PVOID)&c_rCmsb[0], sizeof(CMSBRINFO));
	memcpy((PVOID)&c_doz.cc.real.rCmsb, (PVOID)&c_rCmsb[0], sizeof(CMSBRINFO));
	if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_CMSBR)) {
		memcpy((PVOID)&c_pBucket->cmsb.f.r, (PVOID)&c_rCmsb[0], sizeof(CMSBRINFO));
		c_pBucket->wTicker |= BEILTICKER_CMSBR;
	}
	UnlockA();
}

void CCcx::AbstractDcu()
{
	memset((PVOID)&c_rDcu[0], 0, sizeof(DCURINFO));
	c_rDcu[0].wAddr = GETLWORD(DEVADD_DCU);
	c_rDcu[0].nFlow = DEVFLOW_NORMALR;
	// Modified 2013/01/10
	//c_rDcu[0].nSpr2 = c_doz.tc.real.cDcu.nSpr2;
	c_rDcu[0].nSpr2 = 0;
	c_rDcu[0].test.b.aoac = c_doz.tc.real.cDcu.nMode == DOORMODE_AOAC ? TRUE : FALSE;
	WORD pos = 1 << c_ccRef.real.car.nID;
	c_rDcu[0].test.b.tsr = c_doz.tc.real.cDcu.wTsr & pos ? TRUE : FALSE;
	c_rDcu[0].test.b.tdc = c_doz.tc.real.cDcu.wTdc & pos ? TRUE : FALSE;
	CopyTime(&c_rDcu[0].time, &c_doz.tc.real.time);
	c_rDcu[0].nSpr10 = c_doz.tc.real.cDcu.nSpr10;
	LockA();
	memcpy((PVOID)&c_rDcu[1], (PVOID)&c_rDcu[0], sizeof(DCURINFO));
	UnlockA();
}

//======================================================================

BOOL CCcx::Run(int aich)
{
	MSGLOG("\r\n\r\n\r\nCcx v%.2f\r\nRebuild %ld\r\n", VERSION_CCX, REBUILD_CCX);
	if (aich > 0) {
		c_ccRef.real.wProperty |= PROPERTY_ATLABORATORY;
		MSGLOG("[INIT]external memory ignore mode.\n");
	}

	if (!TakingOff()) {
		fin_mfb();
		return FALSE;
	}

	MSGLOG("\r\n\r\n*****START JOB*****\r\n\r\n");
	memset((PVOID)&c_opTimes[0], 0, sizeof(c_opTimes));
	return TRUE;
}

void CCcx::DevTrigger()
{
	if (++ c_ccRef.real.pt.nDevTimer >= 20)	c_ccRef.real.pt.nDevTimer = 0;
	if (c_ccRef.real.cycle.bOpen) {
		if ((c_ccRef.real.pt.nDevTimer & 1) && (c_ccRef.real.wActiveDev & DEVBEEN_V3F) && c_ccRef.real.pt.w20ms > 0) {
			-- c_ccRef.real.pt.w20ms;
			if (c_v3fch.ta.nState & TRACESTATE_LOAD) {
				if ((c_v3fch.ta.nState & TRACESTATE_LOADDEV) && c_v3fch.ta.wiCc < c_v3fch.ta.wDev) {
					c_rV3f[1].wAddr = GETLWORD(DEVADD_V3F);
					c_rV3f[1].nFlow = DEVFLOW_TRACER;
					c_rV3f[1].d.tr.wIndex = c_v3fch.ta.wiCc;
					c_rV3f[1].d.tr.nSpr = 0;
				}
				else {
					memcpy((PVOID)&c_rV3f[1], (PVOID)&c_rV3f[2], sizeof(V3FRINFO));
					CopyTime(&c_rV3f[1].d.st.time, &c_doz.tc.real.time);
				}
			}
			c_pPocket->SendV3f();
		}
		if ((c_ccRef.real.pt.nDevTimer == 0 || c_ccRef.real.pt.nDevTimer == 5 ||
				c_ccRef.real.pt.nDevTimer == 10 || c_ccRef.real.pt.nDevTimer == 15) && c_ccRef.real.pt.w50ms > 0) {
			-- c_ccRef.real.pt.w50ms;
			if (c_sivch.ta.nState & TRACESTATE_LOAD) {
				if ((c_sivch.ta.nState & TRACESTATE_LOADDEV) && c_sivch.ta.wiCc < c_sivch.ta.wDev) {
					c_rSiv[1].wAddr = GETLWORD(DEVADD_SIV);
					c_rSiv[1].nFlow = DEVFLOW_TRACER;
					c_rSiv[1].d.tr.wIndex = c_sivch.ta.wiCc;
					c_rSiv[1].d.tr.nSpr = 0;
				}
				else {
					memcpy((PVOID)&c_rSiv[1], (PVOID)&c_rSiv[2], sizeof(SIVRINFO));
					CopyTime(&c_rSiv[1].d.st.time, &c_doz.tc.real.time);
				}
			}
			if (c_ecuch.ta.nState & TRACESTATE_LOAD) {
				if ((c_ecuch.ta.nState & TRACESTATE_LOADDEV) && c_ecuch.ta.wiCc < c_ecuch.ta.wDev) {
					c_rEcu[1].wAddr = GETLWORD(DEVADD_ECU);
					c_rEcu[1].nFlow = ECUFLOW_TDTA;
					c_rEcu[1].d.tr.nTrace = (BYTE)(c_ecuch.ta.wiCc / MAXBLOCK_ECU);
					c_rEcu[1].d.tr.nIndex = (BYTE)(c_ecuch.ta.wiCc % MAXBLOCK_ECU);
				}
				else {
					memcpy((PVOID)&c_rEcu[1], (PVOID)&c_rEcu[2], sizeof(ECURINFO));
					CopyTime(&c_rEcu[1].d.st.time, &c_doz.tc.real.time);
				}
			}
			c_pPocket->SendSivEcu();
		}
		if ((c_ccRef.real.pt.nDevTimer == 2 || c_ccRef.real.pt.nDevTimer == 12)
				&& (c_ccRef.real.wActiveDev & DEVBEEN_DCU) && c_ccRef.real.pt.w100ms > 0) {
			-- c_ccRef.real.pt.w100ms;
			c_pPocket->SendDcu();
		}
		if ((c_ccRef.real.pt.nDevTimer == 4) && (c_ccRef.real.wActiveDev & DEVBEEN_CMSB) && c_ccRef.real.pt.w200ms > 0) {
			-- c_ccRef.real.pt.w200ms;
			c_pPocket->SendCmsb();
		}
	}
}

void CCcx::Destroy()
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
	KILL(c_pPocket);
	KILL(c_main.pLine);
	KILL(c_resv.pLine);
}

void CCcx::RecordTime(UCURV n)
{
	RECTIME(n);
}

void CCcx::IoTest()
{
	WORD w;
	for (UCURV n = 0; n < 3; n ++) {
		c_probe.Getd(n, &w);
		if (w != c_ccRef.real.wOut[n]) {
			c_ccRef.real.wOut[n] = w;
			MSGLOG("[MAIN]Output %d-%04X.\n", n, w);
		}
		c_probe.Putd(n, &w);
	}
}

BYTE CCcx::BitwiseIn(BYTE in)
{
	BYTE ret = 0;
	if (in & 0x11)	ret |= ((in & 0x11) << 3);
	if (in & 0x22)	ret |= ((in & 0x22) << 1);
	if (in & 0x44)	ret |= ((in & 0x44) >> 1);
	if (in & 0x88)	ret |= ((in & 0x88) >> 3);
	return ret;
}

BYTE CCcx::BitwiseOut(BYTE out)
{
	BYTE ret = 0;
	for (UCURV n = 0; n < 4; n ++) {
		if (out & (2 << n))
			ret |= (2 << (3 - n));
	}
	return ret;
}

BYTE CCcx::ToHex(BYTE bcd)
{
	BYTE hex = ((bcd >> 4) & 0xf) * 10;
	hex += (bcd & 0xf);
	return hex;
}

BYTE CCcx::BitIndex(BYTE bits)
{
	for (BYTE n = 8; n > 0; n ++) {
		if (bits & (1 << (n - 1)))	return n;
	}
	return 0;
}

void CCcx::LongToByte(BYTE* p, long v)
{
	*p ++ = (BYTE)((v >> 24) & 0xff);
	*p ++ = (BYTE)((v >> 16) & 0xff);
	*p ++ = (BYTE)((v >> 8) & 0xff);
	*p = (BYTE)(v & 0xff);
}

void CCcx::CalcSpeedBase(UCURV n)
{
	if (n < LENGTH_AXEL)
		c_ccRef.real.dbBase[n] = (double)c_ccRef.real.dia[n].wV / 1000.f * M_PI * 1800 / 7.07;
}

//========================================================================

void CCcx::CalcV3f()
{
	if (c_ccRef.real.spt.bOpen) {
		c_ccRef.real.spe.wPulse[0][c_ccRef.real.spe.wPi] = (WORD)(c_aV3f.d.st.nTgf1);
		c_ccRef.real.spe.wPulse[1][c_ccRef.real.spe.wPi] = (WORD)(c_aV3f.d.st.nTgf2);
		c_ccRef.real.spe.wPulse[2][c_ccRef.real.spe.wPi] = (WORD)(c_aV3f.d.st.nTgf3);
		c_ccRef.real.spe.wPulse[3][c_ccRef.real.spe.wPi] = (WORD)(c_aV3f.d.st.nTgf4);

		if (++ c_ccRef.real.spe.wPi >= CYCLE_PULSE) {	// 5 * 20 = 100ms
			c_ccRef.real.opr.nPiH = HIGHBYTE(c_ccRef.real.spe.wPi);
			c_ccRef.real.opr.nPiL = LOWBYTE(c_ccRef.real.spe.wPi);
			c_ccRef.real.spe.wPi = 0;
			uint64_t clk = ClockCycles();	// 100ms
			double term = (double)(clk - c_ccRef.real.spt.clock) / (double)c_ccRef.real.cps;
			if (term > .001f) {
				for (UCURV n = 0; n < LENGTH_AXEL; n ++) {
					double sum = 0.f;
					for (UCURV m = 0; m < CYCLE_PULSE; m ++)
						sum += (double)c_ccRef.real.spe.wPulse[n][m];
					sum /= (double)CYCLE_PULSE;
					double sp = c_ccRef.real.dbBase[n] * sum * (term / (0.02 * CYCLE_PULSE));
					//double sp = ((double)c_ccRef.real.sp.dia[n].wV * M_PI / 423600.f) *
					//			(double)c_ccRef.real.cs.wPulse[n] * (3600.f / term);
					c_ccRef.real.dwSpeed[n] = (DWORD)sp;
				}
				c_ccRef.real.spt.clock = clk;
			}
		}

		Lock();
		++ c_ccRef.real.v3fp.wLength;
		uint64_t ck = ClockCycles();	// ~= 20ms
		double sec = (double)(ck - c_ccRef.real.v3fp.clock) / (double)c_ccRef.real.cps;
		c_ccRef.real.v3fp.clock = ck;
		double power = ((double)c_aV3f.d.st.nEs * 2000.f / 250.f) * (double)c_aV3f.d.st.vIdc * sec / 3600.f;	// w/h
		if (power > 0)	c_ccRef.real.v3fp.p += power;
		else	c_ccRef.real.v3fp.n += fabs(power);

	}
	else {
		c_ccRef.real.spt.bOpen = TRUE;
		c_ccRef.real.spt.clock = c_ccRef.real.v3fp.clock = ClockCycles();
		Lock();
		c_ccRef.real.v3fp.p = c_ccRef.real.v3fp.n = 0;
	}
	memcpy((PVOID)&c_doz.cc.real.eV3f, (PVOID)&c_aV3f, sizeof(V3FAINFO));
	c_doz.cc.real.eV3f.d.st.power = c_ccRef.real.v3fp.p;
	c_doz.cc.real.eV3f.d.st.revival = c_ccRef.real.v3fp.n;
	c_doz.cc.real.eV3f.d.st.dwSpd[0] = c_ccRef.real.dwSpeed[0];
	c_doz.cc.real.eV3f.d.st.dwSpd[1] = c_ccRef.real.dwSpeed[1];
	c_doz.cc.real.eV3f.d.st.dwSpd[2] = c_ccRef.real.dwSpeed[2];
	c_doz.cc.real.eV3f.d.st.dwSpd[3] = c_ccRef.real.dwSpeed[3];
	Unlock();
}

void CCcx::CalcSiv()
{
	Lock();
	if (c_ccRef.real.sivp.wSync > 0) {
		++ c_ccRef.real.sivp.wLength;
		uint64_t clk = ClockCycles();	// ~= 20ms
		double term = (double)(clk - c_ccRef.real.sivp.clock) / (double)c_ccRef.real.cps;
		c_ccRef.real.sivp.clock = clk;
		double power = ((double)c_aSiv.d.st.nEd * 3000.f / 250.f) * ((double)c_aSiv.d.st.nIs * 750.f / 250.f) * term / 3600.f;	// w/h
		c_ccRef.real.sivp.p += power;
	}
	else {
		c_ccRef.real.sivp.wSync = 1;
		c_ccRef.real.sivp.clock = ClockCycles();
		c_ccRef.real.sivp.p = 0;
	}
	memcpy((PVOID)&c_doz.cc.real.eSiv, (PVOID)&c_aSiv, sizeof(SIVAINFO));
	c_doz.cc.real.eSiv.d.st.power = c_ccRef.real.sivp.p;
	Unlock();
}

void CCcx::SimulSiv()
{
	c_aSiv.wAddr = GETLWORD(DEVADD_SIV);
	c_aSiv.nFlow = DEVFLOW_NORMALA;
	c_aSiv.d.st.nEd = 125;
}

void CCcx::SimulV3f()
{
	c_aV3f.wAddr = GETLWORD(DEVADD_V3F);
	c_aV3f.nFlow = DEVFLOW_NORMALA;
	if (c_bTog) {
		c_aV3f.d.st.vIdc = -5;
		c_bTog = FALSE;
	}
	else {
		c_aV3f.d.st.vIdc = 10;
		c_bTog = TRUE;
	}
}

//========================================================================

void CCcx::MaAccept(DWORD dwParam)
{
	BusPresentEntry();
}

void CCcx::MaSendBus(DWORD dwParam)
{
	CalcTimes();
	BOOL bCh = (BOOL)dwParam;
	RECTIME(bCh ? CTIME_BUSSENDA : CTIME_BUSSENDB);
	CBusCarx* pBus = bCh ? c_main.pLine : c_resv.pLine;
	c_doz.cc.real.wAddr = GETLWORD((c_ccRef.real.car.nID + CAR_CC + 1) | BUSADDR_HIGH);
	c_doz.cc.real.dwOrderID = pBus->GetOrderID();
	c_doz.cc.real.wVersion = MAKEWORD(VERSION_HIGH(VERSION_CCX), VERSION_LOW(VERSION_CCX));
	c_doz.cc.real.nCallAddr = c_ccRef.real.nCallID = c_doz.tc.real.nRealAddr;
	c_doz.cc.dummy[0] |= bCh ? 4 : 0x40;
	pBus->Send((PVOID)&c_doz.cc);
	if ((bCh && c_ccRef.real.wActiveBus == ACTIVEBUS_MAIN) ||
			(!bCh && c_ccRef.real.wActiveBus == ACTIVEBUS_RESV)) {
		memset((PVOID)&c_ccRef.real.idc, 0, sizeof(AVERAGEIDC));
	}
	Lock();
	c_ccRef.real.v3fp.p = c_ccRef.real.v3fp.n = c_ccRef.real.sivp.p = c_ccRef.real.sivp.n = 0;
	c_ccRef.real.v3fp.wLength = c_ccRef.real.sivp.wLength = 0;
	Unlock();
	RECTIME(bCh ? CTIME_BUSSENDEDA : CTIME_BUSSENDEDB);
	BusPresent();
}

void CCcx::MaReceiveBus(DWORD dwParam)
{
	CalcTimes();
	BOOL bCh = (BOOL)dwParam;
	c_ccRef.real.cycle.wOver = 0;
	DWORD dwRecvOrderID;

	TCFIRM	tcf;
	if (bCh) {		// ch. A
		c_main.pLine->Combine((BYTE*)&tcf);
		c_doz.cc.dummy[0] |= 1;
		dwRecvOrderID = c_main.pLine->GetOrderID();
		if (c_ccRef.real.dwRecvOrderID == dwRecvOrderID) {
			RECTIME(CTIME_BUSRECVOTHER);
			return;
		}
	}
	else {
		c_resv.pLine->Combine((BYTE*)&tcf);
		c_doz.cc.dummy[0] |= 0x10;
		dwRecvOrderID = c_resv.pLine->GetOrderID();
		if (c_ccRef.real.dwRecvOrderID == dwRecvOrderID) {
			RECTIME(CTIME_BUSRECVOTHER);
			return;
		}
	}
	Lock();
	memcpy((PVOID)&c_doz.tc, (PVOID)&tcf, sizeof(TCFIRM));
	c_ccRef.real.dwRecvOrderID = dwRecvOrderID;
	c_ccRef.real.wActiveBus = bCh ? ACTIVEBUS_MAIN : ACTIVEBUS_RESV;

	RECTIME(CTIME_OPENDEV);
	if (c_ccRef.real.wActiveDev & DEVBEEN_SIV)	AbstractSiv();
	if (c_ccRef.real.wActiveDev & DEVBEEN_V3F)	AbstractV3f();
	if (c_ccRef.real.wActiveDev & DEVBEEN_ECU)	AbstractEcu();
	if (c_ccRef.real.wActiveDev & DEVBEEN_CMSB)	AbstractCmsb();
	AbstractDcu();

	c_ccRef.real.pt.w20ms = INTERVAL_POCK20MS;
	c_ccRef.real.pt.w50ms = INTERVAL_POCK50MS;
	c_ccRef.real.pt.w100ms = INTERVAL_POCK100MS;
	c_ccRef.real.pt.w200ms = INTERVAL_POCK200MS;
	Unlock();

	CopyOut();
	WORD w;
	for (UCURV n = 0; n < 3; n ++) {
		if (!c_probe.Getd(n, &w)) {
			if (n == 2) {
				if (c_ccRef.real.car.nID == (c_ccRef.real.car.nLength - 1)) {	// CC0 door
					w = (w & 0xff00) | (WORD)BitwiseIn((BYTE)(w & 0xff));
					WORD l = w & 0xf;
					WORD r = w & 0xf0;
					l <<= 4;
					r >>= 4;
					w &= 0xff00;
					w |= (l | r);
				}
			}
			c_doz.cc.real.nInput[n * 2] = LOWBYTE(w);
			c_doz.cc.real.nInput[n * 2 + 1] = HIGHBYTE(w);
		}
	}
	if (c_ccRef.real.wActiveDev & DEVBEEN_PWM) {
		WORD pwmdir = (c_doz.tc.real.pwm.nMode == PWMMODE_OUTPUT) ? 0x100 : 0;
		c_probe.Putd(3, &pwmdir);
		DWORD dw;
		if (pwmdir != 0) {
			c_ccRef.real.pwm.high = c_doz.tc.real.pwm.high;
			c_ccRef.real.pwm.period = c_doz.tc.real.pwm.period;
			dw = MAKEDWORD(c_ccRef.real.pwm.high, c_ccRef.real.pwm.period);
			c_pwm.Put(&dw);
		}
		else {
			if (!c_pwm.Get(&dw)) {
				c_doz.cc.real.pwm.high = HIGHWORD(dw);
				c_doz.cc.real.pwm.period = LOWWORD(dw);
			}
		}
	}

	if (c_ccRef.real.wActiveDev & DEVBEEN_V3F) {
		// Modified 2013/11/02
		//BYTE id = c_ccRef.real.car.nID - 1;
		//if (id > 1)	id -= 2;
		//if (id < 4) {
		//	for (UCURV n = 0; n < 4; n ++) {
		//		if (c_doz.tc.real.nDia[id * 4 + n] < (MAX_WHEELDIA - MIN_WHEELDIA) &&
		//				c_doz.tc.real.nDia[id * 4 + n] != c_ccRef.real.dia[n].nCur) {
		//			c_ccRef.real.dia[n].nCur = c_doz.tc.real.nDia[id * 4 + n];
		//			c_ccRef.real.dia[n].nDeb = DEBOUNCE_WHEELDIA;
		//		}
		//		else if (c_ccRef.real.dia[n].nDeb != 0 && -- c_ccRef.real.dia[n].nDeb == 0) {
		//			c_ccRef.real.dia[n].wV = (WORD)c_ccRef.real.dia[n].nCur + MIN_WHEELDIA;
		//			CalcSpeedBase(n);
		//			MSGLOG("[BUS]capture dia%d = %dmm.\r\n", n, c_ccRef.real.dia[n].wV);
		//		}
		//	}
		//}
		BYTE mid = c_ccRef.real.car.nID - 1;
		switch (c_ccRef.real.car.nLength) {
		case 4 :						// 0, 1
			break;
		case 6 :
			if (mid > 0)	-- mid;		// 0, 2, 3 -> 0, 1, 2
			break;
		default :
			if (mid > 1)	mid -= 2;	// 0, 1, 4, 5 -> 0, 1, 2, 3
			break;
		}
		if (mid < 4) {
			for (UCURV n = 0; n < 4; n ++) {
				if (c_doz.tc.real.nDia[mid * 4 + n] < (MAX_WHEELDIA - MIN_WHEELDIA) &&
						c_doz.tc.real.nDia[mid * 4 + n] != c_ccRef.real.dia[n].nCur) {
					c_ccRef.real.dia[n].nCur = c_doz.tc.real.nDia[mid * 4 + n];
					c_ccRef.real.dia[n].nDeb = DEBOUNCE_WHEELDIA;
				}
				else if (c_ccRef.real.dia[n].nDeb != 0 && -- c_ccRef.real.dia[n].nDeb == 0) {
					c_ccRef.real.dia[n].wV = (WORD)c_ccRef.real.dia[n].nCur + MIN_WHEELDIA;
					CalcSpeedBase(n);
					MSGLOG("[BUS]capture dia%d = %dmm.\r\n", n, c_ccRef.real.dia[n].wV);
				}
			}
		}
	}
	c_doz.cc.real.nThumbWheel[0] = (BYTE)((c_ccRef.real.car.dwThumbWheel >> 16) & 0xff);
	c_doz.cc.real.nThumbWheel[1] = (BYTE)((c_ccRef.real.car.dwThumbWheel >> 8) & 0xff);
	c_doz.cc.real.nThumbWheel[2] = (BYTE)(c_ccRef.real.car.dwThumbWheel & 0xff);
	if (c_doz.tc.real.nSlaveCtrl & SLAVECCCTRL_DISABLEDEVICEERRORCOUNT) {
		for (UCURV n = 0; n < 2; n ++) {
			for (UCURV m = 0; m < 4; m ++)
				c_doz.cc.real.wDoorErrorCount[n][m] = 0;
		}
		c_doz.cc.real.wCmsbErrorCount = 0;
	}
}

void CCcx::MaReceiveFault(DWORD dwParam)
{
	if (dwParam != 0)	c_main.pLine->EnableIntr(TRUE);
	else	c_resv.pLine->EnableIntr(TRUE);
	c_panel.SetLamp(0x40);
	c_ccRef.real.cycle.wOverLampOnTimer = 1;
}

void CCcx::MaReceivePocket(DWORD dwParam)
{
	WORD state = HIGHWORD(dwParam);	// ????? Be reference state
	WORD id = LOWWORD(dwParam);
	switch(id) {
	case DEVID_V3F :
		if (GETLWORD(c_aV3f.wAddr) == DEVADD_V3F) {
			if (c_aV3f.nFlow == DEVFLOW_TRACEA) {
				if (c_v3fch.ta.nState & TRACESTATE_LOADDEV) {
					if (c_aV3f.d.tr.wIndex < MAXBLOCK_V3F) {
						memcpy((PVOID)&c_v3fch.d[c_aV3f.d.tr.wIndex][0],
								(PVOID)&c_aV3f.d.tr.nText[0], 100);
						if (c_v3fch.ta.wiCc != c_aV3f.d.tr.wIndex) {
							//sequencial index error
							//c_sivch.ta.wiCc = c_doz.cc.real.aSiv.d.tr.wIndex;
						}
						else if (++ c_v3fch.ta.wiCc >= c_v3fch.ta.wDev)
							c_v3fch.ta.nState &= ~TRACESTATE_LOADDEV;
					}
				}
				//else {
				//	trace ack. error
				//}
			}
			else if (c_aV3f.nFlow == DEVFLOW_NORMALA) {
				if (!(c_v3fch.ta.nState & TRACESTATE_LOAD)) {
					WORD w = c_doz.cc.real.eV3f.d.st.st.trace.a & 0xf;
					if (w > MAXTRACELENGTH_V3F)	w = MAXTRACELENGTH_V3F;
					DWORD dw = (DWORD)w * (DWORD)sizeof(V3FSTABLE);
					WORD leng = (WORD)(dw / 100);
					if (dw % 100)	++ leng;
					if (leng != c_v3fch.ta.wCur) {
						c_v3fch.ta.wCur = leng;
						c_v3fch.ta.nDeb = DEB_TRACING;
					}
					else if (c_v3fch.ta.nDeb != 0 && -- c_v3fch.ta.nDeb == 0) {
						c_v3fch.ta.wDev = c_v3fch.ta.wCur;
						if (c_v3fch.ta.wiCc != c_v3fch.ta.wDev)
							c_v3fch.ta.wiCc = 0;
					}
					CalcV3f();	// 	memcpy((PVOID)&c_doz.cc.real.eV3f, (PVOID)&c_aV3f, sizeof(V3FAINFO));
				}
			}
		}
		else	memcpy((PVOID)&c_doz.cc.real.eV3f, (PVOID)&c_aV3f, sizeof(V3FAINFO));
		if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_V3FA)) {
			memcpy((PVOID)&c_pBucket->v3f.f.a, (PVOID)&c_aV3f, SIZE_V3FSA);
			c_pBucket->wTicker |= BEILTICKER_V3FA;
		}
		RECTIME(CTIME_CLOSEV3F);
		break;

	case DEVID_SIV :
		if (GETLWORD(c_aSiv.wAddr) == DEVADD_SIV) {
			if (c_aSiv.nFlow == DEVFLOW_TRACEA) {
				if (c_sivch.ta.nState & TRACESTATE_LOADDEV) {
					if (c_aSiv.d.tr.wIndex < MAXBLOCK_SIV) {
						memcpy((PVOID)&c_sivch.d[c_aSiv.d.tr.wIndex][0],
								(PVOID)&c_aSiv.d.tr.nText[0], 100);
						if (c_sivch.ta.wiCc != c_aSiv.d.tr.wIndex) {
							//sequencial index error
							//c_sivch.ta.wiCc = c_doz.cc.real.aSiv.d.tr.wIndex;
						}
						else if (++ c_sivch.ta.wiCc >= c_sivch.ta.wDev)
							c_sivch.ta.nState &= ~TRACESTATE_LOADDEV;
					}
				}
				//else {
				//	trace ack. error
				//}
			}
			else if (c_aSiv.nFlow == DEVFLOW_NORMALA) {
				if (!(c_sivch.ta.nState & TRACESTATE_LOAD)) {
					WORD w = ToHex(c_aSiv.d.st.nTrace);
					if (w > MAXTRACELENGTH_SIV)	w = MAXTRACELENGTH_SIV;
					DWORD dw = (DWORD)w * (DWORD)sizeof(SIVSTABLE);
					WORD leng = (WORD)(dw / 100);
					if (dw % 100)	++ leng;
					if (leng != c_sivch.ta.wCur) {
						c_sivch.ta.wCur = leng;
						c_sivch.ta.nDeb = DEB_TRACING;
					}
					else if (c_sivch.ta.nDeb != 0 && -- c_sivch.ta.nDeb == 0) {
						c_sivch.ta.wDev = c_sivch.ta.wCur;
						if (c_sivch.ta.wiCc != c_sivch.ta.wDev)
							c_sivch.ta.wiCc = 0;
					}
					CalcSiv();		// memcpy((PVOID)&c_doz.cc.real.eSiv, (PVOID)&c_aSiv, sizeof(SIVAINFO));
				}
			}
		}
		else	memcpy((PVOID)&c_doz.cc.real.eSiv, (PVOID)&c_aSiv, sizeof(SIVAINFO));
		if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_SIVA)) {
			memcpy((PVOID)&c_pBucket->siv.f.a, (PVOID)&c_aSiv, SIZE_SIVSA);
			c_pBucket->wTicker |= BEILTICKER_SIVA;
		}
		RECTIME(CTIME_CLOSESIV);
		break;
	case DEVID_ECU :
		if (GETLWORD(c_aEcu.wAddr) == DEVADD_ECU) {
			if (c_aEcu.nFlow == ECUFLOW_TDTA) {
				if (c_ecuch.ta.nState & TRACESTATE_LOADDEV) {
					memcpy((PVOID)&c_ecuch.d.c[c_ecuch.ta.wiCc][0],
							(PVOID)&c_aEcu.d.tr.n[0], sizeof(TAECU));
					if (++ c_ecuch.ta.wiCc >= c_ecuch.ta.wDev)
						c_ecuch.ta.nState &= ~TRACESTATE_LOADDEV;
				}
				//else {
				//	trace ack. error
				//}
			}
			else {
				if (!(c_ecuch.ta.nState & TRACESTATE_LOAD)) {
					WORD w = BitIndex(c_doz.cc.real.aEcu.d.st.nTrace & 0x1f);
					if (w > MAXTRACELENGTH_ECU)	w = MAXTRACELENGTH_ECU;
					w *= MAXBLOCK_ECU;
					if (w != c_ecuch.ta.wCur) {
						c_ecuch.ta.wCur = w;
						c_ecuch.ta.nDeb = DEB_TRACING;
					}
					else if (c_ecuch.ta.nDeb != 0 && -- c_ecuch.ta.nDeb == 0) {
						c_ecuch.ta.wDev = c_ecuch.ta.wCur;
						if (c_ecuch.ta.wiCc != c_ecuch.ta.wDev)
							c_ecuch.ta.wiCc = 0;
					}
					memcpy((PVOID)&c_doz.cc.real.aEcu, (PVOID)&c_aEcu, sizeof(ECUAINFO));
				}
			}
		}
		else	memcpy((PVOID)&c_doz.cc.real.aEcu, (PVOID)&c_aEcu, sizeof(ECUAINFO));
		if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_ECUA)) {
			memcpy((PVOID)&c_pBucket->ecu.f.a, (PVOID)&c_aEcu, SIZE_ECUSA);
			c_pBucket->wTicker |= BEILTICKER_ECUA;
		}
		RECTIME(CTIME_CLOSEECU);
		break;
	case DEVID_DCU :
	case DEVID_DCU2 :
		{
			BYTE rev = 0;
			BYTE tar = id == DEVID_DCU ? 0 : 1;
			BYTE from = tar;
			if (c_ccRef.real.car.nID == (c_ccRef.real.car.nLength - 1)) {	// 0 car, need invert
				rev = 3;
				tar ^= 1;
			}
			BYTE add = c_aDcuc[from].nDcua - (from * 4 + 1);
			if (state && add < 4)
				memcpy((PVOID)&c_doz.cc.real.aDcu[tar][add ^ rev], (PVOID)&c_aDcuc[from], sizeof(DCUAINFO));
			else {
				if (c_ccRef.real.nDoorFlow < 4) {
					BYTE add = c_ccRef.real.nDoorFlow;
					memset((PVOID)&c_doz.cc.real.aDcu[tar][add ^ rev], 0, sizeof(DCUAINFO));
					c_doz.cc.real.aDcu[tar][add ^ rev].nStx = DEVFLOW_ERR;
					// Modified 2013/02/05
					if (!(c_doz.tc.real.nSlaveCtrl & SLAVECCCTRL_DISABLEDEVICEERRORCOUNT))
						INCWORD(c_doz.cc.real.wDoorErrorCount[tar][add ^ rev]);	// count with debounce
				}
			}
		}
		RECTIME(CTIME_CLOSEDCU);
		break;
	case DEVID_CMSB :
		if (state) {
			if (c_pBucket != NULL && !(c_pBucket->wTicker & BEILTICKER_CMSBA)) {
				memcpy((PVOID)&c_pBucket->cmsb.f.a, (PVOID)&c_pDoz->cc.real.aCmsb, sizeof(CMSBAINFO));
				c_pBucket->wTicker |= BEILTICKER_CMSBA;
			}
		}
		else {
			memset((PVOID)&c_doz.cc.real.aCmsb, 0, sizeof(CMSBAINFO));
			c_doz.cc.real.aCmsb.nStx = DEVFLOW_ERR;
			// Modified 2013/02/05
			if (!(c_doz.tc.real.nSlaveCtrl & SLAVECCCTRL_DISABLEDEVICEERRORCOUNT))
				INCWORD(c_doz.cc.real.wCmsbErrorCount);	// count with debounce
		}
		RECTIME(CTIME_CLOSECMS);
		break;
	default :
		return;
	}
	//if (c_wTickerDev == TICKER_START) {
	//	c_wTickerDev = TICKER_SCANT;
	//	RECTIME(CTIME_TICKERBEGIN);
	//	CmdLine(BEILCMD_TICKER);
	//}
}

BEGIN_MAIL(CCcx, CAnt)
	_MAIL(MA_ACCEPT, CCcx::MaAccept)
	_MAIL(MA_SENDBUS, CCcx::MaSendBus)
	_MAIL(MA_RECEIVEBUS, CCcx::MaReceiveBus)
	_MAIL(MA_RECEIVEFAULT, CCcx::MaReceiveFault)
	_MAIL(MA_RECEIVEPOCKET, CCcx::MaReceivePocket)
END_MAIL();
