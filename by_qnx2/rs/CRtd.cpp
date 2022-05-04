/*
 * CRtd.cpp
 *
 *  Created on: 2016. 10. 9.
 *      Author: Che
 */
#include <sys/syspage.h>

#include "../Inform2/Track.h"
#include "../Sheet/CArch.h"

#include "CRtd.h"

#define	RS_VERSION	134
#define	PACK	pRtdr->t.s.pack

CRtd::CRtd()
{
	c_cTick = PERIOD_TICK;
	c_cTextTick = 0;
	c_bTextSeqChg = false;
	c_cTextSeq = 0;
	c_clock = 0;
	memset(&c_dt, 0, sizeof(DTSTIME));

	c_pTuner = NULL;
	c_pDev[0] = c_pDev[1] = NULL;
	c_wCycle[0] = c_wCycle[1] = CYCLE_RECEIVEA;
	memset(&c_wRed, 0, sizeof(WORD) * 2 * LCF_MAX);
	c_dwArch[0] = c_dwArch[1] = 0xffff;
	memset(&c_devr, 0, sizeof(DEVRECP) * 2);
}

CRtd::~CRtd()
{
	Destroy();
}

const LOCALLAYERA CRtd::c_layer[] = {
/* RTD */	{	PSCCHB_RTD,		LADD_RTD,	1152000,	0,	(PSZ)"RTDA"	},
/* RTD */	{	PSCCHB_RTD + 1,	LADD_RTD,	1152000,	0,	(PSZ)"RTDB"	},
			{	0xff,			0,			0,			0,	(PSZ)"" }
};

void CRtd::Destroy()
{
	KILLR(c_pTuner);
	KILL(c_pDev[0]);
	KILL(c_pDev[1]);
}

void CRtd::IntroTune(PVOID pVoid)
{
	CRtd* pRtd = (CRtd*)pVoid;
	pRtd->Arteriam();
}

void CRtd::Arteriam()
{
	GETARCHIVE(pArch);
	pArch->Arteriam();

	if (-- c_cTick == 0) {
		c_cTick = PERIOD_TICK;
		char buf[64];
		switch (c_cTextSeq) {
		case 0 :
			{
				PDTSTIME pDtst = pArch->GetDtsTime();
				if (c_bTextSeqChg ||
					pDtst->year != c_dt.year || pDtst->mon != c_dt.mon ||
					pDtst->day != c_dt.day || pDtst->hour != c_dt.hour ||
					pDtst->min != c_dt.min || pDtst->sec != c_dt.sec) {
					memcpy(&c_dt, pDtst, sizeof(DTSTIME));
					sprintf(buf, "%02d/%02d/%02d%02d:%02d:%02d",
							(c_dt.year % 100), c_dt.mon, c_dt.day,
								c_dt.hour, c_dt.min, c_dt.sec);
					c_fio.SetText(0, buf);
					c_bTextSeqChg = false;
				}
			}
			break;
		case 1 :
			{
				DWORD dwIndex = pArch->GetTroubleIndex();
				DWORD dwReduce = pArch->GetTroubleReduce();
				if (c_bTextSeqChg ||
					dwIndex != c_dwArch[0] ||
					dwReduce != c_dwArch[1]) {
					c_dwArch[0] = dwIndex;
					c_dwArch[1] = dwReduce;
					sprintf(buf, "T %06dR %06d", dwIndex, dwReduce);
					c_fio.SetText(0, buf);
					c_bTextSeqChg = false;
				}
			}
			break;
		default :
			{
				WORD period1 = c_devr[0].wPeriod;
				WORD cnt1 = c_devr[0].wCnt;
				WORD period2 = c_devr[1].wPeriod;
				WORD cnt2 = c_devr[1].wCnt;
				if (c_bTextSeqChg || c_devr[0].wPeriodBuf != period1 ||
					c_devr[1].wPeriodBuf != period2 ||
					c_devr[0].wCntBuf != cnt1 || c_devr[1].wCntBuf != cnt2) {
					c_devr[0].wPeriodBuf = period1;
					c_devr[1].wPeriodBuf = period2;
					c_devr[0].wCntBuf = cnt1;
					c_devr[1].wCntBuf = cnt2;
					sprintf(buf, "L%03d %03dR%03d %03d",
							period1, cnt1, period2, cnt2);
					c_fio.SetText(0, buf);
					c_bTextSeqChg = false;
				}
			}
			break;
		}
		if (++ c_cTextTick >= PERIOD_SEQ) {
			c_cTextTick = 0;
			if (++ c_cTextSeq >= 3)	c_cTextSeq = 0;
			c_bTextSeqChg = true;
		}
	}
}

