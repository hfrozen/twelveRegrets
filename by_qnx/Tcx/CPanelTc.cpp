/*
 * CPanelTc.cpp
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <devctl.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/procfs.h>
#include <dirent.h>
#include <printz.h>
#include "CPanelTc.h"

CPanelTc::CPanelTc()
{
	c_pParent = NULL;
	c_pTcRef = NULL;
	c_nLamp = 0;
	c_nFlash = 0;
	c_nState = 0;
	c_nStallCount = 0;
	for (UCURV n = 0; n < 8; n ++)
		c_uFlashTimer[n] = 0;
}

CPanelTc::CPanelTc(PVOID pVoid)
{
	c_pParent = pVoid;
	c_pTcRef = NULL;
	c_nLamp = 0;
	c_nFlash = 0;
	c_nState = 0;
	c_nStallCount = 0;
	for (UCURV n = 0; n < 8; n ++)
		c_uFlashTimer[n] = 0;
}

CPanelTc::~CPanelTc()
{
}

const BYTE CPanelTc::c_nLampForm[] = {
	0x00, 0x40, 0x20, 0x60, 0x02, 0x42, 0x22, 0x62,
	0x01, 0x41, 0x21, 0x61, 0x03, 0x43, 0x23, 0x63
};

void CPanelTc::Interval()
{
	BYTE p2 = c_io.Read(2);
	p2 ^= 0x60;
	p2 &= 0x7f;
	c_io.Write(2, p2);

	if (c_nState & STATE_NORMALTIMER) {
		p2 = c_io.Read(4);
		if (p2 & 0x80) {
			c_nLamp &= 0xfd;
			c_nState &= ~STATE_PARTNERLIVE;
		}
		else {
			c_nLamp |= 2;
			c_nState |= STATE_PARTNERLIVE;
		}
		if (c_nState & STATE_MAIN)	c_nLamp |= 1;
		else	c_nLamp &= 0xfe;
		if (++ c_nStallCount >= COUNT_STALL) {
			c_nStallCount = 0;
			c_nLamp ^= 0x20;
		}
		c_io.Write(3, ~c_nLamp);
	}
	else {
		if (c_nFlash != 0) {
			BYTE f = 1;
			for (UCURV n = 0; n < 8; n ++) {
				if (c_nFlash & f) {
					if (-- c_uFlashTimer[n] == 0) {
						c_uFlashTimer[n] = TIME_FLASH;
						c_nLamp ^= (c_nFlash & f);
					}
				}
				f <<= 1;
			}
			c_io.Write(3, ~c_nLamp);
		}
	}
}

BOOL CPanelTc::Initial(PTCREFERENCE pTcRef)
{
	c_pTcRef = pTcRef;
	if (c_io.Open()) {
		BYTE p2 = c_io.Read(2);
		p2 &= 0x7f;
		c_io.Write(2, p2);
		/*BYTE p3 = c_io.Read(3);
		p3 &= 0xfb;
		c_io.Write(3, p3);*/
		return TRUE;
	}
	return FALSE;
}

UCURV CPanelTc::Capture(DWORD* pDw)
{
	if (!c_io.GetState())	return FROMPANELFAULT_DRIVER;
	if (c_pTcRef == NULL)		return FROMPANELFAULT_REFERENCE;

	*pDw = 0xffffffff;
	UCURV debw = 0;
	BOOL bLoop = TRUE;
	while (bLoop) {
		DWORD buf = c_io.ReadThumb();
		if (buf != *pDw) {
			*pDw = buf;
			debw = DEBOUNCE_PANEL;
		}
		else if (debw > 0 && -- debw == 0)	bLoop = FALSE;
		delay(1);
	}
	MSGLOG("[PANEL]Capture %5X. from panel.\r\n", *pDw);

	BYTE pos = (BYTE)((*pDw >> 16) & 0xf);
	if (pos != 4 && pos != 6 && pos != 8)
		return FROMPANELFAULT_UNMATCHLENGTH;
	c_pTcRef->real.cf.nLength = pos;

	pos = (BYTE)((*pDw >> 8) & 0xff);
	if (!CheckBCD(pos))
		return FROMPANELFAULT_UNFORMATSERIALNO;
	c_pTcRef->real.cf.nSerialNo = (pos >> 4) * 10 + (pos & 0xf);
	pos = c_pTcRef->real.cf.nSerialNo % 10;
#ifdef	LIU_BYPANEL
	if (pos > 3)	return FROMPANELFAULT_UNFORMATSERIALNO;
#else
	if (pos > 1)	return FROMPANELFAULT_UNFORMATSERIALNO;
#endif

	pos = (BYTE)(*pDw & 0xff);
	if (!CheckBCD(pos))
		return FROMPANELFAULT_UNFORMATORGNO;
	c_pTcRef->real.cf.nOrgNo = (pos >> 4) * 10 + (pos & 0xf);
	return FROMPANELFAULT_NON;
}

