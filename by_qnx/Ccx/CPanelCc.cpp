/*
 * CPanelCc.cpp
 *
 *  Created on: 2010. 2010. 12. 9
 *      Author: Che
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <devctl.h>
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/procfs.h>

#include <Msg.h>
#include <Printz.h>

#include "CPanelCc.h"

CPanelCc::CPanelCc()
{
	c_pParent = NULL;
	c_nLamp = 0;
	c_nFlash = 0;
	for (UCURV n = 0; n < MAX_FLASHTIMER; n ++)
		c_uFlashTimer[n] = 0;
}

CPanelCc::CPanelCc(PVOID pVoid)
{
	c_pParent = pVoid;
	c_nLamp = 0;
	c_nFlash = 0;
	for (UCURV n = 0; n < MAX_FLASHTIMER; n ++)
		c_uFlashTimer[n] = 0;
}

CPanelCc::~CPanelCc()
{
}

const BYTE CPanelCc::c_nLampForm[] = {
	0x00, 0x40, 0x20, 0x60, 0x02, 0x42, 0x22, 0x62,
	0x01, 0x41, 0x21, 0x61, 0x03, 0x43, 0x23, 0x63
};

void CPanelCc::Interval()
{
	BYTE p2 = c_io.Read(2);
	p2 ^= 0x20;
	p2 &= 0x7f;
	c_io.Write(2, p2);
	if (c_nFlash != 0) {
		BYTE f = 1;
		for (UCURV n = 0; n < MAX_FLASHTIMER; n ++) {
			if (c_nFlash & f) {
				if (-- c_uFlashTimer[n] == 0) {
					c_uFlashTimer[n] = TIME_FLASH;
					c_nLamp ^= (c_nFlash & f);
				}
			}
			f <<= 1;
		}
		c_io.Write(3, ~c_nLamp & PANEL_LAMPS);
	}
}

BOOL CPanelCc::Initial()
{
	return (c_io.Open());
}

UCURV CPanelCc::Capture(PCARINFO pCar)
{
	if (!c_io.GetState())	return FROMPANELFAULT_DRIVER;

	pCar->dwThumbWheel = 0xffffffff;
	UCURV debw = 0;
	BOOL loop = TRUE;
	while (loop) {
		DWORD buf = c_io.ReadThumb();
		if (buf != pCar->dwThumbWheel) {
			pCar->dwThumbWheel = buf;
			debw = DEBOUNCE_PANEL;
		}
		else if (debw > 0 && -- debw == 0)	loop = FALSE;
		delay(1);
	}
	MSGLOG("[PANEL]capture %5X. from panel.\r\n", pCar->dwThumbWheel);

	BYTE pos = (BYTE)((pCar->dwThumbWheel >> 16) & 0xf);
	if (pos != 4 && pos != 6 && pos != 8)
		return FROMPANELFAULT_UNMATCHLENGTH;
	pCar->nLength = pos;

	pCar->nID = (BYTE)((pCar->dwThumbWheel >> 8) & 0xf);
	// Modified 2013/11/02
	//if (pCar->nID > 7)	return FROMPANELFAULT_UNMATCHCARID;
	if (pCar->nID >= pCar->nLength)	return FROMPANELFAULT_UNMATCHCARID;
	if (pCar->nID == 0)	pCar->nID = pCar->nLength - 1;
	else	-- pCar->nID;
	return FROMPANELFAULT_NON;
}

void CPanelCc::LampCtrl(BYTE nLamp, BYTE nState)
{
	BYTE f = 1;
	for (UCURV n = 0; n < MAX_FLASHTIMER; n ++) {
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

BOOL CPanelCc::CheckBCD(BYTE bcd)
{
	if ((bcd >> 4) > 9)		return FALSE;
	if ((bcd & 0xf) > 9)	return FALSE;
	return TRUE;
}

void CPanelCc::Lamp(BYTE lamp, BYTE cmd)
{
	if (cmd)	c_nLamp |= lamp;
	else	c_nLamp &= ~lamp;
	c_io.Write(3, ~c_nLamp & PANEL_LAMPS);
}

void CPanelCc::StepLamp(BYTE step)
{
	LampCtrl(PANEL_LAMPS, LAMP_OFF);
	LampCtrl(c_nLampForm[step], LAMP_ON);
}

void CPanelCc::AlarmLamp(BYTE alarm)
{
	LampCtrl(PANEL_LAMPS, LAMP_OFF);
	LampCtrl(c_nLampForm[alarm], LAMP_FLASH);
}

int CPanelCc::FindExec(PSZ fname)
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

int CPanelCc::RunExec(PSZ fname, PSZ args[])
{
	pid_t pid;
	if ((pid = spawnv(P_NOWAIT, fname, args)) < 0) {
		MSGLOG("[PANEL]ERROR:can not spawn %s(%s).\r\n",
				fname, strerror(errno));
		return -1;
	}
	return pid;
}

BOOL CPanelCc::RemoveExec(int pid)
{
	if (pid < 0)	return TRUE;
	if (kill(pid, SIGINT) == 0)	return TRUE;
	return FALSE;
}