void CRtd::IncreaseRed(int nId, WORD wErr)
{
	if (nId > 1 || wErr > LCF_MAX)	return;
	INCWORD(c_wRed[nId][wErr]);
}

void CRtd::CalcPeriod(int nId, uint64_t clk)
{
	if (nId > 1)	return;
	if (c_devr[nId].bCont) {
		double ms =
				(double)(clk - c_devr[nId].clk) /
				(double)SYSPAGE_ENTRY(qtime)->cycles_per_sec * 1e3;
		c_devr[nId].wPeriod = (WORD)ms;
	}
	c_devr[nId].clk = clk;
	c_devr[nId].bCont = true;
	if (++ c_devr[nId].wCnt >= 1000)	c_devr[nId].wCnt = 1;
}

void CRtd::Receive(PRTDBINFO pRtdr, int nId, WORD wLength)
{
	GETARCHIVE(pArch);
	switch (PACK.cItem) {
	case RTDITEM_NON :
		break;
	case RTDITEM_CLOSECURRENT :
		if (PACK.ob.b.main) {
			PBULKPACK pBulk = pArch->GetBulk();
			if (PACK.pad.tp.wLength != sizeof(LOGBOOKHEAD))
				TRACK("RTD>ERR:Wrong size at LOGBOOKHEAD!(%d)\n", PACK.pad.tp.wLength);
			memcpy(&pBulk->lbHead, (PVOID)PACK.pad.tp.tin.c, sizeof(LOGBOOKHEAD));
			pArch->AidCmd(AIDCMD_CLOSECURRENT);
		}
		Respond(pRtdr, wLength);
		break;
	case RTDITEM_SAVELOGBOOK :
		if (PACK.ob.b.main) {
			if (PACK.pad.tp.wLength != sizeof(LOGBOOK))
				TRACK("RTD>ERR:Wrong size at LOGBOOK!(%d)\n", PACK.pad.tp.wLength);
			pArch->AidCmdPeriod(AIDCMD_APPENDLOGBOOK,
									(BYTE*)PACK.pad.tp.tin.c, sizeof(LOGBOOK));
		}
		Respond(pRtdr, wLength);
		break;
	case RTDITEM_SAVETROUBLE :
		if (PACK.ob.b.main) {
			if ((PACK.pad.tp.wLength % sizeof(TROUBLEINFO)) != 0)
				TRACK("RTD>ERR:Wrong size at TROUBLE!(%d)\n", PACK.pad.tp.wLength);
			int leng = PACK.pad.tp.wLength / sizeof(TROUBLEINFO);
			if (leng > 0) {
				pArch->AidCmdPeriod(AIDCMD_APPENDTROUBLE,
							(BYTE*)PACK.pad.tp.tin.c, leng * sizeof(TROUBLEINFO));
			}
			else	TRACK("RTD>ERR:Empty trouble information!\n");
		}
		Respond(pRtdr, wLength);
		break;
	case RTDITEM_SAVEINSPECT :
		if (PACK.ob.b.main) {
			if (PACK.pad.tp.wLength != sizeof(INSPREPORT))
				TRACK("RTD>ERR:Wrong size at INSPECT!(%d)\n", PACK.pad.tp.wLength);
			pArch->AidCmdPeriod(AIDCMD_SAVEINSPECT,
								(BYTE*)PACK.pad.tp.tin.c, sizeof(INSPREPORT));
		}
		Respond(pRtdr, wLength);
		break;
	case RTDITEM_REPORTTODESK :
		break;
	case RTDITEM_LOADLOGBOOKENTRY :
		break;
	case RTDITEM_LOADINSPECTENTRY :
		break;
	case RTDITEM_LOADLOGBOOKLIST :
		break;
	case RTDITEM_LOADLOGBOOK :
		break;
	case RTDITEM_LOADSECTIONFIELD :
		break;
	case RTDITEM_SAVEFULLSDASET :
		break;
	default :
		TRACK("RTD>ERR:%s() Unknown request item(%d)!\n", __FUNCTION__, pRr->cItem);
		break;
	}

	if ((pRtdr->t.s.pack.cItem >= RTDITEM_CLOSECURRENT && pRtdr->t.s.pack.cItem <= RTDITEM_CDMAREPORT) ||
		pRtdr->t.s.pack.cItem == RTDITEM_SAVEFULLSDASET) {
		memcpy(&c_rInfo[RTDA_ID], &c_rInfo[RTDR_ID], sizeof(RTDBINFO));
		c_rInfo[RTDA_ID].t.s.cFlow = DEVFLOW_ANS;
		c_pDev[id]->Send((BYTE*)&c_rInfo[RTDA_ID], wLength);
	}
}

