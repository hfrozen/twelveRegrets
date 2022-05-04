/*
 * CLand.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/syspage.h>

#include "Track.h"
#include "../Inform2/Fpga/Face.h"
#include "../Component2/CTool.h"
#include "../Component2/CBand.h"
#include "../Component2/CFsc.h"
#include "CPaper.h"
#include "CSch.h"
#include "CVerse.h"
#include "CProse.h"
#include "CArch.h"
#include "CReview.h"
#include "CSch.h"
#include "CLand.h"

//#define	TS_VERSION		269		// should not be greater then 455
//#define	TS_VERSION		270		// should not be greater then 455
//#define	TS_VERSION		271		// should not be greater then 455  (211210)
//#define	TS_VERSION		272		// should not be greater then 455  (220117)
#define	TS_VERSION		273		// should not be greater then 455  (220125)
								// at 200, last version is 262
#define	LAND_NAME		((PSZ)"LAND")

#define	SIDE(x)			(x ? 0 : 1)
#define	MAKETUFNAME()\
	sprintf((char*)c_cFace, "%s%1d",\
	(pPaper->GetOblige() & (1 << OBLIGE_MAIN)) ? "maj" : "aux", CAR_FSELF())

CLand::CLand()
	: CPump(LAND_NAME)
{
	SetOwnerName(LAND_NAME);
	c_hReservoir = INVALID_HANDLE;
	memset(&c_vehicle, 0, sizeof(VEHICLE));
	c_vehicle.cProperID = 0xff;
	// 190909
	//c_vehicle.length.cActive = c_vehicle.length.cPassive =
	//										DEFAULT_CARLENGTH;
	c_vehicle.length.cCurrent = DEFAULT_CARLENGTH;
	c_vehicle.length.stab.SetDebounceTime(DEBOUNCE_CARLENGTH);
	c_otr.no.SetDebounceTime(DEBOUNCE_OTRNO);
	c_otr.wSkipTimer = TIME_OTRSKIP;
	memset(&c_recip[POPI_SEND], 0, SIZE_RECIP);
	memset(&c_recip[POPI_WORK], 0, SIZE_RECIP);
	memset(&c_recip[POPI_HEAD], 0, SIZE_RECIP);
	c_bColdWakeup = true;
	c_bControlInitialed = false;
	c_wFpgaVersion[0] = c_wFpgaVersion[1] = c_wFpgaVersion[2] = 0;
	c_vExtTemp = 0;
	memset(&c_cmd, 0, sizeof(TCCMDS));
	c_cmd.state.b.waitRequiredDoc = true;
	c_cmd.wWaitRequireDoc = TPERIOD_WAITREQUIREDOC;

	c_bAlone = false;
	c_wArterSeq = 0;

	memset(&c_mill, 0, sizeof(MILLSCALE));
	memset(&c_smooth, 0, sizeof(SMOOTH));
	memset(&c_sysChk, 0, sizeof(SYSTEMCHECK));
	memset(&c_convention, 0, sizeof(CONVENTION));
	memset(&c_pulse, 0, sizeof(PULSECHK));
	c_pulse.bInitial = true;
	memset(&c_bcMon, 0, sizeof(BROADCMON));
	c_bcMon.wTimer = TPERIOD_WAITBROADCASTF;
	c_bcMon.wWaiter[0] = c_bcMon.wWaiter[1] = TPERIOD_WAITBROADCASTF;
	memset(&c_dtbMon, 0, sizeof(DTBMON));

	for (int n = 0; n < 2; n ++) {
		c_dtb[n].p = NULL;
		c_dtb[n].state.a = 0;
		//c_dtb[n].bRxedBrc = c_dtb[n].bRxedCls = false;
	}
	c_pTuner = NULL;
	c_pLoc = NULL;
	c_pPol = NULL;
	Initial();
}

CLand::~CLand()
{
	Destroy();
}

void CLand::Destroy()
{
	c_cmd.state.b.launched = false;
	c_fio.DisableWatchdog();

	KILLR(c_pTuner);
	for (int n = 0; n < 2; n ++) {
		if (c_dtb[n].p != NULL) {
			//TRACK("%sLAND:kill dtb %s bus.\n", __U, u ? "beta" : "alpha");
			c_dtb[n].p->CloseBus();
			delete c_dtb[n].p;
			c_dtb[n].p = NULL;
		}
		c_dtb[n].state.a = 0;
		//c_dtb[n].bRxedBrc = c_dtb[n].bRxedCls = false;
	}
	KILL(c_pLoc);
	KILL(c_pPol);
}

void CLand::Initial()
{
	for (int n = 0; n < 2; n ++)	c_dtb[n].state.a = 0;
		//c_dtb[n].bRxedBrc = c_dtb[n].bRxedCls = false;
	memset(&c_order, 0, sizeof(DTBORDERNO));
	memset(&c_tWatch, 0, sizeof(TASKWATCH));
	memset(&c_tcnt, 0, sizeof(TCOUNTER));
	memset(&c_trcInfo, 0, sizeof(TRACEINFO));
	c_cmd.user.a = 0;
	c_cmd.prevent.wDtb = TPERIOD_DTBERRORPREVENT;
	SetEcuBcfPrevent();
	c_itvm.Initial();
	// 210805
	memset(&c_dbgAtoHcr, 0, sizeof(DBGATOHCR));
}

void CLand::Arteriam()
{
	if (!c_cmd.state.b.launched)	return;

	if (c_cmd.state.b.arteriam) {
		TRACK("LAND>ERR:arteriam overlaped! %d %d\n", c_mill.wPop, c_wArterSeq);
		return;
	}

	c_cmd.state.b.arteriam = true;
	c_wArterSeq = 0;
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	if (CAR_FSELF() == FID_HEAD)	c_fio.SetTrio(0, true);

	uint64_t clk = ClockCycles();
	if (!c_pulse.bInitial) {
		double sec = (double)(clk - c_pulse.clk) /
						(double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
		c_pulse.dwPeriod = (DWORD)(sec * 1e6);
		if (c_pulse.dwPeriod < 6) {
			c_cmd.state.b.arteriam = false;
			TRACK("LAND>ERR:arteriam interval %d!\n", c_pulse.dwPeriod);
			return;
		}
	}
	c_pulse.clk = clk;
	c_pulse.bInitial = false;
	c_sysChk.cVmes = c_fio.Arteriam();	// IO 보드 구조를 가져온다.

	++ c_wArterSeq;
	GETARCHIVE(pArch);

#if	defined(_ENWATCHDOG_)
	BYTE cVmes = (1 << CFio::IOB_OUTP) | (1 << CFio::IOB_INPA);
	if (ISTU())	cVmes |= (1 << CFio::IOB_INPB);
	if (c_sysChk.cVmes == cVmes) {
		c_sysChk.cDebs = 0;
		c_fio.TriggerWatchdog();	// 정상적인 구조여서 watchdog을 트리거하고 동작을 유지한다.
		if (c_sysChk.bIoFail) {
			c_fio.SetText(4, (PSZ)c_cFace);
			c_sysChk.bIoFail = false;
		}
	}
	else {
		if (++ c_sysChk.cDebs >= DEBOUNCE_VME) {
			c_sysChk.bIoFail = true;	// IO 보드에 문제가 있으므로 aux.로 제어를 넘긴다.
			// 171027
			c_fio.EnableOutput(false);
			if (ISTU())	LeaveReason();	// REASON 1. iobf
			// 171219
			TRACK("LAND>ERR:IOBF!!!\n");
			pArch->Evacuate();
			BYTE buf[8];
			sprintf((char*)buf, "iof%1d", cVmes);
			c_fio.SetText(4, (PSZ)buf);
			//c_fio.DisableWatchdog();
			exit(0);
		}
		else	c_fio.TriggerWatchdog();
	}

	++ c_wArterSeq;
	c_sysChk.cPairCtrl = (BYTE)(c_fio.GetPairCondition() & 0xff);
	// 16/06/26, Modify
	//if (ISTU()) {
	if (ISTU() && c_convention.wSeq == DTBISEQ_NON) {
		// HCR/TCR이 유효할 때 동작하도록 한다.
		if (!(pPaper->GetOblige() & (1 << OBLIGE_MAIN))) {
			// 보조에서...
			if (c_sysChk.cPairCtrl & (1 << PAIRCTRL_DEADOPPO)) {
				if (++ c_sysChk.nDeadOppoCnt >= COUNT_DEADOPPO) {
					pPaper->SetOblige(OBLIGE_MAIN);
					ModifyDtb(pPaper->GetOblige());
					// 171027
					c_fio.EnableOutput(true);
					MAKETUFNAME();
					c_fio.SetText(4, (PSZ)c_cFace);
					TRACK("LAND:change oblige from 0x%02X cause main down"
							"(0x%02x - %d)!\n",
							pPaper->GetOblige(), c_sysChk.cPairCtrl,
							c_sysChk.nDeadOppoCnt);
					// 170904
					// !!!!Shutdown를 하고 실행하면 아래와 같은 메시지가 뜨는 것은
					// 카운터 오버플로우 비트가 지워지지 않기 때문이다.
					// "09:26:58:337-LAND:
					// change oblige from 0x83 cause main down(0x03 - 5)!"
				}
			}
			else	c_sysChk.nDeadOppoCnt = 0;
		}
	}
#endif

	c_wArterSeq = 10;
	GETBAND(pBand);
	GETVERSE(pVerse);
	if (++ c_mill.wDtb >= TPERIOD_SPREAD)	c_mill.wDtb = 0;

	BYTE cFloatID = CAR_FSELF();
	if (c_convention.wSeq == DTBISEQ_NON) {	// normal
		if (cFloatID == FID_HEAD) {			// master
			//c_fio.SetTrio(0, true);
			if (c_mill.wDtb == 0) {	// && !c_bSupervising) {
				pBand->Synchro();			// sync. by master
				c_bcMon.wCounter = 0;
				//Spread();					// broadcast
				if (!c_itvm.IsSmall(0.08f, "LAND>WARN:spread interval "))
					Spread();
			}
			c_wArterSeq = 11;
		}
		else {								// slave
			if (-- c_bcMon.wTimer == 0) {
				c_bcMon.wTimer = TPERIOD_WAITBROADCASTC;
				INCWORD(c_bcMon.wRed);
				c_fio.Lamp(EPOS_BROADCASTCTIMEOUT);
				if (c_bcMon.wCounter >= COUNTER_BROADCASTX)	Alone();
				else	++ c_bcMon.wCounter;
			}
			for (int n = 0; n < 2; n ++) {
				if (-- c_bcMon.wWaiter[n] == 0) {
					c_bcMon.wWaiter[n] = TPERIOD_WAITBROADCASTC;
					c_fio.Lamp(n ? EPOS_BROADCASTBTIMEOUT :
									EPOS_BROADCASTATIMEOUT);
				}
			}
			c_wArterSeq = 12;
		}

		if (++ c_mill.wPop >= MAX_CENTIPOP)	c_mill.wPop = 0;

		c_wArterSeq = 13;
		switch (c_mill.wPop) {
		case 0 :		//1 :
			{
				// 210805
				if (pPaper->GetDeviceExFromRecip()) {
					c_dbgAtoHcr.inp[c_dbgAtoHcr.i] =
							GETTIS(FID_HEAD, TUDIB_HCR);
					if (++ c_dbgAtoHcr.i >= 5)	c_dbgAtoHcr.i = 0;
				}
				// 210803
				//if (++ c_mill.wAto >= TPERIOD_ATO) {
				WORD wLimit = pPaper->GetDeviceExFromRecip() ?
										TPERIOD_ATO2 : TPERIOD_ATO1;
				if (++ c_mill.wAto >= wLimit) {
					c_mill.wAto = 0;
					if (c_pPol != NULL && cFloatID < FID_TAIL &&
						c_steer.GetMode() != MOTIVMODE_EMERGENCY &&
						GETTI(FID_HEAD, TUDIB_HCR)) {
						c_pPol->SendAto();
					}
					// 211005, remove debug code.
					// 210728
					//else	pVerse->ClearAtoFlap();
					//else {
					//	// for DEBUG
					//	if (c_pPol == NULL)	pArch->Shot(CAR_PSELF(), 120);
					//	if (cFloatID >= FID_TAIL)	pArch->Shot(CAR_PSELF(), 121);
					//	if (c_steer.GetMode() == MOTIVMODE_EMERGENCY)	pArch->Shot(CAR_PSELF(), 122);
					//	if (!GETTI(FID_HEAD, TUDIB_HCR))	pArch->Shot(CAR_PSELF(), 123);
					//}
				}
			}
			//pBand->Region(3);
			break;
		case 1 :	case 6 :
			if (c_pLoc != NULL)	c_pLoc->Send50();	// 50ms
			//pBand->Region(c_wCentiPop == 1 ? 4 : 9);
			break;
		case 2 :	case 7 :
			if (c_pPol != NULL) {
				//_TWATCHCLICK();
				c_pPol->SendRtd();
				//_TWATCHMEASURE(WORD, c_rtdc.wCurTime, 6);
				//if (c_rtdc.wCurTime > c_rtdc.wMaxTime)
				//	c_rtdc.wMaxTime = c_rtdc.wCurTime;
			}
			//pBand->Region(c_wCentiPop == 2 ? 5 : 10);
			break;
		case 3 :
			if (c_pLoc != NULL)	c_pLoc->SendDcul();	// 100ms
			//pBand->Region(6);
			break;
		case 4 :
			if (c_pPol != NULL)	c_pPol->SendSvc();	// 100ms
			//pBand->Region(7);
			break;
		case 5 :
			if (c_pLoc != NULL)	c_pLoc->SendDcur();	// 100ms
			//pBand->Region(8);
			break;
		case 8 :
			// 200218
			//c_mill.bHvac = !c_mill.bHvac;
			//if (c_mill.bHvac) {						// 200ms
			//	if (c_pLoc != NULL)	c_pLoc->SendHvac();
			//	//c_wick.Send();
			//}
			if (c_pLoc != NULL)	c_pLoc->Send100();	// 100ms
			c_wick.Send();
			//pBand->Region(11);
			break;
		case 9 :
			if (ISTU() && c_pPol != NULL) {
				if (CAR_FSELF() == FID_HEAD)	c_fio.SetTrio(1, true);
				c_stage.Pave();	// 100ms
				if (CAR_FSELF() == FID_HEAD)	c_fio.SetTrio(1, false);
			}
			//pBand->Region(12);
			break;
		default :	break;
		}
		c_wArterSeq = 14;
		if (c_mill.wPopMax < c_mill.wPop)	c_mill.wPopMax = c_mill.wPop;
		if (c_otr.wSkipTimer > 0)	-- c_otr.wSkipTimer;

		c_wArterSeq = 15;
		if (c_dtb[0].p != NULL)	c_dtb[0].p->Arteriam();		// Arteriam2, X
		c_wArterSeq = 16;
		if (c_dtb[1].p != NULL)	c_dtb[1].p->Arteriam();		// Arteriam3, x
		c_wArterSeq = 17;
		if (ISTU()) {
			c_steer.Arteriam();		// Arteriam5, dtb
			c_wArterSeq = 18;
			c_stage.Arteriam();		// Arteriam6
			c_wArterSeq = 19;
			c_lide.Arteriam();		// Arteriam7
		}
		c_wArterSeq = 20;
		c_pel.Arteriam();			// Arteriam8
		c_wArterSeq = 21;
		c_univ.Arteriam();			// Arteriam9
		GETPROSE(pProse);
		c_wArterSeq = 23;
		pProse->Arteriam();			// Arteriam10
		c_wArterSeq = 24;
		pVerse->Arteriam();
		GETREVIEW(pRev);
		c_wArterSeq = 25;
		pRev->Arteriam();			// Arteriam11

		// 타코 메터
		c_wArterSeq = 26;
		if (++ c_smooth.tacho.iCycle >= TACHOREAD_CYCLE) {
			c_smooth.tacho.iCycle = 0;
			c_smooth.tacho.w[c_smooth.tacho.i] = c_fio.GetTacho();
			if (++ c_smooth.tacho.i >= TACHOBUF_MAX)	c_smooth.tacho.i = 0;
			WORD sum = 0;
			for (int n = 0; n < TACHOBUF_MAX; n ++)
				sum += c_smooth.tacho.w[n];
			c_smooth.tacho.wAvr = sum / TACHOBUF_MAX;
		}

		c_wArterSeq = 27;
		// 아나로그 AD7994, 변환 속도가 2us라는데...
		c_fio.GetAdv(c_smooth.ads.ch,
		 &c_smooth.ads.a[c_smooth.ads.ch].w[c_smooth.ads.a[c_smooth.ads.ch].i]);
		if (c_smooth.ads.a[c_smooth.ads.ch].bFull) {
			WORD sum = 0;
			for (int n = 0; n < ADCONVBUF_MAX; n ++)
				sum += c_smooth.ads.a[c_smooth.ads.ch].w[n];
			c_smooth.ads.a[c_smooth.ads.ch].wAvr = sum / ADCONVBUF_MAX;
		}
		if (++ c_smooth.ads.a[c_smooth.ads.ch].i >= ADCONVBUF_MAX) {
			c_smooth.ads.a[c_smooth.ads.ch].i = 0;
			c_smooth.ads.a[c_smooth.ads.ch].bFull = true;
		}
		if (++ c_smooth.ads.ch >= ADCONVCH_MAX)	c_smooth.ads.ch = 0;
	}	//if (c_convention.wSeq == DTBISEQ_NON) {	// normal
	else {
		if (c_mill.wDtb == 2 && Negotiate())
			InitialAll((int)c_cmd.intro.cFrom);
		// c_wCentiDtb가 9일 때만 Negotiate()를 수행한다. 버스 사이클마다 한번만 수행하도록...
	}

	c_wArterSeq = 30;
	pArch->Arteriam();				// Arteriam13
	if (c_cmd.wWaitRequireDoc > 0)	-- c_cmd.wWaitRequireDoc;	// 180308

	c_wArterSeq = 31;
	clk = ClockCycles();
	double duty = (double)(clk - c_pulse.clk) /
					(double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
	c_pulse.wDuty = (WORD)(duty * 1e5);
	c_tWatch.wArt[c_mill.wPop][0] = c_pulse.wDuty;
	if (c_tWatch.wArt[c_mill.wPop][0] > c_tWatch.wArt[c_mill.wPop][1])
		c_tWatch.wArt[c_mill.wPop][1] = c_tWatch.wArt[c_mill.wPop][0];
	if (c_pulse.wDuty > c_pulse.wDutyMax)	c_pulse.wDutyMax = c_pulse.wDuty;
	if (c_pulse.wDuty > 8000)	INCWORD(c_pulse.wExceed);
	pBand->Region(0);
	c_cmd.state.b.arteriam = false;
	c_wArterSeq = 32;
	if (cFloatID == FID_HEAD)	c_fio.SetTrio(0, false);
	c_wArterSeq = 33;
}

bool CLand::Negotiate()
{
	if (c_convention.wSeq == DTBISEQ_NON)	return true;

	c_wArterSeq = 100;
	if (++ c_mill.wPopN >= MAX_CENTIPOP) {
		c_mill.wPopN = 0;
		c_convention.bWicket ^= true;
		if (c_convention.bWicket)	c_wick.Send();
	}

	c_mtCvt.Lock();
	c_wArterSeq = 101;
	GETPAPER(pPaper);
	BYTE uOblige = pPaper->GetOblige();
	PDOZEN pDoz = pPaper->GetDozen();
	INCERT in;
	c_fio.GetInput(in, true);
	c_wArterSeq = 102;
	switch (c_convention.wSeq) {
	case DTBISEQ_DEFER :
		// ===== 01. from POWER ON =====
		if (-- c_convention.wWaitTimer == 0 ||
			c_convention.wBroadcastReceived > 0) {
		//if (-- c_convention.wWaitTimer == 0) {
			// TC0-liu1/2, TC1-liu1/2 실행에 시간차를 두고...
			// 2017/07/06, Modify
			c_wArterSeq = 103;
			//if (c_convention.wBroadcastReceived > 3 &&
			if (c_convention.wBroadcastReceived > 10 && //211210 check time delay
				pDoz->recip[FID_HEAD].real.prol.wAddr == DTB_BROADCASTADDR &&
				(pDoz->recip[FID_HEAD].real.prol.cCarLength & 0x3f) > 0) {
				c_wArterSeq = 104;
				// 마스터에서 수신을 받은 상태...
				// 2017/07/06, Modify
				BYTE cMasterPID = pDoz->recip[FID_HEAD].real.prol.cProperID;
				if (!(uOblige & (1 << OBLIGE_MAIN))) {		// 보조-위치에서...
					// 아래와 같이 하면 양쪽에서 HCR이 들어와 있을 때 곤란하다...
					//if (in.oct.c[0] & (1 << TUDIB_HCR))
					//	uOblige |= (1 << OBLIGE_HEAD);
					//else	uOblige &= ~(1 << OBLIGE_HEAD);
					//	// 선두나 후미로 설정한다.
					c_wArterSeq = 105;
					pPaper->Shoot(&pDoz->recip[FID_HEAD], SIZE_RECIP);
					if ((cMasterPID & 2) == (c_vehicle.cProperID & 2))
						uOblige |= (1 << OBLIGE_HEAD);
					else	uOblige &= ~(1 << OBLIGE_HEAD);
					ModifyDtb(uOblige);
					c_convention.wSeq = DTBISEQ_NON;
					// DTB는 슬레이브로 설정되어있다.
					MAKETUFNAME();
					c_fio.SetText(4, (PSZ)c_cFace);
					//TRACK("LAND>TSP6(S2):slave(0x%02x)\n", in.oct.c[0]);
					TRACK("LAND>TSP6(S2):slave(%d %d)\n",
											cMasterPID, c_vehicle.cProperID);
				}
				else {										// 주-위치에서...
					//TRACK("LAND:S-%d H%d T%d S%d\n",
					//			c_convention.wGatherCycle,
					//			pDoz->recip[FID_HEAD].real.prol.cProperID,
					//			pDoz->recip[FID_TAIL].real.prol.cProperID,
					//			c_cProperID);
					//if (pDoz->recip[FID_HEAD].real.prol.cProperID ==
					//	(c_cProperID + 1) ||
					//	pDoz->recip[FID_TAIL].real.prol.cProperID ==
					//	(c_cProperID + 1)) {
					c_wArterSeq = 106;
					if (cMasterPID == (c_vehicle.cProperID + 1) ||
						pDoz->recip[FID_TAIL].real.prol.cProperID ==
						(c_vehicle.cProperID + 1)) {
						// pPaper->GetRouteState(FID_TAIL)
						// 주-위치에서 보조 장치가 응답을 하고있다는 것은 major-TU가 리셋되었다는 것이다.
						c_wArterSeq = 107;
						c_fio.DisableWatchdog();
						// 171027
						c_fio.EnableOutput(false);
						c_fio.SetText(4, (PSZ)"down");
						TRACK("LAND>TSP6(S3):down\n");
						exit(0);
					}
					else {
						c_wArterSeq = 108;
						uOblige &= ~(1 << OBLIGE_HEAD);
						// 브로드캐스트를 받은 건 마스터(HEAD)가 아니라는...
						ModifyDtb(uOblige);
						c_convention.wSeq = DTBISEQ_NON;
						MAKETUFNAME();
						c_fio.SetText(4, (PSZ)c_cFace);
						TRACK("LAND>TSP6(S1):main\n");
						// c_convention.iBroadcastReceived > 0 ->
						// 마스터가 정해져서 송신을 시작하였다..
					}
				}
				// 상태가 정해져서 초기 기능을 수행한다...
				Intro(INTROFROM_KEY);
				//c_property.wWakeupCmdList = (1 << WAKEUPCMD_LOGMATCHTIME);
			}
			//else {
			else if (c_convention.wWaitTimer == 0) {
				c_wArterSeq = 109;
				sprintf((char*)c_cFace, "wcr%d", c_vehicle.cProperID);
				c_fio.SetText(4, (PSZ)c_cFace);
				c_convention.wSeq = DTBISEQ_WAITHCR;
				TRACK("LAND>TSP6(S0):wait hcr\n");
			}
		}
		break;
	case DTBISEQ_WAITHCR :
		// ===== 02. DTBISEQ_DEFER -> DTBISEQ_WAITHCR =====
		c_wArterSeq = 120;
		if (in.oct.c[0] & (1 << TUDIB_HCR)) {
			c_wArterSeq = 121;
			uOblige |= (1 << OBLIGE_HEAD);
			// 210222
			// 3호선 작업 중에...
			// DTB수정 전에 wSeq를 바꾸면 수정 중에 spread()를 통해 broadcast가 나갈 수 있다.
			// 그러면 segment fault가 날 수도 있다. 주의!!!
			//c_convention.wSeq = DTBISEQ_NON;
			ModifyDtb(uOblige);
			c_convention.wSeq = DTBISEQ_NON;
			TRACK("LAND>TSP7(S0):chg to 0x%02x(0x%02x)\n",
										uOblige, in.oct.c[0]);
		}
		else if (c_convention.wBroadcastReceived > 0) {
			c_wArterSeq = 122;
			uOblige &= ~(1 << OBLIGE_HEAD);
			// 210222
			// 3호선 작업 중에...
			// DTB수정 전에 wSeq를 바꾸면 수정 중에 spread()를 통해 broadcast가 나갈 수 있다.
			// 그러면 segment fault가 날 수도 있다. 주의!!!
			//c_convention.wSeq = DTBISEQ_NON;
			ModifyDtb(uOblige);
			c_convention.wSeq = DTBISEQ_NON;
			TRACK("LAND>TSP7(S1):chg to 0x%02x(0x%02x)\n",
										uOblige, in.oct.c[0]);
		}
		// 상태가 정해져서 초기 기능을 수행한다...
		Intro(INTROFROM_KEY);
		//c_property.wWakeupCmdList = (1 << WAKEUPCMD_LOGMATCHTIME);
		break;

	case DTBISEQ_WAITEMPTY :
		// ===== 11. from TurnObserve(), stick에 의해 모두 슬레이브가 된 다음 이 지점으로 온다. ===
		c_wArterSeq = 130;
		if (-- c_convention.wWaitTimer == 0) {
			c_wArterSeq = 131;
			if (in.oct.c[0] & (1 << TUDIB_HCR)) {
				c_convention.wWaitTimer = c_convention.wTime4Wait =
						(uOblige & (1 << OBLIGE_MAIN)) ? TIME_DTBOBSERVEMAIN :
														TIME_DTBOBSERVEAUX;
			}
			else	c_convention.wWaitTimer = c_convention.wTime4Wait =
														TIME_DTBOBSERVESLAVE;
			c_convention.wBroadcastReceived = 0;
			c_convention.wSeq = DTBISEQ_OBSERVE;
			TRACK("LAND>TSP4:observe(0x%02x-%d)\n",
									in.oct.c[0], c_convention.wWaitTimer);
			// TSP4. 정해진 시간동안 DTB통신이 없으므로 OBSERVE2로 전환한다.
		}
		break;
	case DTBISEQ_OBSERVE :
		// ===== 12. DTBISEQ_WAITEMTPY -> DTBISEQ_OBSERVE =====
		c_wArterSeq = 140;
		if (-- c_convention.wWaitTimer == 0)	 {
			c_wArterSeq = 141;
			if (in.oct.c[0] & (1 << TUDIB_HCR)) {
				c_wArterSeq = 142;
				uOblige |= (1 << OBLIGE_HEAD);
				// 210222
				//c_convention.wSeq = DTBISEQ_NON;
				ModifyDtb(uOblige);
				c_convention.wSeq = DTBISEQ_NON;
				// 상태가 정해져서 초기 기능을 수행한다...
				Intro(INTROFROM_STICK);
				//c_wWakeupCmdList = (1 << WAKEUPCMD_LOGMATCHTIME);
				//c_bClosePrevious = true;
				TRACK("LAND>TSP5(S0):DTB ON(0x%02x)\n", in.oct.c[0]);
			}
			else {
				c_wArterSeq = 142;
				c_convention.wWaitTimer = c_convention.wTime4Wait;
			}
		}
		else if (c_convention.wBroadcastReceived > 0) {
			c_wArterSeq = 143;
			if (in.oct.c[0] & (1 << TUDIB_HCR))	uOblige |= (1 << OBLIGE_HEAD);
			else	uOblige &= ~(1 << OBLIGE_HEAD);
			// 210222
			//c_convention.wSeq = DTBISEQ_NON;
			ModifyDtb(uOblige);
			c_convention.wSeq = DTBISEQ_NON;
			// 상태가 정해져서 초기 기능을 수행한다...
			Intro(INTROFROM_STICK);
			//c_wWakeupCmdList = (1 << WAKEUPCMD_LOGMATCHTIME);
			//c_bClosePrevious = true;
			if (uOblige & (1 << OBLIGE_HEAD))
				TRACK("LAND>TSP5(S1):haux(0x%02X)\n", in.oct.c[0]);
			else if (uOblige & (1 << OBLIGE_MAIN))
				TRACK("LAND>TSP5(S2):tmaj(0x%02X)\n", in.oct.c[0]);
			else	TRACK("LAND>TSP5(S3):taux(0x%02X)\n", in.oct.c[0]);
			// TSP5. 기다리는 동안 브로드캐스팅을 받아 각 상황에 맞게 전환한다.
		}
		break;
	default :	break;
	}

	c_wArterSeq = 150;
	GETARCHIVE(pArch);
	if (c_cmd.state.b.waitRequiredDoc) {
		c_wArterSeq = 151;
		PBULKPACK pBulk = pArch->GetBulk();
		if (pBulk->wState & (1 << BULKSTATE_CULLREQUIREDDOC)) {
			c_wArterSeq = 152;
			pArch->MirrorEnvirons(&pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[0]);
			pArch->MirrorCareers(&pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[100]);
			pArch->MirrorCmjog(&pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[300]);
			if (pPaper->GetLabFromRecip()) {
				PCAREERS pCrr = pArch->GetCareers();
				TRACK("LAND:cull %.3f %.3f %.3f %.3f\n",
						pCrr->real.dbDistance, pCrr->real.dbSivPower[0],
						pCrr->real.dbSivPower[1], pCrr->real.dbSivPower[2]);
			}

			if (pBulk->bcu.v.wAptness != BCAPTNESS_WORD) {
				memset(&pBulk->bcu.c[0], 0, sizeof(V3FBRKCNTU));
				pBulk->bcu.v.wAptness = BCAPTNESS_WORD;
			}
			pArch->MirrorV3fBreaker(&pBulk->bcu);
			c_cmd.state.b.waitRequiredDoc = false;
			c_cmd.wWaitRequireDoc = 0;
			if (pBulk->cCullState != 0) {
				c_wArterSeq = 153;
				if (pBulk->cCullState & (1 << CULLERR_CANNOTOPENENV))
					pArch->Shot(CAR_PSELF(), 163);
				if (pBulk->cCullState & (1 << CULLERR_UNMATCHSIZEENV))
					pArch->Shot(CAR_PSELF(), 164);
				if (pBulk->cCullState & (1 << CULLERR_WRONGLOGBOOKVERSION))
					pArch->Shot(CAR_PSELF(), 166);
				if (pBulk->cCullState & (1 << CULLERR_CANNOTOPENLOGBOOK))
					pArch->Shot(CAR_PSELF(), 167);
				if (pBulk->cCullState & (1 << CULLERR_UNMATCHSIZELOGBOOK))
					pArch->Shot(CAR_PSELF(), 168);
				if (pBulk->cCullState &
					(1 << CULLERR_CANNOTFOUNDLATELYLOGBOOK))
					pArch->Shot(CAR_PSELF(), 169);
				if (pBulk->cCullState & (1 << CULLERR_COUNDNOTFOUNDLOGBOOK))
					pArch->Shot(CAR_PSELF(), 170);
			}

			// 180511
			//if (c_bColdWakeup)	c_ctrlSide.a = 0;
			//else memcpy(&c_ctrlSide,
			//				&pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[400],
			//				sizeof(WORD));
			//TRACK("LAND:CtrlSide %04x\n", c_ctrlSide.a);
		}
		if (c_cmd.wWaitRequireDoc == 0)	pArch->Shot(CAR_PSELF(), 162);
	}

	bool bRes = false;
	c_wArterSeq = 160;
	if (c_convention.wSeq == DTBISEQ_NON) {
		c_wArterSeq = 161;
		if (c_cmd.state.b.waitRequiredDoc) {
			c_wArterSeq = 162;
			c_cmd.state.b.waitRequiredDoc = false;
			c_cmd.wWaitRequireDoc = 0;
			pArch->Shot(CAR_PSELF(), 161);
			TRACK("LAND>WARN:REQUIRED DOC. EMPTY!!!\n");
		}
		//return true;
		bRes = true;
	}
	c_mtCvt.Unlock();
	//return false;
	return bRes;
}

bool CLand::ModifyDtb(BYTE uOblige)
{
	GETPAPER(pPaper);
	CUCUTOFFA();

	pPaper->Setup(uOblige, c_vehicle.length.cCurrent);	// 190909 cActive);
	memset(&c_recip[POPI_SEND], 0, SIZE_RECIP);
	memset(&c_recip[POPI_WORK], 0, SIZE_RECIP);
	memset(&c_recip[POPI_HEAD], 0, SIZE_RECIP);
	for (int n = 0; n < 2; n ++)	c_dtb[n].state.a = 0;
		//c_dtb[n].bRxedBrc = c_dtb[n].bRxedCls = false;

	if (!c_dtb[0].p->ModifyBus((int)CAR_FSELF())) {
		c_fio.SetText(4, (PSZ)"DTAE");
		TRACK("LAND>ERR:can not modify A bus!\n");
		return false;
	}

	if (!c_dtb[1].p->ModifyBus((int)CAR_FSELF())) {
		c_fio.SetText(4, (PSZ)"DTBE");
		TRACK("LAND>ERR:can not modify B bus!\n");
		return false;
	}
	// 171027
	c_fio.EnableOutput((uOblige & (1 << OBLIGE_MAIN)) ? true : false);

	c_fio.MaskSign(0xff, 0xff);
	MAKETUFNAME();
	c_fio.SetText(4, (PSZ)c_cFace);
	c_cmd.state.b.dtbActive = true;

	return true;
}

void CLand::Alone()
{
	// 190110
	//if (!c_cmd.state.b.alone) {
	//	c_cmd.state.b.alone = true;
	if (!c_bAlone) {
		c_bAlone = true;
		TRACK("[LAND]:Alone()\n");
		c_dtb[0].p->ClearAllDozen();
		c_dtb[1].p->ClearAllDozen();
		CaptureLines();
		c_dtb[0].p->SendBus((PVOID)&c_recip[POPI_SEND],
								c_order.dwPassive[0], false);
		c_dtb[1].p->SendBus((PVOID)&c_recip[POPI_SEND],
								c_order.dwPassive[1], false);
		Supervise(true);
		c_fio.SetText(1, (PSZ)"c");
	}
	// ...
	c_pel.Alone();
	c_fio.SetOutput(UDOB_DCL, true);
	c_fio.SetOutput(UDOB_ACL1, true);
	c_fio.SetOutput(UDOB_ACL2, true);
}

bool CLand::Spread()
{
	//WORD w = c_wChapID;
	GETBAND(pBand);
	WORD w = pBand->GetRegionID();
	CaptureLines();		// 현재 입력 -> c_recip[1], 마스터는 브로드캐스트전에...
	Reform();			// master before broadcast, POPI_WORK -> POPI_SEND
	//GETBAND(pBand);
	if (++ c_order.dwActive == 0)
		c_order.dwActive = 1;	// 마스터가 먼저 새롭게 설정하는 값
	c_recip[POPI_SEND].real.prol.cCarLength |= 0x40;
	//c_recip[POPI_SEND].real.prol.cType = 'A';
	if (c_dtb[0].p != NULL) {
		c_dtb[0].p->SendBus((PVOID)&c_recip[POPI_SEND], c_order.dwActive);
		c_dtb[0].state.b.open = true;
		c_dtb[0].state.b.close = false;
		//c_dtb[0].bRxedBrc = true;
		//c_dtb[0].bRxedCls = false;		// 마스터 버스 사이클 시작
		pBand->Section(false, 0);
		//if (!c_dtb[0].bInit) {
		//	c_dtb[0].bInit = true;
		//	c_dtb[0].clk = ClockCycles();
		//}
		//else {
		//	uint64_t cur = ClockCycles();
		//	double sec = (double)(cur - c_dtb[0].clk) /
		//					(double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
		//	if (sec < 0.06f) {
		//		GETTOOL(pTool);
		//		pTool->Rollf("LAND>ERR:dtb0 send interval %fms!\n",
		//													sec * 1e3);
		//	}
		//	c_dtb[0].clk = cur;
		//}
	}
	pBand->Region(1);
	c_recip[POPI_SEND].real.prol.cCarLength &= 0xf;
	c_recip[POPI_SEND].real.prol.cCarLength |= 0x80;
	//c_recip[POPI_SEND].real.prol.cType = 'B';
	if (c_dtb[1].p != NULL) {
		c_dtb[1].p->SendBus((PVOID)&c_recip[POPI_SEND], c_order.dwActive);
		c_dtb[1].state.b.open = true;
		c_dtb[1].state.b.close = false;
		//c_dtb[1].bRxedBrc = true;
		//c_dtb[1].bRxedCls = false;		// 마스터 버스 사이클 시작
		pBand->Section(true, 0);
		//if (!c_dtb[1].bInit) {
		//	c_dtb[1].bInit = true;
		//	c_dtb[1].clk = ClockCycles();
		//}
		//else {
		//	uint64_t cur = ClockCycles();
		//	double sec = (double)(cur - c_dtb[1].clk) /
		//					(double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
		//	if (sec < 0.06f) {
		//		GETTOOL(pTool);
		//		pTool->Rollf("LAND>ERR:dtb1 send interval %fms!\n", sec * 1e3);
		//	}
		//	c_dtb[1].clk = cur;
		//}
	}

	pBand->Region(2);
	// 190909
	//if (CheckCarLength(c_vehicle.length.cActive)) {
	if (CheckCarLength(c_vehicle.length.cCurrent)) {
		// meaningless perhaps this is time sync. to slave
		GETPAPER(pPaper);
		pPaper->SetLength(c_vehicle.length.cCurrent);
		AlignLocal(c_vehicle.length.cCurrent);
		c_dtb[0].p->SetCarLength(c_vehicle.length.cCurrent);
		c_dtb[1].p->SetCarLength(c_vehicle.length.cCurrent);
		if (pPaper->GetLabFromRecip())
			TRACK("LAND:master match car length %d\n",
									c_vehicle.length.cCurrent);
		// just match slaves
		//190813, to six car
		//c_vehicle.length.cPassive = c_vehicle.length.cActive;
		//AlignLocal(c_vehicle.length.cPassive);
		//if (c_vehicle.length.cPassive != c_vehicle.length.cActive) {
		//	c_vehicle.length.cPassive = c_vehicle.length.cActive;
		//	AlignLocal(c_vehicle.length.cPassive);
		//	c_dtb[0].p->SetCarLength(c_vehicle.length.cPassive);
		//	c_dtb[1].p->SetCarLength(c_vehicle.length.cPassive);
		//}
		//if (c_vehicle.length.cPassive != c_vehicle.length.cActive) {
		//	c_vehicle.length.cPassive = c_vehicle.length.cActive;
		//	AlignLocal(c_vehicle.length.cPassive);
		//	c_dtb[0].p->SetCarLength(c_vehicle.length.cPassive);
		//	c_dtb[1].p->SetCarLength(c_vehicle.length.cPassive);
		//	GETPAPER(pPaper);
		//	pPaper->SetLength(c_vehicle.length.cPassive);
		//	if (pPaper->GetLabFromRecip())
		//		TRACK("LAND:master match car length %d %d\n",
		//			c_vehicle.length.cActive, c_vehicle.length.cPassive);
		//}
	}

	if (w != 0)	TRACK("LAND>ERR:region crash to %d!!!\n", w);
	return true;
}

// 1.05부터는 Begin과 End가 없고 모두 DTB 송수신 완료 후(End Chapter 시점)에 실행한다
// 제어 결과는 c_recip[1]프레임에 담긴다.
void CLand::Supervise(bool bAlone)
{
	//c_bSupervising = true;
	_TWATCHCLICK();
	GETPAPER(pPaper);
	if (CAR_FSELF() == FID_HEAD)	c_fio.SetTrio(2, true);

	GETBAND(pBand);
	pBand->Region(3);
	Gather();
	if (c_convention.wSeq != DTBISEQ_NON) {
		//++ c_convention.wGatherCycle;
		return;
	}

	PDOZEN pDoz = pPaper->GetDozen();
	// 190110
	//if (!bAlone)
	//	c_cmd.state.b.domin = pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin;
	//else
	//	pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin = c_cmd.state.b.domin;
	if (!bAlone)
		c_bAlone = pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin;
	else	pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin = c_bAlone;

	GETARCHIVE(pArch);
	// 작업을 시작하기 전에 열차 진행 방향이 바뀌었다면 기록 디렉토리부터 정리한다.
	if (ISTU() && c_cmd.intro.cSeq == INTROSEQ_UNFOLD &&
		pPaper->GetRouteState(FID_HEAD)) {
		// 180911, 마스터의 통신 조건을 본다는 건? 어쨋든 확인
		c_cmd.intro.cSeq = INTROSEQ_NON;
		if (pDoz->recip[FID_HEAD].real.logMatch.dw > 0) {
			// 새로운 디렉토리를 생성한다. SetLogbookHead()
			if (c_cmd.intro.cFrom == INTROFROM_KEY) {
				if (pArch->CloseCurrent()) {
					//c_wWakeupCmdList &= ~(1 << WAKEUPCMD_CLOSECURRENT);
					// 170821, 여기서 하면 dominate전에 할 수 있다.
					if (CAR_FSELF() == FID_HEAD)	StirEnvironMatch();
					TRACK("LAND:new sheet from key!\n");
				}
			}
			else {	// INTROFROM_STICK, INTROFROM_OTRN
				if (pArch->NewSheet()) {
					pArch->AddRtdEdge();
					//c_stage.RegisterPage(ADDR_HEADBOXSENTENCE);
					if (c_cmd.intro.cFrom == INTROFROM_STICK)
						TRACK("LAND:new sheet from stick!\n");
					else	TRACK("LAND:new sheet from otrn xch!\n");
				}
			}
		}
		//else	TRACK("LAND:un recip!\n");
		else {
			TRACK("LAND:ERR>log un-match!!! %02d/%02d/%02d %02d:%02d:%02d\n",
					pDoz->recip[FID_HEAD].real.logMatch.t.year,
					pDoz->recip[FID_HEAD].real.logMatch.t.mon,
					pDoz->recip[FID_HEAD].real.logMatch.t.day,
					pDoz->recip[FID_HEAD].real.logMatch.t.hour,
					pDoz->recip[FID_HEAD].real.logMatch.t.min,
					pDoz->recip[FID_HEAD].real.logMatch.t.sec);
		}
	}

	// 200218
	pPaper->SetDeviceExByRecip(pDoz->recip[FID_HEAD].real.sign.pcmd.b.devEx);

	c_stick.CheckDomination();
	// 각 함수의 결과는 c_recip[POPI_WORK]에 남기므로 호출하는 클래스 순서를 주의한다.
	c_turf.Monitor(&c_recip[POPI_WORK]);
	if (ISTU())	c_stick.Monitor();
	pBand->Region(4);
	if (ISTU()) {
		// 감시를 먼저하고 제어를 한다.
		c_steer.Monitor(&c_recip[POPI_WORK]);
		pBand->Region(5);
		c_lide.Monitor(&c_recip[POPI_WORK]);
		pBand->Region(6);
		c_pel.Monitor(&c_recip[POPI_WORK]);
		pBand->Region(7);
		c_univ.Monitor(&c_recip[POPI_WORK]);
		pBand->Region(8);
		DtbControl();					// inspect뿐이다.
		if (pPaper->GetDeviceExFromRecip())	c_stage.Monitor();
	}
	GETPROSE(pProse);
	pProse->Monitor();
	c_turf.Control(&c_recip[POPI_WORK]);
	if (ISTU()) {
		pBand->Region(9);
		c_stick.Control(&c_recip[POPI_WORK]);
		pBand->Region(10);
		c_steer.Control(&c_recip[POPI_WORK]);
		pBand->Region(11);
		c_lide.Control(&c_recip[POPI_WORK]);
		pBand->Region(12);
		c_lide.LampControl(&c_recip[POPI_WORK]);
	}
	c_pel.Control(&c_recip[POPI_WORK]);		// cm & esk control
	pBand->Region(13);
	c_univ.Control(&c_recip[POPI_WORK]);	// lamp control
	pBand->Region(14);

	if (ISTU()) {
		if (++ c_mill.wClosureCounter >= TPERIOD_CLOSURE)
			c_mill.wClosureCounter = 0;
		pArch->ClosureInside(!c_mill.wClosureCounter ? true : false);
		pBand->Region(15);

		// 17/07/06, Append
		//PDOZEN pDoz = pPaper->GetDozen();
		GETVERSE(pVerse);
		if (pDoz->recip[FID_HEAD].real.sign.ecmd.b.relblk ||
			(pPaper->GetRouteState(FID_TAIL) &&
			pDoz->recip[FID_TAIL].real.sign.ecmd.b.relblk)) {
			// 180911, 전원 투입 후, 확인
			if ((pDoz->recip[FID_HEAD].real.cPoleBlock &
				((1 << PDEVID_PAU) | (1 << PDEVID_PIS))) ||
				(pDoz->recip[FID_TAIL].real.cPoleBlock &
				((1 << PDEVID_PAU) | (1 << PDEVID_PIS))))
				pVerse->ReleaseServiceBlock();
			else	c_cmd.user.b.releaseServiceBlk = false;
		}

		// 171130
		// PIS CONTROL CODE #6 :
		if (pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
			if (pDoz->recip[FID_HEAD].real.motiv.lead.b.epreq ||
				(pPaper->GetRouteState(FID_TAIL) &&
				pDoz->recip[FID_TAIL].real.motiv.lead.b.epreq)) {
				// 180911, 전원 투입 후, 확인
				bool bPisAim;
				int cid;
				bool bUser;
				if (pDoz->recip[FID_HEAD].real.motiv.lead.b.epreq) {
					bPisAim = pDoz->recip[FID_HEAD].real.motiv.lead.b.epaim;
					if ((pDoz->recip[FID_HEAD].real.cSvcCtrlCode & 0xf) ==
						SVCCODEB_TAIL)
						pVerse->TurnPisCode();
					cid = CID_BOW;
					bUser = false;	// by head
				}
				else {
					bPisAim = pDoz->recip[FID_TAIL].real.motiv.lead.b.epaim;
					if (((pDoz->recip[FID_TAIL].real.cSvcCtrlCode &
						0xf0) >> 4) == SVCCODEB_TAIL)
						pVerse->TurnPisCode();
					cid = CID_STERN;
					bUser = true;	// by tail
				}
				c_cmd.user.b.pisRequest = false;
				// 다음번 전송에서 motiv.lead.b.epreq는 0이 된다!!!
				pVerse->SetPisMode(bUser, bPisAim);
				pArch->Shot(cid, bPisAim ? 840 : 841);
				if (pPaper->GetLabFromRecip())
					TRACK("LAND:pis %s at %s.\n",
											bPisAim ? "manual" : "auto",
											bUser ? "tail" : "head");
			}
		}
		pVerse->SetServiceControlCode();

		// 170905
		if (pDoz->recip[FID_HEAD].real.sign.ecmd.b.clrvb) {
			if (!c_cmd.user.b.v3fBreakCntClear) {
				c_cmd.user.b.v3fBreakCntClear = true;
				pArch->ClearV3fBreaker();
			}
		}
		else	c_cmd.user.b.v3fBreakCntClear = false;

		pBand->Region(16);
		// 170817
		// 17/07/08, Append
		//BYTE cOblige = pPaper->GetOblige();
		//if (!(cOblige & (1 << OBLIGE_MAIN))) {
		//	BYTE cChkOblige = cOblige;
		//	if ((pDoz->recip[FID_HEAD].real.prol.cProperID & 2) ==
		//		(c_cProperID & 2))
		//		cChkOblige |= (1 << OBLIGE_HEAD);
		//	else	cChkOblige &= ~(1 << OBLIGE_HEAD);
		//	if (cOblige != cChkOblige) {
		//		pArch->Shot(CAR_PSELF(), 174);
		//		pPaper->ChangeOblige(cChkOblige);
		//		TRACK("LAND:chg %d -> %d (%d-%04X)\n",
		//				cOblige, cChkOblige,
		//				pDoz->recip[FID_HEAD].real.prol.cProperID,
		//				pDoz->recip[FID_HEAD].real.prol.wAddr);
		//	}
		//}

		if (c_trcInfo.bClear) {
			CProse::PLSHARE pLsv = pProse->GetShareVars();
			if (!pLsv->sivcf.bTrace && !pLsv->v3fcf.bTrace &&
				!pLsv->ecucf.bTrace)
				c_trcInfo.bClear = false;
		}
		pBand->Region(17);
		if (c_trcInfo.wSeq == TRACESEQ_COLLECT) {
			// 각 객차로부터 ECU TRACE 데이터 모은다.
			if (c_trcInfo.wItem & (1 << TRACEITEM_SIV))
				CollectTrace(DEVID_SIV);
			else if (c_trcInfo.wItem & (1 << TRACEITEM_V3F))
				CollectTrace(DEVID_V3F);
			else if (c_trcInfo.wItem & (1 << TRACEITEM_ECU))
				CollectTrace(DEVID_ECU);
			else if (c_trcInfo.wItem & (1 << TRACEITEM_ALIGN)) {
				// * TRACE 7-1 :
				// 선택된 객차에서 align이 완료되기를 기다린다. ->
				// 완료되면 (1 << TRACEITEM_ALIGN)을 지우고 (1 << TRACEITEM_ECU)로 전환한다.
				if (-- c_trcInfo.wWaitArrangeTimer == 0)
					memset(&c_trcInfo, 0, sizeof(TRACEINFO));
				else {
					WORD wRespDevMap = 0;
					WORD wNullDevMap = 0;
					GETSCHEME(pSch);
	// ===== UNTAP 4 =========================================================
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		if (c_trcInfo.wItem & (1 << n)) {
			int fid = pSch->C2F(pPaper->GetTenor(), n);
			if (pPaper->GetRouteState(fid)) {
				// 180911, 전원 투입 후, 확인
				if (pDoz->recip[fid].real.cst.cID == CSTID_TRACE &&
					(pDoz->recip[fid].real.cst.roll.trc.ans.wDevIDMap &
					(1 << TRACEITEM_ALIGN)) &&
					(pDoz->recip[fid].real.cst.roll.trc.ans.wDevIDMap &
					(1 << n))) {
					if (pDoz->recip[fid].real.cst.roll.trc.ans.wChapID ==
						AETSTEP_END)
						wRespDevMap |= (1 << n);
					else if (pDoz->recip[fid].real.cst.roll.trc.ans.wChapID ==
						0xffff)
						wNullDevMap |= (1 << n);
				}
			}
		}
	}
	// ===== TAP 4 ===========================================================
					if ((c_trcInfo.wItem & pSch->GetCarBitmap()) ==
						(wRespDevMap | wNullDevMap)) {
						c_trcInfo.wItem &= ~(1 << TRACEITEM_ALIGN);
						c_trcInfo.wItem = wRespDevMap | (1 << TRACEITEM_ECU);
						TRACK("LAND:trace align end. %04x\n", wRespDevMap);
					}
					c_trcInfo.wRespDevMap = wRespDevMap;
					c_trcInfo.wNullDevMap = wNullDevMap;
				}
			}
		}

		if (GETTI(FID_HEAD, TUDIB_HCR) && c_cmd.prevent.wEcuBcf > 0) {
			-- c_cmd.prevent.wEcuBcf;
			//if (c_wEcuBcfPreventTimer == 0)
			//	TRACK("LAND:end ecu bcf prevent.\n");
		}

		pBand->Region(18);
		int ePos = EPOS_TMMBUFOVER;
		for (int n = SPARKLE_TROUBLEMM; n < SPARKLE_MAX; n ++) {
			for (int m = 0; m < 2; m ++) {
				if (pArch->GetSparkle(n, m))	c_fio.Lamp(ePos);
				++ ePos;
			}
		}
		pArch->ShiftCount(&c_tcnt.wShot, &c_tcnt.wCut);
	}
	else	pArch->ClosureInside();		// cc...

	if (ISTU()) {
		// 17/12/12
		GETVERSE(pVerse);
		if ((pDoz->recip[FID_HEAD].real.drift.cNbrk &
			(1 << NDETECT_ZEROSPEED)) &&
			c_otr.no.Stabilize(pVerse->GetCurOtrn())) {
			// 열차 번호가 바뀌었다...
			if (c_otr.no.cur != DEFAULT_OTRN && c_otr.wSkipTimer == 0) {
				c_bColdWakeup = false;
				c_bControlInitialed = false;
				pArch->DownloadCancel();
				Intro(INTROFROM_OTRN);
				InitialAll(INTROFROM_OTRN);
			}
		}
		// 181012
		// 180626
		WORD wh = pDoz->recip[FID_HEAD].real.avs.wCurve[ADCONVCH_OUTTEMP];
		if (wh > 0xfff)	pArch->Shot(CID_BOW, 642);
		else	pArch->Cut(CID_BOW, 642);
		WORD wt = 0xffff;
		// 180911
		//if (pPaper->GetRouteState(FID_TAIL)) {
		if (pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL)) {
			wt = pDoz->recip[FID_TAIL].real.avs.wCurve[ADCONVCH_OUTTEMP];
			if (wt > 0xfff)	pArch->Shot(CID_STERN, 642);
			else	pArch->Cut(CID_STERN, 642);
		}
		WORD w = 0;
		if (wh <= 0xfff) {
			if (wt <= 0xfff)	w = (wh + wt) / 2;
			else	w = wh;
		}
		else if (wt <= 0xfff)	w = wt;
		else	w = 2048;
		GETSCHEME(pSch);
		// 191001
		int etemp = pSch->GetProportion((int)w, 1023, 4095, -50, 100);
		c_vExtTemp = (short)etemp;

		if (pPaper->GetLabFromRecip()) {
			for (int n = FID_HEADBK; n < LENGTH_TU; n ++) {
				// 190826
				//if ((pDoz->recip[FID_HEAD].real.cst.cID == CSTID_ENVIRONS &&
				//	pDoz->recip[n].real.cst.cID != CSTID_ENVIRONS) ||
				//	(pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS &&
				//	pDoz->recip[n].real.cst.cID != CSTID_CAREERS))
				// 200218
				//if ((pDoz->recip[FID_HEAD].real.cst.cID == CSTID_ENVIRONS &&
				//	pPaper->GetRouteState(n) &&
				//	pDoz->recip[n].real.cst.cID != CSTID_ENVIRONS) ||
				//	(pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS &&
				//	pPaper->GetRouteState(n) &&
				//	pDoz->recip[n].real.cst.cID != CSTID_CAREERS))
				if (pPaper->GetRouteState(n) &&
					((pDoz->recip[FID_HEAD].real.cst.cID == CSTID_ENVIRONS &&
					pDoz->recip[n].real.cst.cID != CSTID_ENVIRONS) ||
					(pDoz->recip[FID_HEAD].real.cst.cID == CSTID_CAREERS &&
					pDoz->recip[n].real.cst.cID != CSTID_CAREERS) ||
					(pDoz->recip[FID_HEAD].real.cst.cID == CSTID_BMSEX &&
					pDoz->recip[n].real.cst.cID != CSTID_BMSEX)))
					TRACK("LAND:ERROR different consult id %d!\n", n);
			}
		}
	}

	pBand->Region(19);
	GETFSC(pFsc);
	pFsc->Pull();		// 밀린 메시지들을 출력한다.
	pBand->Region(20);
	pBand->ResetRegionID();

	if (!c_wick.GetState())
		pBand->Bale(false, c_cWickBuf);		// dtb 시간을 계산한다.

	_TWATCHMEASURED();
	c_tWatch.dwSupervise[0] = __sec_ * 1e6;
	if (c_tWatch.dwSupervise[0] > c_tWatch.dwSupervise[1]) {
		c_tWatch.dwSupervise[1] = c_tWatch.dwSupervise[0];
		c_tWatch.dwOrderAtMax = pDoz->recip[FID_HEAD].real.prol.dwOrder;
		pBand->Backup();
	}

	if (CAR_FSELF() == FID_HEAD)	c_fio.SetTrio(2, false);
}

void CLand::Gather()
{
	GETPAPER(pPaper);
	pPaper->Freshly();		// cur -> back/work,	cur <- 0
	PDOZEN pDoz = pPaper->GetDozen();		// *** current !!!

	RECIP recb;
	PRECIP pRecb;
	//if (c_dtb[0].bRxedCls) {
	if (c_dtb[0].state.b.close) {
		c_dtb[0].p->Bunch((BYTE*)pDoz);
		for (int n = 0; n < (int)(c_vehicle.length.cCurrent + 2); n ++) {
			// 190909 cPassive + 2); n ++) {
			BYTE route = CPaper::ROUTE_NON;
			//if (PrologConfirm(&pDoz->recip[n].real.prol, n ?
			//										c_dwOrder[2] : 0)) {
			if (PrologConfirm(&pDoz->recip[n].real.prol, c_order.dwActive)) {
				route = CPaper::ROUTE_MAIN;
				pRecb = &recb;
			}
			else	pRecb = &pDoz->recip[n];

			//if (c_dtb[1].bRxedCls) {
			if (c_dtb[1].state.b.close) {
				c_dtb[1].p->Demount((BYTE*)pRecb, n);
				//if (PrologConfirm(&pRecb->real.prol, n ? c_dwOrder[2] : 0))
				if (PrologConfirm(&pRecb->real.prol, c_order.dwActive))
					route |= CPaper::ROUTE_AUX;
			}
			pPaper->Routing(n, route);
		}
	}
	//else if (c_dtb[1].bRxedCls) {
	else if (c_dtb[1].state.b.close) {
		c_dtb[1].p->Bunch((BYTE*)pDoz);
		for (int n = 0; n < (c_vehicle.length.cCurrent + 2); n ++) {
			// 190909 cPassive + 2); n ++) {
			//if (PrologConfirm(&pDoz->recip[n].real.prol,
			//										n ? c_dwOrder[2] : 0))
			if (PrologConfirm(&pDoz->recip[n].real.prol, c_order.dwActive))
				pPaper->Routing(n, CPaper::ROUTE_AUX);
		}
	}
	pPaper->Repair();
	// 171122
	GETARCHIVE(pArch);
	GETSCHEME(pSch);
	if (pDoz->recip[FID_HEAD].real.prol.cProperID & 1)
		pArch->Shot(pSch->F2C(pPaper->GetTenor(), FID_HEAD), 9);
	// 19/04/08
	else	pArch->Cut(pSch->F2C(pPaper->GetTenor(), FID_HEAD), 9);
	if (pDoz->recip[FID_TAIL].real.prol.cProperID & 1)
		pArch->Shot(pSch->F2C(pPaper->GetTenor(), FID_TAIL), 9);
		// pPaper->GetRouteState(FID_TAIL)
	// 19/04/08
	else	pArch->Cut(pSch->F2C(pPaper->GetTenor(), FID_TAIL), 9);

	GETPROSE(pProse);
	// 180911
	//if (pPaper->GetRouteState(FID_TAIL)) {	// && c_ctrlSide.a != 0xffff) {
	if (pPaper->GetRouteState(FID_TAIL) && VALIDRECIP(FID_TAIL)) {
		// 180511
		int left = pDoz->recip[FID_HEAD].real.prol.cProperID < PID_RIGHT ?
														FID_HEAD : FID_TAIL;
		int right = left == FID_HEAD ? FID_TAIL : FID_HEAD;
		// 180626
		if (!c_bControlInitialed) {
			c_bControlInitialed = true;
			if (!pProse->c_hvacMode.IsInit()) {
				pProse->c_hvacMode.Init((BYTE)GETTIS(left, TUDIB_CHCS1) & 0xf,
									(BYTE)GETTIS(right, TUDIB_CHCS1) & 0xf);
				TRACK("LAND:side initial hvac\n");
			}
			if (!pProse->c_hlfsSw.IsInit()) {
				pProse->c_hlfsSw.Init(PTI2B(left, TUDIB_HLFS),
												PTI2B(right, TUDIB_HLFS));
				TRACK("LAND:side initial hlfs\n");
			}
			if (!pProse->c_elfsSw.IsInit()) {
				pProse->c_elfsSw.Init(PTI2B(left, TUDIB_ELFS),
												PTI2B(right, TUDIB_ELFS));
				TRACK("LAND:side initial elfs\n");
			}
			if (!pProse->c_vfsSw.IsInit()) {
				pProse->c_vfsSw.Init(PTI2B(left, TUDIB_VFS),
												PTI2B(right, TUDIB_VFS));
				TRACK("LAND:side initial vfs\n");
			}
			if (!pProse->c_apdsSw.IsInit()) {
				if (!pPaper->GetDeviceExFromRecip())
					pProse->c_apdsSw.Init(PTI2B(left, TUDIB_APDS),
												PTI2B(right, TUDIB_APDS));
				else {
					pProse->c_apdsSw.Init(PCI2B(left, CUDIB_APDS1),
												PCI2B(right, CUDIB_APDS1));
					pProse->c_apdsASw.Init(PCI2B(left, CUDIB_APDS2),
												PCI2B(right, CUDIB_APDS2));
				}
				TRACK("LAND:side initial apds\n");
			}
			if (!pProse->c_dcSw.IsInit()) {
				pProse->c_dcSw.Init(NTI2B(left, TUDIB_DLCS),
												NTI2B(right, TUDIB_DLCS));
				TRACK("LAND:side initial dlcs\n");
			}
			if (!pProse->c_acSw.IsInit()) {
				pProse->c_acSw.Init(PTI2B(left, TUDIB_ALCS),
												PTI2B(right, TUDIB_ALCS));
				TRACK("LAND:side initial alcs\n");
			}
		}
	}

	//GETBAND(pBand);
	//pBand->Region(14);

	if (c_convention.wSeq != DTBISEQ_NON) {
		++ c_convention.wGatherCycle;
		return;
	}
	// === trouble detection begin
	if (ISTU() && pDoz->recip[FID_HEAD].real.sign.gcmd.b.domin) {
		WORD wFail = 0;
		GETREVIEW(pRev);
		WORD wItem = pRev->GetInspectItem();
		WORD wStep = pRev->GetInspectStep();
		CReview::PLFCOUNT pLfc = NULL;
		if ((wItem & (1 << INSPITEM_TCL)) && wStep == INSPSTEP_DTBCHECK)
			pLfc = pRev->GetLineFaultCount();

		//BYTE cDtbChk[PID_MAX];
		// 210510
		//for (int fid = FID_HEAD; fid < PID_MAX; fid ++)
		//	c_cDtbChk[fid] = fid;
		//for (int fid = FID_HEAD;
		//	fid < (int)pPaper->GetLength() + 2; fid ++) {
		//	if (pPaper->GetRouteState(fid))
		//		c_cDtbChk[pDoz->recip[fid].real.prol.cProperID] |= 0x80;
		//		// 180911, 통신 이상, 확인
		//}
		//for (int fid = FID_HEAD;
		//	fid < (int)pPaper->GetLength() + 2; fid ++) {
		//	if (c_cDtbChk[fid] < 0x80) {
		//		if (fid < FID_PAS)
		//			pArch->Shot(pSch->F2C(pPaper->GetTenor(), fid),
		//											(fid & 1) ? 12 : 10);
		//		else	pArch->Shot(pSch->F2C(pPaper->GetTenor(), fid), 11);
		//		if (pLfc != NULL)	++ pLfc->wDtb[fid];
		//		++ wFail;
		//	}
		//	else {
		//		BYTE fida = c_cDtbChk[fid] & 0x7f;
		//		if (fida < FID_PAS)
		//			pArch->Cut(pSch->F2C(pPaper->GetTenor(), fid),
		//											(fida & 1) ? 12 : 10);
		//		else	pArch->Cut(pSch->F2C(pPaper->GetTenor(), fid), 11);
		//	}
		//}
		for (int pid = PID_LEFT; pid < PID_MAX; pid ++)
			c_cDtbChk[pid] = pid;
		for (int pid = PID_LEFT;
			pid < (int)pPaper->GetLength() + 2; pid ++) {
			if (pPaper->GetRouteState(pid))
				c_cDtbChk[pDoz->recip[pid].real.prol.cProperID] |= 0x80;
				// 180911, 통신 이상, 확인
		}
		for (int pid = PID_LEFT;
			pid < (int)pPaper->GetLength() + 2; pid ++) {
			if (c_cDtbChk[pid] < 0x80) {
				if (pid < PID_PAS)
					pArch->Shot(pSch->P2C(pid), (pid & 1) ? 12 : 10);
				else	pArch->Shot(pSch->P2C(pid), 11);
				if (pLfc != NULL)	++ pLfc->wDtb[pid];
				++ wFail;
			}
			else {
				BYTE pida = c_cDtbChk[pid] & 0x7f;
				if (pida < PID_PAS)
					pArch->Cut(pSch->P2C(pid), (pida & 1) ? 12 : 10);
				else	pArch->Cut(pSch->P2C(pid), 11);
			}
		}
		CProse::PLSHARE pLsv = pProse->GetShareVars();
		pLsv->wFailedCCs = wFail;

		for (int fid = FID_HEAD;
			fid < (int)pPaper->GetLength() + 2; fid ++) {
			// 180911
			//if (pPaper->GetRouteState(fid)) {
			if (pPaper->GetRouteState(fid) && VALIDRECIP(fid)) {
				int cid = pSch->F2C(pPaper->GetTenor(), fid);
				if (pDoz->recip[fid].real.cMmState != 0) {
					if (fid < FID_PAS) {
						pArch->Shot(cid, (fid & 1) ? 75 : 74);

						if (pDoz->recip[fid].real.cMmState &
							(1 << MMSTATE_INVALIDQUEUE))
							pArch->Shot(cid, (fid & 1) ? 81 : 77);
						else	pArch->Cut(cid, (fid & 1) ? 81 : 77);

						if (pDoz->recip[fid].real.cMmState &
							(1 << MMSTATE_INVALIDSHAREMEM))
							pArch->Shot(cid, (fid & 1) ? 82 : 78);
						else	pArch->Cut(cid, (fid & 1) ? 82 : 78);

						if (pDoz->recip[fid].real.cMmState &
							(1 << MMSTATE_INVALIDMAPPING))
							pArch->Shot(cid, (fid & 1) ? 83 : 79);
						else	pArch->Cut(cid, (fid & 1) ? 83 : 79);

						if (pDoz->recip[fid].real.cMmState &
							(1 << MMSTATE_INVALIDLOGAREA))
							pArch->Shot(cid, (fid & 1) ? 84 : 80);
						else	pArch->Cut(cid, (fid & 1) ? 74 : 80);
					}
					else	pArch->Shot(cid, 76);

					if (pDoz->recip[fid].real.cMmState &
						(1 << MMSTATE_INVALIDQUEUE))	pArch->Shot(cid, 85);
					else	pArch->Cut(cid, 85);

					if (pDoz->recip[fid].real.cMmState &
						(1 << MMSTATE_INVALIDSHAREMEM))	pArch->Shot(cid, 86);
					else	pArch->Cut(cid, 86);

					if (pDoz->recip[fid].real.cMmState &
						(1 << MMSTATE_INVALIDMAPPING))	pArch->Shot(cid, 87);
					else	pArch->Cut(cid, 87);

					if (pDoz->recip[fid].real.cMmState &
						(1 << MMSTATE_INVALIDLOGAREA))	pArch->Shot(cid, 88);
					else	pArch->Cut(cid, 88);
				}
			}
		}
		if (c_cmd.prevent.wDtb > 0) {
			-- c_cmd.prevent.wDtb;
			pDoz = pPaper->GetDozen();
			for (int fi = FID_HEADBK;
				fi < (int)pPaper->GetLength() + 2; fi ++) {
				int cid = pSch->F2C(pPaper->GetTenor(), fi);
				// 180911
				//if (pPaper->GetRouteState(cid)) {
				if (pPaper->GetRouteState(fi) && VALIDRECIP(fi)) {
					if (pDoz->recip[FID_HEAD].real.prol.cVersion !=
						pDoz->recip[fi].real.prol.cVersion)
						pArch->Shot(cid,
							(fi == FID_HEADBK || fi == FID_TAILBK) ? 14 : 13);
					else
						pArch->Cut(cid,
							(fi == FID_HEADBK || fi == FID_TAILBK) ? 14 : 13);

					if (fi < LENGTH_TU) {
				// ===== UNTAP 2 =============================================
				if ((pDoz->recip[FID_HEAD].real.prol.cCarLength & 0xf) !=
					(pDoz->recip[fi].real.prol.cCarLength & 0xf))
					pArch->Shot(cid,
							(fi == FID_HEADBK || fi == FID_TAILBK) ? 16 : 15);
				else
					pArch->Cut(cid,
							(fi == FID_HEADBK || fi == FID_TAILBK) ? 16 : 15);

				if ((pDoz->recip[FID_HEAD].real.prol.cPermNo !=
					pDoz->recip[fi].real.prol.cPermNo))
					pArch->Shot(cid,
							(fi == FID_HEADBK || fi == FID_TAILBK) ? 18 : 17);
				else
					pArch->Cut(cid,
							(fi == FID_HEADBK || fi == FID_TAILBK) ? 18 : 17);
				// ===== TAP 2 ===============================================
					}
				}
			}
		}
	}
	// === trouble detection end

	//c_dtbMon.cRxedBrc[0] = c_dtb[0].bRxedBrc ? 1 : 0;
	//c_dtbMon.cRxedBrc[1] = c_dtb[1].bRxedBrc ? 1 : 0;
	//c_dtbMon.cRxedCls[0] = c_dtb[0].bRxedCls ? 1 : 0;
	//c_dtbMon.cRxedCls[1] = c_dtb[1].bRxedCls ? 1 : 0;
	c_dtbMon.dwRoute = pPaper->GetRoute();
	pPaper->GetWakeupEach(&c_dtbMon.wWakeupEach[0]);
	//pBand->Region(15);
}

// * ECU 7 :
// 선택된 객차들에서 트레이스 데이터들을 취합한다.
bool CLand::CollectTrace(int ldi)
{
	WORD wRespDevMap = 0;
	WORD wNullDevMap = 0;
	GETPAPER(pPaper);
	PDOZEN pDoz = pPaper->GetDozen();
	GETSCHEME(pSch);
	GETARCHIVE(pArch);
	for (int n = 0; n < (int)pPaper->GetLength(); n ++) {
		if (c_trcInfo.wItem & (1 << n)) {
			int fid = pSch->C2F(pPaper->GetTenor(), n);
			if (pPaper->GetRouteState(fid)) {		// 180911, 전원 투입 후, 확인
	// ===== UNTAP 3 =========================================================
	if (pDoz->recip[fid].real.cst.cID == CSTID_TRACE &&
		(pDoz->recip[FID_HEAD].real.cst.roll.trc.req.wDevIDMap & (1 << n)) &&
		(pDoz->recip[fid].real.cst.roll.trc.ans.wDevIDMap & (1 << n))) {
		if (pDoz->recip[fid].real.cst.roll.trc.ans.wChapID <
			c_trcInfo.wChapterMax) {
			if (pDoz->recip[fid].real.cst.roll.trc.ans.wChapID ==
				c_trcInfo.wChapterID &&
				pDoz->recip[fid].real.cst.roll.trc.ans.wPageID ==
				c_trcInfo.wPageID) {
				wRespDevMap |= (1 << n);
				pArch->LocateTraceSphere(ldi, n, c_trcInfo.wChapterID,
										c_trcInfo.wPageID,
										pDoz->recip[fid].real.cst.roll.trc.c);
			}
			//else	id unmatch but contine tracing...
		}
		else	wNullDevMap |= (1 << n);		// close device
	}
	// ===== TAP 3 ===========================================================
			}
		}
	}

	if ((c_trcInfo.wItem & DEFAULT_CARBITMAP) ==
		(wRespDevMap | wNullDevMap)) {
		if (wRespDevMap != 0) {
			if (++ c_trcInfo.wPageID >= c_trcInfo.wPageMax) {
				c_trcInfo.wPageID = 0;
				if (++ c_trcInfo.wChapterID >= c_trcInfo.wChapterMax) {
					c_stage.SetDownloadMsg(45);
					memset(&c_trcInfo, 0, sizeof(TRACEINFO));
					pArch->SaveTraceSphere(ldi);
					return true;
				}
			}
		}
	}
	c_trcInfo.wRespDevMap = wRespDevMap;
	c_trcInfo.wNullDevMap = wNullDevMap;
	return false;
}

void CLand::CaptureLines()
{
	INCERT in;
	c_fio.GetInput(in);
	memcpy(&c_recip[POPI_WORK].real.inp[0], &in, sizeof(_OCTET));
	// get from tu's [1], cu's [0]
	GETPAPER(pPaper);
	if (ISTU()) {
		c_fio.GetInput(in, true);
		memcpy(&c_recip[POPI_WORK].real.inp[1], &in, sizeof(_OCTET));
		// get from tu's [0]
		// 16/09/05 위 줄에서 _OCTET을 _INCERT로 하는 버그가 있었는데
		// [0]이 0x3055로 들어오는 현상이 몇분마다 한번씩 발생하였다.
		// 매번 발생하여야 맞질않나 싶어 로그를 남기는데 추후 생각해보자...
		// 또한 슬레이브에서만 이런 현상이 나오는데 이 또한 마스터에서도 발생했어야 하는 문제이다...
	}
	c_recip[POPI_WORK].real.avs.dwPwm = c_fio.GetPwm();
	c_recip[POPI_WORK].real.avs.wTacho = c_smooth.tacho.wAvr;
	c_recip[POPI_WORK].real.avs.wCurve[ADCONVCH_BATTERY] =
							c_smooth.ads.a[ADCONVCH_BATTERY].wAvr;
	c_recip[POPI_WORK].real.avs.wCurve[ADCONVCH_PRESSURE] =
							c_smooth.ads.a[ADCONVCH_PRESSURE].wAvr;
	c_recip[POPI_WORK].real.avs.wCurve[ADCONVCH_OUTTEMP] =
							c_smooth.ads.a[ADCONVCH_OUTTEMP].wAvr;
	OUTCERT out;
	c_fio.GetOutput(out);
	memcpy(&c_recip[POPI_WORK].real.outp, &out, sizeof(_DUET));
}

void CLand::Reform()
{
	GETPAPER(pPaper);
	c_recip[POPI_WORK].real.prol.cProperID = c_vehicle.cProperID;
	c_recip[POPI_WORK].real.prol.cCarLength = c_vehicle.length.cCurrent;
	// 190909
	// CAR_FSELF() == FID_HEAD ?
	//				c_vehicle.length.cActive : c_vehicle.length.cPassive;
	GETVERSE(pVerse);
	c_recip[POPI_WORK].real.prol.cPermNo = pVerse->GetPermNo();
	c_recip[POPI_WORK].real.prol.cVersion = (BYTE)(TS_VERSION - 200);
	c_recip[POPI_WORK].real.prol.wOtrn = pVerse->GetCurOtrn();

	GETARCHIVE(pArch);
	if (ISTU()) {
		if (c_cmd.intro.cSeq == INTROSEQ_TIMEMATCH) {
			// 각자의 기동 시간을 가져온다. 이것을 recip에 싣는다.
			pArch->GetSysTimeToDbf(&c_cmd.logMatchTime);
			if (pPaper->GetLabFromRecip()) {
				TRACK("LAND:load log match %02d/%02d/%02d %02d:%02d:%02d\n",
						c_cmd.logMatchTime.t.year, c_cmd.logMatchTime.t.mon,
						c_cmd.logMatchTime.t.day, c_cmd.logMatchTime.t.hour,
						c_cmd.logMatchTime.t.min, c_cmd.logMatchTime.t.sec);
			}
			c_cmd.intro.cSeq = INTROSEQ_UNFOLD;
		}
		c_recip[POPI_WORK].real.logMatch.dw = c_cmd.logMatchTime.dw;
	}
	pArch->ReformC(&c_recip[POPI_WORK]);

	if (ISTU()) {
		c_recip[POPI_WORK].real.sign.ecmd.b.tdc = c_trcInfo.bClear;
		c_recip[POPI_WORK].real.sign.ecmd.b.clrvb =
											c_stage.GetV3fBreakerClearCmd();
		c_recip[POPI_WORK].real.cExtTemp = (signed char)c_vExtTemp;
		c_recip[POPI_WORK].real.sign.ecmd.b.relblk =
											c_cmd.user.b.releaseServiceBlk;
		DWORD dw = c_stage.GetDoorTestMap();
		c_recip[POPI_WORK].real.sign.wDoorTest = (WORD)(dw & 0xffff);
		// 200218
		c_recip[POPI_WORK].real.sign.pcmd.b.devEx = pPaper->GetDeviceEx();
		c_recip[POPI_WORK].real.sign.pcmd.a &= 0xe0;
		c_recip[POPI_WORK].real.sign.pcmd.a |= (BYTE)((dw >> 16) & 0x1f);
		// 171129
		// PIS CONTROL CODE #5 :
		if (c_stage.GetPisModeRequest()) {
			c_recip[POPI_WORK].real.motiv.lead.b.epaim =
												c_stage.GetDesiredPisMode();
			// PIS CONTROL CODE #6까지 real.motiv.lead.b.epreq를 유효하게 한다.
			c_cmd.user.b.pisRequest = true;
			c_stage.ClearPisModeRequest();
		}
		c_recip[POPI_WORK].real.motiv.lead.b.epreq = c_cmd.user.b.pisRequest;
		// 17/06/20
		BYTE oblige = pPaper->GetOblige();
		c_recip[POPI_WORK].real.sign.hcmdEx.b.main =
							(oblige & (1 << OBLIGE_MAIN)) ? true : false;
		c_recip[POPI_WORK].real.sign.hcmdEx.b.head =
							(oblige & (1 << OBLIGE_HEAD)) ? true : false;
		c_recip[POPI_WORK].real.sign.hcmdEx.b.active =
							(oblige & (1 << OBLIGE_ACTIVE)) ? true : false;
		c_recip[POPI_WORK].real.sign.hcmdEx.b.cab =
							(oblige & (1 << OBLIGE_CAB)) ? true : false;
		// 170914
		c_recip[POPI_WORK].real.sign.scmdEx.b.lab = pPaper->GetLaboratory();

		pArch->ReformT(&c_recip[POPI_WORK]);
		pVerse->Reform(&c_recip[POPI_WORK]);

		if ((CAR_FSELF() == FID_HEAD && c_trcInfo.wSeq == TRACESEQ_COLLECT) ||
			(CAR_FSELF() != FID_HEAD &&
			c_recip[POPI_HEAD].real.cst.cID == CSTID_TRACE)) {
			if (CAR_FSELF() == FID_HEAD) {
				// * TRACE 4 :
				// 데이터 모으기위해 각 객차로 명령을 송출한다.
				c_recip[POPI_WORK].real.cst.cID = CSTID_TRACE;
				c_recip[POPI_WORK].real.cst.roll.trc.req.wDevIDMap =
														c_trcInfo.wItem;
				c_recip[POPI_WORK].real.cst.roll.trc.req.wChapID =
														c_trcInfo.wChapterID;
				c_recip[POPI_WORK].real.cst.roll.trc.req.wPageID =
														c_trcInfo.wPageID;
			}
			//else	trace work of slave is done in CProse::Reform()
		}
		else if (ISTU()) {
			//if (c_recip[POPI_WORK].real.sign.gcmd.b.unifEr) {
			//	c_mill.nConsult = CSTID_ENVIRONS;
			//	if (!c_mill.bEnvConsult) {
			//		c_mill.bEnvConsult = true;
			//		if (pPaper->GetLabFromRecip())
			//			TRACK("LAND:enter env. set\n");
			//	}
			//}
			//else {
			//	if (c_mill.bEnvConsult) {
			//		c_mill.bEnvConsult = false;
			//		if (pPaper->GetLabFromRecip())
			//			TRACK("LAND:exit env. set\n");
			//	}
			//	if (c_recip[POPI_WORK].real.sign.gcmd.b.unifCr) {
			//		c_mill.nConsult = CSTID_CAREERS;
			//		if (!c_mill.bCrrConsult) {
			//			c_mill.bCrrConsult = true;
			//			if (pPaper->GetLabFromRecip())
			//				TRACK("LAND:enter crr. set\n");
			//		}
			//	}
			//	else {
			//		if (c_mill.bCrrConsult) {
			//			c_mill.bCrrConsult = false;
			//			if (pPaper->GetLabFromRecip())
			//				TRACK("LAND:exit crr. set\n");
			//		}
			//		c_mill.nConsult =
			//			c_mill.nConsult != CSTID_CAREERS ?
			//			CSTID_CAREERS : CSTID_ENVIRONS;
			//	}
			//}

			// 200218
			//if (CAR_FSELF() == FID_HEAD)
			//	c_mill.nConsult =
			//			c_mill.nConsult != CSTID_CAREERS ?
			//			CSTID_CAREERS : CSTID_ENVIRONS;
			if (CAR_FSELF() == FID_HEAD) {
				if (++ c_mill.nConsult > CSTID_BMSEX)
					c_mill.nConsult = CSTID_ENVIRONS;
			}
			else {
				if (c_recip[POPI_HEAD].real.cst.cID != CSTID_TRACE)
					c_mill.nConsult = c_recip[POPI_HEAD].real.cst.cID;
				else	c_mill.nConsult = CSTID_NON;
			}

			c_recip[POPI_WORK].real.cst.cID = c_mill.nConsult;
			switch (c_mill.nConsult) {
			case CSTID_ENVIRONS :
				{
					PENVIRONS pEnv = pArch->GetEnvirons();
					memcpy(c_recip[POPI_WORK].real.cst.roll.ecm.env.c,
											pEnv->c, sizeof(ENVIRONS));
					PCMJOGB pCmj = pArch->GetCmjog();
					memcpy(c_recip[POPI_WORK].real.cst.roll.ecm.cmj.c,
											pCmj->c, sizeof(CMJOGB));
					PV3FBRKCNTF pVbf = pArch->GetV3fBreaker();
					memcpy(c_recip[POPI_WORK].real.cst.roll.ecm.bcu.c,
											pVbf->u.c, sizeof(V3FBRKCNTU));
					// 210806
					if (pPaper->GetDeviceExFromRecip()) {
						GETPROSE(pProse);
						CProse::PLSHARE pLsv = pProse->GetShareVars();
						c_recip[POPI_WORK].real.cst.roll.ecm.wFire =
											pLsv->fire.wDefectMap & 0x3f;
					}
				}
				break;
			case CSTID_CAREERS :
				{
					PCAREERS pCrr = pArch->GetCareers();
					memcpy(&c_recip[POPI_WORK].real.cst.roll.crr,
											pCrr, sizeof(CAREERS));
				}
				break;
			// 200218
			case CSTID_BMSEX :
				{
					GETVERSE(pVerse);
					PBMSAINFO pBmsa =
						(PBMSAINFO)pVerse->GetInfo(true, PDEVID_BMS, LI_WORK);
					memcpy(&c_recip[POPI_WORK].real.cst.roll.bcv,
											&pBmsa->t.se.v, sizeof(BMSCVS));
				}
				break;
			default :
				//{
				//	PENVIRONS pEnv = pArch->GetEnvirons();
				//	for (int n = 0; n < LENGTH_WHEELDIA; n ++)
				//		c_recip[POPI_WORK].real.cst.roll.tds.cDiai[n] =
				//									pEnv->real.cWheelDiai[n];
				//	for (int n = 0; n < CID_MAX; n ++)
				//		c_recip[POPI_WORK].real.cst.roll.tds.cTempSv[n] =
				//									pEnv->real.cTempSv[n];
				//	c_recip[POPI_WORK].real.cst.cID = CSTID_TEMPDIA;
				//}
				break;
			}
		}
	}

	GETPROSE(pProse);
	pProse->Reform(&c_recip[POPI_WORK], CAR_FSELF() == FID_HEAD ?
								&c_recip[POPI_WORK] : &c_recip[POPI_HEAD]);

	// 170809
	c_recip[POPI_WORK].real.cMmVer = (BYTE)(pArch->GetMmVersion() - 200);
	memcpy(&c_recip[POPI_SEND], &c_recip[POPI_WORK], sizeof(RECIP));
	// 180625
	if (c_wFpgaVersion[0] == c_wFpgaVersion[1] &&
		c_wFpgaVersion[1] == c_wFpgaVersion[2])
		c_recip[POPI_WORK].real.wFpgaVersion = c_wFpgaVersion[0];
	else
		c_recip[POPI_WORK].real.wFpgaVersion = 0xfedb;
}

bool CLand::PrologConfirm(PPROLOG pProl, DWORD dwOrder)
{
	// 190909
	if (VALID_DTBADDR(pProl->wAddr, c_vehicle.length.cCurrent) &&
		(pProl->dwOrder == dwOrder))
		return true;
	return false;
}

bool CLand::CheckCarLength(BYTE cCarLength)
{
	if (c_vehicle.length.stab.Stabilize(cCarLength)) {
#if	defined(DEBUGDTB_STATETEXT)
		TRACK("LAND:capture car length.(%d)\n", c_vehicle.length.cPassive);
#endif
		return true;
	}
	return false;
}

bool CLand::AlignLocal(BYTE cCarLength)
{
	GETSCHEME(pSch);
	pSch->Drawup(cCarLength);
	int cid = pSch->F2C(false, (int)c_vehicle.cProperID);
	WORD wDevMap = pSch->GetLDeviceMap(cid);
	if (wDevMap == 0) {
		TRACK("LAND>ERR:UID#%d have been no lots!\n", c_vehicle.cProperID);
		return false;
	}

	if (c_pLoc == NULL) {
		TRACK("LAND>ERR:align local failure!\n");
		return false;
	}
	if (!c_pLoc->ReSetup(wDevMap)) {
		KILL(c_pLoc);
		c_fio.SetText(4, (PSZ)"LOCE");
		TRACK("LAND>ERR:UID#%d can not initial local port!\n",
													c_vehicle.cProperID);
		return false;
	}
	c_fio.SetText(1, (PSZ)"C");
	return true;
}

void CLand::DtbControl()
{
	GETPAPER(pPaper);
	if (ISTU() && (pPaper->GetOblige() & (1 << OBLIGE_HEAD))) {
		PDOZEN pDoz = pPaper->GetDozen();
		GETREVIEW(pRev);
		WORD wItem = pRev->GetInspectItem();
		WORD wStep = pRev->GetInspectStep();
		if ((pDoz->recip[FID_HEAD].real.drift.cNbrk &
			(1 << NDETECT_ZEROSPEED)) && wItem != 0 &&
			wStep >= INSPSTEP_RESET && wStep <= INSPSTEP_DTBJUDGE) {
			GETARCHIVE(pArch);
			PINSPREPORT pInsp = pArch->GetInspectReport();
			CReview::PLFCOUNT pLfc = pRev->GetLineFaultCount();
			int nPage = 0;
			switch (wStep) {
	// ===== UNTAP 2 =========================================================
	case INSPSTEP_RESET : {
			GETARCHIVE(pArch);
			pArch->PreInspect(wItem);		//pRev->GetInspectItem());
			c_stage.InspectReset(wItem);	//pRev->GetInspectItem());
			GETPROSE(pProse);
			pProse->SetInspectCancel(false);
			nPage = pRev->NextInspectStep();
		}
		break;
	case INSPSTEP_DTBREADY : {
			pInsp->ics.wTcRes = 0xfd0f;
			// 1101 1101 0000 1101 "시험 진행 중"
			pInsp->ics.wTul = 0xff00;		// "시험"
			// 201029
			//pInsp->ics.wCul = 0xffff;		// "시험"
			switch (pPaper->GetLength()) {
			case 4 :	pInsp->ics.wCul = XWORD(0x3c0);		break;
			case 6 :	pInsp->ics.wCul = XWORD(0xff0);		break;
			case 8 :	pInsp->ics.wCul = XWORD(0x3ff0);	break;
			default :	pInsp->ics.wCul = 0xffff;			break;
			}
			for (int n = 0; n < FID_MAX; n ++)	pLfc->wDtb[n] = 0;
			// 19/12/05
			pInsp->wMsg = 0;
			pRev->NextInspectStep();
		}
		break;
	case INSPSTEP_DTBCHECK :
		// 19/12/05
		if (!VALIDMRPS()) {
			pRev->InspectPause();
			pInsp->wMsg = XWORD(52);
			// "주 공기압 저하로 시험 중단. 확인후 재시작을 누르십시오."
		}
		else if (pRev->GetInspectLapse()) {
		//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0) {
			pRev->NextInspectStep();
			// 미리 스텝을 바꿔놓으면 카운터 값이 바뀌지 않는다!!!!
			for (int pid = 0; pid < LENGTH_TU; pid ++) {
				pRev->CoordDuo(&pInsp->ics.wTul,
						pLfc->wDtb[pid] > INSPECTNGLIMIT_DTB ? 2 : 1,
						pid * 2 + 8);	// 1:OK/2:NG
			}
			int hofs = pPaper->GetHeadCarOffset();
			for (int cid = 0;
				cid < (int)pPaper->GetLength() - 2; cid ++)
				pRev->CoordDuo(&pInsp->ics.wCul,
					pLfc->wDtb[cid + FID_PAS] > INSPECTNGLIMIT_DTB ? 2 : 1,
					((cid + hofs) ^ 4) * 2);

			pRev->CoordDuo(&pInsp->ics.wTcRes, 2, 8);			// end
			if (((pInsp->ics.wTul & 0xaaaa) | (pInsp->ics.wCul & 0xaaaa)) !=
				0) {
				pRev->CoordDuo(&pInsp->ics.wTcRes, 2, 10);	// 1:OK/2:NG
				pRev->InspectPause();
			}
			else	pRev->CoordDuo(&pInsp->ics.wTcRes, 1, 10);
		}
		break;
	case INSPSTEP_DTBJUDGE :
		if (pRev->GetInspectLapse())	nPage = pRev->NextInspectStep();
		//if (pDoz->recip[FID_HEAD].real.cst.roll.insp.wTime == 0)
		//	nPage = pRev->NextInspectStep();
		break;
	default :	break;
	// ===== TAP 2 ===========================================================
			}
			if (nPage > 0)	RegisterDuPage((WORD)nPage);
		}
	}
}

void CLand::TurnObserve()
{
	c_mtCvt.Lock();
	c_bColdWakeup = false;
	c_bControlInitialed = false;
	c_otr.wSkipTimer = TIME_OTRSKIP;
	memset(&c_convention, 0, sizeof(CONVENTION));
	GETPAPER(pPaper);
	BYTE uOblige = pPaper->GetOblige();
	uOblige &= ~(1 << OBLIGE_HEAD);
	ModifyDtb(uOblige);
	c_cmd.state.b.dtbActive = false;
	c_convention.wSeq = DTBISEQ_WAITEMPTY;
	c_convention.wWaitTimer = TIME_DTBWAITEMPTY;
	// 이 시간은 DTB가 비어있기를 기다리는 시간이다.
	TRACK("LAND:wait empty of dtb %d\n", c_convention.wWaitTimer);

	// 여기서 mm이 기초 파일을 읽어들이도록 한다.
	c_cmd.state.b.waitRequiredDoc = true;
	c_cmd.wWaitRequireDoc = TPERIOD_WAITREQUIREDOC;

	GETARCHIVE(pArch);
	PBULKPACK pBulk = pArch->GetBulk();
	pBulk->wState &= ~(1 << BULKSTATE_CULLREQUIREDDOC);
	//c_ctrlSide.a = 0xffff;
	pArch->CullRequiredDoc();
	// Negotiate()에서 BULKSTATE_CULLREQUIREDDOC를 기다린다.
	TRACK("LAND:TurnObserve() End\n");
	c_mtCvt.Unlock();
}

// * TRACE 3 :
// 데이터 모으기를 시작한다.
// c_trcInfo.wSeq = TRACESEQ_COLLECT;
// c_trcInfo.wItem = (1 << TRACEITEM_ALIGN) | (1 << each ecu);
// c_trcInfo.wTotal = (WORD)(dwMax / SIZE_TRACECHAR);
// c_trcInfo.wWaitArrangeTimer = TIME_WAITTRACEARRANGE;
bool CLand::LeadTraceSphere(int ldi, WORD* pwTraceLengthFromCID)
{
	GETARCHIVE(pArch);
	pArch->ClearTraceSphere(ldi);
	memset(&c_trcInfo, 0, sizeof(TRACEINFO));

	int nItem = 0;
	switch (ldi) {
	case DEVID_SIV :
		// 190909 sivsda has extended length, but no trace
		//{
		nItem = TRACEITEM_SIV;
		//	GETPROSE(pProse);
		//	if (pProse->GetSivType() == 1)
		//		c_trcInfo.wPageMax = SIVTRACEPAGE_MAX;
		//	else
		c_trcInfo.wPageMax = SIVTRACEPAGE_MAX;
		// 180730,	SIVTRACEPAGEEX_MAX;
		//}
		break;
	case DEVID_V3F :
		nItem = TRACEITEM_V3F;
		c_trcInfo.wPageMax = V3FTRACEPAGE_MAX;
		break;
	case DEVID_ECU :
		nItem = TRACEITEM_ALIGN;
		c_trcInfo.wPageMax = ECUTRACEPAGE_MAX;
		break;
	default	:	return false;	break;
	}

	WORD wMax = 0;
	GETPAPER(pPaper);
	for (int n = 0; n < pPaper->GetLength(); n ++) {
		if (pwTraceLengthFromCID != NULL) {
			if (*pwTraceLengthFromCID > 0) {
				c_trcInfo.wItem |= (1 << n);
				if (*pwTraceLengthFromCID > wMax)
					wMax = *pwTraceLengthFromCID;
				pArch->SetTraceLengthSphere(ldi, n, *pwTraceLengthFromCID);
			}
			++ pwTraceLengthFromCID;
		}
	}
	if (wMax > 0) {
		c_trcInfo.wChapterMax = wMax;
		c_trcInfo.wChapterID = c_trcInfo.wPageID = 0;
		if (ldi == DEVID_ECU)
			c_trcInfo.wWaitArrangeTimer = TIME_WAITTRACEARRANGE;
		c_trcInfo.wItem |= (1 << nItem);
		c_trcInfo.wSeq = TRACESEQ_COLLECT;
		return true;
	}
	return false;
}

void CLand::SuspendTraceSphere()
{
	memset(&c_trcInfo, 0, sizeof(TRACEINFO));
	c_stage.SetDownloadMsg(47);			// "받기를 중단하였습니다"
}

void CLand::ReleaseServiceBlock()
{
	if (c_cmd.user.b.releaseServiceBlk)	return;

	GETARCHIVE(pArch);
	GETPAPER(pPaper);
	pArch->Shot(CAR_PSELF(), 182);
	c_cmd.user.b.releaseServiceBlk = true;
}

// 200218
void CLand::ClearAllRed()
{
	GETPAPER(pPaper);
	pPaper->ClearAllRed();
	GETVERSE(pVerse);
	pVerse->ClearAllRed();
	GETPROSE(pProse);
	pProse->ClearAllRed();
}

// call by CStick or boot
void CLand::InitialAll(int iFrom)
{
	Initial();
	c_fio.SetOutput(UDOB_DISTP, false);

	GETSCHEME(pSch);
	pSch->Initial();
	GETARCHIVE(pArch);
	pArch->Initial(iFrom);

	if (iFrom != INTROFROM_OTRN) {
		GETPAPER(pPaper);
		pPaper->Initial();		// CYCLE_DTBWAKEUP, CYCLE_EACHWAKEUP
		GETVERSE(pVerse);
		pVerse->Initial();		// TIME_pdevWARMUP
		GETPROSE(pProse);
		pProse->Initial();		// TIME_ldevWARMUP
		if (c_pPol != NULL)	c_pPol->Initial();
		if (c_pLoc != NULL)	c_pLoc->Initial();

		c_turf.Initial();
		c_stick.Initial();
		// TPERIOD_WARMUPB -> active로 되면서 TPERIOD_WARMUPA로 치환되는데...

		c_steer.Initial();
		c_lide.Initial();
		c_pel.Initial();
		c_univ.Initial();
	}
	c_stage.Initial(iFrom);
	TRACK("LAND:Initial all %d.\n", iFrom);
}

void CLand::Roll(char* p, const char* fmt, ...)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	sprintf(p, "%02d:%02d:%02d:%03d-",
			pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec / 1000);

	va_list vl;
	va_start(vl, fmt);
	p += 13;
	vsprintf(p, fmt, vl);
	va_end(vl);
}

void CLand::TakeReceivePole(DWORD dw)
{
	WORD wID = LOWWORD(dw);
	switch (wID) {
	case PDEVID_ATO :	break;
	case PDEVID_HDU :
		c_stage.ReceiveUserRequest((BYTE)HIGHWORD(dw));
		break;
	case PDEVID_RTD :	break;
	default :	break;
	}
}

void CLand::TakeReceiveLocal(DWORD dw)
{
}

void CLand::TakeSendBus(DWORD dw)
{
	if (c_convention.wSeq != DTBISEQ_NON)
		++ c_convention.wBroadcastReceived;
	// 170822, 마스터에 응답하지 않는다.
	if (c_convention.wSeq == DTBISEQ_WAITEMPTY)	return;

	GETFSC(pFsc);
	GETPAPER(pPaper);
	if (CAR_FSELF() == FID_HEAD) {
		pFsc->Pile("LAND>ERR:TakeSendBus() UNREACHABLE LINE A!(0x%04x)\n", dw);
		return;
	}

	BYTE id = dw ? 1 : 0;
	if (c_cmd.state.b.dtbActive || ISCU()) {
		// 181012
		//Reform();
		c_dtb[id].state.b.ack = true;
		if (!c_dtb[0].state.b.ack || !c_dtb[1].state.b.ack)	Reform();
		c_recip[POPI_SEND].real.prol.cCarLength &= 0xf;
		c_recip[POPI_SEND].real.prol.cCarLength |= id ? 0x80 : 0x40;
		c_dtb[id].p->SendBus((PVOID)&c_recip[POPI_SEND],
												c_order.dwPassive[id]);
	}
	// 190110
	//c_cmd.state.b.alone = false;
	c_bAlone = false;
}

void CLand::TakeReceiveBus(DWORD dw)
{
	WORD recf = LOWWORD(dw);
	BYTE id = HIGHWORD(dw) ? 1 : 0;
	GETPAPER(pPaper);
	GETFSC(pFsc);
	if (recf == DTB_RBROADCAST) {
		//c_wCentiPop = 0;
		if (CAR_FSELF() == FID_HEAD) {		// !!!!! meaningless
			pFsc->Pile("LAND>ERR:TakeReceiveBus()"
						"UNREACHABLE LINE A!(0x%04x)\n", dw);
			return;
		}
		GETBAND(pBand);
		pBand->Region(id ? 2 : 1);

		c_bcMon.wCounter = 0;
		c_bcMon.wWaiter[id] = TPERIOD_WAITBROADCASTF;
		if (c_order.dwPassive[id] != c_dtb[id].p->GetRecvOrder()) {
			// 새로운 프레임
			//c_dtb[id].bRxedBrc = true;		// 슬레이브 버스 사이클 시작
			c_dtb[id].state.b.open = true;
			c_order.dwActive = c_order.dwPassive[id] =
										c_dtb[id].p->GetRecvOrder();
			c_bcMon.wTimer = TPERIOD_WAITBROADCASTF;

			c_dtb[id].p->Demount((BYTE*)&c_recip[POPI_HEAD], FID_HEAD);
			// Car Length만 가져올 수는 없을까?

			// just match slaves
			//190813, to six car
			//if (CheckCarLength(c_recip[POPI_HEAD].real.prol.cCarLength &
			//	0xf)) {
			//	c_vehicle.length.cPassive = c_vehicle.length.stab.cur;
			//	pPaper->SetLength(c_vehicle.length.cPassive);
			//	AlignLocal(c_vehicle.length.cPassive);
			//}
			// 190909
			// following line is executed only once
			// because the initial input value must be changed again to
			//		Stabilize() has true
			if (CheckCarLength(
				c_recip[POPI_HEAD].real.prol.cCarLength & 0xf)) {
				// 190909
				//c_vehicle.length.cPassive = c_vehicle.length.stab.cur;
				//AlignLocal(c_vehicle.length.cPassive);
				//c_dtb[0].p->SetCarLength(c_vehicle.length.cPassive);
				//c_dtb[1].p->SetCarLength(c_vehicle.length.cPassive);
				//pPaper->SetLength(c_vehicle.length.cPassive);
				//if (pPaper->GetLabFromRecip())
				//	TRACK("LAND:slave match car length %d %d\n",
				//		c_vehicle.length.cActive, c_vehicle.length.cPassive);
				c_vehicle.length.cCurrent = c_vehicle.length.stab.cur;
				pPaper->SetLength(c_vehicle.length.cCurrent);
				AlignLocal(c_vehicle.length.cCurrent);
				c_dtb[0].p->SetCarLength(c_vehicle.length.cCurrent);
				c_dtb[1].p->SetCarLength(c_vehicle.length.cCurrent);
				if (pPaper->GetLabFromRecip())
					TRACK("LAND:slave match car length %d\n",
										c_vehicle.length.cCurrent);
			}
			CaptureLines();			// 현재 입력 -> c_recip[1], 슬레이브는 브로드캐스트를 받고...
									// 마스터와 슬레이브가 읽는 시점을 접근시키기 위한 자리...
			//Roll(c_cMsg0, "BRC %04X %02X %08X\n",
			//		recip.real.prol.wAddr, recip.real.prol.cProperID,
			//		recip.real.prol.dwOrder);
		}
		else	c_dtb[id].p->KillTrig();

		if (c_convention.wSeq == DTBISEQ_WAITEMPTY) {
			++ c_convention.wWaitTimer;	// = TIME_DTBWAITEMPTY;
			TRACK("LAND:br at wait\n");
		}
	}
	else if (recf == DTB_RFULL || recf == DTBF_RTIMEOUT) {
		// CU는 timeout에 의해 버스 사이클을 끝낸다.
		//if (c_dtb[id].bRxedBrc) {
		if (c_dtb[id].state.b.open) {
			//c_dtb[id].bRxedCls = true;
			c_dtb[id].state.b.close = true;
			//if (c_dtb[0].bRxedBrc == c_dtb[0].bRxedCls &&
			//	c_dtb[1].bRxedBrc == c_dtb[1].bRxedCls) {
			if (c_dtb[0].state.b.open == c_dtb[0].state.b.close &&
				c_dtb[1].state.b.open == c_dtb[1].state.b.close) {
				Supervise();
				//c_dtb[0].bRxedBrc = c_dtb[0].bRxedCls =
				//c_dtb[1].bRxedBrc = c_dtb[1].bRxedCls = false;
				c_dtb[0].state.a = c_dtb[1].state.a = 0;
			}
			else {
				// 다른 버스가 브로드캐스트를 수신했고 마감이 되지 않았으므로 기다린다.
			}
		}
		else {
			// 170817
			// Supervise가 제시간에 실행되지 않는 문제가 있어 검사해봤더니
			// 아마 큐에서 메시지를 미루다가 연속으로 나오는 현상같은데...
		}
	}
	else {
		//INCWORD(c_wXivRed[id][DTBERRF(recf)]);
	}
}

#if	defined(_DIRECT_CALL_)
void CLand::SendDtbBus(DWORD dw)
{
	TakeSendBus(dw);
}

void CLand::ReceiveDtbBus(DWORD dw)
{
	TakeReceiveBus(dw);
}
#endif

void CLand::TakeSaveEcuTrace(DWORD dw)
{
}

BEGIN_TAGTABLE(CLand, CPump)
	_TAGMAP(TAG_RECEIVEBUS, CLand::TakeReceiveBus)
	_TAGMAP(TAG_SENDBUS, CLand::TakeSendBus)
	_TAGMAP(TAG_RECEIVEPOLE, CLand::TakeReceivePole)
	_TAGMAP(TAG_RECEIVELOCAL, CLand::TakeReceiveLocal)
	_TAGMAP(TAG_SAVEECUTRACE, CLand::TakeSaveEcuTrace)
END_TAGTABLE();

bool CLand::Launch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("ts begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
						pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday,
						pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

	TRACK("ts ver. %.2f %s %s\n",
			(double)TS_VERSION / 100.f, __DATE__, __TIME__);

	// open i/o
	if (!c_fio.Initial()) {
		c_fio.SetText(4, (PSZ)"IOBF");	// io board initial fail
		TRACK("LAND>ERR:peripheral io configuration failure!\n");
		return false;
	}
	c_fio.SetText(4, (PSZ)".   ");
	c_fio.ShowCounters();

	WORD wCuLifeCycle = 0;
#if	defined(_ENWATCHDOG_)
	if (c_fio.GetTwin()) {		// TU
		c_sysChk.cPast = (BYTE)c_fio.GetGeneralReg();
		if (c_sysChk.cPast != 0)
			c_sysChk.wIntroTimer = TIME_SYSCHKINTRO;
		WORD w = GetStablePairCondition(false);
		TRACK("LAND:REASON 0x%02X\n", c_sysChk.cPast);
		TRACK("LAND:pair 0x%04X\n", w);
	}
	else {
		WORD w = c_fio.GetGeneralReg();
		if (w > 10)	w = 1;
		else if (w < 10)	++ w;
		wCuLifeCycle = w - 1;
		c_fio.SetGeneralReg(w);
	}
#endif

	GETFSC(pFsc);
	if (!pFsc->Initial(c_fio.GetSpace())) {
	//if (!pFsc->Initial()) {
		c_fio.SetText(4, (PSZ)"CLBF");
		// communication board initial fail
		TRACK("LAND>ERR:peripheral port initial failure!\n");
		return false;
	}
	pFsc->SetFio((PVOID)&c_fio);

	for (int n = 0; n < 3; n ++)
		c_wFpgaVersion[n] = pFsc->GetVersion(n);

	c_fio.SetText(4, (PSZ)"..  ");
	// 170808
	//BYTE ios = c_fio.GetDioReg();
	//if ((ios & ((1 << CFio::IOB_OUTP) | (1 << CFio::IOB_INPA))) !=
	//		((1 << CFio::IOB_OUTP) | (1 << CFio::IOB_INPA))) {
	//	TRACK("LAND>ERR:vme-io installation failure!(0x%02X)\n", ios);
	//	return false;
	//}

	//c_fio.SetText(4, PSZ("... "));

	BYTE buf[8];
	GETPAPER(pPaper);
	bool bTwin = c_fio.GetTwin();
	WORD w = c_fio.GetBorn(0);		// TUDIB[63..48]
	if (bTwin) {					// tu
		w >>= 4;					// TUDIB_HIGH, TUDIB_LOW,
									// TUDIB_CL02, TUDIB_CL01,
									// TUDIB_TCP, TUDIB_CT04,
									// TUDIB_CT02, TUDIB_CT01
		if ((w & 0xc0) != 0x80) {	// TUDIB_HIGH, TUDIB_LOW
			c_fio.SetText(4, (PSZ)"TIDF");
			TRACK("LAND>ERR:unknown car!(0x%04X)\n", w);
			return false;
		}
		// 17/07/07
		if ((w & 3) == 1) {			// (TUDIB_CT02, TUDIB_CT01) = (01)
			pPaper->SetLaboratory(true);
			c_fio.SetLaboratory(true);
		}
		// 200218
		if (w & 4)	pPaper->SetDeviceEx(true);	// TUDIB_CT04

		c_vehicle.cProperID = w & 8 ? 1 : 0;	// TUDIB_TCP
		//c_cProperID = w & 8 ? 1 : 0;	// TUDIB_TCP
		w >>= 4;						// TUDIB_HIGH, TUDIB_LOW,
										// TUDIB_CL02, TUDIB_CL01
		switch (w & 3) {				// TUDIB_CL02, TUDIB_CL01
		case 0 :	c_vehicle.length.cCurrent = 10;	break;
		// 190909 cActive = 10;	break;
		case 1 :	c_vehicle.length.cCurrent = 8;	break;
		// 190909 cActive = 8;	break;
		case 2 :	c_vehicle.length.cCurrent = 6;	break;
		// 190909 cActive = 6;	break;
		default :	c_vehicle.length.cCurrent = 4;	break;
		// 190909 cActive = 4;	break;
		}
		// 200218
		if (c_vehicle.length.cCurrent == 6)	pPaper->SetDeviceEx(true);

		//	* for OBLIGE
		//						OBLIGE_HEAD		OBLIGE_MAIN		c_uOblige
		//	LEFT	MAIN		1				1				3
		//			RESERVE		1				0				2
		//	RIGHT	MAIN		0				1				1
		//			RESERVE		0				1				0
		BYTE cOblige = (1 << OBLIGE_CAB);
		if (c_vehicle.cProperID == PID_LEFT)
			cOblige |= (1 << OBLIGE_HEAD);	// 고유 번호를 설정한다.
		if (c_fio.GetMain())	cOblige |= (1 << OBLIGE_MAIN);
		// LIU2는 백플레인 점퍼를 통해 설정된다.
		c_vehicle.cProperID = ~cOblige &  OBLIGEBF_ONLYPOS;
		// c_cProperID는 변하지 않는다.

		cOblige &= ~(1 << OBLIGE_HEAD);		// 17/03/15. 마스터를 정하지 않는다.
		pPaper->Setup(cOblige, c_vehicle.length.cCurrent);
		// 190909 cActive);				// 모두 슬레이브로 설정한다.
		// 200218
		//BYTE cCID = c_vehicle.cProperID < PID_RIGHT ? 0 : 9;
		BYTE cCID = c_vehicle.cProperID < PID_RIGHT ?
								0 : c_vehicle.length.cCurrent - 1;
		pPaper->SetCarID(cCID);
		w = c_fio.GetBorn(1) & 0xff;
		w |= ((WORD)(cCID | 0x20) << 8);		// 2XYY, BCD, 0:20YY, 9:29XX
		c_vehicle.wPermNo = w;
		pPaper->SetPermNo(c_vehicle.wPermNo);
		sprintf((char*)buf, "Tc%02d", cCID);
		c_fio.SetText(0, (PSZ)buf);
		// 171027
		//c_fio.EnableOutput((cOblige & (1 << OBLIGE_MAIN)) ? true : false);

		GETVERSE(pVerse);
		pVerse->SetPermNo((BYTE)(c_fio.GetBorn(1) & 0xff));
		pVerse->SetCurOtrn(DEFAULT_OTRN);

		GETSCHEME(pSch);
		pSch->Drawup(c_vehicle.length.cCurrent);	// 190909 cActive);
	}
	else {								// cc, // CUDIB[63..48]
		// 171029
		if (w & 1)	c_fio.SetLaboratory(true);	// CUDIB_CT01(48)
		w >>= 4;						// CUDIB_CN08, CUDIB_CN04,
										// CUDIB_CN02, CUDIB_CN01 (55~52)
		if (w == 0 || w > 8) {
			c_fio.SetText(4, (PSZ)"CIDF");
			TRACK("LAND>ERR:invalid car id!(%02d)\n", w & 0xf);
			return false;
		}
		c_vehicle.cProperID = (BYTE)w + PID_PAS - 1;
		//PID_RIGHTBK;		// 4 ~
		delay(1000);

		//			0	1	2	3	4	5	6	7	8	9
		//	LIU1	0									2
		//	LIU2	1									3
		//	CU			4	5	6	7	8	9	10	11
		pPaper->Setup(0, 0);
		pPaper->SetFloatID(c_vehicle.cProperID);
		BYTE cCID = c_vehicle.cProperID - PID_PAS + 1;	//PID_RIGHTBK;	// 1 ~
		pPaper->SetCarID(cCID);
		sprintf((char*)buf, "Cc%02d", cCID);
		c_fio.SetText(0, (PSZ)buf);
		// 171027
		//c_fio.EnableOutput(true);
	}
	c_fio.SetText(4, (PSZ)"-   ");

	// open queue
	c_hReservoir = CreateReservoir((PSZ)MAINQ_STR);
	if (!Validity(c_hReservoir)) {
		c_fio.SetText(4, (PSZ)"QUEF");		// queue fail
		TRACK("LAND>ERR:can not create message queue!\n");
		return false;
	}

	// conform time
	GETARCHIVE(pArch);
	pArch->SetCarrier(this);
	if (!pArch->OpenAid()) {
		c_fio.SetText(4, (PSZ)"MMIF");		// mm(aid) fail
		TRACK("LAND>ERR:archive - aid!\n");
		return false;
	}
	pArch->Arteriam();						// 시간을 초기화한다.

	// dtb setup
	c_fio.SetText(4, (PSZ)"--  ");
	c_vehicle.wBusCycleTime =
			(c_vehicle.length.cCurrent + LENGTH_TU - 1) *
			DTBPERIOD_SECTION + DTBPERIOD_MARGIN + DTBPERIOD_CUMARGIN;
			// cycle time (2 node margin)
	//WORD wCycleTime = ISTU() ? c_wBusCycleTime : 0;
	// 여기서 CU의 사이클 시간을 0으로 하면 CPort에서 리 트리거를 하지않는다!!!
	c_bcMon.wTimer = TPERIOD_WAITBROADCASTF;
	c_bcMon.wWaiter[0] = c_bcMon.wWaiter[1] = TPERIOD_WAITBROADCASTF;

	//pFsc->TimerStop(false);
	//pFsc->TimerStop(true);
	if (c_dtb[0].p != NULL) {
		c_dtb[0].p->CloseBus();
		delete c_dtb[0].p;
	}
	//c_dtb[0].bRxedBrc = c_dtb[0].bRxedCls = false;
	c_dtb[0].state.a = 0;
	c_dtb[0].p = new CDtb(this, c_hReservoir, (PSZ)"DTBA");
	// at first, bus open default(10) car
	if (!c_dtb[0].p->OpenBus(false, (int)c_vehicle.cProperID,
						(int)CAR_FSELF(), (int)c_vehicle.length.cCurrent,
						c_vehicle.wBusCycleTime)) {	//wCycleTime)) {
		c_fio.SetText(4, (PSZ)"DTAF");			// dtb a fail
		TRACK("LAND>ERR:can not open alpha bus!\n");
		return false;
	}

	if (c_dtb[1].p != NULL) {
		c_dtb[1].p->CloseBus();
		delete c_dtb[1].p;
	}
	//c_dtb[1].bRxedBrc = c_dtb[1].bRxedCls = false;
	c_dtb[1].state.a = 0;
	c_dtb[1].p = new CDtb(this, c_hReservoir, (PSZ)"DTBB");
	if (!c_dtb[1].p->OpenBus(true, (int)c_vehicle.cProperID,
						(int)CAR_FSELF(), (int)c_vehicle.length.cCurrent,
						c_vehicle.wBusCycleTime)) {	//wCycleTime)) {
		c_fio.SetText(4, (PSZ)"DTBF");
		TRACK("LAND>ERR:can not open beta bus!\n");
		return false;
	}

	if (bTwin) {
		c_convention.wSeq = DTBISEQ_DEFER;
		c_convention.wWaitTimer =
							(c_vehicle.cProperID + 1) * REALBYSPREAD(1000);
							// HCR을 감지하기 전 대기 시간... ID 마다 1초 차이...
		// 17/07/06, Modify
		if (c_vehicle.cProperID == 1 || c_vehicle.cProperID == 3)
			c_convention.wWaitTimer += (REALBYSPREAD(2000));
		//c_wWakeupCmdList = (1 << WAKEUPCMD_LOGMATCHTIME);
		// 전원 투입에서는 mm이 close current dir.을 하므로 ts가 따로 할 필요는 없다.
		TRACK("LAND:observe time %d\n", c_convention.wWaitTimer);
	}

	c_fio.SetText(4, (PSZ)"--- ");
	KILL(c_pPol);
	if (bTwin) {
		c_pPol = new CPole(this, c_hReservoir, (PSZ)"POLE");
		if (!c_pPol->Setup()) {
			c_fio.SetText(4, (PSZ)"POLF");		// pole fail
			TRACK("LAND>ERR:can not initial local port!\n");
			return false;
		}
	}

	c_fio.SetText(4, (PSZ)"----");

	KILL(c_pLoc);
	c_pLoc = new CLocal(this, c_hReservoir, (PSZ)"LOCAL");
	if (!c_pLoc->Setup()) {
		KILL(c_pLoc);
		c_fio.SetText(4, (PSZ)"LOCF");			// local fail
		TRACK("LAND>ERR:UID#%d can not initial local port!\n",
													c_vehicle.cProperID);
		return false;
	}

	c_fio.SetText(4, (PSZ)"...>");
	c_stick.SetParent(this);
	c_turf.SetParent(this);
	c_steer.SetParent(this);
	c_pel.SetParent(this);
	c_stage.SetParent(this);
	c_lide.SetParent(this);
	c_univ.SetParent(this);

	c_wick.SetParent(this);
	c_wick.SetOwnerName(LAND_NAME);
	if (!c_wick.Open())
		TRACK("LAND>WARNING:can not open interface to Server!\n");
	else	c_wick.SendContinue();

	if (pPaper->GetLaboratory()) {
		TRACK("DTBCONTENTS  %d\n", sizeof(DTBCONTENTS));
		TRACK("CONSULTANT   %d\n", sizeof(CONSULTANT));
		TRACK("ENVIRONS     %d\n", sizeof(ENVIRONS));
		TRACK("PROPENSEECM  %d\n", sizeof(PROPENSEECM));
		TRACK("CAREERS      %d\n", sizeof(CAREERS));
		TRACK("TRACESCROLL  %d\n", sizeof(TRACESCROLL));
		//TRACK("CAREERS      %d\n", sizeof(CAREERS));
		TRACK("BUNDLES      %d\n", sizeof(_BUNDLES));
		//TRACK("inp start %d end %d\n", offsetof(DTBCONTENTS, inp),
		//									offsetof(DTBCONTENTS, outp));
		//TRACK("TROUBLE      %d\n", sizeof(TROUBLEARCH));
		TRACK("ENV. VERSION %x\n", ENV_VERSION);
		TRACK("LOGCONTENTS  %d\n", sizeof(LOGCONTENTS));
		TRACK("LOGBOOK      %d\n", sizeof(LOGBOOK));
		TRACK("RTDBINFO     %d\n", sizeof(RTDBINFO));
		TRACK("RTDTINE      %d\n", sizeof(RTDTINE));
		TRACK("CDMACONTENTS %d\n", sizeof(CDMACONTENTS));
		TRACK("CDMAREPORT   %d\n", sizeof(CDMAREPORT));
		TRACK("FULLSDASET   %d\n", SIZE_FULLSDASET);		// 19/12/19
		TRACK("BULK         %d\n", sizeof(BULKPACK));
		TRACK("DEV  SR   SA\n");
		TRACK("ATO  %03d  %03d\n", SIZE_ATOSR, SIZE_ATOSA);
		TRACK("HDU  %03d  %03d\n", SIZE_HDUSR, SIZE_HDUSA);
		TRACK("PAU  %03d  %03d\n", SIZE_PAUSR, SIZE_PAUSA);
		TRACK("PIS  %03d  %03d\n", SIZE_PISSR, SIZE_PISSA);
		TRACK("TRS  %03d  %03d\n", SIZE_TRSSR, SIZE_TRSSA);
		// 200218
		TRACK("FDU  %03d  %03d  %03d\n", SIZE_FDUSR, SIZE_FDUSA, SIZE_FDUSAAB);
		TRACK("RTD  %03d  %03d\n", SIZE_RTDSB, SIZE_RTDSB);
		TRACK("SIV  %03d  %03d\n", SIZE_SIVSR, sizeof(SIVAINFOEX));
		TRACK("SIV2 %03d  %03d\n", SIZE_SIVSR, sizeof(SIVAINFO2));
		TRACK("V3F  %03d  %03d\n", SIZE_V3FSR, sizeof(V3FAINFO));
		TRACK("SIVT %03d  %03d\n", SIZE_SIVSR, sizeof(SIVAINFOTRCEX));
		TRACK("V3FT %03d  %03d\n", SIZE_V3FSR, sizeof(V3FAINFOTRC));
		TRACK("ECU  %03d  %03d\n", SIZE_ECUSR, SIZE_ECUSA);
		TRACK("HVAC %03d  %03d\n", SIZE_HVACSR, SIZE_HVACSA);
		TRACK("DCU  %03d  %03d\n", SIZE_DCUSR, SIZE_DCUSA);
		// 200218
		TRACK("CMSB %03d  %03d\n", SIZE_CMSBSR, SIZE_CMSBSA);
		TRACK("BMS  %03d  %03d  %03d\n", SIZE_BMSSR, SIZE_BMSSA, SIZE_BMSSAAB);
		TRACK("FDUS %03d  %03d\n", SIZE_FDUSR, SIZE_FDUSAAB);
		TRACK("FDU  %03d  %03d\n", SIZE_FDUSR, SIZE_FDUSA);
		TRACK("SVC  %03d\n", sizeof(SVCFLAP));
		c_stage.ConfirmSize();
	}
	//if (ISTU())	c_stick.SetDtbWaitTime();

	// open ignition
	//c_cStableCounter = COUNTER_STABLE;
	KILLR(c_pTuner);
	c_pTuner = new CTimer(this, (PSZ)"TUNE");
	if (c_pTuner->Initial(PRIORITY_PERIODTUNE,
					(CTimer::PTIMERFUNC)(&CLand::Arteriam),
					(CPump*)this, TIMERCODE_BASE, true) == 0) {
		if (!c_pTuner->Trigger(TPERIOD_BASE)) {
			c_fio.SetText(4, (PSZ)"TIMF");		// timer fail
			TRACK("LAND>ERR:timer creation failure!\n");
			return false;
		}
	}
	else {
		TRACK("LAND>ERR:timer initial failure!\n");
		return false;
	}

	// enable watchdog
	if (bTwin) {
		//MAKETUFNAME();
		sprintf((char*)c_cFace, "osv%d", c_vehicle.cProperID);
#if	defined(_ENWATCHDOG_)
		c_fio.SetWatchdogCycle(WAKEUPWATCHDOG_CYCLE);
		c_fio.EnableWatchdog(false, true);
		BYTE cOblige = pPaper->GetOblige();
		//c_wWakeupCounter = COUNT_WAKEUP;
		// 171027
		c_fio.EnableOutput((cOblige & (1 << OBLIGE_MAIN)) ? true : false);
#endif
	}
	else {
		sprintf((char*)c_cFace, "run%d", wCuLifeCycle);
#if	defined(_ENWATCHDOG_)
		c_fio.SetWatchdogCycle(NORMALWATCHDOG_CYCLE);
		c_fio.EnableWatchdog(false, false);
		// 171027
		c_fio.EnableOutput(true);
#endif
	}
	c_fio.SetText(4, (PSZ)c_cFace);

	TRACK("RUN ==============================\n");
	InitialAll(INTROFROM_KEY);		// boot initial
	pFsc->Enable(true);		// enable CFsc::Receive()
	//pArch->CloseCurrentPath();	//17/06/28, ignore

	c_cmd.state.b.launched = true;
	return true;
}

WORD CLand::GetStablePairCondition(bool bTrig)
{
	WORD w = c_fio.GetPairCondition();
	for (int n = 0; n < 10; n ++) {
		if (bTrig)	c_fio.TriggerWatchdog();
		delay(10);
		WORD v = c_fio.GetPairCondition();
		if (v != w) {
			n = 0;
			w = v;
		}
	}
	return w;
}

void CLand::LeaveReason(int reason)
{
	c_sysChk.cPast =
			(c_sysChk.cVmes & 0x7) | ((c_sysChk.cPairCtrl << 4) & 0x70);
	if (reason == 1)	c_sysChk.cPast |= 8;
	else if (reason == 2)	c_sysChk.cPast |= 0x80;
	c_fio.SetGeneralReg((WORD)c_sysChk.cPast);
	TRACK("LAND:LEAVE 0x%02X!\n", c_sysChk.cPast);
}

void CLand::LeaveOnly(BYTE reason)
{
	c_sysChk.cPast = reason;
	c_fio.SetGeneralReg((WORD)c_sysChk.cPast);
	TRACK("LAND:LEAVE ONLY 0x%02X!\n", c_sysChk.cPast);
}

void CLand::CopySysChk(PSYSTEMCHECK pSysChk)
{
	if (c_sysChk.wIntroTimer > 0)	-- c_sysChk.wIntroTimer;
	memcpy(pSysChk, &c_sysChk, sizeof(SYSTEMCHECK));
}

void CLand::SetAtoLinePrevent()
{
	GETVERSE(pVerse);
	pVerse->InitAtoWarmup(0);	// 180511
	pVerse->InitAtoWarmup(1);
	//pVerse->SetAtoWarmup();
}

void CLand::SetEcuBcfPrevent()
{
	c_cmd.prevent.wEcuBcf = TPERIOD_ECUBCFPREVENT;
	//TRACK("LABD:begin ecu bcf prevent.\n");
}

void CLand::StirTimeMatch(bool byRtd)
{
	c_turf.StirTimeMatch(byRtd);
}

int CLand::Bale(bool bType, BYTE* pMesh)
{
	int leng = 0;
	if (bType) {
		sprintf((char*)pMesh, "BAS,6,");
		while (*pMesh != '\0')	{ ++ pMesh; ++ leng; }
	}
	else {
		GETPAPER(pPaper);
		*pMesh ++ = CAR_FSELF();
		*pMesh ++ = pPaper->GetOblige();
		DWORD dw = pPaper->GetRoute();
		memcpy(pMesh, &dw, sizeof(DWORD));
		pMesh += sizeof(DWORD);
		leng = sizeof(DWORD) + 2;

	}
	int i = Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	GETSCHEME(pSch);
	i = pSch->Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	i = c_turf.Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	i = c_stick.Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	i = c_steer.Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	//i = c_lide.Contain(bType, pMesh);
	//leng += i;
	//pMesh += i;
	GETVERSE(pVerse);
	i = pVerse->Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	GETPROSE(pProse);
	i = pProse->Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	i = c_stage.Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	i = c_pel.Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	i = c_univ.Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	GETREVIEW(pRev);
	i = pRev->Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	if (c_dtb[0].p != NULL) {
		i = c_dtb[0].p->Contain(bType, pMesh);
		leng += i;
		pMesh += i;
	}
	if (c_dtb[1].p != NULL) {
		i = c_dtb[1].p->Contain(bType, pMesh);
		leng += i;
		pMesh += i;
	}
	GETARCHIVE(pArch);
	i = pArch->Contain(bType, pMesh);
	leng += i;
	pMesh += i;
	//GETFSC(pFsc);
	//i = pFsc->Bowl(bType, pMesh);
	//leng += i;
	//pMesh += i;
	if (c_pPol != NULL) {
		i = c_pPol->Contain(bType, pMesh);
		leng += i;
		pMesh += i;
	}
	if (c_pLoc != NULL) {
		i = c_pLoc->Contain(bType, pMesh);
		leng += i;
		pMesh += i;
	}
	i = c_fio.Contain(bType, pMesh);
	leng += i;
	pMesh += i;

	c_mill.wPopMax = 0;
	return leng;
}

ENTRY_CONTAINER(CLand)
	SCOOP(&c_vehicle.cProperID,			sizeof(BYTE),		"Land")
	SCOOP(&c_vehicle.length.cCurrent,	sizeof(BYTE),		"")
	//SCOOP(&c_vehicle.length.cPassive,	sizeof(BYTE),		"")
	SCOOP(&c_vehicle.wBusCycleTime,		sizeof(WORD),		"")
	SCOOP(&c_vehicle.wPermNo,			sizeof(WORD),		"")
	SCOOP(&c_cmd.intro.cFrom,			sizeof(BYTE),		"intro")
	SCOOP(&c_cmd.intro.cSeq,			sizeof(BYTE),		"")
	SCOOP(&c_cmd.user.a,				sizeof(BYTE),		"")
	SCOOP(&c_mill.wPopMax,				sizeof(WORD),		"cen")
	SCOOP(&c_mill.nConsult,				sizeof(BYTE),		"")
	SCOOP(&c_pulse.dwPeriod,			sizeof(DWORD),		"puls")
	SCOOP(&c_pulse.wDuty,				sizeof(WORD),		"")
	SCOOP(&c_pulse.wDutyMax,			sizeof(WORD),		"")
	SCOOP(&c_pulse.wExceed,				sizeof(WORD),		"")
	SCOOP(&c_order,						sizeof(DTBORDERNO),	"ord")
	SCOOP(&c_tWatch.wArt[0][0],			sizeof(WORD),		"at0")
	SCOOP(&c_tWatch.wArt[0][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[1][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[1][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[2][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[2][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[3][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[3][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[4][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[4][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[5][0],			sizeof(WORD),		"at5")
	SCOOP(&c_tWatch.wArt[5][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[6][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[6][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[7][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[7][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[8][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[8][1],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[9][0],			sizeof(WORD),		"")
	SCOOP(&c_tWatch.wArt[9][1],			sizeof(WORD),		"")
	SCOOP(&c_cDtbChk[0],				sizeof(BYTE) * 12,	"dchk");
	//SCOOP(&c_tacho.w[0],				sizeof(WORD),		"tach")
	SCOOP(&c_tWatch.dwSupervise[0],		sizeof(DWORD),		"svt")
	SCOOP(&c_tWatch.dwSupervise[1],		sizeof(DWORD),		"")
	SCOOP(&c_tWatch.dwOrderAtMax,		sizeof(DWORD),		"")
	SCOOP(&c_sysChk.cPairCtrl,			sizeof(BYTE),		"cros")
	SCOOP(&c_sysChk.cVmes,				sizeof(BYTE),		"")
	SCOOP(&c_sysChk.nDeadOppoCnt,		sizeof(BYTE),		"")
	//SCOOP(&c_bcMon.wRed,				sizeof(WORD),		"BCTO")
	//SCOOP(&c_dwActiveOrder,			sizeof(DWORD),		"Ordr")
	SCOOP(&c_dtb[0].state.a,			sizeof(BYTE),		"brm")
	SCOOP(&c_dtb[1].state.a,			sizeof(BYTE),		"")
	SCOOP(&c_dtbMon.dwRoute,			sizeof(DWORD),		"")
	SCOOP(&c_dtbMon.wWakeupEach[0],		sizeof(WORD),		"")
	SCOOP(&c_dtbMon.wWakeupEach[1],		sizeof(WORD),		"")
	SCOOP(&c_dtbMon.wWakeupEach[2],		sizeof(WORD),		"")
	SCOOP(&c_dtbMon.wWakeupEach[3],		sizeof(WORD),		"")
	SCOOP(&c_trcInfo,					sizeof(TRACEINFO),	"trc")
	SCOOP(&c_tcnt.wShot,				sizeof(WORD),		"scnt")
	SCOOP(&c_tcnt.wCut,					sizeof(WORD),		"")
	SCOOP(&c_bColdWakeup,				sizeof(BYTE),		"")
	SCOOP(&c_bControlInitialed,			sizeof(BYTE),		"")
	//SCOOP(&c_ctrlSide.a,				sizeof(WORD),		"")
EXIT_CONTAINER()
