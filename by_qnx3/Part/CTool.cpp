/**
 * @file	CTool.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#define	__EXT_LF64SRC
#undef	__OFF_BITS__
#define	__OFF_BITS__	64

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

int CTool::GetFirstBit(QWORD qw, bool bState)
{
	for (int n = 0; n < 64; n ++) {
		bool bCur = (qw & (1 << n)) ? true : false;
		if (bCur == bState)	return n;
	}
	return -1;
}

int CTool::GetFirstBit(DWORD dw, bool bState)
{
	for (int n = 0; n < 16; n ++) {
		bool bCur = (dw & (1 << n)) ? true : false;
		if (bCur == bState)	return n;
	}
	return -1;
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

int CTool::FindStr(char* pDest, char* pSrc)
{
	if (pDest == NULL || pSrc == NULL)	return -1;

	int leng = (int)(strlen(pDest) - strlen(pSrc) + 1);
	for (int n = 0; n < leng; n ++) {
		if (!strncmp(pDest, pSrc, strlen(pSrc)))	return n;
		++ pDest;
	}
	return -1;
}

int CTool::FindOneOf(char* pDest, char* pSrc)
{
	if (pDest == NULL || pSrc == NULL)	return -1;

	char buf[8] = { 0 };
	while (*pSrc != '\0') {
		buf[0] = *pSrc;
		int n = FindStr(pDest, buf);
		if (n >= 0)	return n;
		++ pSrc;
	}
	return -1;
}

char* CTool::TrimRight(char* p)
{
	if (p == NULL)	return p;
	size_t len = strlen(p);
	while (p[len - 1] < '!' || p[len - 1] > '~') {
		p[len - 1] = '\0';
		if (-- len == 0)	break;
	}
	return p;
}

char* CTool::TrimLeft(char* p)
{
	while (*p < '!' || *p > '~') {
		if (*p == '\0')	return p;
		++ p;
	}
	return p;
}

char* CTool::Extract(char* pDest, char* pSrc, char* pConst)
{
	if (pDest == NULL || pSrc == NULL || pConst == NULL)	return NULL;

	int pos = FindStr(pSrc, pConst);
	if (pos >= 0) {
		pSrc += (pos + strlen(pConst));
		pSrc = TrimLeft(pSrc);
		pos = FindOneOf(pSrc, (char*)"\r\n; ");
		if (pos < 0)	pos = strlen(pSrc);
		strncpy(pDest, pSrc, pos);
		TrimRight(pDest);
		return pSrc;
	}
	return NULL;
}

/*int CTool::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority, PSZ szName)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("%s>ERR:can not set inherit!(%s)\n", szName, strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("%s>ERR:can not set sch param!(%s)\n", szName, strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("%s>ERR:can not set sch policy!(%s)\n", szName, strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("%s>ERR:can not create thread!(%s)\n", szName, strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

int CTool::MakeDirectory(PSZ pPath)
{
	DIR* pDir = opendir(pPath);
	if (pDir == NULL)	return (mkdir(pPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));
	else	closedir(pDir);
	return 0;
}

uint64_t CTool::GetFilesSize(PSZ pPath, char cSign)
{
	uint64_t tsize = 0;
	if (pPath == NULL || cSign == '\0')	return tsize;

	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign) {
				int64_t size = GetFileSize(pPath, pEntry->d_name);
				if (size >= 0)	tsize += (uint64_t)size;
			}
		}
		closedir(pDir);
	}
	return tsize;
}

int64_t CTool::GetFileSize(PSZ pPath, PSZ pFile)
{
	if (pPath == NULL || pFile == NULL)	return -1;

	char buf[256];
	sprintf(buf, "%s/%s", pPath, pFile);
	struct stat st;
	if (stat(buf, &st) >= 0)	return st.st_size;
	return -1;
}

int CTool::GetFileSize(PSZ pPath)
{
	if (pPath == NULL)	return -1;

	struct stat st;
	if (stat(pPath, &st) >= 0)	return (int)st.st_size;
	return -1;
}

int CTool::SearchFile(PSZ pPath, char cSign1, char cSign2)
{
	if (pPath == NULL || pPath[0] == '\0')	return 0;

	int iLength = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (cSign1 == '*' || pEntry->d_name[0] == cSign1 || (cSign2 != '\0' && pEntry->d_name[0] == cSign2))
				++ iLength;
		}
		closedir(pDir);
	}
	return iLength;
}

bool CTool::FindFile(char* pBuf, PSZ pPath, int iFileNameLength, char cSign1, char cSign2)
{
	if (pPath == NULL || pPath[0] == '\0')	return false;

	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if ((pEntry->d_name[0] == cSign1 || (cSign2 != '\0' && pEntry->d_name[0] == cSign2)) &&
				(int)strlen(pEntry->d_name) >= iFileNameLength) {
				strncpy(pBuf, pEntry->d_name, 256);
				closedir(pDir);
				return true;
			}
		}
		closedir(pDir);
	}
	return false;
}

bool CTool::RenameFile(PSZ pNew, PSZ pOld)
{
	if (pNew == NULL || pOld == NULL)	return false;

	if (rename(pOld, pNew) >= 0)	return true;
	return false;
}

bool CTool::MoveFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile)
{
	if (CopyFile(pDestPath, pDestFile, pSrcPath, pSrcFile)) {
		return DeleteFile(pSrcPath, pSrcFile);
	}
	return false;
}

bool CTool::CopyFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile)
{
	char dbuf[256];
	sprintf(dbuf, "%s/%s", pDestPath, pDestFile);
	char sbuf[256];
	sprintf(sbuf, "%s/%s", pSrcPath, pSrcFile);
	return CopyFile(dbuf, sbuf);
}

bool CTool::CopyFile(PSZ pDest, PSZ pSrc)
{
	int fs = open(pSrc, O_RDONLY);
	if (fs < 0)	return false;

	int fd = open(pDest, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		return false;
	}

	int resr = 0;
	char buf[8192];
	do {
		resr = read(fs, buf, 8192);
		if (resr < 0) {
			close(fs);
			close(fd);
			return false;
		}
		int resw = write(fd, buf, resr);
		if (resw < 0) {
			close(fs);
			close(fd);
			return false;
		}
	} while (resr >= 8192);
	close(fs);
	close(fd);
	return true;
}

int CTool::DeleteFile(PSZ pPath, PSZ pFile)
{
	char buf[256];
	sprintf(buf, "%s/%s", pPath, pFile);
	return (remove(buf));
}
*/
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