void CRtd::TakeReceiveDev(DWORD dw)
{
	c_mtxf.Lock();
	TAGUNPAR(dw, wLeng, state, ch);

	uint64_t clk = ClockCycles();
	if (ch < DEVCHB_RTD && ch > (DEVCHB_RTD + 1)) {
		TRACK("RTD>ERR:!!!!!!!!!!:%s() UNREACHABLE CH.(%d) !!!!!!!!\n", __FUNCTION__, ch);
		c_mtxf.Unlock();
		return;
	}
	if (wLeng >= 8000) {
		TRACK("RTD>ERR:!!!!!!!!!!:%s() UNREACHABLE LINE CH.(%d) !!!!!!!!\n", __FUNCTION__, ch);
		c_mtxf.Unlock();
		return;
	}

	BYTE id = (ch == (DEVCHB_RTD & 0x7f)) ? 0 : 1;
	if ((state & ~8) != 0) {
		IncreaseRed(id, LCF_RCRC);
		CalcPeriod(id, clk);
		TRACK("RTD>ERR:%s() state(%02X) at ch.%d!\n", __FUNCTION__, state, id);
		c_mtxf.Unlock();
		return;
	}

	c_mtxr.Lock();
	c_pDev[id]->Bunch((BYTE*)&c_rInfo[RTDR_ID], SIZE_RTDSB);
	if (c_rInfo[RTDR_ID].wAddr != LADD_RTD) {
		IncreaseRed(id, LCF_RADDR);
		CalcPeriod(id, clk);
		TRACK("RTD>ERR:%s() address(%04X) at ch.%d!\n", __FUNCTION__, c_rInfo[RTDR_ID].wAddr, id);
		c_mtxr.Unlock();
		c_mtxf.Unlock();
		return;
	}

	if (c_rInfo[RTDR_ID].cCtrl != DEVCHAR_CTL || c_rInfo[RTDR_ID].t.s.cFlow != DEVFLOW_REQ) {
		IncreaseRed(id, LCF_RFRAME);
		CalcPeriod(id, clk);
		TRACK("RTD>ERR:%s() flow control(%02X-%02X) at ch.%d!\n", __FUNCTION__, c_rInfo[RTDR_ID].cCtrl, c_rInfo[RTDR_ID].t.s.cFlow, id);
		c_mtxr.Unlock();
		c_mtxf.Unlock();
		return;
	}

	c_wCycle[id] = CYCLE_RECEIVEB;
	Receive(&c_rInfo[RTDR_ID], id, wLeng);
	CalcPeriod(id, clk);

	c_mtxr.Unlock();
	c_mtxf.Unlock();
}

BEGIN_TAGTABLE(CRtd, CPump)
	_TAGMAP(TAG_RECEIVEDEV, &CRtd::TakeReceiveDev)
END_TAGTABLE();


