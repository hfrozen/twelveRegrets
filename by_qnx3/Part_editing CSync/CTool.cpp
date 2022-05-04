/**
 * @file	CTool.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>
#include <process.h>
#include <fcntl.h>
#include <devctl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/procfs.h>
#include <sys/syspage.h>

#include "../Common/Refer.h"
#include "../Base/Track.h"
#include "CTool.h"

CTool::CTool()
{
}

CTool::~CTool()
{
}

DWORD CTool::ToBcd(DWORD dw)
{
	dw %= 100000000;
	return ((((DWORD)((WORD)ToBcd(WORD(dw / 10000)))) << 16) | ToBcd((WORD)(dw % 10000)));
}

WORD CTool::ToBcd(WORD w)
{
	w %= 10000;
	return (((WORD)((BYTE)ToBcd((BYTE)(w / 100))) << 8) | ToBcd((BYTE)(w % 100)));
}

WORD CTool::ToBcd(BYTE* p, int length)
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

BYTE CTool::ToBcd(BYTE c)
{
	c %= 100;
	return (((c / 10) << 4) | (c % 10));
}

WORD CTool::ToHex(WORD w)
{
	return (((WORD)((BYTE)ToHex((BYTE)(w >> 8)))) * 100 + ToHex((BYTE)(w & 0xff)));
}

BYTE CTool::ToHex(BYTE c)
{
	return (((c >> 4) & 0xf) * 10 + (c & 0xf));
}

DWORD CTool::ToBigEndian(DWORD dw)
{
	_QUARTET quar;
	quar.dw = dw;
	BYTE c = quar.c[0];
	quar.c[0] = quar.c[3];
	quar.c[3] = c;
	c = quar.c[1];
	quar.c[1] = quar.c[2];
	quar.c[2] = c;
	return quar.dw;
}

WORD CTool::ToBigEndian(WORD w)
{
	_DUET du;
	du.w = w;
	BYTE c = du.c[0];
	du.c[0] = du.c[1];
	du.c[1] = c;
	return du.w;
}

WORD CTool::BcdnXword(BYTE c0, BYTE c1)
{
	_DUET du;
	du.c[0] = ToBcd(c0);
	du.c[1] = ToBcd(c1);
	return du.w;
}

bool CTool::WithinTolerance(WORD w1, WORD w2, WORD tol)
{
	WORD diff = (WORD)abs((int)w1 - (int)w2);
	WORD max = w1 > w2 ? w1 : w2;
	if (diff <= (WORD)((double)max * (double)tol / 100.f))	return true;
	return false;
}

bool CTool::WithinTolerance(double db1, double db2, double tol)
{
	double diff = fabs(db1 - db2);
	double max = db1 > db2 ? db1 : db2;
	if (diff <= (max * tol / 100.f))	return true;
	return false;
}

bool CTool::WithinCriterion(double db1, double db2, double crit)
{
	double v, max;
	if (db1 > db2) {
		max = db1;
		v = db2;
	}
	else {
		max = db2;
		v = db1;
	}
	double min = max > crit ? max - crit : 0.f;
	max += crit;
	return (v <= max && v >= min) ? true : false;
}

double CTool::Deviround10(double db)
{
	double quotient = db / 10.f;
	double remainder = fmod(fabs(db), 10.f);
	if (remainder >= 5.f) {
		if (db > 0.f)	quotient += 1.0f;
		else if (db < 0.f)	quotient -= 1.0f;
	}
	return quotient;
}

BYTE CTool::GetBitNumbers(WORD w, bool bState)
{
	BYTE sum = 0;
	WORD bitw = 1;
	for (int n = 0; n < 16; n ++) {
		if (bState && (w & bitw))	++ sum;
		else if (!bState && !(w & bitw))	++ sum;
		bitw <<= 1;
	}
	return sum;
}

BYTE CTool::GetBitNumbers(BYTE c, bool bState)
{
	BYTE sum = 0;
	BYTE bitc = 1;
	for (int n = 0; n < 8; n ++) {
		if (bState && (c & bitc))	++ sum;
		else if (!bState && !(c & bitc))	++ sum;
		bitc <<= 1;
	}
	return sum;
}

int CTool::GetFirstBit(WORD w, bool bState)
{
	for (int n = 0; n < 16; n ++) {
		bool bCur = (w & (1 << n)) ? true : false;
		if (bCur == bState)	return n;
	}
	return -1;
}

int CTool::GetFirstBit(BYTE c, bool bState)
{
	for (int n = 0; n < 8; n ++) {
		bool bCur = (c & (1 << n)) ? true : false;
		if (bCur == bState)	return n;
	}
	return -1;
}

BYTE CTool::Backword(BYTE c)
{
	BYTE bc = 0;
	if (c & 1)	bc |= 0x80;
	if (c & 2)	bc |= 0x40;
	if (c & 4)	bc |= 0x20;
	if (c & 8)	bc |= 0x10;
	if (c & 0x10)	bc |= 8;
	if (c & 0x20)	bc |= 4;
	if (c & 0x40)	bc |= 2;
	if (c & 0x80)	bc |= 1;
	return bc;
}

void CTool::Rollf(const char* pMsg, ...)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	char* p = c_cBuf;
	sprintf(p, "%02d:%02d:%02d:%03d-", pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec / 1000);

	va_list vl;
	va_start(vl, pMsg);
	p += 13;
	vsprintf(p, pMsg, vl);
	va_end(vl);
	printf(c_cBuf);
}

int CTool::FindProc(PSZ procName)
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
		//if ((file = open(fn, O_RDONLY | O_NONBLOCK)) >= 0) {
		if (Validity(file = open(fn, O_RDONLY | O_NONBLOCK))) {
			int status = devctl(file, DCMD_PROC_MAPDEBUG_BASE,
					&dinfo, sizeof(dinfo), NULL);
			close(file);
			/*if (status == 0) {
				MSGLOG("[PANEL]Find %s\n", dinfo.info.path);
			}*/
			if (status == 0 && !strcmp(dinfo.info.path, (char*)procName)) {
				closedir(dirp);
				return pid;
			}
		}
	}
	closedir(dirp);
	return -1;
}

int CTool::RunProc(PSZ procName, PSZ args[])
{
	pid_t pid;
	if (!Validity(pid = spawnv(P_NOWAIT, (char*)procName, (char**)args))) {
		TRACK("TOOL>ERR:can not spawn %s(%s).\n", procName, strerror(errno));
		return -1;
	}
	return pid;
}

bool CTool::RemoveProc(int pid)
{
	if (!Validity(pid))	return true;
	if (kill(pid, SIGINT) == 0)	return true;
	return false;
}
