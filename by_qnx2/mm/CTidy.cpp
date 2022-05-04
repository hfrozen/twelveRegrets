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
#include <pthread.h>
#include <dirent.h>
#include <sys/neutrino.h>
#include <sys/stat.h>

#include "Track.h"
#include "Prefix.h"
#include "slight.h"
#include "Insp.h"
#include "CBand.h"
#include "CFlux.h"

#include "CTidy.h"

#define	_OLDTROUBLE_ALL_
#define	_OLDTROUBLE_TIMELIMIT_

CTidy::CTidy()
{
	c_pParent = NULL;
	c_bDebug = false;
	c_bCancel = false;
	c_hThreadAlign = INVALID_HANDLE;
	c_hThreadBad = INVALID_HANDLE;
	c_bAlign = false;
	c_bBadFurture = false;
	memset(&c_entrys, 0, sizeof(ENTRYSHAPE) * MAX_ENTRY);
	memset(&c_fl, 0, sizeof(FILELIST));
	memset(&c_curf, 0, sizeof(FILEGUIDE));
	c_curf.iSfd = c_curf.iDfd = INVALID_HANDLE;
	memset(c_szDestTrace, 0, SIZE_DEFAULTPATH);
	c_wLogbookVersion = 0;
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

DWORD CTidy::GetDateIndex(bool bPrev)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	BYTE year = (BYTE)((pTm->tm_year + 1900) % 100);
	BYTE mon = pTm->tm_mon + 1;
	BYTE day = pTm->tm_mday;
	if (bPrev) {
		if (mon > 1)	-- mon;
		else {
			mon = 12;
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
	int dleng = 0;
	char buf[256];
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			sprintf(buf, "%s/%s", pPath, pEntry->d_name);
			if (remove(buf) != 0)	return BULKRES_ERRCANNOTREMOVEFILE;
			++ dleng;
			//if (c_bDebug)	TRACK("MM:delete file.(%s)\n", buf);		// 170821, remove
		}
		closedir(pDir);
		if (remove(pPath) != 0)	return BULKRES_ERRCANNOTREMOVEFILE;
		//if (c_bDebug)	TRACK("MM:delete dir.(%s)\n", pPath);		// 170821, remove
	}
	return dleng;		//BULKRES_ERRNON;
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

int CTidy::DeletePrev6(const char* pPath)
{
	DWORD dwDate = GetDateIndex(true);
	//if (c_bDebug)	TRACK("MM:delete before %06d.\n", dwDate);

	int dleng = 0;
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
				int res;
				if ((res = DeleteDir(buf)) > 0)	dleng += res;
			}
		}
	}
	return dleng;
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

