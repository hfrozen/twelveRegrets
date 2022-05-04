/*
 * CTidy.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#define	__EXT_LF64SRC
#undef	__OFF_BITS__
#define	__OFF_BITS__	64

#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/syspage.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/neutrino.h>

#include "Track.h"
#include "Prefix.h"
#include "slight.h"
#include "Insp.h"
#include "CBand.h"
#include "CFlux.h"

#include "CTidy.h"

#define	_OLDTROUBLE_ALL_

CTidy::CTidy()
{
	c_pParent = NULL;
	c_bDebug = false;
	c_bCancel = false;
	memset(&c_entrys, 0, sizeof(ENTRYSHAPE) * MAX_ENTRY);
	memset(&c_fl, 0, sizeof(FILELIST));
	c_wOldListFullMap = 0;
	memset(&c_oldt, 0, sizeof(OLDTROUBLES));
	memset(&c_curf, 0, sizeof(FILEGUIDE));
	c_curf.iSfd = c_curf.iDfd = INVALID_HANDLE;
	memset(&c_curd, 0, sizeof(DIRGUIDE));
	memset(c_szDestTrace, 0, SIZE_DEFAULTPATH);
}

CTidy::~CTidy()
{
	CloseCurFile();
}

void CTidy::CloseCurFile()
{
	if (Validity(c_curf.iSfd)) {
		close(c_curf.iSfd);
		c_curf.iSfd = INVALID_HANDLE;
	}
	if (Validity(c_curf.iDfd)) {
		close(c_curf.iDfd);
		c_curf.iDfd = INVALID_HANDLE;
	}
}

int64_t CTidy::GetSpace(const char* pPath, bool bSpace)
{
	// bSpace = 1 -> total, = 0 -> free
	struct statvfs64 st;
	int64_t size = 0;
	if (!Validity(statvfs64(pPath, &st)))
		TRACK("MM>ERR:can not found %s!(%s)\n", pPath, strerror(errno));
	else {
		if (bSpace)	size = st.f_bsize * st.f_blocks;
		else	size = st.f_bsize * st.f_bfree;
	}
	return size;
}

// 기능 : 폴더 및 파일 지우기 위한 기준 날짜 구하기
DWORD CTidy::GetDateIndex(bool bPrev, int delMon)
{
	int difMon = 0;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	BYTE year = (BYTE)((pTm->tm_year + 1900) % 100);
	BYTE mon = pTm->tm_mon + 1;
	BYTE day = pTm->tm_mday;
	if (bPrev && (delMon < 13)) {
		difMon = mon-delMon ;
		if (difMon > 0)	mon = difMon;	// 같은 해이면 달 수만 줄이기
		else {							// 이전 해이면 해당 월 구하기
			-- year;
			mon = difMon + 12;
		}
	}
	return ((DWORD)year * 10000 + (DWORD)mon * 100 + (DWORD)day);
}

// 기능 : 폴더 및 파일 지우기 위한 기준 날짜 구하기
DWORD CTidy::GetDateIndex(bool bPrev)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	BYTE year = (BYTE)((pTm->tm_year + 1900) % 100);
	BYTE mon = pTm->tm_mon + 1;
	BYTE day = pTm->tm_mday;
	if (bPrev) {
		if (mon > 1)	-- mon;		// 2월 이상이면 한달을 뺌, delete_month = current_month - 1;
		else {
			mon = 12;				// 1월 이면 작년 12월로 지정
			-- year;
		}
	}
	return ((DWORD)year * 10000 + (DWORD)mon * 100 + (DWORD)day);
}

DWORD CTidy::GetTimeIndex()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	return ((DWORD)pTm->tm_hour * 10000 + (DWORD)pTm->tm_min * 100 + (DWORD)pTm->tm_sec);
}

int64_t CTidy::GetFileSize(const char* pPath)
{
	struct stat st;
	if (stat(pPath, &st) >= 0)	return ((int64_t)st.st_size);
	return 0;
}

int CTidy::GetFilesLength(const char* pPath, char cSign)
{
	int leng = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign)	++ leng;
		}
		closedir(pDir);
	}
	return leng;
}

int64_t CTidy::GetDirSize(const char* pPath)
{
	_TWATCHCLICK();
	char buf[256];
	int64_t sum = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			sprintf(buf, "%s/%s", pPath, pEntry->d_name);
			sum += GetFileSize(buf);
		}
		closedir(pDir);
	}
	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("MM:GetDirSize() %f sec\n", __sec_);

	return sum;
}

int CTidy::DeleteDir(const char* pPath)
{
	char buf[256];
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			sprintf(buf, "%s/%s", pPath, pEntry->d_name);
			if (remove(buf) != 0)	return BULKRES_ERRCANNOTREMOVEFILE;
			if (c_bDebug)	TRACK("MM:delete file.(%s)\n", buf);
		}
		closedir(pDir);
		if (remove(pPath) != 0)	return BULKRES_ERRCANNOTREMOVEFILE;
		if (c_bDebug)	TRACK("MM:delete dir.(%s)\n", pPath);
	}
	return BULKRES_ERRNON;
}

void CTidy::DeleteFiles(const char* pPath, char cSign)
{
	char buf[256];
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign) {
				sprintf(buf, "%s/%s", pPath, pEntry->d_name);
				remove(buf);
			}
		}
		closedir(pDir);
	}
}

// 기능 : 오래된 폴더, 파일 지우기, 이번달 기준 month전 월 해당월의 데이터 지우기
void CTidy::DeletePrevCpm(const char* pPath, int month)
{
	DWORD dwDate ;

	dwDate = GetDateIndex(true, month);		// 현재 날짜 구하기
	if (c_bDebug)	TRACK("MM:delete before %06d.\n", dwDate);

	char buf[256];
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			DWORD dw = 0;
			if (pEntry->d_name[0] == LOGDIR_SIGN &&
				(pEntry->d_name[1] >= '0' && pEntry->d_name[1] <= '9') &&
				strlen(pEntry->d_name) <= 7) {
				strcpy(buf, pEntry->d_name);
				dw = (DWORD)atol(&buf[1]);
			}
			if (dw < dwDate) {
				sprintf(buf, "%s/%s", pPath, pEntry->d_name);
				DeleteDir(buf);
			}
		}
	}
}

void CTidy::DeletePrev6(const char* pPath)
{
	DWORD dwDate = GetDateIndex(true);
	if (c_bDebug)	TRACK("MM:delete before %06d.\n", dwDate);

	char buf[256];
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			DWORD dw = 0;
			if (pEntry->d_name[0] == LOGDIR_SIGN &&
				(pEntry->d_name[1] >= '0' && pEntry->d_name[1] <= '9') &&
				strlen(pEntry->d_name) <= 7) {
				strncpy(buf, pEntry->d_name, 256);
				dw = (DWORD)atol(&buf[1]);
			}
			if (dw < dwDate) {
				sprintf(buf, "%s/%s", pPath, pEntry->d_name);
				DeleteDir(buf);
			}
		}
	}
}

DWORD CTidy::GetLately6(const char* pPath, char cSign, DWORD dwLimit)
{
	//_TWATCHCLICK();
	char buf[256];
	DWORD dwLately = 0;

	if (dwLimit == 0)	dwLimit = 1000000;	// greater then 999999
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign) {
				strncpy(buf, pEntry->d_name, 256);
				DWORD dw = (DWORD)atol(&buf[1]);
				if (dw < dwLimit && dw > dwLately)	dwLately = dw;
			}
		}
		closedir(pDir);
	}
	//_TWATCHMEASURED();
	//if (c_bDebug)	TRACK("MM:GetLately6() %f sec\n", __sec_);
	return dwLately;
}

// 엔트리된 파일의 갯수를 리턴한다.
int CTidy::SortDirEntry6(const char* pPath, char cSign, int iLength)
{
	_TWATCHCLICK();
	char buf[256];
	DWORD dwIndex = 0;
	int leng = 0;
	while (leng < MAX_ENTRY) {
		dwIndex = GetLately6(pPath, cSign, dwIndex);
		if (dwIndex == 0)	break;
		sprintf(buf, "%s/%c%06d", pPath, cSign, dwIndex);
		strncpy(&c_entrys[leng].szName[0], &buf[strlen(pPath) + 1], SIZE_ENTRYNAME);		// path는 빼고...
		if (cSign == LOGDIR_SIGN)	c_entrys[leng].dwSize = (DWORD)GetDirSize(buf);
		else	c_entrys[leng].dwSize = (DWORD)GetFileSize(buf);
		if (++ leng >= iLength)	break;
	}
	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("MM:SortDirEntry6() %f sec\n", __sec_);
	return leng;
}

void CTidy::MakeLogbookList(const char* pPath)
{
	_TWATCHCLICK();
	CFlux* pFlux = (CFlux*)c_pParent;
	pFlux->FileWorkFlag(true);

	memset(&c_fl, 0, sizeof(FILELIST));
	char buf[256];
	sprintf(buf, "%s/%s", ARCHIVE_PATH, pPath);
	DIR* pDir = opendir(buf);
	if (pDir != NULL) {
		struct dirent* pEntry;
		char fullpath[256];
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			strncpy(c_fl.list[c_fl.wLength].szName, pEntry->d_name, SIZE_ENTRYNAME);
			strncpy(fullpath, buf, 256);
			strncat(fullpath, "/", 255);
			strncat(fullpath, c_fl.list[c_fl.wLength].szName, 255 - strlen(c_fl.list[c_fl.wLength].szName));
			if (c_bDebug)	TRACK("MM:list %s\n", fullpath);
			//c_fl.list[c_fl.wLength].dwSize = (DWORD)GetFileSize(fullpath);
			++ c_fl.wLength;
		}
		closedir(pDir);
	}
	pFlux->FileWorkFlag(false);
	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("MM:MakeLogbookList() %d %f sec\n", c_fl.wLength, __sec_);
}

DWORD CTidy::MakeLogbookEntry(const char* pPath, DWORD dwTotal)
{
	CFlux* pFlux = (CFlux*)c_pParent;
	pFlux->FileWorkFlag(true);
	memset(&c_entrys[0], 0, sizeof(ENTRYSHAPE) * MAX_ENTRY);
	char buf[256];
	if (pPath == NULL)	sprintf(buf, "%s", ARCHIVE_PATH);
	else	sprintf(buf, "%s/%s", ARCHIVE_PATH, pPath);
	//if (c_bDebug)	TRACK("MM:make entry of %s\n", buf);
	DWORD dwLeng = (DWORD)SortDirEntry6(buf, LOGDIR_SIGN, (int)dwTotal);	// 패스없이 파일(디렉토리) 이름만 기록한다.
	pFlux->FileWorkFlag(false);
	return dwLeng;
}

int CTidy::DeleteOld12(const char* pPath, char cSign)
{
	char buf[256];

	QWORD qw = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		QWORD min = 999999999999;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign) {
				strncpy(buf, pEntry->d_name, 256);
				qw = (QWORD)atoll(&buf[1]);
				if (min > qw)	min = qw;
			}
		}
		closedir(pDir);
		if (qw != 0) {
			sprintf(buf, "%s/%c%012llu", pPath, cSign, qw);
			if (remove(buf) == 0)	return BULKRES_ERRCANNOTREMOVEFILE;
			if (c_bDebug)	TRACK("MM:delete file.(%s)\n", buf);
		}
	}
	return BULKRES_ERRNON;
}

void CTidy::LimitPrev12(const char* pPath, int iLimit, char cSign)
{
	while (GetFilesLength(pPath, cSign) > iLimit) {
		if (DeleteOld12(pPath, cSign) != BULKRES_ERRNON)	break;
	}
}

QWORD CTidy::GetLately12(const char* pPath, char cSign, QWORD qwLimit)
{
	char buf[256];
	QWORD qwLately = 0;

	if (qwLimit == 0)	qwLimit = 1000000000000;	// greater then 999999999999
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			if (pEntry->d_name[0] == cSign) {
				strncpy(buf, pEntry->d_name, 256);
				QWORD qw = (QWORD)atoll(&buf[1]);
				if (qw < qwLimit && qw > qwLately)	qwLately = qw;
			}
		}
		closedir(pDir);
	}
	return qwLately;
}

// 엔트리된 파일의 갯수를 리턴한다.
int CTidy::SortDirEntry12(const char* pPath, char cSign, int iSi, int iLength)
{
	char buf[256];
	QWORD qwIndex = 0;
	int leng = 0;
	while (leng < MAX_ENTRY) {
		qwIndex = GetLately12(pPath, cSign, qwIndex);
		if (qwIndex == 0)	break;
		sprintf(buf, "%s/%c%012llu", pPath, cSign, qwIndex);
		strncpy(&c_entrys[iSi].szName[0], &buf[strlen(pPath) + 1], SIZE_ENTRYNAME);			// path는 빼고...
		c_entrys[iSi ++].dwSize = (DWORD)GetFileSize(buf);
		//if (c_bDebug)	TRACK("MM:lista%d %s(%d)\n", leng, c_entrys[si].szName, c_entrys[si].dwSize);
		if (++ leng >= iLength)	break;
	}
	return leng;
}

DWORD CTidy::MakeInspectEntry(DWORD dwTotal)
{
	memset(&c_entrys[0], 0, sizeof(ENTRYSHAPE) * MAX_ENTRY);
	DWORD dw = (DWORD)(SortDirEntry12(INSPECT_PATH, PDTINSPECT_SIGN, 0, (int)(dwTotal & 0xff)) & 0xff);
	DWORD dw1 = (DWORD)(SortDirEntry12(INSPECT_PATH, DAILYINSPECT_SIGN, (int)dw, (int)((dwTotal >> 8) & 0xff)) & 0xff);
	DWORD dw2 = (DWORD)(SortDirEntry12(INSPECT_PATH, MONTHLYINSPECT_SIGN, (int)(dw + dw1), (int)((dwTotal >> 16) & 0xff)) & 0xff);
	return (dw | (dw1 << 8) | (dw2 << 16));
}

bool CTidy::MakeTraceDirOnUsb()
{
	CFlux* pFlux = (CFlux*)c_pParent;
	PBULKPACK pBulk = pFlux->GetBulk();

	sprintf(c_szDestTrace, "%s/n%04x", USB_PATH, pBulk->lbHead.wProperNo);		// /dos/c/n2XYY
	int res = mkdir(c_szDestTrace, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make %s!(%s)\n", c_szDestTrace, strerror(errno));
		return false;
	}
	TRACK("MM:make %s\n", c_szDestTrace);

	strcat(c_szDestTrace, USBTRACE_NAME);										// /dos/c/n2XYY/trace
	res = mkdir(c_szDestTrace, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make %s!(%s)\n", c_szDestTrace, strerror(errno));
		return false;
	}
	TRACK("MM:make %s\n", c_szDestTrace);
	return true;
}

WORD CTidy::ReadFile(char* pBuf, WORD wLength)
{
	CFlux* pFlux = (CFlux*)c_pParent;
	PBULKPACK pBulk = pFlux->GetBulk();
	_TWATCHCLICK();
	if (!Validity(c_curf.iSfd)) {
		c_curf.iSfd = open(c_curf.szSrc, O_RDONLY);
		if (c_curf.iSfd < 0) {
			TRACK("MM>ERR:ropen!(%s)\n", strerror(errno));
			return 0;
		}
	}
	int size = read(c_curf.iSfd, pBuf, wLength);
	if (size < 0) {
		close(c_curf.iSfd);
		c_curf.iSfd = INVALID_HANDLE;
		TRACK("MM>ERR:read!(%s)\n", strerror(errno));
		return 0;
	}
	if (size < (int)wLength) {
		close(c_curf.iSfd);
		c_curf.iSfd = INVALID_HANDLE;
	}

	_TWATCHMEASURE(WORD, pBulk->fts.rd.wCur, 3);
	if (pBulk->fts.rd.wCur > pBulk->fts.rd.wMax)	pBulk->fts.rd.wMax = pBulk->fts.rd.wCur;
	return (WORD)size;
}

int CTidy::CopyFile(const char* pDest, const char* pSrc, bool bPermitCancel)
{
	CFlux* pFlux = (CFlux*)c_pParent;
	PBULKPACK pBulk = pFlux->GetBulk();

	int fs = open(pSrc, O_RDONLY);
	if (fs < 0) {
		if (c_bDebug)	TRACK("MM:%s() can not open %s!\n", __FUNCTION__, pSrc);
		return BULKRES_ERRCANNOTFOUNDSRC;
	}
	int fd = open(pDest, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		if (c_bDebug)	TRACK("MM:%s() can not open %s!\n", __FUNCTION__, pDest);
		return BULKRES_ERRCANNOTOPENDEST;
	}

	int ret = 0;
	char buf[4096];
	do {
		ret = read(fs, buf, 4096);
		if (ret < 0) {
			close(fs);
			close(fd);
			if (c_bDebug)	TRACK("MM:%s() can not read %s!\n", __FUNCTION__, pSrc);
			return BULKRES_ERRCANNOTREADSRC;
		}
		int res = write(fd, buf, ret);
		if (res < 0) {
			close(fs);
			close(fd);
			if (c_bDebug)	TRACK("MM:%s() can not write %s!\n", __FUNCTION__, pDest);
			return BULKRES_ERRCANNOTWRITEDEST;
		}
		pBulk->uiCurSize += ret;
	} while (ret >= 4096 && (!c_bCancel || !bPermitCancel));
	close(fs);
	close(fd);
	return BULKRES_ERRNON;
}

void CTidy::CopyLogbook(PRTDSPADE pSpd, char* pDestPath, char* pLogDir)
{
	sprintf(c_curf.szSrc, "%s/%s/%s", ARCHIVE_PATH, pLogDir, pSpd->hoe.h.f.szName);
	sprintf(c_curf.szDest, "%s/%s", pDestPath, pSpd->hoe.h.f.szName);
	if (c_bDebug)	TRACK("MM:copy %s to %s\n", c_curf.szSrc, c_curf.szDest);
	c_bCancel = false;

	CFlux* pFlux = (CFlux*)c_pParent;
	pFlux->FileWorkFlag(true);
	CopyFile(c_curf.szDest, c_curf.szSrc, true);
	pFlux->FileWorkFlag(false);
}

void CTidy::ExcerptFile(PRTDSPADE pSpd, char* pLogDir)
{
	c_bCancel = false;
	//DWORD offset;
	WORD wLength;
	char* pBuf;
	if (pSpd->dwIndex == 0) {
		if (Validity(c_curf.iSfd)) {
			close(c_curf.iSfd);
			c_curf.iSfd = INVALID_HANDLE;
		}
		if (pSpd->hoe.h.f.szName[0] == LOGBOOK_SIGN || pSpd->hoe.h.f.szName[0] == TROUBLE_SIGN)
			sprintf(c_curf.szSrc, "%s/%s/%s", ARCHIVE_PATH, pLogDir, pSpd->hoe.h.f.szName);
		else	sprintf(c_curf.szSrc, "%s/%s", INSPECT_PATH, pSpd->hoe.h.f.szName);
		//offset = 0;
		wLength = SIZE_HEADCONTEXT;
		pBuf = (char*)&pSpd->hoe.h.c[0];
		pSpd->dwTotal |= (1 << BULKFID_HEAD);
	}
	else {
		//offset = (pSpd->dwIndex - 1) * SIZE_RTDCONTEXT + SIZE_HEADCONTEXT;
		wLength = SIZE_RTDCONTEXT;
		pBuf = (char*)&pSpd->hoe.c[0];
	}
	pSpd->wLength = ReadFile(pBuf, wLength);
#if	defined(_DEBUG_DOWNLOAD_)
	if (c_bDebug)	TRACK("MM:read %s(%d - %d).\n", c_curf.szSrc, wLength, pSpd->wLength);		// ?????
#endif
	if (pSpd->wLength < wLength)	pSpd->dwTotal |= (1 << BULKFID_FILEEND);
}

int CTidy::BindFileToUsb(PRTDSPADE pSpd, char* pPath)
{
	int offset = 0;
	if (pSpd->dwTotal & (1 << BULKFID_HEAD)) {
		if (Validity(c_curf.iDfd)) {
			close(c_curf.iDfd);
			c_curf.iDfd = INVALID_HANDLE;
		}
		if (pPath == NULL || *pPath == '\0') {
			TRACK("MM>ERR:target name!\n");
			return -1;
		}
		sprintf(c_curf.szDest, "%s/%s", pPath, pSpd->hoe.h.f.szName);
		c_curf.iDfd = open(c_curf.szDest, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (c_curf.iDfd < 0) {
			TRACK("MM>ERR:wopen!(%s)\n", strerror(errno));
			return -1;
		}
		offset = sizeof(ENTRYSHAPE) + sizeof(BYTE);
	}
	CFlux* pFlux = (CFlux*)c_pParent;
	PBULKPACK pBulk = pFlux->GetBulk();
	_TWATCHCLICK();
	int size = write(c_curf.iDfd, &pSpd->hoe.c[offset], pSpd->wLength);
	if (size < 0) {
		close(c_curf.iDfd);
		c_curf.iDfd = INVALID_HANDLE;
		TRACK("MM>ERR:write!(%s)\n", strerror(errno));
		return -1;
	}
	else if (pSpd->dwTotal & (1 << BULKFID_FILEEND)) {
		close(c_curf.iDfd);
		c_curf.iDfd = INVALID_HANDLE;
	}
	_TWATCHMEASURE(WORD, pBulk->fts.wr.wCur, 3);
	if (pBulk->fts.wr.wCur > pBulk->fts.wr.wMax)	pBulk->fts.wr.wMax = pBulk->fts.wr.wCur;
#if	defined(_DEBUG_DOWNLOAD_)
	if (c_bDebug)	TRACK("MM:write %s(%d).\n", c_curf.szDest, pSpd->wLength);	// ?????
#endif
	//if (size != (int)pSpd->wLength) {
	//	TRACK("MM>ERR:write to usb!(%d:%d-%s)\n", size, pSpd->wLength, strerror(errno));
	//	return BULKRES_ERRCANNOTWRITEDEST;
	//}
	return BULKRES_ERRNON;
}

void CTidy::AlignOldTroubleFile(const char* pFile)
{
	int64_t size = GetFileSize(pFile);
	if (size == 0)	return;

	char* pBuf = (char*)malloc(size);
	if (pBuf == NULL) {
		TRACK("MM>ERR:malloc!\n");
		return;
	}

	FILE* fp;
	size_t leng = 0;
	//_TWATCHCLICK();
	if ((fp = fopen(pFile, "r")) != NULL) {
		leng = fread(pBuf, 1, size, fp);
		fclose(fp);
		if (leng != size)	TRACK("MM>ERR:file load!(%d - %s)\n", leng, strerror(errno));
		//else	TRACK("MM:file load %d.\n", leng);
	}
	if (leng < sizeof(TROUBLEARCH)) {
		free(pBuf);
		return;
	}
	//_TWATCHMEASURED();
	//TRACK("mm:read %s. %f sec\n", pFile, __sec_);

	int li = leng / 8;
	if (!(leng % 8))	-- li;
	memset(&c_abacList, 0, sizeof(ABACUSLIST));

	do {
		TROUBLEARCH ta;
		memcpy(&ta, pBuf + li * sizeof(TROUBLEARCH), sizeof(TROUBLEARCH));
		if (ta.b.cid >= DEFAULT_CARLENGTH || ta.b.code > 999 || ta.b.type < TROUBLE_STAT) {
			//TRACK("MM:wrong! %d %d.\n", ta.b.cid, ta.b.code);
			-- li;
			continue;
		}
		int cid = ta.b.cid + 1;		// 0~9 -> 1~10
		if ((ta.b.code < TCODE_ONLYTU) || (ta.b.code >= 660 && ta.b.code < 850))
			cid = (cid < 2) ? 0 : 11;		// 0, 11		// TU/CU 객차 고장을 분리..
		if (c_oldt.blk[cid].wLength < MAX_OLDTROUBLE) {
			if (ta.b.down) {
				c_abacList[ta.b.code].wCarBitmap = (1 << ta.b.cid);
				c_abacList[ta.b.code].dhour = ta.b.dhour;
				c_abacList[ta.b.code].dmin = ta.b.dmin;
				c_abacList[ta.b.code].dsec = ta.b.dsec;
			}
			else {
				// 발생 기록이면 따로 보관된 소거 기록에서 소거 기록을 찾아본다.
				if (c_abacList[ta.b.code].wCarBitmap & (1 << ta.b.cid)) {
					c_abacList[ta.b.code].wCarBitmap &= ~(1 << ta.b.cid);
					ta.b.dhour = c_abacList[ta.b.code].dhour;
					ta.b.dmin = c_abacList[ta.b.code].dmin;
					ta.b.dsec = c_abacList[ta.b.code].dsec;
				}
				memcpy(&c_oldt.blk[cid].list[c_oldt.blk[cid].wLength ++], &ta, sizeof(TROUBLEARCH));
			}
		}
		else {
			c_wOldListFullMap |= (1 << cid);
#if	defined(_OLDTROUBLE_ALL_)
			if (c_wOldListFullMap == 0xfff)	return;
#else
			if (c_wOldListFullMap != 0)	return;
#endif
		}
		-- li;
	} while (li >= 0);

	free(pBuf);
}

void CTidy::AlignOldTroubleDir(const char* pDir)
{
	char buf[256];
	DWORD dwIndex = 0;
	//_TWATCHCLICK();
#if	defined(_OLDTROUBLE_ALL_)
	while (c_wOldListFullMap != 0xfff)
#else
	while (c_wOldListFullMap == 0)
#endif
	{
		dwIndex = GetLately6(pDir, TROUBLE_SIGN, dwIndex);		// 가장 마지막 코장 목록부터 정리한다.
		if (dwIndex == 0)	break;
		sprintf(buf, "%s/t%06d", pDir, dwIndex);
		//TRACK("MM:align %s.\n", buf);
		AlignOldTroubleFile(buf);
	}
	//_TWATCHMEASURED();
	//if (c_bDebug)	TRACK("MM:alignd %s. %f sec\n", pDir, __sec_);
}

void CTidy::AlignOldTrouble()
{
	char buf[256];

	_TWATCHCLICK();
	memset(&c_oldt, 0, sizeof(OLDTROUBLES));
	c_wOldListFullMap = 0;
	DWORD dwIndex = 0;
#if	defined(_OLDTROUBLE_ALL_)
	while (c_wOldListFullMap != 0xfff)
#else
	while (c_wOldListFullMap == 0)
#endif
	{
		dwIndex = GetLately6(ARCHIVE_PATH, LOGDIR_SIGN, dwIndex);	// 가장 마지막 로그 디렉토리부터 정리한다.
		if (dwIndex == 0)	break;
		sprintf(buf, "%s/d%06d", ARCHIVE_PATH, dwIndex);
		//TRACK("MM:align %s.\n", buf);
		AlignOldTroubleDir(buf);
	}
	CFlux* pFlux = (CFlux*)c_pParent;
	PBULKPACK pBulk = pFlux->GetBulk();
	pBulk->wState |= (1 << BULKSTATE_OLDTROUBLE);
	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("MM:AlignOldTrouble() %f sec\n", __sec_);
}

void CTidy::SaveOldTrouble()
{
	for (int n = 0; n < SIZE_OLDTROUBLEBLOCK; n ++) {
		if (c_oldt.blk[n].wLength == 0)	continue;
		char buf[256];
		sprintf(buf, "%s%d", OLDTROUBLE_PATH, n);

		FILE* fp;
		if ((fp = fopen(buf, "w")) != NULL) {
			size_t leng = fwrite(c_oldt.blk[n].list, 1, sizeof(TROUBLEARCH) * c_oldt.blk[n].wLength, fp);
			fclose(fp);
			if (leng == sizeof(TROUBLEARCH) * c_oldt.blk[n].wLength) {
				if (c_bDebug)	TRACK("MM:save old.(%d:%d)\n", n, c_oldt.blk[n].wLength);
			}
			else	TRACK("MM>:ERR:save old!(%d:%d:%d:%s)\n", n, c_oldt.blk[n].wLength, leng, strerror(errno));
		}
		else	TRACK("MM>ERR:open old!(%s)\n", strerror(errno));
	}
}

void CTidy::ArrangeEcuTrace()
{
	_TWATCHCLICK();
	CFlux* pFlux = (CFlux*)c_pParent;
	PBULKPACK pBulk = pFlux->GetBulk();
	if (pBulk->cEcuTraceLength == 0)	return;

	char path[256];
	memset(&c_etm, 0, sizeof(ECUTRACEM));
	QWORD qwIndex = 0;
	for (int n = (int)pBulk->cEcuTraceLength; n > 0; n --) {
		qwIndex = GetLately12(TRACE_PATH, TRACEECU_SIGN, qwIndex);
		if (qwIndex == 0)	continue;
		sprintf(path, "%s/%c%012llu", TRACE_PATH, TRACEECU_SIGN, qwIndex);
		FILE* fps;
		if ((fps = fopen(path, "r")) == NULL) {
			TRACK("MM>ERR:fopen %s!\n", path);
			continue;
		}
		fread(&c_etm.tds.c2[c_etm.wLength ++][0], 1, sizeof(SAECU) * SIZE_ECUTRACE, fps);	// 3030bytes
		fclose(fps);
	}

	FILE* fpd;
	if ((fpd = fopen(ECUTRACEALL_PATHTMP, "r")) != NULL) {	// 타깃파일이 있으면 지운다.
		fclose(fpd);
		remove(ECUTRACEALL_PATHTMP);
	}
	if ((fpd = fopen(ECUTRACEALL_PATHTMP, "a")) == NULL) {
		TRACK("MM>ERR:fopen %s\n", ECUTRACEALL_PATHTMP);
		return;
	}
	fwrite(&c_etm.tds.c1[0], 1, c_etm.wLength * ECUTRACEPAGE_MAX * SIZE_TRACETEXT, fpd);	// 3100 * wLength bytes
	fclose(fpd);
	pBulk->wState |= (1 << BULKSTATE_ARRANGEECUTRACE);
	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("MM:arrange ecu trace.(%f sec)\n", __sec_);
}

void CTidy::MoveTraceSphere(int ldi)
{
	if (ldi > DEVID_ECU)	return;

	_TWATCHCLICK();
	if (!MakeTraceDirOnUsb()) {
		CFlux* pFlux = (CFlux*)c_pParent;
		PBULKPACK pBulk = pFlux->GetBulk();
		pBulk->m[BPID_OCCASN].iResult = BULKRES_ERRCANNOTMAKEDIRONUSB;
		return;
	}

	char dev[256];
	char buf[256];
	char src[256];
	char dest[256];
	int max = 0;
	switch (ldi) {
	case DEVID_SIV :	max = SIV_MAX;	sprintf(dev, "%s", SIVTRACESPHERE_NAME);	break;
	case DEVID_V3F :	max = V3F_MAX;	sprintf(dev, "%s", V3FTRACESPHERE_NAME);	break;
	default :			max = ECU_MAX;	sprintf(dev, "%s", ECUTRACESPHERE_NAME);	break;
	}
	int leng = 0;
	for (int n = 0; n < max; n ++) {
		for (int m = 0; m < LIMIT_TRACE; m ++) {
			sprintf(buf, "%s%01d_%02d", dev, n, m);		// t'dev'x_yy
			sprintf(src, "%s/", PATHTMP);
			strcat(src, buf);							// /tmp/t'dev'x_yy
			if (GetFileSize(src) > 0) {
				sprintf(dest, "%s/", c_szDestTrace);	// /dos/c/n2XYY/trace/
				strcat(dest, buf);						// /dos/c/n2XYY/trace/t'dev'x_yy
				CopyFile(dest, src, false);
				remove(src);
				++ leng;
			}
		}
	}
	_TWATCHMEASURED();
	TRACK("MM:move %s %d. %f sec\n", dev, leng, __sec_);
}

void CTidy::MoveEcuTraceSphere()
{
	_TWATCHCLICK();
	if (!MakeTraceDirOnUsb()) {
		CFlux* pFlux = (CFlux*)c_pParent;
		PBULKPACK pBulk = pFlux->GetBulk();
		pBulk->m[BPID_OCCASN].iResult = BULKRES_ERRCANNOTMAKEDIRONUSB;
		return;
	}
	char buf[256];
	char src[256];
	char dest[256];
	int leng = 0;
	for (int n = 0; n < ECU_MAX; n ++) {
		for (int m = 0; m < LIMIT_TRACE; m ++) {
			sprintf(buf, "%s%01d_%02d", ECUTRACESPHERE_NAME, n, m);		// tecux_yy
			sprintf(src, "%s/", PATHTMP);								// /tmp/
			strcat(src, buf);											// /tmp/tecux_yy
			if (GetFileSize(src) > 0) {
				sprintf(dest, "%s/", c_szDestTrace);					// /dos/c/n2XYY/trace/
				strcat(dest, buf);										// /dos/c/n2XYY/trace/tecux_yy
				CopyFile(dest, src, false);
				remove(src);
				++ leng;
			}
		}
	}
	_TWATCHMEASURED();
	TRACK("MM:move ecu traces %d. %f sec\n", leng, __sec_);
}