bool CRtd::Launch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("rs begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

	TRACK("rs ver. %.2f %s %s\n", (double)RS_VERSION / 100.f, __DATE__, __TIME__);

	// open i/o
	c_fio.Initial(true, false);
	// rtd has not io board, but must use vfd
	//BYTE ios = c_fio.GetDioReg();
	//if ((ios & ((1 << CFio::IOB_OUTP) | (1 << CFio::IOB_INPA) | (1 << CFio::IOB_INPB))) != 0) {
	//	c_fio.SetText(0, (PSZ)"RTD STATIO ERROR");
	//	TRACK("RTD>ERR:vme-io installation failure!(0x%02X)\n", ios);
	//	return false;
	//}
	//					   0123456789ABCDEF
	c_fio.SetText(0, (PSZ)"RTD STATCHECK...");

#if	defined(_ENWATCHDOG_)
	WORD w = c_fio.GetGeneralReg();
	if (w > 10)	w = 1;
	else if (w < 10)	++ w;
	WORD wRtdLifeCycle = w;
	c_fio.SetGeneralReg(wRtdLifeCycle);
#endif

	GETFSC(pFsc);
	if (!pFsc->Initial(c_fio.GetSpace())) {
		c_fio.SetText(8, (PSZ)"COMM ERR");
		TRACK("RTD>ERR:peripheral port initial failure!\n");
		return false;
	}

	// open queue
	c_hReservoir = CreateReservoir((PSZ)MAINQ_STR);
	if (!Validity(c_hReservoir)) {
		//					   89ABCDEF
		c_fio.SetText(8, (PSZ)"SQUE ERR");
		TRACK("RTD>ERR:can not create message queue!\n");
		return false;
	}

	// conform time
	GETARCHIVE(pArch);
	pArch->SetCarrier(this);
	if (!pArch->OpenAid()) {
		c_fio.SetText(8, (PSZ)"MMIF ERR");	// mm(aid) fail
		TRACK("LAND>ERR:archive - aid!\n");
		return false;
	}
	pArch->Arteriam();						// initial time.

	for (int n = 0; c_layer[n].iCh < (int)0xff; n ++) {
		KILLC(c_pDev[n]);
		c_pDev[n] = new CDev(this, c_hReservoir, c_layer[n].pszName);
		if (!c_pDev[n]->Open(c_layer[n].iCh, c_layer[n].wAddr, c_layer[n].dwBps, 0, 0)) {
			char buf[16];
			//			  89ABCDEF
			sprintf(buf, "CH.%d ERR", n);
			c_fio.SetText(8, buf);
			TRACK("RTD>%s>ERR:intial failure!\n", c_layer[n].pszName);
			delete c_pDev[n];
			c_pDev[n] = NULL;
			return false;
		}
		else	TRACK("RTD>%s:intial.\n", c_layer[n].pszName);
	}

	TRACK("LOGCONTENTS  %d\n", sizeof(LOGCONTENTS));
	TRACK("RTDBINFO     %d\n", sizeof(RTDBINFO));
	TRACK("RTDTINE      %d\n", sizeof(RTDTINE));

	KILLR(c_pTuner);
	c_pTuner = new CTimer(this, (PSZ)"TUNE");
	if (c_pTuner->Initial(PRIORITY_PERIODTUNE, (CTimer::PTIMERFUNC)(&CRtd::Arteriam), (CPump*)this, TIMERCODE_BASE, true) == 0) {
		if (!c_pTuner->Trigger(TPERIOD_BASE)) {
			//					   89ABCDEF
			c_fio.SetText(8, (PSZ)"TIMC ERR");
			TRACK("RTD>ERR:timer creation failure!\n");
			return false;
		}
	}
	else {
		c_fio.SetText(8, (PSZ)"TIMI ERR");
		TRACK("RTD>ERR:timer initial failure!\n");
		return false;
	}

	c_fio.SetWatchdogCycle(NORMALWATCHDOG_CYCLE);
	c_fio.EnableWatchdog(false, false);
	//					   0123456789ABCDEF
	c_fio.SetText(0, (PSZ)"        RTD V1.0");

	return true;
}