int CPanelTc::Junction(WORD w)
{
#ifdef	LIU_BYPANEL
	BYTE pos = c_pTcRef->real.cf.nSerialNo % 10;
	BYTE where = 0;
	if (pos == 1 || pos == 2)	where = 1;
	w = 0x40;
	if (pos == 1 || pos == 3)	w = 0x20;
#else
	BYTE where = c_pTcRef->real.cf.nSerialNo % 10;
	if (where > 1)	return FROMPANELFAULT_UNFORMATSERIALNO;
	w &= 0x60;
#endif
	switch (w) {
	case 0x20 :
		// main
		if (where) {
			c_pTcRef->real.nAddr = 1;
			c_pTcRef->real.nObligation = OBLIGATION_HEAD | OBLIGATION_MAIN;
			c_pTcRef->real.nDuty = 0;
		}
		else {
			c_pTcRef->real.nAddr = 3;
			c_pTcRef->real.nObligation = OBLIGATION_MAIN;
			c_pTcRef->real.nDuty = 2;
		}
		break;
	case 0x40 :
		// reserve
		if (where) {
			c_pTcRef->real.nAddr = 2;
			c_pTcRef->real.nObligation = OBLIGATION_HEAD;
			c_pTcRef->real.nDuty = 1;
		}
		else {
			c_pTcRef->real.nAddr = 4;
			c_pTcRef->real.nObligation = 0;
			c_pTcRef->real.nDuty = 3;
		}
		break;
	default :
		return FROMPANELFAULT_UNMATCHID;
	}
	//Active(c_pTcRef->real.nDuty);
	return FROMPANELFAULT_NON;
}

/*void CPanelTc::Active(BYTE id)
{
	BYTE p3 = c_io.Read(3);
	if (id == 0 || id == 2)	p3 |= 4;
	else	p3 &= 0xfb;
	c_io.Write(3, p3);
}
*/
void CPanelTc::LampCtrl(BYTE nLamp, BYTE nState)
{
	BYTE f = 1;
	for (UCURV n = 0; n < 8; n ++) {
		if (nLamp & f) {
			if (nState == LAMP_FLASH) {
				c_uFlashTimer[n] = TIME_FLASH;
				c_nFlash = nLamp;
				Lamp(nLamp, TRUE);
			}
			else {
				c_uFlashTimer[n] = 0;
				c_nFlash &= ~f;
				Lamp(nLamp, nState == LAMP_ON ? TRUE : FALSE);
			}
		}
		f <<= 1;
	}
}

BOOL CPanelTc::CheckBCD(BYTE bcd)
{
	if ((bcd >> 4) > 9)		return FALSE;
	if ((bcd & 0xf) > 9)	return FALSE;
	return TRUE;
}

void CPanelTc::Lamp(BYTE lamp, BYTE cmd)
{
	if (cmd)	c_nLamp |= lamp;
	else	c_nLamp &= ~lamp;
	c_io.Write(3, ~c_nLamp);
}

void CPanelTc::StepLamp(BYTE step)
{
	LampCtrl(PANEL_LAMPS, LAMP_OFF);
	LampCtrl(c_nLampForm[step], LAMP_ON);
}

void CPanelTc::AlarmLamp(BYTE alarm)
{
	LampCtrl(PANEL_LAMPS, LAMP_OFF);
	LampCtrl(c_nLampForm[alarm], LAMP_FLASH);
}

int CPanelTc::FindExec(PSZ fname)
{
	struct dinfo_s {
		procfs_debuginfo	info;
		char	pathbuf[1024];
	};
	//struct stat fs;
	//char path[256];
	char fn[256];

	DIR* dirp = opendir("/proc");
	struct dirent* ent;
	while ((ent = readdir(dirp)) != NULL) {
		//lstat(fn, &fs);
		int pid = atoi(ent->d_name);
		sprintf(fn, "/proc/%d/as", pid);
		if (pid <= 0)	continue;

		struct dinfo_s dinfo;
		int file;
		if ((file = open(fn, O_RDONLY | O_NONBLOCK)) >= 0) {
			int status = devctl(file, DCMD_PROC_MAPDEBUG_BASE,
					&dinfo, sizeof(dinfo), NULL);
			close(file);
			/*if (status == 0) {
				MSGLOG("[PANEL]Find %s\r\n", dinfo.info.path);
			}*/
			if (status == 0 && !strcmp(dinfo.info.path, fname)) {
				closedir(dirp);
				return pid;
			}
		}
	}
	closedir(dirp);
	return -1;
}

int CPanelTc::RunExec(PSZ fname, PSZ args[])
{
	pid_t pid;
	if ((pid = spawnv(P_NOWAIT, fname, args)) < 0) {
		MSGLOG("[PANEL]ERROR:Can not spawn %s(%s).\r\n",
				fname, strerror(errno));
		return -1;
	}
	return pid;
}

BOOL CPanelTc::RemoveExec(int pid)
{
	if (pid < 0)	return TRUE;
	if (kill(pid, SIGINT) == 0)	return TRUE;
	return FALSE;
}

void CPanelTc::SetMain(BOOL cmd)
{
	if (cmd)	c_nState |= STATE_MAIN;
	else	c_nState &= ~STATE_MAIN;
}

void CPanelTc::SetTimer(BOOL cmd)
{
	if (cmd)	c_nState |= STATE_NORMALTIMER;
	else	c_nState &= ~STATE_NORMALTIMER;
}

BOOL CPanelTc::GetPartner()
{
	if (c_nState & STATE_PARTNERLIVE)	return TRUE;
	return FALSE;
}
