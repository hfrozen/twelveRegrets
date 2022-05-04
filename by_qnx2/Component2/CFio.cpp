/* CFio.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
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
#include "Prefix.h"
#include "../Inform2/Fpga/Face.h"
#include "CFio.h"

#define	WSPACE(ofs)			(*(WORD*)((BYTE*)c_pSpace + ofs))
#define	VMEAMC_CODE			0x500
#define	VMEALLOCPTR(x)		((DWORD)(x << 16) | 0x4000)
#define	CURVEFILE			"/dev/i2c2"

const WORD CFio::c_wAllocCode[MAX_ALLOCCODE] =	{	0x10,	0x20,	0x30	};
const DWORD CFio::c_wCertify[MAX_ALLOCCODE] =	{	0x1055,	0x2055,	0x3055	};
const PSZ CFio::c_szName[MAX_ALLOCCODE] = {
	(PSZ)"output board",	(PSZ)"input board #1",	(PSZ)"input board #2"
};

const int CFio::c_iTextUnit[2][LENGTH_FIOTEXT] = {
	{	 7,  6,  5,  4,  3,  2,  1,  0, 15, 14, 13, 12, 11, 10,  9,  8	},
	{	 3,  2,  1,  0,  7,  6,  5,  4, 99, 99, 99, 99, 99, 99, 99, 99	}
};

//const BYTE CFio::c_cLamp[5] =	{	0x01, 0x01, 0x01, 0x01, 0x01	};

#if	defined(DISP_RTD)
#define	HALF_DISP	7
#else
#define	HALF_DISP	3
#endif

#define	PSCUNIT(x)	(((x & HALF_DISP) ^ HALF_DISP) | (x & (0xf & ~HALF_DISP)))

#include "../Inform2/Fpga/Font57.h"

CFio::CFio()
{
	c_bTwin = false;
	c_bMain = false;
	c_bInLab = false;
	c_pSpace = MAP_FAILED;
	c_cDioReg = 0;
	c_wBorn[0] = c_wBorn[1] = 0;
	c_fdAdv = INVALID_HANDLE;
	// 171101
	c_octBackup[0].qw = c_octBackup[1].qw = 0;
	memset(&c_io, 0, sizeof(IOBUFFER));
	memset(&c_pwm, 0, sizeof(PWMBUFFER));
	memset(&c_cText, 0, sizeof(BYTE) * LENGTH_FIOTEXT);
	memset(&c_cLamp, 0, sizeof(BYTE) * (EPOS_MAX / 5));
	memset(&c_wLampTimer, 0, sizeof(WORD) * EPOS_MAX);
	//memset(&c_wLampTimer, 0, sizeof(WORD) * LENGTH_FIOTEXT);
	//memset(&c_cInpMons, 0, sizeof(BYTE) * 10);
	c_cInpMons[0] = c_cInpMons[1] = 0;
	c_cTrio = 0;

	c_wTacho = c_wTachi = 0;
}

CFio::~CFio()
{
	DisableWatchdog();		// destructor
	Destroy();
}

WORD CFio::GetInp(QWORD& qw)
{
	c_mtx.Lock();
	WSPACE(EXTBANK_SEL) = VMEAMC_CODE | c_wAllocCode[IOB_INPA];
	WORD wc = WSPACE(VMEALLOCPTR(c_wAllocCode[IOB_INPA]));

	_OCTET oct;
	// 171101
	if (wc == c_wCertify[IOB_INPA]) {
		for (int n = 0; n < 4; n ++)	oct.w[n] = WSPACE(n * 2);
		c_octBackup[0].qw = oct.qw;
	}
	else	oct.qw = c_octBackup[0].qw;
	c_mtx.Unlock();

	qw = oct.qw;
	return wc;
}

WORD CFio::GetInpEx(QWORD& qw)
{
	c_mtx.Lock();
	WSPACE(EXTBANK_SEL) = VMEAMC_CODE | c_wAllocCode[IOB_INPB];
	WORD wc = WSPACE(VMEALLOCPTR(c_wAllocCode[IOB_INPB]));

	_OCTET oct;
	// 171101
	if (wc == c_wCertify[IOB_INPB]) {
		for (int n = 0; n < 4; n ++)	oct.w[n] = WSPACE(n * 2);
		c_octBackup[1].qw = oct.qw;
	}
	else	oct.qw = c_octBackup[1].qw;
	c_mtx.Unlock();

	qw = oct.qw;
	return wc;
}

WORD CFio::GetOutp(WORD& w, WORD ofs)
{
	c_mtx.Lock();
	WSPACE(EXTBANK_SEL) = VMEAMC_CODE | c_wAllocCode[IOB_OUTP];
	WORD wc = WSPACE(VMEALLOCPTR(c_wAllocCode[IOB_OUTP]));
	w = WSPACE(ofs);
	c_mtx.Unlock();

	return wc;
}

BYTE CFio::Debounce()
{
	if (c_cDioReg & (1 << IOB_INPA)) {
		c_io.in[0].vib.cur.wCertify = GetInp(c_io.in[0].vib.cur.oct.qw);
		if (c_io.in[0].vib.cur.wCertify != c_wCertify[IOB_INPA])	INCBYTE(c_cInpMons[0]);		// 171029	[0]);
		// 171029
		//for (int n = 0; n < 4; n ++) {
		//	if (c_io.in[0].vib.cur.oct.w[n] == c_wCertify[IOB_INPA] ||
		//		c_io.in[0].vib.cur.oct.w[n] == c_wCertify[IOB_INPB])	INCBYTE(c_cInpMons[0][n + 1]);
		//}
		if (c_io.in[0].vib.cur.wCertify != c_io.in[0].vib.prev.wCertify ||
			c_io.in[0].vib.cur.oct.qw != c_io.in[0].vib.prev.oct.qw) {
			c_io.in[0].vib.prev.wCertify = c_io.in[0].vib.cur.wCertify;
			c_io.in[0].vib.prev.oct.qw = c_io.in[0].vib.cur.oct.qw;
			c_io.in[0].vib.deb = DEBOUNCE_CYCLE;
		}
		else if (c_io.in[0].vib.deb > 0 && -- c_io.in[0].vib.deb == 0) {
			c_io.in[0].stab.wCertify = c_io.in[0].vib.prev.wCertify;
			c_io.in[0].stab.oct.qw = c_io.in[0].vib.prev.oct.qw;
		}
	}
	if (c_cDioReg & (1 << IOB_INPB)) {
		c_io.in[1].vib.cur.wCertify = GetInpEx(c_io.in[1].vib.cur.oct.qw);
		if (c_io.in[1].vib.cur.wCertify != c_wCertify[IOB_INPB])	INCBYTE(c_cInpMons[1]);		// 171029	[0]);
		// 171029
		//for (int n = 0; n < 4; n ++) {
		//	if (c_io.in[1].vib.cur.oct.w[n] == c_wCertify[IOB_INPB] ||
		//		c_io.in[1].vib.cur.oct.w[n] == c_wCertify[IOB_INPA])	INCBYTE(c_cInpMons[1][n + 1]);
		//}
		if (c_io.in[1].vib.cur.wCertify != c_io.in[1].vib.prev.wCertify ||
			c_io.in[1].vib.cur.oct.qw != c_io.in[1].vib.prev.oct.qw) {
			c_io.in[1].vib.prev.wCertify = c_io.in[1].vib.cur.wCertify;
			c_io.in[1].vib.prev.oct.qw = c_io.in[1].vib.cur.oct.qw;
			c_io.in[1].vib.deb = DEBOUNCE_CYCLE;
		}
		else if (c_io.in[1].vib.deb > 0 && -- c_io.in[1].vib.deb == 0) {
			c_io.in[1].stab.wCertify = c_io.in[1].vib.prev.wCertify;
			c_io.in[1].stab.oct.qw = c_io.in[1].vib.prev.oct.qw;
		}
	}
	c_mto.Lock();
	if (c_cDioReg & (1 << IOB_OUTP)) {
		c_io.out.vib.cur.wCertify = GetOutp(c_io.out.vib.cur.duet.w);
		if (c_io.out.vib.cur.wCertify != c_io.out.vib.prev.wCertify ||
			c_io.out.vib.cur.duet.w != c_io.out.vib.prev.duet.w) {
			c_io.out.vib.prev.wCertify = c_io.out.vib.cur.wCertify;
			c_io.out.vib.prev.duet.w = c_io.out.vib.cur.duet.w;
			c_io.out.vib.deb = DEBOUNCE_CYCLE;
		}
		else if (c_io.out.vib.deb > 0 && -- c_io.out.vib.deb == 0) {
			c_io.out.stab.wCertify = c_io.out.vib.prev.wCertify;
			c_io.out.stab.duet.w = c_io.out.vib.prev.duet.w;
		}
	}
	c_mto.Unlock();

	BYTE ret = 0;
	if (c_io.out.stab.wCertify == c_wCertify[IOB_OUTP])		ret |= (1 << IOB_OUTP);		// 1
	if (c_io.in[0].stab.wCertify == c_wCertify[IOB_INPA])	ret |= (1 << IOB_INPA);		// 2
	if (c_io.in[1].stab.wCertify == c_wCertify[IOB_INPB])	ret |= (1 << IOB_INPB);		// 4

	return ret;
}

bool CFio::Terminate(int iTerm)
{
	BYTE buf[8];
	sprintf((char*)buf, "ER%02d", iTerm);
	SetText(4, (PSZ)buf);
	return false;
}

bool CFio::Certifier(enIOTYPE io, bool bBoot, bool bMsg)
{
	if (!bBoot) {
		ASSERTP(c_pSpace);
		if (!(c_cDioReg & (1 << io))) {
			TRACK("FIO>ERR:%s empty!\n", c_szName[io]);
			return false;
		}
	}

	c_mtx.Lock();
	WSPACE(EXTBANK_SEL) = VMEAMC_CODE | c_wAllocCode[io];
	WORD w = WSPACE(VMEALLOCPTR(c_wAllocCode[io]));
	c_mtx.Unlock();
	if (w != c_wCertify[io]) {
		if (bMsg)	TRACK("FIO>ERR:%s incorrect!(0x%04X)\n", c_szName[io], w);
		return false;
	}
	return true;
}

bool CFio::TriggerAdv(BYTE ch)
{
	if (!Validity(c_fdAdv)) {
		TRACK("FIO>ERR:adc incorrect!\n");
		return false;
	}

	// AD7994-1 conversion time is 2us
	BYTE chf = 0x10 << (ch & 3);
	BYTE cmd = 2;

	i2c_send_t hdr;
	hdr.slave.addr = 0x24;
	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr.len = 2;
	hdr.stop = 1;

	iov_t msg[3];
	SETIOV(&msg[0], &hdr, sizeof(hdr));
	SETIOV(&msg[1], &cmd, 1);
	SETIOV(&msg[2], &chf, 1);

	if (!Validity(devctlv(c_fdAdv, DCMD_I2C_SEND, 3, 0, msg, NULL, NULL))) {
		TRACK("FIO>ERR:adc trigger failed!\n");
		return false;
	}
	return true;
}

bool CFio::ReadAdv(BYTE ch, WORD* pw)
{
	if (!Validity(c_fdAdv)) {
		TRACK("FIO>ERR:adc incorrect!\n");
		return false;
	}

	BYTE chf = 0x10 << (ch & 3);

	i2c_sendrecv_t hdr;
	hdr.slave.addr = 0x24;
	hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	hdr.send_len = 1;
	hdr.recv_len = 2;
	hdr.stop = 1;

	iov_t smsg[2], rmsg[2];
	WORD res;
	SETIOV(&smsg[0], &hdr, sizeof(hdr));
	SETIOV(&smsg[1], &chf, 1);
	SETIOV(&rmsg[0], &hdr, sizeof(hdr));
	SETIOV(&rmsg[1], &res, 2);

	if (!Validity(devctlv(c_fdAdv, DCMD_I2C_SENDRECV, 2, 2, smsg, rmsg, NULL))) {
		TRACK("FIO>ERR:adc conversion failed!\n");
		return false;
	}
	*pw = ((res & 0xf) << 8) | ((res & 0xff00) >> 8);
	//		XWORD(res) & 0xfff;
	return true;
}

void CFio::SetChar(int x, BYTE ch)
{
	ASSERTP(c_pSpace);
	if (x >= LENGTH_FIOTEXT)	return;
	int ofs = c_iTextUnit[(c_cDioReg & (1 << IOB_OUTP)) ? 1 : 0][x];
	if (ofs >= LENGTH_FIOTEXT)	return;

	if (ch < 0x20 || ch > 126)	ch = 0;
	else	ch -= 0x20;
	ofs *= 5;
	int fi = 4;
	if (ofs & 1) {
		c_mtx.Lock();
		WSPACE(TEXT_BUFF + ofs - 1) &= 0xff;
		WSPACE(TEXT_BUFF + ofs - 1) |= ((WORD)(c_cFont[ch][fi --]) << 8);
		c_mtx.Unlock();
		++ ofs;
	}
	for (int n = 0; n < 2; n ++) {
		c_mtx.Lock();
		WSPACE(TEXT_BUFF + ofs) = (WORD)(c_cFont[ch][fi] | (c_cFont[ch][fi - 1] << 8));
		c_mtx.Unlock();
		ofs += 2;
		fi -= 2;
	}
	if (fi >= 0) {
		c_mtx.Lock();
		WSPACE(TEXT_BUFF + ofs) &= 0xff00;
		WSPACE(TEXT_BUFF + ofs) |= (WORD)c_cFont[ch][fi];
		c_mtx.Unlock();
	}
}

void CFio::SetChar(int x, BYTE* pCh)
{
	ASSERTP(c_pSpace);
	if (x >= LENGTH_FIOTEXT)	return;
	int ofs = c_iTextUnit[(c_cDioReg & (1 << IOB_OUTP)) ? 1 : 0][x];
	if (ofs >= LENGTH_FIOTEXT)	return;

	ofs *= 5;
	int fi = 4;
	if (ofs & 1) {
		c_mtx.Lock();
		WSPACE(TEXT_BUFF + ofs - 1) &= 0xff;
		WSPACE(TEXT_BUFF + ofs - 1) |= ((WORD)(pCh[fi --]) << 8);
		c_mtx.Unlock();
		++ ofs;
	}
	for (int n = 0; n < 2; n ++) {
		c_mtx.Lock();
		WSPACE(TEXT_BUFF + ofs) = (WORD)(pCh[fi] | pCh[fi - 1] << 8);
		c_mtx.Unlock();
		ofs += 2;
		fi -= 2;
	}
	if (fi >= 0) {
		c_mtx.Lock();
		WSPACE(TEXT_BUFF + ofs) &= 0xff00;
		WSPACE(TEXT_BUFF + ofs) |= (WORD)pCh[fi];
		c_mtx.Unlock();
	}
}

void CFio::SetBklight(WORD w)
{
	ASSERTP(c_pSpace);
	if (w > 12)	w = 12;
	w |= 0x40;
	WSPACE(TEXT_LUMI) = (w << 8) | w;
	WSPACE(TEXT_LUMI + 2) = 0x8080;
}

bool CFio::CheckVmeNode(enIOTYPE io, bool bMsg)
{
	WORD wBeen, wEmpty;
	for (wBeen = 0, wEmpty = 0; ; ) {
		if (Certifier(io, true, bMsg)) {
			++ wBeen;
			wEmpty = 0;
		}
		else {
			++ wEmpty;
			wBeen = 0;
		}
		if (wBeen >= 20 || wEmpty >= 20)	break;
		usleep(1000);
	}
	return wBeen >= 20 ? true : false;
}

WORD CFio::GetInnersw()
{
	ASSERTP(c_pSpace);
	c_mtx.Lock();
	WORD w = WSPACE(GINP_DIPSW) & 0xff;
	w |= (~WSPACE(GINP_BPLAN) << 8) & 0xff00;
	c_mtx.Unlock();
	return w;
}

void CFio::SetTrio(BYTE bi, bool bState)
{
	if (bi > 2)	return;

	if (bState)	c_cTrio |= (1 << bi);
	else	c_cTrio &= ~(1 << bi);
	ASSERTP(c_pSpace);
	WSPACE(GOUTP) = c_cTrio & 7;
}

bool CFio::ReadOutp(WORD* pw, WORD ofs)
{
	if (!Certifier(IOB_OUTP))	return false;

	*pw = WSPACE(ofs);
	return true;
}

bool CFio::SetOutp(WORD w, WORD ofs)
{
	c_mto.Lock();
	if (!Certifier(IOB_OUTP)) {
		c_mto.Unlock();
		return false;
	}
	WSPACE(ofs) = w;
	c_mto.Unlock();

	return true;
}

bool CFio::SetOutp(BYTE bi, bool bState, WORD ofs)
{
	if (bi >= 16)	return false;

	WORD w;
	c_mto.Lock();
	if (!ReadOutp(&w, ofs)) {
		c_mto.Unlock();
		return false;
	}
	if (bState)	w |= (1 << bi);
	else	w &= ~(1 << bi);
	WSPACE(ofs) = w;
	c_mto.Unlock();
	return true;
}

bool CFio::MaskOutp(WORD wOut, WORD wMask, WORD ofs)
{
	WORD w;
	c_mto.Lock();
	if (!ReadOutp(&w, ofs)) {
		c_mto.Unlock();
		return false;
	}
	w &= ~wMask;
	w |= wOut;
	WSPACE(ofs) = w;
	c_mto.Unlock();
	return true;
}

bool CFio::GetAdv(BYTE ch, WORD* pw)
{
	if (!TriggerAdv(ch))	return false;
	return ReadAdv(ch, pw);
}

WORD CFio::GetTacho()
{
	ASSERTP(c_pSpace);
	//WORD w = WSPACE(GINP_TACHO);
	//return w;
	++ c_wTachi;
	c_wTacho = WSPACE(GINP_TACHO);
	return c_wTacho;
}

DWORD CFio::GetPwm()
{
	ASSERTP(c_pSpace);
	_QUARTET quar;
	//LOCK();
	quar.w[0] = WSPACE(GINP_PWMPD);
	quar.w[1] = WSPACE(GINP_PWMDT);
	//UNLOCK();
	return quar.dw;
}

void CFio::SetText(int x, PSZ pszText)
{
	ASSERTP(c_pSpace);
	for (int n = 0; n < LENGTH_FIOTEXT; n ++) {
		BYTE ch = pszText[n];
		if (ch == 0)	break;
		c_cText[x] = ch;
		SetChar(x, ch);
		if (++ x >= LENGTH_FIOTEXT)	break;
	}
	WSPACE(TEXT_CTRL) = 3;
}

//void CFio::AlterChar(int x, BYTE ch)
//{
//	ASSERTP(c_pSpace);
//	if (x >= LENGTH_FIOTEXT)	return;
//	SetChar(x, ch);
//	WSPACE(TEXT_CTRL) = 3;
//}
//
//void CFio::RepChar(int x)
//{
//	ASSERTP(c_pSpace);
//	if (x >= LENGTH_FIOTEXT)	return;
//	SetChar(x, c_cText[x]);
//	WSPACE(TEXT_CTRL) = 3;
//}
//

// x = 0 ~ 4
void CFio::LampChar(int x)
{
	if (x >= (EPOS_MAX / 5))	return;

	BYTE ch = c_cText[x + 4];
	if (ch < 0x20 || ch > 126)	ch = 0;
	else	ch -= 0x20;

	BYTE lamp[5];
	for (int n = 0; n < 5; n ++) {
		lamp[n] = c_cFont[ch][n];
		if (c_cLamp[x] & (1 << n))	lamp[n] |= 1;
	}
	SetChar(x + 4, &lamp[0]);
	WSPACE(TEXT_CTRL) = 3;
}

// pos = 0 ~ 19
void CFio::Lamp(int pos, bool bState)
{
	ASSERTP(c_pSpace);
	if (pos >= 20)	return;

	if (bState) {
		c_wLampTimer[pos] = TPERIOD_ERRORLAMP;
		if (c_cLamp[pos / 5] & (1 << (pos % 5)))	return;
		c_cLamp[pos / 5] |= (1 << (pos % 5));
	}
	else {
		if (!(c_cLamp[pos / 5] & (1 << (pos % 5))))	return;
		c_cLamp[pos / 5] &= ~(1 << (pos % 5));
		c_wLampTimer[pos] = 0;
	}
	LampChar(pos / 5);
}

void CFio::GetInput(INCERT& in, bool bSide)
{
	if (c_bTwin) {
		in.wCertify = c_io.in[bSide ? 0 : 1].stab.wCertify;
		in.oct.qw = c_io.in[bSide ? 0 : 1].stab.oct.qw;
	}
	else {
		in.wCertify = c_io.in[0].stab.wCertify;
		in.oct.qw = c_io.in[0].stab.oct.qw;
	}
}

void CFio::GetOutput(OUTCERT& out)
{
	out.wCertify = c_io.out.stab.wCertify;
	out.duet.w = c_io.out.stab.duet.w;
}

void CFio::EnableOutput(bool bEn)
{
	// 191001
	//c_mtx.Lock();
	//WORD w = WSPACE(PAIR_CTRL);
	//if (bEn)	w |= (1 << PAIRCTRL_OUTMINE);
	//else	w &= ~(1 << PAIRCTRL_OUTMINE);
	//WSPACE(PAIR_CTRL) = w;
	WORD w = WSPACE(PAIR_CTRL);
	if ((bEn && !(w & (1 << PAIRCTRL_OUTMINE))) || (!bEn && (w & (1 << PAIRCTRL_OUTMINE)))) {
		if (bEn)	w |= (1 << PAIRCTRL_OUTMINE);
		else	w &= ~(1 << PAIRCTRL_OUTMINE);
		c_mtx.Lock();
		WSPACE(PAIR_CTRL) = w;
		c_mtx.Unlock();
	}

	// 171029
	if (c_bInLab) {
		if (bEn) {
			SetOutput(7, true);
			TRACK("FIO:enable output.\n");
		}
		else {
			SetOutput(7, false);
			TRACK("FIO:disable output.\n");
		}
	}
	//c_mtx.Unlock();		// 191001
}

void CFio::SetWatchdogCycle(WORD wCycle)
{
	c_mtx.Lock();
	WSPACE(XWATCH_SV) = wCycle;
	WSPACE(SWATCH_SV) = wCycle;
	c_mtx.Unlock();
}

WORD CFio::GetWatchdogCycle(bool bCh)
{
	c_mtx.Lock();
	WORD w = WSPACE(bCh ? XWATCH_SV : SWATCH_SV);
	c_mtx.Unlock();
	return w;
}

void CFio::EnableWatchdog(bool bFs, bool bXchg)
{
	c_mtx.Lock();
	WORD w = 0;
	if (bFs)	w |= (1 << WATCHCTRL_FSAFE);
	else	w |= (1 << WATCHCTRL_VARIA);
	if (bXchg)	w |= (1 << WATCHCTRL_CROSS);
	WSPACE(WATCH_CTRL) = w;
	c_mtx.Unlock();
}

void CFio::DisableWatchdog()
{
	c_mtx.Lock();
	WSPACE(WATCH_CTRL) = 0;
	c_mtx.Unlock();
	TRACK("FIO:disable watchdog.\n");
}

WORD CFio::TriggerWatchdog()
{
	c_mtx.Lock();
	WORD w = WSPACE(WATCH_CTRL);
	c_mtx.Unlock();
	return w;
}

WORD CFio::GetPairCondition()
{
	c_mtx.Lock();
	WORD w = WSPACE(PAIR_CTRL);
	c_mtx.Unlock();
	return w;
}

WORD CFio::GetGeneralReg()
{
	c_mtx.Lock();
	WORD w = WSPACE(GENREGB);
	c_mtx.Unlock();
	return w;
}

void CFio::SetGeneralReg(WORD w)
{
	c_mtx.Lock();
	WSPACE(GENREGB) = w;
	c_mtx.Unlock();
}

void CFio::ShowCounters()
{
	WORD w = WSPACE(XWATCH_SV);
	TRACK("FIO:xwatch sv %d\n", w);
	w = WSPACE(XWATCH_CV);
	TRACK("FIO:xwatch cv %d\n", w);
	w = WSPACE(FWATCH_SV);
	TRACK("FIO:fwatch sv %d\n", w);
	w = WSPACE(FWATCH_CV);
	TRACK("FIO:fwatch cv %d\n", w);
	w = WSPACE(SWATCH_SV);
	TRACK("FIO:swatch sv %d\n", w);
	w = WSPACE(SWATCH_CV);
	TRACK("FIO:swatch cv %d\n", w);
}

bool CFio::Initial(bool bRtd, bool bMsg)
{
	c_mtx.Lock();
	// 1st, extends io space
	c_pSpace = (PVOID)mmap_device_io(SIZE_BASE, ADDR_BASE);
	if (c_pSpace == MAP_FAILED) {
		c_mtx.Unlock();
		//SetText(4, (PSZ)"VMEF");		// 170808, 나타날 수가 없다...
		TRACK("FIO>ERR:%s()-space mapping of base address failed!(%s)\n", __FUNCTION__, strerror(errno));
		return false;
	}

	// 2nd, reset output and display
	WSPACE(EXTBANK_SEL) = VMEAMC_CODE | c_wAllocCode[IOB_OUTP];	// output board
	WSPACE(0) = 0;		// perhaps ...
	WORD* pdisp = (WORD*)c_pSpace + TEXT_BUFF;
	for (int n = 0; n < 40; n ++)	*pdisp = 0;
	WSPACE(TEXT_LUMI) = 0x4c4c;
	WSPACE(TEXT_LUMI + 2) = 0x8080;

	// 200809
	if (bRtd)	return true;

	SetText(0, (PSZ)"boot");
	SetText(4, (PSZ)"s.  ");
	TRACK("FIO:attribute 0x%04X\n", GetInnersw());

	// 3nd, scan vme bus
	WORD w = ~WSPACE(GINP_BPLAN) & 7;	// backplan setting value
	if (w & ((1 << GINPBPLAN_LIU1) | (1 << GINPBPLAN_LIU2)))	c_bTwin = true;
	if (w & (1 << GINPBPLAN_LIU1))	c_bMain = true;

	if (CheckVmeNode(IOB_OUTP, bMsg)) {
		// 171029
		c_io.out.stab.wCertify = c_wCertify[IOB_OUTP];
		c_cDioReg |= (1 << IOB_OUTP);
		WSPACE(EXTBANK_SEL) = 0;		// ????? SHOULD NOT CONVINCE !!!!!
		TRACK("FIO:found output board.\n");
	}
	else {
		// 170808
		c_mtx.Unlock();
		SetText(4, (PSZ)"OUTF");
		TRACK("FIO>ERR:can not found output board.\n");
		return false;
	}

	SetText(4, (PSZ)"s.. ");
	c_wBorn[0] = 0;
	if (CheckVmeNode(IOB_INPA, bMsg)) {
		// 171029
		c_io.in[0].stab.wCertify = c_wCertify[IOB_INPA];
		c_cDioReg |= (1 << IOB_INPA);
		for (int n = 0; n < 20; n ++) {
			w = WSPACE(6);		// read 48~63
			if (w != c_wBorn[0]) {
				c_wBorn[0] = w;
				n = 0;
			}
			usleep(1000);
		}
		TRACK("FIO:found input#1 board.(0x%04X)\n", c_wBorn[0]);
	}
	else {
		// 170808
		c_mtx.Unlock();
		SetText(4, (PSZ)"INBF");
		TRACK("FIO>ERR:can not found input#1 board.\n");
		return false;
	}

	if (c_bTwin) {
		TRACK("FIO:input#2 board reserved.\n");
		SetText(4, (PSZ)"s...");
		c_wBorn[1] = 0;
		if (CheckVmeNode(IOB_INPB, bMsg)) {
			// 171029
			c_io.in[1].stab.wCertify = c_wCertify[IOB_INPB];
			c_cDioReg |= (1 << IOB_INPB);
			//c_bTwin = true;
			for (int n = 0; n < 20; n ++) {
				w = WSPACE(6);
				if (w != c_wBorn[1]) {
					c_wBorn[1] = w;
					n = 0;
				}
				usleep(1000);
			}
			TRACK("FIO:found input#2 board.(0x%04X)\n", c_wBorn[1]);
		}
		else {
			c_mtx.Unlock();
			// 170808
			SetText(4, (PSZ)"EINF");
			TRACK("FIO>ERR:not found input#2 board!\n");
			return false;
		}
	}

	// 4th, adc file
	if (!Validity(c_fdAdv = open((char*)CURVEFILE, O_RDWR))) {
		c_mtx.Unlock();
		// 170808
		SetText(4, (PSZ)"ADCF");
		TRACK("FIO>ERR:%s()-can not open adc file!(%s)\n", __FUNCTION__, strerror(errno));
		return false;
	}
	else	TRACK("FIO:found adc file.\n");

	c_mtx.Unlock();
	return true;
}

BYTE CFio::Arteriam()
{
	BYTE res = Debounce();
	bool bChg;
	for (int n = 0; n < EPOS_MAX; n ++) {
		if ((n % 5) == 0)	bChg = false;
		if (c_wLampTimer[n] != 0 && -- c_wLampTimer[n] == 0) {
			c_cLamp[n / 5] &= ~(1 << (n % 5));
			bChg = true;
		}
		if (((n % 5) == 4) && bChg)	LampChar(n / 5);
	}
	return res;		// IO 보드의 구조를 리턴한다.
}

void CFio::Destroy()
{
	if (Validity(c_fdAdv))	close(c_fdAdv);
}

ENTRY_CONTAINER(CFio)
	//SCOOP(&c_cInpMons[0],	sizeof(BYTE) * 10,		"inFc")
	//SCOOP(&c_cLamp[0],		sizeof(BYTE) * 4,		"Lamp")
	SCOOP(&c_wTachi,		sizeof(WORD),			"tac")
	SCOOP(&c_wTacho,		sizeof(WORD),			"")
EXIT_CONTAINER()