BYTE CTidy::GetLatestLogbookBlock(PLOGINFO pLog)
{
	// 180308
	BYTE cRes = 0;
	TRACK("MM:get latest logbook block\n");
	_TWATCHCLICK();
	CFlux* pFlux = (CFlux*)c_pParent;
	pFlux->FileWorkFlag(true);
	DWORD dwLatestD = 0;
	DWORD dwLatestL = 0;	// 210807
	bool bFileEnd = true;
	for (int n = 0; n < 4; n ++) {
		// 210807
		//if (bFileEnd)	dwLatestD = GetLately6(ARCHIVE_PATH, LOGDIR_SIGN, dwLatestD);
		if (bFileEnd) {
			dwLatestL = 0;
			dwLatestD = GetLately6(ARCHIVE_PATH, LOGDIR_SIGN, dwLatestD);
		}
		if (dwLatestD != 0 && dwLatestD != 1000000) {
			char path[256];
			sprintf(path, "%s/%c%06d", ARCHIVE_PATH, LOGDIR_SIGN, dwLatestD);
			//TRACK("MM:dir %s\n", path);		// ?????
			// 210807
			//DWORD dwLatestL = 0;
			//dwLatestL = GetLately6(path, LOGBOOK_SIGN, 0);
			dwLatestL = GetLately6(path, LOGBOOK_SIGN, dwLatestL);
			if (dwLatestL == 0 || dwLatestL == 1000000)	bFileEnd = true;
			else	bFileEnd = false;
			if (!bFileEnd) {
				char file[256];
				sprintf(file, "%s/%c%06d/%c%06d", ARCHIVE_PATH, LOGDIR_SIGN, dwLatestD, LOGBOOK_SIGN, dwLatestL);
				//TRACK("MM:file %s\n", file);	// ?????

				int size = (int)GetFileSize(file);
				if (size >= (int)(sizeof(LOGBOOKHEAD) + sizeof(LOGINFO))) {
					FILE* fp;
					if ((fp = fopen(file, "r")) != NULL) {
						LOGBOOKHEAD lbh;
						bool v203later = false;		// checksum version
						size_t rleng = fread(&lbh, 1, sizeof(LOGBOOKHEAD), fp);
						if (rleng == sizeof(LOGBOOKHEAD)) {
							c_wLogbookVersion = lbh.wVer;
							if (c_wLogbookVersion >= LOGBOOKCSUM_VERSION)	v203later = true;
						}
						if (!v203later) {
							fclose(fp);
							TRACK("MM>ERR:saved logbook version %x!\n", lbh.wVer);
							cRes |= (1 << CULLERR_WRONGLOGBOOKVERSION);
							return cRes;
						}
						int remain = (size - sizeof(LOGBOOKHEAD)) % sizeof(LOGINFO);
						int offset = sizeof(LOGINFO) + remain;
						int cur = size - offset;
						int whence = SEEK_END;
						while (cur >= (int)sizeof(LOGBOOKHEAD)) {
							if (!(fseek(fp, -offset, whence))) {
								//int pos = ftell(fp);
								rleng = fread(pLog->c, 1, sizeof(LOGINFO), fp);
								if (rleng == sizeof(LOGINFO)) {
									bool bEmpty = true;
									WORD csum = 0;
									for (int m = 0; m < (int)(sizeof(LOGINFO) - sizeof(WORD)); m ++) {
										csum += (WORD)pLog->c[m];
										if (pLog->c[m] != 0)	bEmpty = false;
									}
									if (!bEmpty && csum == pLog->r.wCSum) {
										fclose(fp);
										_TWATCHMEASURED();
										TRACK("MM:found last logbook block %d %d %s %f s.\n", n, pLog->r.wSeq, file, __sec_);
										// /h/sd/arc/d170911/l155250 - 0.024648sec
										pFlux->FileWorkFlag(false);
										return 0;
									}
									//else {
									//	TRACK("MM:empty = %d sum = %d %d %d\n", bEmpty, csum, pLog->r.wCSum, pLog->r.wSeq);
									//	TRACK("MM:pos = %d cur = %d offset = %d\n", pos, cur, offset);
									//}
								}
							}
							else	TRACK("MM:seek failure! %s\n", strerror(errno));
							offset = sizeof(LOGINFO);
							cur -= offset;
							offset += sizeof(LOGINFO);		// file location increased after fread!!!
							whence = SEEK_CUR;
						}
						fclose(fp);
					}
					else {	// if ((fp = fopen(file, "r")) != NULL) {
						cRes |= (1 << CULLERR_CANNOTOPENLOGBOOK);
						TRACK("MM:can not open %s!\n", file);
					}
				}
				else {	// if (size >= (int)(sizeof(LOGBOOKHEAD) + sizeof(LOGINFO))) {
					cRes |= (1 << CULLERR_UNMATCHSIZELOGBOOK);
					TRACK("MM:unmatch file size! %d %s\n", size, file);
				}
			}	// if (!bFileEnd) {
		}
		else {	// if (dwLatestD != 0 && dwLatestD != 1000000) {
			cRes |= (1 << CULLERR_CANNOTFOUNDLATELYLOGBOOK);
			TRACK("MM:can not found lately!\n");
		}
	}	// for (int n = 0; n < 4; n ++) {
	cRes |= (1 << CULLERR_COUNDNOTFOUNDLOGBOOK);
	_TWATCHMEASURED();
	TRACK("MM>ERR:find last logbook block! %f s!\n", __sec_);
	pFlux->FileWorkFlag(false);
	return cRes;
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
		// 170904
		// if (qw != 0) {
		if (min > 0 && min < 999999999999) {
			sprintf(buf, "%s/%c%012llu", pPath, cSign, min);
			//if (c_bDebug)	TRACK("MM:delete file.(%s)\n", buf);	// 170821, remove
			if (remove(buf) == 0)	return BULKRES_ERRCANNOTREMOVEFILE;
		}
	}
	return BULKRES_ERRNON;
}

