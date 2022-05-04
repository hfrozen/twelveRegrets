/* CFsc.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <devctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/i2c.h>

#include "Slight.h"
//#include "Prefix.h"
#include "Mm.h"
#include "../Inform2/Fpga/Face.h"
#include "CProse.h"
#include "CBand.h"
//#include "CTool.h"
#include "CFio.h"
#include "CFsc.h"

//#define	__DEBUG_TIFALL__

#define	MX6X_IOMUXC_BASE	0x020e0000
#define	MX6X_GPIO2_BASE		0x020a0000
#define	MX6X_GPIO3_BASE		0x020a4000
#define	MX6X_GPIO5_BASE		0x020ac000
#define	MX6X_GPIO6_BASE		0x020b0000

#define	MX6X_GPIO_DR		0x00
#define	MX6X_GPIO_GDIR		0x04

#define	DWREG(p)			*(DWORD*)((BYTE*)p)
#define	STREG(ch, v)		(*(WORD*)((BYTE*)c_pSpace + TIMER0_SV + ((ch << 2) & 4) + ((v << 1) & 2)))

#define SVREG(id)			(*(WORD*)((BYTE*)c_pSpace + ((0x90 + (id << 4)) << 8) + 0xf0))
// 00														90h + 00000000b => 90h << 8 => 9000h + f0h = 90f0h
// 01														90h + 00010000b => a0h << 8 => a000h + f0h = a0f0h
// 10														90h + 00100000b => b0h << 8 => b000h + f0h = b0f0h
#define	SBREG(ch)			(*(WORD*)((BYTE*)c_pSpace + ((0x90 + ((ch & 0xc) << 2)) << 8)))
// 00XX														90h + 00000000b =>	90h << 8 => 9000h
// 01XX														90h + 00010.00b =>	a0h << 8 => a000h
// 10XX														90h + 0010.000b =>	b0h << 8 => b000h
#define	SCREG(ch, reg)		(*(WORD*)((BYTE*)c_pSpace + ((0x90 + ((ch & 0xc) << 2) + ((ch & 3) + 1)) << 8) + reg))
// 00XX														90h + 00000000b =>	(90h + (0~3) + 1)) * 100h => 9100h~9400h
// 01XX														90h + 00010000b =>	(a0h + (0~3) + 1)) * 100h => a100h~a400h
// 10XX														90h + 00100000b =>	(b0h + (0~3) + 1)) * 100h => b100h~b400h

#define	CHECK_ROADCH(ch)\
	do {\
		if (ch == 2 || ch == 3 || ch >= MAX_VHDLCH) {\
			Pile("FSC>ERR:%s()-invalid channel!(%d)\n", __FUNCTION__, ch);\
			return false;\
		}\
	} while (0)

const DWORD	CFsc::c_dwIntrConst[MAXFPGA][5][2] = {
	{{ MX6X_GPIO5_BASE, 0xd4 }, { 20, (1 <<  4) }, { 4, (1 <<  4) }, { 12, (3 <<  8) }, { 24, (1 <<  4) }},
	{{ MX6X_GPIO2_BASE, 0xdc }, { 20, (1 << 16) }, { 4, (1 << 16) }, { 16, (3 <<  0) }, { 24, (1 << 16) }},
	{{ MX6X_GPIO6_BASE, 0xd8 }, { 20, (1 <<  6) }, { 4, (1 <<  6) }, { 12, (3 << 12) }, { 24, (1 <<  6) }}
};

const int CFsc::c_iIntrNo[MAXFPGA] =	{ 292, 208, 326 };

const int CFsc::c_iPriority[MAXFPGA] =	{ PRIORITY_INTRBUS, PRIORITY_INTRPORTB, PRIORITY_INTRPORTC };

const CFsc::PBLOCKHANDLER CFsc::c_pHandlers[MAXFPGA] = {
	&(CFsc::BlockEntryA), &(CFsc::BlockEntryB), &(CFsc::BlockEntryC)
};

CFsc::CFsc()
	: CLabel()
{
	c_pSpace = MAP_FAILED;
	c_bEnable = false;
	c_pFIO = NULL;
	memset(&c_bc, 0, sizeof(BLOCKCONFIG) * MAXFPGA);
	memset(&c_ic, 0, sizeof(CHANCONF) * MAX_VHDLCH);
	c_iRxChMonit = PSCCHB_ECU;
	for (int n = 0; n < MAXFPGA; n ++) {
		c_bc[n].iid = c_bc[n].hThread = INVALID_HANDLE;
		c_bc[n].pSpace = (DWORD*)MAP_FAILED;
	}
//#if	defined(DEBUG_TINT)
//	memset(&c_dtm, 0, sizeof(DEBUGTM) * 2);
//#endif
	memset(&c_pile, 0, sizeof(PILE));
}

CFsc::~CFsc()
{
	Destroy();
}

void CFsc::PileA(char* pBuf)
{
	c_pile.mtx.Lock();
	WORD leng = (WORD)strlen(pBuf);
	WORD rem;
	if (c_pile.ri <= c_pile.wi)	rem = c_pile.ri + MAX_PILEBUF - c_pile.wi;
	else	rem = c_pile.ri - c_pile.wi;
	if (leng < rem) {
		rem = MAX_PILEBUF - c_pile.wi;
		if (leng < rem) {
			memcpy(&c_pile.sBuf[c_pile.wi], pBuf, leng);
			c_pile.wi += leng;
		}
		else {
			memcpy(&c_pile.sBuf[c_pile.wi], pBuf, rem);
			memcpy(&c_pile.sBuf[0], pBuf + rem, leng - rem);
			c_pile.wi = leng - rem;
		}
	}

	c_pile.mtx.Unlock();
}

void CFsc::Pile0(const char* fmt, ...)
{
	char buf[1024];

	va_list vl;
	va_start(vl, fmt);
	vsprintf(&buf[0], fmt, vl);
	va_end(vl);

	PileA(buf);
}

void CFsc::Pile(const char* fmt, ...)
{
	char buf[1024];

	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	sprintf(buf, "%02d:%02d:%02d:%03d-", pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec / 1000);

	va_list vl;
	va_start(vl, fmt);
	vsprintf(&buf[13], fmt, vl);
	va_end(vl);

	PileA(buf);
}

void CFsc::Pull()
{
	WORD wi = c_pile.wi;

	c_pile.mtx.Lock();
	while (c_pile.ri != wi) {
		char buf[1024];
		int n = 0;
		while ((buf[n ++] = c_pile.sBuf[c_pile.ri ++]) != '\n') {
			if (c_pile.ri == wi) {
				buf[n ++] = '@';
				break;
			}
			if (c_pile.ri >= MAX_PILEBUF)	c_pile.ri = 0;
		}
		buf[n] = 0;
		printf(buf);
	}
	c_pile.mtx.Unlock();
}

bool CFsc::Initial()
{
	if (c_pSpace == MAP_FAILED) {
		c_pSpace = (PVOID)mmap_device_io(SIZE_BASE, ADDR_BASE);
		if (c_pSpace == MAP_FAILED) {
			Pile("FSC>ERR:%s()-space mapping of base address failed!(%s)\n", __FUNCTION__, strerror(errno));
			return false;
		}
		memset(&c_ic, 0, sizeof(CHANCONF) * MAX_VHDLCH);
		memset(&c_bc, 0, sizeof(BLOCKCONFIG) * MAXFPGA);
		for (int n = 0; n < MAXFPGA; n ++) {
			c_bc[n].iid = c_bc[n].hThread = INVALID_HANDLE;
			c_bc[n].pSpace = (DWORD*)MAP_FAILED;
		}
	}
	return true;
}

bool CFsc::Initial(PVOID pVoid)
{
	if (pVoid == NULL) {
		Pile("FSC>ERR:%s()-space mapping of base address is null!\n", __FUNCTION__);
		return false;
	}
	c_pSpace = pVoid;

	memset(&c_ic, 0, sizeof(CHANCONF) * MAX_VHDLCH);
	memset(&c_bc, 0, sizeof(BLOCKCONFIG) * MAXFPGA);
	for (int n = 0; n < MAXFPGA; n ++) {
		c_bc[n].iid = c_bc[n].hThread = INVALID_HANDLE;
		c_bc[n].pSpace = (DWORD*)MAP_FAILED;
	}
	return true;
}

// ===== communication
bool CFsc::Clean(int iCh)
{
	ASSERTP(c_pSpace);
	CHECK_ROADCH(iCh);

	WORD wFlag;
	int n = 0;
	for ( ; (wFlag = (SBREG(iCh))) & (1 << (iCh & 3)); n ++) {
		WORD leng = SCREG(iCh, COMRXLN);	// & (MAX_ROADBUF - 1);
		if (leng > 0) {
#if	defined(DEBUG_FPGASC)
			Pile("FSC:ch%d clean %d bytes!\n", iCh, leng);
#endif
			if (leng & 1)	++ leng;	// odd?
			leng >>= 1;				// word
			WORD rc;
			for ( ; leng > 0; leng --)	rc = SCREG(iCh, COMRXBF);
			++ rc;		// meaningless..., just to hide warning...
		}
		else	Pile("FSC>WARN:%s() ch%d flagged but empty!\n", __FUNCTION__, iCh);
		if (n > 3)	break;
	}
#if	defined(DEBUG_FPGASC)
	if (n > 0)	Pile("FSC:ch%d clean %d cycle!\n", iCh, n);
#endif
	return true;
}

bool CFsc::ModeConservationCheck(int iCh)
{
	WORD mode = SCREG(iCh, COMCTRL) & COMCTRL_ENCOD;
	if (iCh < MAX_MANCHESTER && mode != COMCTRL_MANCH) {
		Pile("FSC>MODE WARNING!!!!!!!!!! NOT MANCHESTER at ch%d !!!!!!!!!!(0x%04X)\n", iCh, mode);
		return false;
	}
	else if (iCh >= MAX_MANCHESTER && mode != COMCTRL_NRZI) {
		Pile("FSC>MODE WARNING!!!!!!!!!!!!! NOT NRZI at ch%d !!!!!!!!!!!!!(0x%04X)\n", iCh, mode);
		return false;
	}
	return true;
}

bool CFsc::SetInterruptBlock(int iBlock)
{
	if (iBlock >= MAXFPGA)	return false;

	PVOID pMux = (PVOID)mmap_device_io(0x100, MX6X_IOMUXC_BASE);	// VERY IMPORTENT PVOID type !!!!!
	if (pMux == MAP_FAILED) {
		Pile("FSC>ERR:%s() of 0x%08X failed!(%s)\n", __FUNCTION__, MX6X_IOMUXC_BASE, strerror(errno));
		return false;
	}

	DWREG(pMux + c_dwIntrConst[iBlock][0][1]) = (DWORD)5;
	c_bc[iBlock].pSpace = (DWORD*)mmap_device_io(0x100, c_dwIntrConst[iBlock][0][0]);
	if (c_bc[iBlock].pSpace == MAP_FAILED) {
		Pile("FSC>ERR:%s() of 0x%08X failed!(%s)", __FUNCTION__, c_dwIntrConst[iBlock][0][0], strerror(errno));
		return false;
	}
	DWREG(c_bc[iBlock].pSpace + c_dwIntrConst[iBlock][1][0]) &= ~c_dwIntrConst[iBlock][1][1];	// IMR:0 -> disable intr.
	DWREG(c_bc[iBlock].pSpace + c_dwIntrConst[iBlock][2][0]) &= ~c_dwIntrConst[iBlock][2][1];	// DIR:0 -> gpio5-4 is input
	DWREG(c_bc[iBlock].pSpace + c_dwIntrConst[iBlock][3][0]) &= ~c_dwIntrConst[iBlock][3][1];	// ICR:0 -> low level intr.
	//DWREG(c_bc[iBlock].pSpace + c_dwIntrConst[iBlock][3][0]) |= c_dwIntrConst[iBlock][3][1];	// ICR:0 -> egde intr.
	DWREG(c_bc[iBlock].pSpace + c_dwIntrConst[iBlock][4][0]) |= c_dwIntrConst[iBlock][4][1];	// ISR:0
	if (!Validity(c_bc[iBlock].hThread)) {
		c_bc[iBlock].irq = c_iIntrNo[iBlock];
		if (!Validity(CreateThread(&(c_bc[iBlock].hThread), c_pHandlers[iBlock], (PVOID)this, c_iPriority[iBlock], (PSZ)"FSC"))) {
			Pile("FSC>ERR:%s()-interrupt block thread #%d creation failed!(%s)\n", __FUNCTION__, iBlock, strerror(errno));
			return false;
		}
#if	defined(DEBUG_FPGASCIB)
		Pile("FSC:interrupt block thread #%d creation success.(%d)\n", iBlock, c_bc[iBlock].iid);
#endif
	}
	return true;
}

bool CFsc::GetConfig(ROADCONF& conf)
{
	CHECK_ROADCH(conf.iCh);

	ASSERTP(c_pSpace);
	int block = (conf.iCh >> 2) & 3;
	c_bc[block].mtx.Lock();
	if (conf.wItem & (1 << ROADITEM_ADDR))	conf.wAddr = SCREG(conf.iCh, COMSADD);
	if (conf.wItem & (1 << ROADITEM_BPS)) {
		WORD w = SCREG(conf.iCh, COMBAUD);
		if (w > 0)	w <<= 1;
		else	w = 1;
		conf.dwBps = (DWORD)CLK_BAUDRATE / ((DWORD)w * 32L);
	}
	WORD wc = SCREG(conf.iCh, COMCTRL);
	if (conf.wItem & (1 << ROADITEM_AMATCH))
		conf.mode.b.amen = (wc & (1 << COMCTRL_AMEN)) ? true : false;
	if (conf.wItem & (1 << ROADITEM_ENCOD))
		conf.mode.b.enco = (wc & COMCTRL_ENCOD) >> COMCTRL_ENC0;
	if (conf.wItem & (1 << ROADITEM_FLAGL))
		conf.mode.b.flen = SCREG(conf.iCh, COMSYNC) & 7;
	if (conf.wItem & (1 << ROADITEM_INTR))
		conf.mode.b.intr = c_ic[conf.iCh].bEnIntr;
	if (conf.wItem & (1 << ROADITEM_INTRBLK)) {
		DWORD dw = DWREG(c_bc[block].pSpace + c_dwIntrConst[block][1][0]);
		dw &= c_dwIntrConst[block][1][0];
		conf.mode.b.intrblk = dw ? true : false;
	}
	c_bc[block].mtx.Unlock();
	return true;
}

bool CFsc::SetConfig(ROADCONF conf, bool bModify)
{
	CHECK_ROADCH(conf.iCh);

	if ((conf.wItem & (1 << ROADITEM_ADDR)) && conf.wAddr == 0) {
		Pile("FSC>ERR:%s()-invalid address!(0x%04X)\n", __FUNCTION__, conf.wAddr);
		return false;
	}
	if ((conf.wItem & (1 << ROADITEM_BPS)) &&
		(conf.dwBps < MIN_BAUDRATE || conf.dwBps > MAX_BAUDRATE)) {
		Pile("FSC>ERR:%s()-invalid baudrate!(%ld)\n", __FUNCTION__, conf.dwBps);
		return false;
	}

	int block = (conf.iCh >> 2) & 3;

	c_bc[block].mtx.Lock();
	if (conf.wItem & (1 << ROADITEM_ADDR))	SCREG(conf.iCh, COMSADD) = conf.wAddr;

	if (conf.wItem & (1 << ROADITEM_BPS)) {
		WORD w = (WORD)((DWORD)CLK_BAUDRATE % (conf.dwBps * 32L));
		if (w != 0)	Pile("FSC>WARNING:%s()-incorrect baudrate!\n", __FUNCTION__);
		w = (WORD)((DWORD)CLK_BAUDRATE / (conf.dwBps * 32L));
		if (w > 2)	w >>= 1;
		else	w = 0;
		SCREG(conf.iCh, COMBAUD) = w;
	}

	if (conf.wItem & (1 << ROADITEM_FLAGL))
		SCREG(conf.iCh, COMSYNC) = conf.mode.b.flen;

	if (!(conf.wItem & ROADITEM_MODE)) {
		if (!bModify)	Clean(conf.iCh);
		c_bc[block].mtx.Unlock();
		return true;
	}

	WORD mode = SCREG(conf.iCh, COMCTRL);
	if (conf.wItem & (1 << ROADITEM_AMATCH)) {
		if (conf.mode.b.amen)	mode |= (1 << COMCTRL_AMEN);
		else	mode &= ~(1 << COMCTRL_AMEN);
	}
	if (conf.wItem & (1 << ROADITEM_ENCOD)) {
		mode &= ~COMCTRL_ENCOD;
		mode |= ((conf.mode.b.enco << COMCTRL_ENC0) & COMCTRL_ENCOD);
	}
	SCREG(conf.iCh, COMCTRL) = c_ic[conf.iCh].wMode = mode;

	WORD w;
	for (w = 0 ; w < 1000; w ++) {
		mode = SCREG(conf.iCh, COMCTRL);
		if (mode == c_ic[conf.iCh].wMode)	break;
		SCREG(conf.iCh, COMCTRL) = c_ic[conf.iCh].wMode;
	}
	if (w > 0)	Pile("FSC>MODE WARNING!!!!!!!!SETTING CYCLE %d at ch%d!!!!!!!!!(0x%04X-0x%04X)\n",
								w, conf.iCh, c_ic[conf.iCh].wMode, mode);	//mode);

	ModeConservationCheck(conf.iCh);
	if (!bModify)	Clean(conf.iCh);

	bool bRes = true;
	if (conf.wItem & (1 << ROADITEM_INTR)) {
		ThreadCtl(_NTO_TCTL_IO, NULL);
		InterruptDisable();

		if (conf.mode.b.intr) {		// enable
			if (conf.pRecvIntr != NULL && conf.pBase != NULL) {
				c_ic[conf.iCh].pRecvIntr = conf.pRecvIntr;
				//c_ic[conf.iCh].pOnesIntr = conf.pOnesIntr;
				c_ic[conf.iCh].pBase = conf.pBase;
				if (!Validity(c_bc[block].hThread))	SetInterruptBlock(block);
				if (Validity(c_bc[block].hThread)) {
					c_ic[conf.iCh].bEnIntr = true;
					DWREG(c_bc[block].pSpace + c_dwIntrConst[block][1][0]) |= c_dwIntrConst[block][1][1];	// IMR:0 -> enable intr.

					c_ic[conf.iCh].wMode |= (1 << COMCTRL_RXEN);
					SCREG(conf.iCh, COMCTRL) = c_ic[conf.iCh].wMode;
					// 17/07/03
					//if (conf.iCh < 2 && !(c_ic[conf.iCh].wMode & (1 << COMCTRL_AMEN)))	c_bAttention[0] = c_bAttention[1] = true;
#if	defined(DEBUG_FPGAS)
					Pile("FSC:ch%d interrupt enable.\n", conf.iCh);
#endif
				}
				else {
					Pile("FSC:ch%d interrupt can not enable.\n", conf.iCh);
					bRes = false;
				}
			}
			else {
				Pile("FSC>ERR:ch%d interrupt function parameter failed!(0x%08X-0x%08X)\n", conf.iCh, conf.pRecvIntr, conf.pBase);
				bRes = false;
			}
		}
		else {						// disable
			c_ic[conf.iCh].bEnIntr = false;
#if	defined(DEBUG_FPGAS)
			Pile("FSC:ch%d interrupt disable.\n", conf.iCh);
#endif
			int n = 0;
			for ( ; n < 4; n ++) {
				if (c_ic[(conf.iCh & 0xc) + n].bEnIntr)	break;
			}
			if (n >= 4) {
				DWREG(c_bc[block].pSpace + c_dwIntrConst[block][1][0]) &= ~c_dwIntrConst[block][1][1];	// IMR:0 -> disable intr.
#if	defined(DEBUG_FPGAS)
				Pile("FSC:block%d interrupt disable.\n", block);
#endif
			}
		}
		ThreadCtl(_NTO_TCTL_IO, NULL);
		InterruptEnable();
	}
	c_bc[block].mtx.Unlock();
	return bRes;
}

//void CFsc::Elapsed(bool bCh)
//{
//	c_bc[0].mtx.Lock();
//	SetTimer(bCh, 0xffff);
//
//	if (!c_bEnable) {
//		c_bc[0].mtx.Unlock();
//		return;
//	}
//
//	int ch = bCh ? TMR1 : TMR0;
//	CFio* pFio = (CFio*)c_pFIO;
//#if	defined(DEBUG_TINT)
//	uint64_t clk = ClockCycles();
//	double duty = (double)(clk - c_dtm[ch].clk) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec * 1e3;
//	if (duty < c_dtm[ch].min || duty > c_dtm[ch].max) {
//		if (pFio != NULL)	pFio->Lamp(bCh ? EPOS_TINTBPERIOD : EPOS_TINTAPERIOD);
//		c_bc[0].mtx.Unlock();
//		Pile("FSC>ONES>ERR:%d tf %f-%d %02d:%02d:%02d:%03d!\n", ch, duty, c_dtm[ch].wSetTime, c_dtm[ch].t.hour, c_dtm[ch].t.min, c_dtm[ch].t.sec, c_dtm[ch].t.msec);
//		return;
//	}
//#endif
//
//#if	defined(__DEBUG_TIFALL__)
//	bool bAst = false;
//#endif
//	WORD wf = bCh ? 8 : 4;
//	WORD w;
//	do {
//		GetTimer(bCh);				// clear timer interrupt flag
//		//SetTimer(bCh, 0xffff);
//		w = SBREG(0) & wf;
//		if ((w & wf) && pFio != NULL)	pFio->Lamp(bCh ? EPOS_TINTBFALL : EPOS_TINTAFALL);
//#if	defined(__DEBUG_TIFALL__)
//		if (w & wf) {				// verify to clear it
//			if (!bAst) {
//				Pile("TI:0x%04X", w);
//				bAst = true;
//			}
//			else	Pile0(" 0x%04X", w);
//		}
//#endif
//	} while (w != 0);
//
//#if	defined(__DEBUG_TIFALL__)
//	if (bAst)	Pile0("\n");
//#endif
//
//	SetTimer(bCh, 0xffff);
//
//	CPump* pPump = (CPump*)c_ic[ch].pBase;
//	PONESINTR pOnes = c_ic[ch].pOnesIntr;
//	if (pOnes != NULL && pPump != NULL)	(pPump->*pOnes)();
//	else	Pile("FSC>ONES>ERR:%d null func.!\n", ch);
//	c_bc[0].mtx.Unlock();
//}

void CFsc::ReceiveA(int iCh)
{
	if (!c_bEnable)	return;
	if (iCh > 1) {
		Pile("FSC>INTR>ERR:rec.a but ch%d!\n", iCh);
		return;
	}

	c_bc[0].mtx.Lock();
	WORD state = SCREG(iCh, COMSTAT);
	if ((state & COMSTAT_RXES) == 0) {
		// 190920
		// good at first, but now tc often dies. Monitor a some day. the bottom line was constantly output.
		//Pile("FSC>RECVA>ERR:%d s%04X!\n", iCh, state);
		c_bc[0].mtx.Unlock();
		return;
	}

	memset(c_ic[iCh].wRxBuf, 0, sizeof(WORD) * MAX_ROADBUF);
	WORD wLeng = 0;
	if (state & (1 << COMSTAT_RXBE)) {
		bool lack = false;
		WORD leng = SCREG(iCh, COMRXLN);
		if ((iCh < 2 && leng != 512) || leng == 0) {
			// 190920
			// good at first, but now tc often dies. Monitor a some day. the bottom line was constantly output.
			//Pile("FSC>RECVA>ERR:%d s%04X w%d!\n", iCh, state, leng);
			if (leng == 0) {
				c_bc[0].mtx.Unlock();
				return;
			}
			lack = true;
		}
		wLeng = leng;
		if (leng & 1)	++ leng;
		leng >>= 1;
		WORD w = 0;
		if (c_ic[iCh].wMode & (1 << COMCTRL_AMEN)) {
			c_ic[iCh].wRxBuf[0] = SCREG(iCh, COMRADD);
			w = 1;
		}
		for ( ; w < leng; w ++)	c_ic[iCh].wRxBuf[w] = SCREG(iCh, COMRXBF);
		if (lack) {
			for (int n = 0; n < 5; n ++)	Pile0(" %04x", c_ic[iCh].wRxBuf[n]);
			Pile0("\n");
			c_bc[0].mtx.Unlock();
			return;
		}
	}
	++ c_ic[iCh].wRxSeq;

	CPump* pPump = (CPump*)c_ic[iCh].pBase;
	PRECVINTR pRecv = c_ic[iCh].pRecvIntr;
	if (c_ic[iCh].bEnIntr && pRecv != NULL && pPump != NULL)
		(pPump->*pRecv)((BYTE*)c_ic[iCh].wRxBuf, wLeng, state & COMSTAT_RXERROR);
	else {
		Clean(iCh);
		Pile("FSC>INTR>ERR:ch%d null func. or disable!\n", iCh);
	}
	c_bc[0].mtx.Unlock();
}

void CFsc::ReceiveB(int iCh)
{
	if (!c_bEnable)	return;
	if (iCh < 4 || iCh > 7) {
		Pile("FSC>INTR>ERR:rec.b but ch%d!\n", iCh);
		return;
	}

	c_bc[1].mtx.Lock();
	WORD state = SCREG(iCh, COMSTAT);
	if ((state & COMSTAT_RXES) == 0) {
		Pile("FSC>RECVB>ERR:%d s%04X!\n", iCh, state);
		c_bc[1].mtx.Unlock();
		return;
	}

	memset(c_ic[iCh].wRxBuf, 0, sizeof(WORD) * MAX_ROADBUF);
	WORD wLeng = 0;
	if (state & (1 << COMSTAT_RXBE)) {
		WORD leng = SCREG(iCh, COMRXLN);
		if (leng == 0) {
			Pile("FSC>RECVB>ERR:%d s%04X w%d!\n", iCh, state, leng);
			c_bc[1].mtx.Unlock();
			return;
		}
		wLeng = leng;
		if (leng & 1)	++ leng;
		leng >>= 1;
		WORD w = 0;
		if (c_ic[iCh].wMode & (1 << COMCTRL_AMEN)) {
			c_ic[iCh].wRxBuf[0] = SCREG(iCh, COMRADD);
			w = 1;
		}
		for ( ; w < leng; w ++)	c_ic[iCh].wRxBuf[w] = SCREG(iCh, COMRXBF);
	}
	++ c_ic[iCh].wRxSeq;
	if (iCh != 4) {
		c_ic[iCh].wRxBuf[253] = state;
		c_ic[iCh].wRxBuf[254] = wLeng;
		c_ic[iCh].wRxBuf[255] = c_ic[iCh].wRxSeq;
	}

	CPump* pPump = (CPump*)c_ic[iCh].pBase;
	PRECVINTR pRecv = c_ic[iCh].pRecvIntr;
	if (c_ic[iCh].bEnIntr && pRecv != NULL && pPump != NULL) {
		(pPump->*pRecv)((BYTE*)c_ic[iCh].wRxBuf, wLeng, state & COMSTAT_RXERROR);
		// 170918
		if (c_ic[iCh].wRxBuf[0] == LADD_ECU && c_ic[iCh].wRxBuf[1] == 0x3013) {
			GETPROSE(pProse);
			pProse->CorralEcuTrace((BYTE*)c_ic[iCh].wRxBuf, state);
		}
	}
	else {
		Clean(iCh);
		Pile("FSC>INTR>ERR:ch%d null func. or disable!\n", iCh);
	}
	c_bc[1].mtx.Unlock();
}

void CFsc::ReceiveC(int iCh)
{
	if (!c_bEnable)	return;
	if (iCh < 8) {
		Pile("FSC>INTR>ERR:rec.c but ch%d!\n", iCh);
		return;
	}

	c_bc[2].mtx.Lock();
	WORD state = SCREG(iCh, COMSTAT);
	if ((state & COMSTAT_RXES) == 0) {
		Pile("FSC>RECVC>ERR:%d s%04X!\n", iCh, state);
		c_bc[2].mtx.Unlock();
		return;
	}

	memset(c_ic[iCh].wRxBuf, 0, sizeof(WORD) * MAX_ROADBUF);
	WORD wLeng = 0;
	if (state & (1 << COMSTAT_RXBE)) {
		WORD leng = SCREG(iCh, COMRXLN);
		if (leng == 0) {
			Pile("FSC>RECVC>ERR:%d s%04X w%d!\n", iCh, state, leng);
			c_bc[2].mtx.Unlock();
			return;
		}
		wLeng = leng;
		if (leng & 1)	++ leng;
		leng >>= 1;
		WORD w = 0;
		if (c_ic[iCh].wMode & (1 << COMCTRL_AMEN)) {
			c_ic[iCh].wRxBuf[0] = SCREG(iCh, COMRADD);
			w = 1;
		}
		for ( ; w < leng; w ++)	c_ic[iCh].wRxBuf[w] = SCREG(iCh, COMRXBF);
	}
	++ c_ic[iCh].wRxSeq;
	c_ic[iCh].wRxBuf[253] = state;
	c_ic[iCh].wRxBuf[254] = wLeng;
	c_ic[iCh].wRxBuf[255] = c_ic[iCh].wRxSeq;

	CPump* pPump = (CPump*)c_ic[iCh].pBase;
	PRECVINTR pRecv = c_ic[iCh].pRecvIntr;
	if (c_ic[iCh].bEnIntr && pRecv != NULL && pPump != NULL) {
		(pPump->*pRecv)((BYTE*)c_ic[iCh].wRxBuf, wLeng, state & COMSTAT_RXERROR);
		// 171209
		//if (c_ic[iCh].wRxBuf[0] == LADD_SIV && c_ic[iCh].wRxBuf[1] == 0x3013) {
		//	GETPROSE(pProse);
		//	pProse->CorralSivDetect((BYTE*)c_ic[iCh].wRxBuf, state);
		//}
	}
	else {
		Clean(iCh);
		Pile("FSC>INTR>ERR:ch%d null func. or disable!\n", iCh);
	}
	c_bc[2].mtx.Unlock();
}

void CFsc::BlockHandlerA()
{
	SIGEV_INTR_INIT(&c_bc[0].se);
	if (!Validity(c_bc[0].iid = InterruptAttachEvent(c_bc[0].irq, &c_bc[0].se, _NTO_INTR_FLAGS_TRK_MSK))) {
		Pile("FSC>ERR:%s()-interrupt A attach failed!(%s)\n", __FUNCTION__, strerror(errno));
		return;
	}

	WORD wIntrFlag;
	while (true) {
		InterruptWait(0, NULL);
		do {
			c_bc[0].mtx.Lock();
			wIntrFlag = SBREG(0) & 0xf;
			c_bc[0].mtx.Unlock();
			if (wIntrFlag & 1)	ReceiveA(0);
			if (wIntrFlag & 2)	ReceiveA(1);
			//if (wIntrFlag & 4)	Elapsed(false);
			//if (wIntrFlag & 8)	Elapsed(true);
		} while ((wIntrFlag & 3) != 0);
		InterruptUnmask(c_bc[0].irq, c_bc[0].iid);
	}
}

void CFsc::BlockHandlerB()
{
	SIGEV_INTR_INIT(&c_bc[1].se);
	if (!Validity(c_bc[1].iid = InterruptAttachEvent(c_bc[1].irq, &c_bc[1].se, _NTO_INTR_FLAGS_TRK_MSK))) {
		Pile("FSC>ERR:%s()-interrupt B attach failed!(%s)\n", __FUNCTION__, strerror(errno));
		return;
	}

	WORD wIntrFlag;
	while (true) {
		InterruptWait(0, NULL);
		do {
			c_bc[1].mtx.Lock();
			wIntrFlag = SBREG(4) & 0xf;
			c_bc[1].mtx.Unlock();
			if (wIntrFlag & 1)	ReceiveB(4);
			if (wIntrFlag & 2)	ReceiveB(5);
			if (wIntrFlag & 4)	ReceiveB(6);
			if (wIntrFlag & 8)	ReceiveB(7);
		} while (wIntrFlag != 0);
		InterruptUnmask(c_bc[1].irq, c_bc[1].iid);
	}
}

void CFsc::BlockHandlerC()
{
	SIGEV_INTR_INIT(&c_bc[2].se);
	if (!Validity(c_bc[2].iid = InterruptAttachEvent(c_bc[2].irq, &c_bc[2].se, _NTO_INTR_FLAGS_TRK_MSK))) {
		Pile("FSC>ERR:%s()-interrupt C attach failed!(%s)\n", __FUNCTION__, strerror(errno));
		return;
	}

	WORD wIntrFlag;
	while (true) {
		InterruptWait(0, NULL);
		do {
			c_bc[2].mtx.Lock();
			wIntrFlag = SBREG(8) & 0xf;
			c_bc[2].mtx.Unlock();
			if (wIntrFlag & 1)	ReceiveC(8);
			if (wIntrFlag & 2)	ReceiveC(9);
			if (wIntrFlag & 4)	ReceiveC(10);
			if (wIntrFlag & 8)	ReceiveC(11);
		} while (wIntrFlag != 0);
		InterruptUnmask(c_bc[2].irq, c_bc[2].iid);
	}
}

PVOID CFsc::BlockEntryA(PVOID pVoid)
{
	CFsc* pFsc = (CFsc*)pVoid;
	pFsc->BlockHandlerA();
	return NULL;
}

PVOID CFsc::BlockEntryB(PVOID pVoid)
{
	CFsc* pFsc = (CFsc*)pVoid;
	pFsc->BlockHandlerB();
	return NULL;
}

PVOID CFsc::BlockEntryC(PVOID pVoid)
{
	CFsc* pFsc = (CFsc*)pVoid;
	pFsc->BlockHandlerC();
	return NULL;
}

//void CFsc::SetTimer(bool bCh, WORD wTime)
//{
//	int ch = bCh ? TMR1 : TMR0;
////#if	defined(DEBUG_TINT)
//	c_dtm[ch].wRemain = STREG(ch, PV);
//	if (wTime < 0xffff) {
//		c_dtm[ch].clk = ClockCycles();
//		c_dtm[ch].wSetTime = wTime;
//		c_dtm[ch].min = (double)wTime * 0.05f;
//		c_dtm[ch].max = (double)wTime * 0.15f;
//
//		struct timeval tv;
//		gettimeofday(&tv, NULL);
//		struct tm* pTm = localtime(&tv.tv_sec);
//		c_dtm[ch].t.hour = pTm->tm_hour;
//		c_dtm[ch].t.min = pTm->tm_min;
//		c_dtm[ch].t.sec = pTm->tm_sec;
//		c_dtm[ch].t.msec = (int)tv.tv_usec / 1000;
//	}
////#endif
//	STREG(ch, SV) = wTime;
//}
//
//WORD CFsc::GetTimer(bool bCh)
//{
//	return STREG(bCh ? TMR1 : TMR0, PV);
//}
//
bool CFsc::Appear(int iCh, WORD wAddr, DWORD dwBps, bool bMatch, int iFlagLength, PRECVINTR pRecvIntr, CPump* pBase)
{
//	return Appear(iCh, wAddr, dwBps, bMatch, iFlagLength, pRecvIntr, NULL, pBase);
//}
//
//bool CFsc::Appear(int iCh, WORD wAddr, DWORD dwBps, bool bMatch, int iFlagLength, PRECVINTR pRecvIntr, PONESINTR pOnesIntr, CPump* pBase)
//{
	CHECK_ROADCH(iCh);

	if (wAddr == 0) {
		Pile("FSC>ERR:%s()-invalid address!(0x%08X)", __FUNCTION__, wAddr);
		return false;
	}

	ROADCONF conf;
	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wItem = (1 << ROADITEM_INTR);
	GetConfig(conf);
	if (conf.mode.b.intr) {
		Pile("FSC>ERR:%s()-ch%d already opened!\n", __FUNCTION__, iCh);
		return false;
	}

	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wAddr = wAddr;
	conf.dwBps = dwBps;
	conf.mode.b.amen = bMatch;
	conf.mode.b.enco = (conf.iCh < MAX_MANCHESTER) ? ENCM_MANCH : ENCM_NRZI;
	conf.mode.b.crcg = true;
	conf.mode.b.flen = iFlagLength & 7;
	if (pRecvIntr != NULL && pBase != NULL) {
		conf.mode.b.intr = true;
		conf.pRecvIntr = pRecvIntr;
		//conf.pOnesIntr = pOnesIntr;
		conf.pBase = (PVOID)pBase;
	}
	else	conf.mode.b.intr = false;

	conf.wItem = ROADITEM_ALL;
	return SetConfig(conf, false);
}

bool CFsc::Alter(int iCh, WORD wAddr, bool bMatch)
{
	CHECK_ROADCH(iCh);

	ROADCONF conf;
	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wItem = (1 << ROADITEM_INTR);
	GetConfig(conf);
	if (!conf.mode.b.intr) {
		Pile("FSC>WARNING:%s()-ch%d already closed!\n", __FUNCTION__, iCh);
		return false;
	}

	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wAddr = wAddr;
	conf.mode.b.amen = bMatch;
	conf.wItem = (1 << ROADITEM_ADDR) | (1 << ROADITEM_AMATCH);
	return SetConfig(conf, true);
}

bool CFsc::Alter(int iCh, WORD wAddr)
{
	CHECK_ROADCH(iCh);

	ROADCONF conf;
	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wItem = (1 << ROADITEM_INTR);
	GetConfig(conf);
	if (!conf.mode.b.intr) {
		Pile("FSC>ALTER>WARN:%d closed!\n", iCh);
		return false;
	}

	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wAddr = wAddr;
	conf.wItem = (1 << ROADITEM_ADDR);
	return SetConfig(conf, true);
}

bool CFsc::Disappear(int iCh)
{
	CHECK_ROADCH(iCh);

	ROADCONF conf;
	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.wItem = (1 << ROADITEM_INTR);
	GetConfig(conf);
	if (!conf.mode.b.intr) {
		Pile("FSC>ALTER>WARN:%d closed!\n", iCh);
		return false;
	}

	memset(&conf, 0, sizeof(ROADCONF));
	conf.iCh = iCh;
	conf.mode.b.intr = false;
	conf.wItem = (1 << ROADITEM_INTR);
	return SetConfig(conf, false);
}

bool CFsc::Shoot(int iCh, PVOID p, WORD wLength)
{
	CHECK_ROADCH(iCh);

	DWORD dw = 0;
	while (!((SCREG(iCh, COMSTAT)) & (1 << COMSTAT_TXCM))) {
		if (++ dw >= 100)	break;
	}
	if (dw >= 100) {
		if (iCh == 0 || iCh == 1) {
			CFio* pFio = (CFio*)c_pFIO;
			if (pFio != NULL)	pFio->Lamp(iCh != 0 ? EPOS_SHOTB : EPOS_SHOTA);
		}
		Pile("FSC>SHOOT>ERR:%d TXCM!\n", iCh);
		return false;
	}

	WORD* pw = (WORD*)p;
	int block = (iCh >> 2) & 3;
	c_bc[block].mtx.Lock();
	SCREG(iCh, COMCTRL) &= ~(1 << COMCTRL_TRUN);
	for (WORD w = 0; w < (wLength / 2); w ++)	SCREG(iCh, COMTXBF) = *pw ++;
	if (wLength & 1)	SCREG(iCh, COMTXBB) = *pw;	// lsb = lsb
	SCREG(iCh, COMCTRL) |= (1 << COMCTRL_TRUN);
	c_bc[block].mtx.Unlock();

	// about 240us to 512bytes
	return true;
}

WORD CFsc::GetVersion(int id)
{
	WORD w = 0;
	if (id < 3)	w = SVREG(id);
	return w;
}

void CFsc::Destroy()
{
	for (int n = 0; n < MAX_VHDLCH; n ++) {
		if (n != 2 && n != 3)	Disappear(n);
	}

	for (int n = 0; n < MAXFPGA; n ++) {
		if (Validity(c_bc[n].iid))	InterruptDetach(c_bc[n].iid);
		if (Validity(c_bc[n].hThread))	pthread_cancel(c_bc[n].hThread);
	}
}

bool CFsc::IsSpaceMap()
{
	return c_pSpace == MAP_FAILED ? false : true;
}

void CFsc::SetRxChMonit(int iMonit)
{
	if (iMonit < PSCCHB_MAX) {
		c_iRxChMonit = iMonit;
		//TRACK("FSC:set monit %d\n", c_iRxChMonit);
	}
}

void CFsc::CopyReceiveBuf(int iCh, WORD* pBuf)
{
	memcpy(pBuf, c_ic[iCh].wRxBuf, 512);
}

int CFsc::Bale(bool bType, BYTE* pMesh)
{
	int leng = 0;
	if (bType) {
		sprintf((char*)pMesh, "$RXB, 512,");
		while (*pMesh != '\0') {
			++ pMesh;
			++ leng;
		}
	}
	else {
		memcpy(pMesh, c_ic[c_iRxChMonit].wRxBuf, 512);
		pMesh += 512;
		leng += 512;
	}
	return leng;
}

//ENTRY_BOWL(CFsc)
//	SCOOP(&c_pile.ri,	sizeof(WORD),	"PILE")
//	SCOOP(&c_pile.wi,	sizeof(WORD),	"");
//EXIT_BOWL()