void CTidy::LimitPrev12(const char* pPath, int iLimit, char cSign)
{
	//WORD w;
	//while ((w = GetFilesLength(pPath, cSign)) > iLimit) {
	//	TRACK("limited length = %d\n", w);
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
		TRACK("MM:ilist %d %s(%d)\n", leng, c_entrys[iSi - 1].szName, c_entrys[iSi - 1].dwSize);
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

	sprintf(c_szDestTrace, "%s/n%04x", USB_PATH, pBulk->lbHead.wPermNo);		// /dos/c/n2XYY
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
		//if (c_bDebug)	TRACK("MM:%s() can not open %s!\n", __FUNCTION__, pSrc);
		if (c_bDebug)	printf("MM:%s() can not open %s!\n", __FUNCTION__, pSrc);
		return BULKRES_ERRCANNOTFOUNDSRC;
	}
	int fd = open(pDest, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		//if (c_bDebug)	TRACK("MM:%s() can not open %s!\n", __FUNCTION__, pDest);
		if (c_bDebug)	printf("MM:%s() can not open %s!\n", __FUNCTION__, pDest);
		return BULKRES_ERRCANNOTOPENDEST;
	}

	int ret = 0;
	char buf[4096];
	do {
		ret = read(fs, buf, 4096);
		if (ret < 0) {
			close(fs);
			close(fd);
			//if (c_bDebug)	TRACK("MM:%s() can not read %s!\n", __FUNCTION__, pSrc);
			if (c_bDebug)	printf("MM:%s() can not read %s!\n", __FUNCTION__, pSrc);
			return BULKRES_ERRCANNOTREADSRC;
		}
		int res = write(fd, buf, ret);
		if (res < 0) {
			close(fs);
			close(fd);
			//if (c_bDebug)	TRACK("MM:%s() can not write %s!\n", __FUNCTION__, pDest);
			if (c_bDebug)	printf("MM:%s() can not write %s!\n", __FUNCTION__, pDest);
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

//====== ALIGN OLD TROUBLE =====

WORD CTidy::AlignOldTroubleFile(const char* pFile, WORD wOldListFullMap, POLDTROUBLELIST pOtl)
{
	int64_t size = GetFileSize(pFile);
	if (size == 0)	return wOldListFullMap;

	char* pBuf = (char*)malloc(size);
	if (pBuf == NULL) {
		TRACK("AOT>ERR:malloc!\n");
		return wOldListFullMap;
	}

	FILE* fp;
	size_t leng = 0;
	if ((fp = fopen(pFile, "r")) != NULL) {
		leng = fread(pBuf, 1, size, fp);
		fclose(fp);
		if (leng != size)	TRACK("AOT>ERR:file load!(%d - %s)\n", leng, strerror(errno));
	}
	if (leng < sizeof(TROUBLEINFO)) {
		free(pBuf);
		return wOldListFullMap;
	}

	int li = leng / sizeof(TROUBLEINFO);
	if (!(leng % sizeof(TROUBLEINFO)))	-- li;

	ABACUSLIST abacList[MAX_TROUBLE];
	memset(&abacList, 0, sizeof(ABACUSLIST) * MAX_TROUBLE);

	do {
		TROUBLEINFO ti;
		memcpy(&ti, pBuf + li * sizeof(TROUBLEINFO), sizeof(TROUBLEINFO));
		if (ti.b.cid >= DEFAULT_CARLENGTH || ti.b.code > 999 || ti.b.type < TROUBLE_STATUS) {
			//TRACK("AOT:wrong! %d %d.\n", ti.b.cid, ti.b.code);
			-- li;
			continue;
		}
		int cid = ti.b.cid + 1;		// 0~9 -> 1~10
		if (IsTuCode(ti.b.code))	cid = (cid < 2) ? 0 : 11;		// 0, 11		// TU/CU 객차 고장을 분리..
		// 190923
		//if (pOtl->iLength[cid] > 0) {
		if (pOtl->s[cid].iSize > 0) {
			if (ti.b.down) {
				abacList[ti.b.code].wCarBitmap |= (1 << ti.b.cid);
				abacList[ti.b.code].dhour[ti.b.cid] = ti.b.dhour;
				abacList[ti.b.code].dmin[ti.b.cid] = ti.b.dmin;
				abacList[ti.b.code].dsec[ti.b.cid] = ti.b.dsec;
				if (ti.b.dhour > 23 || ti.b.dmin > 59 || ti.b.dsec > 59)
					TRACK("AOT:wrong down a! %d %d %d\n", ti.b.dhour, ti.b.dmin, ti.b.dsec);
				if (abacList[ti.b.code].dhour[ti.b.cid] > 23 || abacList[ti.b.code].dmin[ti.b.cid] > 59 || abacList[ti.b.code].dsec[ti.b.cid] > 59)
					TRACK("AOT:wrong down b! %d %d %d\n", abacList[ti.b.code].dhour[ti.b.cid], abacList[ti.b.code].dmin[ti.b.cid], abacList[ti.b.code].dsec[ti.b.cid]);
			}
			else {
				// 발생 기록이면 따로 보관된 소거 기록에서 소거 기록을 찾아본다.
				if (abacList[ti.b.code].wCarBitmap & (1 << ti.b.cid)) {
					abacList[ti.b.code].wCarBitmap &= ~(1 << ti.b.cid);
					ti.b.dhour = abacList[ti.b.code].dhour[ti.b.cid];
					ti.b.dmin = abacList[ti.b.code].dmin[ti.b.cid];
					ti.b.dsec = abacList[ti.b.code].dsec[ti.b.cid];
					if (ti.b.dhour > 23 || ti.b.dmin > 59 || ti.b.dsec > 59)
						TRACK("AOT:wrong down c! %d %d %d %d %d\n", ti.b.code, ti.b.cid, ti.b.dhour, ti.b.dmin, ti.b.dsec);
				}
				//memcpy(&pOtl->ts[cid][-- pOtl->iLength[cid]], &ti, sizeof(TROUBLEINFO));
				memcpy(&pOtl->s[cid].ls[-- pOtl->s[cid].iSize], &ti, sizeof(TROUBLEINFO));
			}
		}
		else {
			if (!(wOldListFullMap & (1 << cid)))	TRACK("AOT:cid %d close %s.\n", cid, pFile);
			wOldListFullMap |= (1 << cid);
#if	defined(_OLDTROUBLE_ALL_)
			if (wOldListFullMap == 0xfff) {
				free(pBuf);
				return wOldListFullMap;
			}
#else
			if (wOldListFullMap != 0) {
				free(pBuf);
				return wOldListFullMap;
			}
#endif
		}
		-- li;
	} while (li >= 0);

	free(pBuf);
	return wOldListFullMap;
}

WORD CTidy::AlignOldTroubleDir(const char* pDir, WORD wOldListFullMap, POLDTROUBLELIST pOtl, uint64_t begin_t)
{
	DWORD dwIndex = 0;
#if	defined(_OLDTROUBLE_ALL_)
	while (wOldListFullMap != 0xfff)
#else
	while (wOldListFullMap == 0)
#endif
	{
		dwIndex = GetLately6(pDir, TROUBLE_SIGN, dwIndex);		// 가장 마지막 코장 목록부터 정리한다.
		if (dwIndex == 0)	break;
		char path[256];
		sprintf(path, "%s/t%06d", pDir, dwIndex);
		//TRACK("AOT:align %s.\n", buf);
		++ pOtl->wFileCount;
		wOldListFullMap = AlignOldTroubleFile(path, wOldListFullMap, pOtl);
#if	defined(_OLDTROUBLE_TIMELIMIT_)
		// 190923
		uint64_t elapse_t = ClockCycles();
		double elapse_s = (double)(elapse_t - begin_t) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
		if (elapse_s > 30.f)	break;
#endif
	}
	return wOldListFullMap;
}

void CTidy::AlignOldTrouble()
{
	_TWATCHCLICK();
	WORD wOldListFullMap = 0;
	OLDTROUBLELIST otl;
	// 190923
	//for (int n = 0; n < SIZE_TROUBLELIST; n ++) {
	//	memset(&otl.ts[n], 0, sizeof(TROUBLEINFO) * MAX_OLDTROUBLE);
	//	otl.iLength[n] = MAX_OLDTROUBLE;
	//}
	memset(&otl, 0, sizeof(OLDTROUBLELIST));
	for (int n = 0; n < SIZE_TROUBLELIST; n ++)
		otl.s[n].iSize = MAX_OLDTROUBLE;

	char path[256];

	DWORD dwIndex = 0;
#if	defined(_OLDTROUBLE_ALL_)
	while (wOldListFullMap != 0xfff)
#else
	while (wOldListFullMap == 0)
#endif
	{
		DWORD dw = dwIndex;
		dwIndex = GetLately6(ARCHIVE_PATH, LOGDIR_SIGN, dwIndex);	// 가장 마지막 로그 디렉토리부터 정리한다.
		if (dwIndex == 0) {
			TRACK("AOT:final %06d.\n", dw);
			break;
		}
		sprintf(path, "%s/d%06d", ARCHIVE_PATH, dwIndex);
		//TRACK("AOT:align %s.\n", path);
		wOldListFullMap = AlignOldTroubleDir(path, wOldListFullMap, &otl, __begin_);
#if	defined(_OLDTROUBLE_TIMELIMIT_)
		// 190923
		uint64_t elapse_t = ClockCycles();
		double elapse_s = (double)(elapse_t - __begin_) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
		if (elapse_s > 30.f)	break;
#endif
	}

	FILE* fp;
	for (int n = 0; n < SIZE_TROUBLELIST; n ++) {
		sprintf(path, "%s%02d", OLDTROUBLE_PATHTMP, n);
		if ((fp = fopen(path, "w")) != NULL) {
			//short i = MAX_OLDTROUBLE - otl.iLength[n];
			//if (i > 0)	fwrite(&otl.ts[n][otl.iLength[n]], 1, i * sizeof(TROUBLEINFO), fp);
			short i = MAX_OLDTROUBLE - otl.s[n].iSize;
			if (i > 0)	fwrite(&otl.s[n].ls[otl.s[n].iSize], 1, i * sizeof(TROUBLEINFO), fp);
			fclose(fp);
			//remove(path);
		}
	}

	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("AOT:align map=0x%04x, filec=%d, %f sec\n", wOldListFullMap, otl.wFileCount, __sec_);
	// 200109
	// 190923
	if ((fp = fopen(OLDTROUBLEEND_PATHTMP, "w")) != NULL) {
		__sec_ *= 1000.f;
		WORD w = (__sec_ > 65535.f) ? 65535 : (WORD)__sec_;
		fwrite(&w, 1, sizeof(WORD), fp);
		fclose(fp);
	}
}

PVOID CTidy::AlignOldTroubleEntry(PVOID pVoid)
{
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CTidy* pTidy = (CTidy*)pVoid;
	if (pTidy != NULL) {
		pTidy->c_bAlign = true;
		pTidy->AlignOldTrouble();
		pTidy->c_bAlign = false;
	}
	//TRACK("AOT:thread exit %d\n", pTidy->c_bAlign);

	pthread_exit(0);
	return NULL;
}

bool CTidy::StirAlignOldTrouble()
{
	if (!c_bAlign) {
		CFlux* pFlux = (CFlux*)c_pParent;
		if (pFlux->CreateThread(&c_hThreadAlign, &CTidy::AlignOldTroubleEntry, (PVOID)this, PRIORITY_MSG) == EOK) {
			TRACK("AOT:begin...\n");
			return true;
		}
		else {
			TRACK("AOT:can not begin thread!\n");
			return false;
		}
	}
	TRACK("AOT:thread progress...\n");
	return true;
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

// 181011
void CTidy::DeleteBadFurtureFiles(const char* pPath)
{
	char buf[256];
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;
			sprintf(buf, "%s/%s", pPath, pEntry->d_name);
			struct stat st;
			if (lstat(buf, &st) >= 0 && S_ISDIR(st.st_mode))	DeleteBadFurtureFiles(buf);
			else	remove(buf);
		}
		closedir(pDir);
		remove(pPath);
	}
}

void CTidy::DeleteBadFurtureDir()
{
	_TWATCHCLICK();

	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	BYTE year = (BYTE)((pTm->tm_year + 1900) % 100);
	BYTE mon = pTm->tm_mon + 1;
	BYTE day = pTm->tm_mday;
	DWORD dwCur = ((DWORD)year * 10000 + (DWORD)mon * 100 + (DWORD)day);

	char buf[256];
	int dleng = 0;
	DIR* pDir = opendir(ARCHIVE_PATH);
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
				if (dw > dwCur) {
					++ dleng;
					sprintf(buf, "%s/%s", ARCHIVE_PATH, pEntry->d_name);
					DeleteBadFurtureFiles(buf);
				}
			}
		}
		closedir(pDir);
	}
	_TWATCHMEASURED();
	TRACK("DBFF:delete %d %f sec\n", dleng, __sec_);
	//TRACK("DBFF:delete %f sec\n", __sec_);
}

PVOID CTidy::DeleteBadFurtureDirEntry(PVOID pVoid)
{
	CTidy* pTidy = (CTidy*)pVoid;
	if (pTidy != NULL) {
		pTidy->c_bBadFurture = true;
		pTidy->DeleteBadFurtureDir();
		pTidy->c_bBadFurture = false;
	}
	//CFlux* pFlux = (CFlux*)pTidy->c_pParent;
	//PBULKPACK pBulk = pFlux->GetBulk();
	//pBulk->wState &= ~(1 << BULKSTATE_DELETEBAD);

	pthread_exit(0);
	return NULL;
}

bool CTidy::StirDeleteBadFurtureDir()
{
	if (!c_bBadFurture) {
		CFlux* pFlux = (CFlux*)c_pParent;
		if (pFlux->CreateThread(&c_hThreadBad, &CTidy::DeleteBadFurtureDirEntry, (PVOID)this, PRIORITY_MSG) == EOK) {
			TRACK("DBFF:begin...\n");
			return true;
		}
		else {
			TRACK("DBFF:can not begin thread!\n");
			return false;
		}
	}
	TRACK("DBFF:thread process...\n");
	return true;
}

