/*
 * CBeil.cpp
 *
 *  Created on: 2011. 5. 23
 *      Author: Che
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/syspage.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/neutrino.h>

#include <Printz.h>
#include "CBeil.h"
#include "Archive.h"
#include "Draft.h"

#define	BYARCH_PATH	((c_pBucket->wState & BEILSTATE_BEENMEM) ? ARCH_PATHE : ARCH_PATHN)

void* CBeil::Handler(PVOID pVoid)
{
	CBeil* pBeil = (CBeil*)pVoid;
	if (pBeil == NULL)	return (0);

	while (TRUE) {
		int res;
		WORD msg;
		if ((res = mq_receive(pBeil->c_hMsg, (char*)&msg, sizeof(WORD), NULL)) >= 0)
			pBeil->Working(msg);
	}
	if (IsValid(pBeil->c_hMsg)) {
		mq_close(pBeil->c_hMsg);
		pBeil->c_hMsg = -1;
	}
	mq_unlink(NAME_BEIL);
	return (0);
}

CBeil::CBeil()
{
	c_hMsg = -1;
	c_hShm = -1;
	c_pBucket = NULL;
	c_hThread = NULL;
	c_nDebug = 0;
	c_nByCap = 0;
}

CBeil::~CBeil()
{
	Shutoff();
}

BOOL CBeil::Run(BYTE debug, BYTE cap)
{
	c_nDebug = debug;
	c_nByCap = cap;
	MSGLOG("Beil v%.2f\r\n", VERSION_BEIL);
	if (c_nDebug != 0)	MSGLOG("[BEIL]Debug mode.\r\n");
	if (!CreateQueue()) {
		MSGLOG("[BEIL]ERROR:can not create message quene.\r\n");
		return FALSE;
	}
	if (!CreateShmem()) {
		MSGLOG("[BEIL]ERROR:can not create shared memory.\r\n");
		mq_close(c_hMsg);
		mq_unlink(NAME_BEIL);
		return FALSE;
	}

	struct statvfs64 st;
	if (statvfs64(MEM_PATHE, &st) < 0) {
		MSGLOG("[BEIL]Can not found external memory.\r\n");
	}
	else {
		c_pBucket->wState |= BEILSTATE_BEENMEM;

		// Modified 2013/03/12
		int res;
		DIR* pDir = opendir(ARCH_PATHE);
		if (pDir == NULL) {
			res = mkdir(ARCH_PATHE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			if (res < 0)	MSGLOG("[BEIL]ERROR:%s at mkdir().\r\n", strerror(errno));
		}

		int64_t total = (int64_t)(st.f_bsize * st.f_blocks);
		int64_t free;
		if (c_nByCap) {
			free = (int64_t)(st.f_bsize * st.f_bfree);
			int64_t need = total / 2;
			MSGLOG("[BEIL]Find external memory, size is %lld.\r\n", total);
			BYTE cycle = 0;
			while (free < need) {
				free = DeleteTopDirectory(ARCH_PATHE);
				if (++ cycle > 100) {
					MSGLOG("[BEIL]ERROR:can not prepare external memory (%lld)\r\n", free);
					break;
				}
			}
		}
		else	free = DeleteOutsideDay(ARCH_PATHE);
		MSGLOG("[BEIL]Free space of external memory is %lld.\r\n", free);

		pDir = opendir(CUR_PATHE);
		if (pDir != NULL) {
			FILE* fp = fopen(DRV_PATHE, "r");
			if (fp != NULL) {
				fclose(fp);
				res = rename(DRV_PATHE, BKDRV_PATHE);
				MSGLOG("[BEIL]Find logbook file, exchange to backup file (result %d).\r\n", res);
			}
			else	MSGLOG("[BEIL]Find current directory.\r\n");
			closedir(pDir);
		}
		else {
			res = mkdir(CUR_PATHE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			MSGLOG("[BEIL]Make current directory.(result %d).\r\n", res);
		}

		c_index.atc = c_index.ato = c_index.siv = c_index.v3f = c_index.ecu = c_index.cmsb = 0;
		res = 0;
		pDir = opendir(TICKER_PATHE);
		if (pDir == NULL) {
			res = mkdir(TICKER_PATHE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
			MSGLOG("[BEIL]Make ticker directory.(result %d).\r\n", res);
		}
		else	closedir(pDir);
		if (res == 0) {
			FILE* fp = fopen(TINDEX_PATHE, "r");
			if (fp != NULL) {
				fread((PVOID)&c_index, sizeof(DWORD), 6, fp);
				fclose(fp);
			}
			MakeTickerbox(TATC_PATHE, &c_index.atc);
			MakeTickerbox(TATO_PATHE, &c_index.ato);
			MakeTickerbox(TSIV_PATHE, &c_index.siv);
			MakeTickerbox(TV3F_PATHE, &c_index.v3f);
			MakeTickerbox(TECU_PATHE, &c_index.ecu);
			MakeTickerbox(TCMSB_PATHE, &c_index.cmsb);
		}
	}

	return TRUE;
}

void CBeil::MakeTickerbox(const char* pName, DWORD* pIndex)
{
	char fn[256];

	sprintf(fn, "%s", pName);
	DIR* pDir = opendir(fn);
	if (pDir == NULL) {
		int res = mkdir(fn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		*pIndex = 0;
		MSGLOG("[BEIL]Make \"%s\" directory.(result is %d).\r\n", pName, res);
	}
	else {
		closedir(pDir);
		MSGLOG("[BEIL]\"%s\" index is %ld.\r\n", pName, *pIndex);
	}
}

void CBeil::DeleteArchives()
{
	char dn[256];
	if (c_pBucket->wState & BEILSTATE_BEENMEM)	sprintf(dn, "rm -r %s/D*", ARCH_PATHE);
	else	sprintf(dn, "rm -r %s/*", ARCH_PATHN);
	system(dn);
	MSGLOG("[BEIL]Delete all archives.\r\n");
}

void CBeil::DeleteDirectory(char* pTarget)
{
	char fn[256];
	DIR* pDir = opendir(pTarget);
	if (pDir != NULL) {
		struct dirent* ent;
		while (ent = readdir(pDir)) {
			if (ent->d_name[0] == '.')	continue;
			sprintf(fn, "%s/%s", pTarget, ent->d_name);
			remove(fn);
			//if (remove(fn) != 0)	return FILEERROR_CANNOTREMOVEFILE;
		}
		closedir(pDir);
		remove(pTarget);
		//if (remove(pTarget) != 0)	return FILEERROR_CANNOTREMOVEDIR;
	}
}

BOOL CBeil::SearchUsb()
{
	DIR* pDir = opendir("/dev");
	if (pDir != NULL) {
		struct dirent* pEnt;
		while ((pEnt = readdir(pDir)) != NULL) {
			strcpy(c_usbName, pEnt->d_name);
			if (strlen(c_usbName) > 4) {
				c_usbName[4] = 0;			// !!! very important !!!
				if (!strcmp(c_usbName, "hd1t")) {
					strcpy(c_usbName, pEnt->d_name);
					closedir(pDir);
					return TRUE;
				}
			}
		}
		closedir(pDir);
	}
	return FALSE;
}

int64_t CBeil::GetFileLength(const char* fn)
{
	struct stat st;

	if (stat(fn, &st) > -1)	return ((int64_t)st.st_size);
	return 0;
}

/*int CBeil::GetFileLength(FILE* fp)
{
	if (fp == NULL)	return 0L;
	int org, size;
	if ((org = ftell(fp)) < 0)			return -1L;
	if (fseek(fp, 0L, SEEK_END) != 0)	return -2L;
	if ((size = ftell(fp)) < 0)			return -3L;
	if (fseek(fp, org, SEEK_SET) != 0)	return -4L;
	return size;
}
*/
// use only nand working
SHORT CBeil::BuildArchiveDir()
{
	DIR* pDir = opendir(ARCH_PATHN);
	if (pDir == NULL) {
		int res = mkdir(ARCH_PATHN, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0)	return FILEERROR_CANNOTCREATEBASEDIR;
	}
	else closedir(pDir);
	return FILEERROR_NON;
}

SHORT CBeil::BuildTargetCamp(BOOL bOppo)
{
	char dn[256];
	BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 1 : 0;
	BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
	if (bOppo)	id ^= 1;
	sprintf(dn, "/dos/c/TC%d%04X%d", id, c_pBucket->drv.wFno, liu);
	if (c_nDebug)	MSGLOG("[BEIL]Build camp %s.\r\n", dn);
	DIR* pDir = opendir(dn);
	if (pDir == NULL) {
		int res = mkdir(dn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0)	return FILEERROR_CANNOTCREATETARGETDIR;
	}
	else	closedir(pDir);
	return FILEERROR_NON;
}

SHORT CBeil::BuildTargetDir(const char* d, BOOL bOppo)
{
	SHORT res = BuildTargetCamp(bOppo);
	if (res != FILEERROR_NON)	return res;

	char dn[256];
	BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 1 : 0;
	BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
	if (bOppo)	id ^= 1;
	sprintf(dn, "/dos/c/TC%d%04X%d/%s", id, c_pBucket->drv.wFno, liu, d);
	if (c_nDebug)	MSGLOG("[BEIL]Build %s.\r\n", dn);
	DIR* pDir = opendir(dn);
	if (pDir == NULL) {
		int res = mkdir(dn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0)	return FILEERROR_CANNOTCREATETARGETDIR;
	}
	else	closedir(pDir);
	return FILEERROR_NON;
}

int64_t CBeil::GetFreeSpace(const char* pTarget)
{
	struct statvfs64 st;

	int64_t size = 0;
	if (statvfs64(pTarget, &st) < 0) {
		c_pBucket->tar.s.size =
			c_pBucket->tar.s.free = 0;
		MSGLOG("[BEIL]ERROR:statvfs64() %s at %s\r\n", strerror(errno), pTarget);
	}
	else {
		c_pBucket->tar.s.size = (DWORD)st.f_bsize;
		c_pBucket->tar.s.free = (DWORD)st.f_bfree;
		size = st.f_bfree * st.f_bsize;
		if (c_nDebug)	MSGLOG("[BEIL]free = %lld at %s\r\n", size, pTarget);
	}
	return size;
}

int64_t CBeil::DeleteTopDirectory(const char* pTarget)
{
	char dn[256];

	DIR* pDir = opendir(pTarget);
	if (pDir != NULL) {
		struct dirent* ent;
		long long min = MAX_FILEDATE;
		while ((ent = readdir(pDir)) != NULL) {
			strcpy(dn, ent->d_name);
			long long v = 0;
			if (dn[0] == 'D') {
				v = atoll(&dn[1]);
				if (min > v)	min = v;
			}
		}
		closedir(pDir);
		if (min < MAX_FILEDATE) {
			sprintf(dn, "%s/D%012lld", pTarget, min);
			DeleteDirectory(dn);
			return GetFreeSpace(pTarget);
		}
		return GetFreeSpace(pTarget);
	}
	return GetFreeSpace(pTarget);
}

int64_t CBeil::DeleteOutsideDay(const char* pTarget)
{
	char dn[256];
	char cn[256];

	time_t cur = time(NULL);
	struct tm* lc = localtime(&cur);
	int month = lc->tm_mon + 1;
	int day = lc->tm_mday;
	int min = (month - 1) * 100 + day;
	int max = month * 100 + day;
	if (c_nDebug)	MSGLOG("[BEIL]Scan directory.(%04d - %04d)\r\n", min, max);

	DIR* pDir = opendir(pTarget);
	if (pDir != NULL) {
		struct dirent* ent;
		while ((ent = readdir(pDir)) != NULL) {
			strcpy(dn, ent->d_name);
			if (dn[0] == 'D') {
				strcpy(cn, dn);
				cn[7] = 0;				// without time
				int day = atoi(&cn[3]);	// without year, only month & day
				if (month == 1 && day > 1200)	day -= 1200;	// december to 0 at january
				// scan battery condition so no touch year
				if (day < min || day > max) {
					sprintf(cn, "%s/%s", pTarget, dn);
					DeleteDirectory(cn);
					if (c_nDebug)	MSGLOG("[BEIL]Delete %s.\r\n", cn);
				}
			}
		}
		closedir(pDir);
	}
	return GetFreeSpace(pTarget);
}

BOOL CBeil::CompareDirectory(char* pCmp, const char* pOrg)
{
	int size = strlen(pOrg);
	if (size == 0)	return TRUE;
	if (size > 100)	size = 100;
	char buf[128];
	strcpy(buf, pCmp);
	buf[size] = 0;
	if (!strcmp(buf, pOrg))	return TRUE;
	return FALSE;
}

int CBeil::GetFileQuantity(const char* pTarget)
{
	char dn[256];

	long quan = 0;
	DIR* pDir = opendir(pTarget);
	if (pDir != NULL) {
		struct dirent* ent;
		while ((ent = readdir(pDir)) != NULL) {
			strcpy(dn, ent->d_name);
			if (dn[0] == 'D')	++ quan;
		}
		closedir(pDir);
	}
	return quan;
}

void CBeil::GetFileQuantity(const char* pDirName, PDIRINFO pQuan)
{
	DIR* pDir = opendir(pDirName);
	if (pDir == NULL) {
		pQuan->quantity = -1;
		return;
	}
	char buf[128];
	pQuan->quantity = 0;
	pQuan->size = 0;
	struct dirent* pEnt = readdir(pDir);
	if (pEnt != NULL) {
		struct stat st;
		sprintf(buf, "%s/%s", pDirName, DRV_FILE);
		if (stat(buf, &st) > -1) {
			++ pQuan->quantity;
			pQuan->size += (int64_t)st.st_size;
		}
		sprintf(buf, "%s/%s", pDirName, ARM_FILE);
		if (stat(buf, &st) > -1) {
			++ pQuan->quantity;
			pQuan->size += (int64_t)st.st_size;
		}
		sprintf(buf, "%s/%s", pDirName, BDRV_FILE);
		if (stat(buf, &st) > -1) {
			++ pQuan->quantity;
			pQuan->size += (int64_t)st.st_size;
		}
		sprintf(buf, "%s/%s", pDirName, BARM_FILE);
		if (stat(buf, &st) > -1) {
			++ pQuan->quantity;
			pQuan->size += (int64_t)st.st_size;
		}
	}
	closedir(pDir);
}

void CBeil::GetFileQuantity(const char* pDirName, const char* pFileName, PDIRINFO pQuan)
{
	DIR* pDir = opendir(pDirName);
	if (pDir == NULL) {
		pQuan->quantity = -1;
		return;
	}
	struct dirent* pEnt;
	pQuan->quantity = 0;
	pQuan->size = 0;
	char buf[128];
	while ((pEnt = readdir(pDir)) != NULL) {
		if (CompareDirectory(pEnt->d_name, pFileName)) {
			struct stat st;
			sprintf(buf, "%s/%s/%s", pDirName, pEnt->d_name, DRV_FILE);
			if (stat(buf, &st) > -1) {
				++ pQuan->quantity;
				pQuan->size += (int64_t)st.st_size;
			}
			sprintf(buf, "%s/%s/%s", pDirName, pEnt->d_name, ARM_FILE);
			if (stat(buf, &st) > -1) {
				++ pQuan->quantity;
				pQuan->size += (int64_t)st.st_size;
			}
			sprintf(buf, "%s/%s/%s", pDirName, pEnt->d_name, BDRV_FILE);
			if (stat(buf, &st) > -1) {
				++ pQuan->quantity;
				pQuan->size += (int64_t)st.st_size;
			}
			sprintf(buf, "%s/%s/%s", pDirName, pEnt->d_name, BARM_FILE);
			if (stat(buf, &st) > -1) {
				++ pQuan->quantity;
				pQuan->size += (int64_t)st.st_size;
			}
		}
	}
	closedir(pDir);
}

BOOL CBeil::GetAlarmFileNameByIndex(WORD id, char* pName)
{
	*pName = 0;
	BOOL res = FALSE;
	FILE* fp;
	int size = (int)GetFileLength(DIR_PATHT);
	if (size > 0) {
		if ((fp = fopen(DIR_PATHT, "r")) == NULL) {
			MSGLOG("[BEIL]ERROR:can not open directory list file.\r\n");
			return FALSE;
		}
		char* pBufA = (char*)malloc(size);
		if (pBufA == NULL) {
			MSGLOG("[BEIL]ERROR:malloc() at GetAlarmNameByIndex.\r\n");
			return FALSE;
		}
		char* pBuf = pBufA;
		fread(pBuf, sizeof(BYTE), size, fp);
		fclose(fp);
		while (*pBuf != 0 && !res) {
			if (*pBuf == '*') {
				++ pBuf;
				WORD index = (WORD)atoi(pBuf);
				if (index == id) {
					while (*pBuf != ':' && *pBuf != '\r' && *pBuf != '\n' && *pBuf != 0)	++ pBuf;
					if (*pBuf == ':') {
						++ pBuf;
						while (*pBuf != '\r' && *pBuf != '\n' && *pBuf != 0)
							*pName ++ = *pBuf ++;
						*pName = 0;
						res = TRUE;
					}
				}
			}
			else	++ pBuf;
		}
		free(pBufA);
	}
	return res;
}

void CBeil::CopyAlarmOnly(char* pName, const char* pTarget)
{
	char fn[256];
	unlink(ARM_PATHET);
	sprintf(fn, "%s/%s/arm", pTarget, pName);
	int size = (int)GetFileLength(fn);
	if (size < 1) {
		MSGLOG("BEIL]ERROR:file length error.(%d-%s)\r\n", size, fn);
		return;
	}
	FILE *fs, *fd;
	if ((fs = fopen(fn, "r")) == NULL) {
		MSGLOG("[BEIL]ERROR:can not open file.(%s)\r\n", fn);
		return;
	}
	WORD line = size / sizeof(ARM_ARCH);
	if ((line % sizeof(ARM_ARCH)) > 0)	++ line;
	if ((fd = fopen(ARM_PATHET, "w")) == NULL) {
		fclose(fs);
		MSGLOG("[BEIL]ERROR:can not open file.(/tmp/armed)\r\n");
		return;
	}
	ARM_ARCH arm;
	WORD sort = 0;
	for (WORD w = 0; w < line; w ++) {
		fread((PVOID)&arm, sizeof(ARM_ARCH), 1, fs);
		if (arm.b.alarm != 0) {
			fwrite((PVOID)&arm, sizeof(ARM_ARCH), 1, fd);	// to tmp/armed
			++ sort;
		}
	}
	MSGLOG("[BEIL]Alarm sort %ld at %ld.\r\n", sort, line);
	fclose(fs);
	fclose(fd);
}

SHORT CBeil::FileCopy(const char* dn, const char* sn)
{
	int fs = open(sn, O_RDONLY);
	if (fs < 0)	return FILEERROR_CANNOTFOUNDSRC;
	int fd = open(dn, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		return FILEERROR_CANNOTOPENDEST;
	}
	int size = 0;
	char buf[SIZE_BUF];
	do {
		size = read(fs, buf, SIZE_BUF);
		if (size < 0) {
			close(fs);
			close(fd);
			return FILEERROR_CANNOTREADSRC;
		}
		int res = write(fd, buf, size);
		if (res < 0) {
			close(fs);
			close(fd);
			return FILEERROR_CANNOTWRITEDEST;
		}
		//MSGLOG("[BEIL]File copy %d\r\n", size);
	} while (size >= SIZE_BUF);
	close(fs);
	close(fd);
	return FILEERROR_NON;
}

BOOL CBeil::FileCopyA(const char* dn, const char* sn)
{
	BOOL bRes = TRUE;
	int fs, fd;
	if ((fs = open(sn, O_RDONLY)) >= 0) {
		if ((fd = open(dn, O_RDWR | O_CREAT | O_TRUNC,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) >= 0) {
			int size;
			do {
				size = read(fs, c_buf, MAX_BUF);
				if (size > -1) {
					write(fd, c_buf, size);
					c_tsize += size;
				}
				if (c_sizes > 0)
					c_pBucket->wProgress = (WORD)(c_tsize * 100L / c_sizes);
			} while (size >= MAX_BUF && !IsExitCmd());
			close(fd);
		}
		else {
			if (c_nDebug)	MSGLOG("[BEIL]ERROR:write at Copy(%s-%s).\r\n", dn, strerror(errno));
			//c_pBucket->iRespond = (SHORT)errno;
			bRes = FALSE;
		}
		close(fs);
	}
	else {
		if (c_nDebug)	MSGLOG("[BEIL]ERROR:read at Copy(%s-%s).\r\n", sn, strerror(errno));
		//c_pBucket->iRespond = (SHORT)errno;
		bRes = FALSE;
	}
	return bRes;
}

BOOL CBeil::TickerDev(WORD flag, unsigned long* pl, int size, char* pPath, BYTE* pBuf)
{
	if ((c_pBucket->wTicker & flag) == flag) {
		char fn[256];
		sprintf(fn, "%s/f%03d", pPath, *pl / MAX_TICKERINDEXOFFILE);
		FILE* fp;
		if (!(*pl % MAX_TICKERINDEXOFFILE))	fp = fopen(fn, "w");
		else	fp = fopen(fn, "a");
		if (fp != NULL) {
			fwrite(pBuf, size, 1, fp);		// nfd
			fclose(fp);
			if (++ *pl >= MAX_TICKERSAMPLE)	*pl = 0;
			c_pBucket->wTicker &= ~(flag | (flag << 1));
			return TRUE;
		}
		c_pBucket->wTicker &= ~(flag | (flag << 1));
		return FALSE;
	}
	return FALSE;
}

void CBeil::Tickering()
{
	TickerDev(BEILTICKER_ATCR, &c_index.atc, sizeof(TBATC), (char*)TATC_PATHE, c_pBucket->atc.n);
	TickerDev(BEILTICKER_ATOR, &c_index.ato, sizeof(TBATO), (char*)TATO_PATHE, c_pBucket->ato.n);
	TickerDev(BEILTICKER_SIVR, &c_index.siv, sizeof(TBSIV), (char*)TSIV_PATHE, c_pBucket->siv.n);
	TickerDev(BEILTICKER_V3FR, &c_index.v3f, sizeof(TBV3F), (char*)TV3F_PATHE, c_pBucket->v3f.n);
	TickerDev(BEILTICKER_ECUR, &c_index.ecu, sizeof(TBECU), (char*)TECU_PATHE, c_pBucket->ecu.n);
	TickerDev(BEILTICKER_CMSBR, &c_index.cmsb, sizeof(TBCMSB), (char*)TCMSB_PATHE, c_pBucket->cmsb.n);

	FILE* fp = fopen(TINDEX_PATHE, "w");
	if (fp != NULL) {
		fwrite((PVOID)&c_index, sizeof(DWORD), 6, fp);	// nfd
		fclose(fp);
	}
}

void CBeil::MakeDirectoryList()
{
	char dn[256];
	char fn[256];

	if (c_nDebug)	MSGLOG("[BEIL]Start make directory list.\r\n");
	c_pBucket->iRespond = FILEWORK_WORKING;

	unlink(DIR_PATHT);
	int total = GetFileQuantity(BYARCH_PATH);
	if (total < 1) {
		MSGLOG("[BEIL]ERROR:not found any directory at %s.\r\n", BYARCH_PATH);
		c_pBucket->iRespond = FILEERROR_CANNOTFOUNDDATA;
		return;
	}
	long long* pv = (long long*)malloc(sizeof(long long) * total * 2);
	if (pv == NULL) {
		MSGLOG("[BEIL]ERROR:malloc() at MakeDirectoryList().\r\n");
		c_pBucket->iRespond = FILEERROR_CANNOTFOUNDDATA;
		return;
	}

	DIR* pDir = opendir(BYARCH_PATH);
	if (pDir == NULL) {
		MSGLOG("[BEIL]ERROR:can not open (%s).\r\n", BYARCH_PATH);
		c_pBucket->iRespond = FILEERROR_CANNOTFOUNDSRC;
		free(pv);
		return;
	}
	memset((PVOID)pv, 0, sizeof(long long) * total * 2);
	struct dirent* ent;
	int length = 0;
	while ((ent = readdir(pDir)) != NULL) {
		strcpy(dn, ent->d_name);
		if (dn[0] == 'D') {
			if (dn[1] == '9' && dn[2] == '9')	dn[1] = '-';	// untill 2098
			*(pv + length ++) = atoll(&dn[1]);
		}
	}
	closedir(pDir);

	if (length > 0) {
		*(pv + total) = *pv;	// first element
		int size = 1;
		for (int n = 1; n < length; n ++) {
			BOOL insert = FALSE;
			BOOL bSrc = FALSE;
			long long aSrc = *(pv + n);
			if (aSrc < 0LL) {
				bSrc = TRUE;		// minus
				aSrc = llabs(aSrc);
			}
			for (int m = 0; m < size; m ++) {
				insert = FALSE;
				BOOL bDest = FALSE;
				long long aDest = *(pv + total + m);
				if (aDest < 0LL) {
					bDest = TRUE;
					aDest = llabs(aDest);
				}
				if (bSrc != bDest) {
					if (!bSrc && bDest)	insert = TRUE;	// plus < minus
				}
				else if (aSrc > aDest)	insert = TRUE;
				if (insert) {
					for (int sh = size; sh > m; -- sh)
						*(pv + total + sh) = *(pv + total + sh - 1);
					*(pv + total + m) = *(pv + n);
					break;
				}
			}
			if (!insert)	*(pv + total + size) = *(pv + n);
			++ size;
		}

		FILE* fp;
		if ((fp = fopen(DIR_PATHT, "w")) == NULL) {
			MSGLOG("[BEIL]ERROR:can not open directory list file.\r\n");
			c_pBucket->iRespond = FILEERROR_CANNOTFOUNDSRC;
			free(pv);
			return;
		}
		for (int n = 0; n < length; n ++) {
			//sprintf(fn, "%s/D%012lld", BYARCH_PATH, *(pv + total + n));
			//DIRINFO di;
			//di.quantity = 0;
			//di.size = 0;
			//GetFileQuantity(fn, &di);
			if (*(pv + total + n) < 0)
				sprintf(fn, "*%d:D99%010lld\r\n", n + 1, llabs(*(pv + total + n)) % 10000000000LL);	//, di.quantity, di.size);
			else	sprintf(fn, "*%d:D%012lld\r\n", n + 1, *(pv + total + n));	//, di.quantity, di.size);
			fwrite(fn, sizeof(BYTE), strlen(fn), fp);	// tmp
		}
		fclose(fp);
	}
	free(pv);
}

void CBeil::LoadAlarm()
{
	char fn[256];

	unlink(ARM_PATHET);
	if (GetAlarmFileNameByIndex(c_pBucket->wParam1, fn)) {
		MSGLOG("[BEIL]Open alarm %d(%s).\r\n", c_pBucket->wParam1, fn);
		if (fn[0] != 0)	CopyAlarmOnly(fn, BYARCH_PATH);
	}
}

void CBeil::AppendDrv()
{
	c_pBucket->iRespond = FILEWORK_WORKING;	// 1

	uint64_t size = GetFileLength((c_pBucket->wState & BEILSTATE_BEENMEM) ? DRV_PATHE : DRV_PATHT);
	FILE* fp = fopen((c_pBucket->wState & BEILSTATE_BEENMEM) ? DRV_PATHE : DRV_PATHT, "a");	//"a+");
	if (fp != NULL) {
		if (size == 0)	fwrite((PVOID)DRVLOG2_7, 30, 1, fp);	// nfd or tmp
		size_t num = fwrite(&c_pBucket->drv, 1, sizeof(DRV_ARCH), fp);	// nfd or tmp
		if (num != sizeof(DRV_ARCH)) {
			c_pBucket->iRespond = FILEERROR_CANNOTWRITEDEST;	// -17
			MSGLOG("[BEIL]ERROR:operate append size(%d-%s).\r\n", num, strerror(errno));
		}
		else	c_pBucket->iRespond = FILEWORK_END;	// 2
		fclose(fp);
	}
	else {
		c_pBucket->iRespond = FILEERROR_CANNOTOPENDEST;	// -15
		MSGLOG("[BEIL]ERROR:can not open drive data current file.(%d)\r\n", strerror(errno));
	}
}

void CBeil::WriteEnv()
{
	if (c_nDebug)	MSGLOG("[BEIL]Begin env file writing.\r\n");
	c_pBucket->iRespond = FILEWORK_WORKING;

	sprintf(c_src, "%s", ENV_PATHT);
	struct stat st;
	if (stat(c_src, &st) > -1) {
		c_pBucket->iRespond = FILEWORK_WORKING;
		c_sizes = st.st_size;
		c_pBucket->wProgress = 0;
		int fs, fd;
		if ((fs = open(c_src, O_RDONLY)) >= 0) {
			sprintf(c_dest, "%s", (c_pBucket->wState & BEILSTATE_BEENMEM) ? ENV_PATHE : ENV_PATHN);
			if (c_nDebug)	MSGLOG("[BEIL]Target = %s.\r\n", c_dest);
			if ((fd = open(c_dest, O_RDWR | O_CREAT | O_TRUNC,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) >= 0) {
				int size = read(fs, c_buf, MAX_BUF);
				if (c_nDebug)	MSGLOG("[BEIL]Env. size = %d.\r\n", size);
				if (size > -1)	write(fd, c_buf, size);
				close(fd);
				c_pBucket->iRespond = FILEWORK_END;
			}
			else {
				if (c_nDebug)	MSGLOG("[BEIL]ERROR:write - %s.\r\n", strerror(errno));
				c_pBucket->iRespond = FILEERROR_CANNOTWRITEENVFILE;
				//c_pBucket->iRespond = (SHORT)errno;-----
			}
			close(fs);
		}
		else	c_pBucket->iRespond = FILEERROR_CANNOTREADENVFILE;
			//c_pBucket->iRespond = fs;-----
	}
	else	c_pBucket->iRespond = FILEERROR_CANNOTFOUNDSRC;
	unlink(ENV_PATHT);

	if (c_nDebug)	MSGLOG("[BEIL]Env file write result is %d.\r\n", c_pBucket->iRespond);
}

void CBeil::WorkLogbook()
{
	if (c_nDebug)	MSGLOG("[BEIL]Begin logbook working.\r\n");
	c_pBucket->iRespond = FILEWORK_WORKING;

	char fn[256];
	sprintf(fn, "%s/D%02d%02d%02d%02d%02d%02d",
			ARCH_PATHE, c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day,
			c_pBucket->src.dt.hour, c_pBucket->src.dt.minute, c_pBucket->src.dt.sec);
	int res = rename(CUR_PATHE, fn);
	if (res != 0)	c_pBucket->iRespond = FILEERROR_CANNOTRENAMEDIR;
	else {
		strcat(fn, "/arm");
		if ((res = FileCopy(fn, ARM_PATHT)) != FILEERROR_NON)
			c_pBucket->iRespond = res;
		unlink(ARM_PATHT);

		res = mkdir(CUR_PATHE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0)	c_pBucket->iRespond = FILEERROR_CANNOTCREATEBASEDIR;
	}
	if (c_pBucket->iRespond == FILEWORK_WORKING)	c_pBucket->iRespond = FILEWORK_END;

	if (c_nDebug)	MSGLOG("[BEIL]Log book work result is %d.\r\n", c_pBucket->iRespond);
}

void CBeil::WriteLogbook()
{
	if (c_nDebug)	MSGLOG("[BEIL]Begin save logbook.\r\n");
	c_pBucket->iRespond = FILEWORK_WORKING;

	SHORT res = BuildArchiveDir();
	if (res != FILEERROR_NON) {
		c_pBucket->iRespond = res;
		return;
	}
	BYTE fs = 0;
	int64_t need = 0;
	int64_t size = GetFileLength(DRV_PATHT);
	if (size > 0) {
		fs |= 1;
		need += size;
	}
	else {
		c_pBucket->iRespond = FILEERROR_CANNOTOPENDRV;
		if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not open %s.\r\n", DRV_PATHT);
	}
	size = GetFileLength(ARM_PATHT);
	if (size > 0) {
		fs |= 2;
		need += size;
	}
	else {
		c_pBucket->iRespond = FILEERROR_CANNOTOPENARM;
		if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not open %s.\r\n", ARM_PATHT);
	}
	need += 16384;
	if (c_nDebug)	MSGLOG("[BEIL]Need %lld.\r\n", need);
	UCURV limit = 0;
	int64_t hfree = GetFreeSpace(ARCH_PATHN);
	if (c_nDebug)	MSGLOG("[BEIL]GetFreeSpace(Hard) = %lld\r\n", hfree);
	while (hfree < need) {
		if (hfree <= 0) {
			//c_pBucket->iRespond = (SHORT)hfree;-----
			c_pBucket->iRespond = FILEERROR_CANNOTGETFREESIZE;
			if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not get free size %lld.\r\n", hfree);
			return;
		}
		if (++ limit >= MAX_DELETECYCLE) {
			c_pBucket->iRespond = FILEERROR_CYCLEOVERDELETE;
			if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not get free size for cycling.\r\n");
			return;
		}
		hfree = DeleteTopDirectory(ARCH_PATHN);
		if (c_nDebug)	MSGLOG("[BEIL]GetFreeSpaceA(Hard) = %lld\r\n", hfree);
	}

	char fn[256];
	sprintf(fn, "%s/D%02d%02d%02d%02d%02d%02d",
			ARCH_PATHN, c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day,
			c_pBucket->src.dt.hour, c_pBucket->src.dt.minute, c_pBucket->src.dt.sec);
	if (mkdir(fn, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH) != 0) {
		c_pBucket->iRespond = FILEERROR_CANNOTCREATEDIR;
		if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not create dir %s.\r\n", fn);
		return;
	}
	char dn[256];
	if (fs & 1) {
		sprintf(dn, "%s/%s", fn, DRV_FILE);
		if ((res = FileCopy(dn, DRV_PATHT)) != FILEERROR_NON) {
			c_pBucket->iRespond = res;
			if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not copy file %d.\r\n", res);
		}
	}
	if (fs & 2) {
		sprintf(dn, "%s/%s", fn, ARM_FILE);
		if ((res = FileCopy(dn, ARM_PATHT)) != FILEERROR_NON) {
			c_pBucket->iRespond = res;
			if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not copy file %d.\r\n", res);
		}
	}
	unlink(DRV_PATHT);
	unlink(ARM_PATHT);
	c_pBucket->iRespond = FILEWORK_END;

	if (c_nDebug)	MSGLOG("[BEIL]End save file.\r\n");
}

void CBeil::SelectToDelete()
{
	c_pBucket->iRespond = FILEWORK_WORKING;
	if (c_nDebug)	MSGLOG("[BEIL]Begin logbook deleting.\r\n");

	char fn[256];
	sprintf(fn, "rm -r %s/D%02d%02d%02d*", BYARCH_PATH,
			c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day);
	if (c_nDebug)	MSGLOG("BEIL]System(%s).\r\n", fn);
	int res = system(fn);
	c_pBucket->iRespond = FILEWORK_END;

	if (c_nDebug)	MSGLOG("[BEIL]Logbook delete result is %d.\r\n", res);
}

void CBeil::SelectToUsb(BOOL bCmd)
{
	if (c_nDebug)	MSGLOG("[BEIL]Begin logbook downloading.\r\n");
	c_pBucket->iRespond = FILEWORK_WORKING;

	if (c_pBucket->wState & BEILSTATE_BEENUSB) {
		char fn[256];
		if (bCmd)	sprintf(fn, "D");		// all files
		else	sprintf(fn, "D%02d%02d%02d", c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day);
		DIRINFO di;
		GetFileQuantity(BYARCH_PATH, fn, &di);
		if (di.quantity > 0) {
			WORD length;
			c_pBucket->src.s.length = length = (WORD)di.quantity;
			c_sizes = di.size;
			c_pBucket->src.s.size = (DWORD)(c_sizes & 0xffffffff);
			c_pBucket->src.s.free = (DWORD)(c_sizes >> 32);
			c_pBucket->wProgress = 0;
			if (c_nDebug)	MSGLOG("[BEIL]Find %d files (%lld).\r\n", length, c_sizes);
			int64_t free = GetFreeSpace("/dos/c");
			if (c_sizes < free) {
				c_tsize = 0;
				BOOL bStop = FALSE;
				DIR* pDir = opendir(BYARCH_PATH);
				if (pDir != NULL) {
					struct dirent* pEnt;
					BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 1 : 0;
					BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
					while ((pEnt = readdir(pDir)) != NULL && !IsExitCmd()) {
						if (CompareDirectory(pEnt->d_name, fn)) {
							if (c_nDebug)	MSGLOG("[BEIL]Select file %s.\r\n", pEnt->d_name);
							int fs;
							if ((fs = BuildTargetDir(pEnt->d_name, FALSE)) == FILEERROR_NON) {
								sprintf(c_src, "%s/%s/%s", BYARCH_PATH, pEnt->d_name, DRV_FILE);
								FILE* fp = fopen(c_src, "r");
								if (fp != NULL) {
									fclose(fp);
									sprintf(c_dest, "/dos/c/TC%d%04X%d/%s/%s", id, c_pBucket->drv.wFno, liu, pEnt->d_name, DRV_FILE);
									FileCopyA(c_dest, c_src);
								}
								sprintf(c_src, "%s/%s/%s", BYARCH_PATH, pEnt->d_name, ARM_FILE);
								fp = fopen(c_src, "r");
								if (fp != NULL) {
									fclose(fp);
									sprintf(c_dest, "/dos/c/TC%d%04X%d/%s/%s", id, c_pBucket->drv.wFno, liu, pEnt->d_name, ARM_FILE);
									FileCopyA(c_dest, c_src);
								}
								sprintf(c_src, "%s/%s/%s", BYARCH_PATH, pEnt->d_name, BDRV_FILE);
								fp = fopen(c_src, "r");
								if (fp != NULL) {
									fclose(fp);
									sprintf(c_dest, "/dos/c/TC%d%04X%d/%s/%s", id, c_pBucket->drv.wFno, liu, pEnt->d_name, BDRV_FILE);
									FileCopyA(c_dest, c_src);
								}
								sprintf(c_src, "%s/%s/%s", BYARCH_PATH, pEnt->d_name, BARM_FILE);
								fp = fopen(c_src, "r");
								if (fp != NULL) {
									fclose(fp);
									sprintf(c_dest, "/dos/c/TC%d%04X%d/%s/%s", id, c_pBucket->drv.wFno, liu, pEnt->d_name, BARM_FILE);
									FileCopyA(c_dest, c_src);
								}
							}
							else {
								c_pBucket->iRespond = fs;
								bStop = TRUE;
							}
						}
						if (bStop)	break;
					}
					if (!bStop) {
						c_pBucket->wProgress = 100;
						c_pBucket->iRespond = FILEWORK_END;
					}
					else if (IsExitCmd())	c_pBucket->iRespond = FILEERROR_EXITCMD;
				}
				else	c_pBucket->iRespond = FILEERROR_CANNOTOPENDIR;
				closedir(pDir);
			}
			else	c_pBucket->iRespond = FILEERROR_NOTENOUGHSPACE;
		}
		else	c_pBucket->iRespond = FILEERROR_CANNOTFOUNDDATA;
	}
	else	c_pBucket->iRespond = FILEERROR_NOTFOUNDUSB;

	if (c_nDebug)	MSGLOG("[BEIL]Logbook download result is %d.\r\n", c_pBucket->iRespond);
}

void CBeil::InspectToUsb()
{
	c_pBucket->iRespond = FILEWORK_WORKING;
	if (c_nDebug)	MSGLOG("[BEIL]Begin inspect downloading.\r\n");

	if (c_pBucket->wState & BEILSTATE_BEENUSB) {
		SHORT res = BuildTargetCamp(FALSE);
		if (res == FILEERROR_NON) {
			sprintf(c_src, "%s", INSP_PATHT);
			struct stat st;
			if (stat(c_src, &st) > -1) {
				c_pBucket->src.s.length = 1;
				c_pBucket->src.s.size = c_sizes = st.st_size;
				c_pBucket->wProgress = 0;
				int64_t free = GetFreeSpace("/dos/c");
				if (c_sizes < free) {
					c_tsize = 0;
					int fs, fd;
					if ((fs = open(c_src, O_RDONLY)) >= 0) {
						time_t cur = time(NULL);
						struct tm* lc = localtime(&cur);
						int year = (lc->tm_year + 1900) % 100;
						BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 1 : 0;
						BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
						sprintf(c_dest, "/dos/c/TC%d%04X%d/insp%02d%02d%02d%02d%02d%02d", id, c_pBucket->drv.wFno, liu,
								year, lc->tm_mon + 1, lc->tm_mday, lc->tm_hour, lc->tm_min, lc->tm_sec);
						if (c_nDebug)	MSGLOG("[BEIL]Target = %s\r\n", c_dest);
						if ((fd = open(c_dest, O_RDWR | O_CREAT | O_TRUNC,
								S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) >= 0) {
							int size;
							do {
								size = read(fs, c_buf, MAX_BUF);
								if (size > -1) {
									write(fd, c_buf, size);
									c_tsize += size;
								}
								if (c_sizes > 0)
									c_pBucket->wProgress = (WORD)(c_tsize * 100L / c_sizes);
							} while (size >= MAX_BUF && !IsExitCmd());
							close(fd);
							if (IsExitCmd())	c_pBucket->iRespond = FILEERROR_EXITCMD;
							else	c_pBucket->iRespond = FILEWORK_END;
						}
						else {
							if (c_nDebug)	MSGLOG("[BEIL]ERROR:write - %s.\r\n", strerror(errno));
							c_pBucket->iRespond = FILEERROR_CANNOTCREATEINSPFILE;
							//c_pBucket->iRespond = (SHORT)errno;-----
						}
						close(fs);
					}
					else	c_pBucket->iRespond = FILEERROR_CANNOTREADINSPFILE;
						//c_pBucket->iRespond = fs;-----
				}
				else	c_pBucket->iRespond = FILEERROR_NOTENOUGHSPACE;
			}
			else	c_pBucket->iRespond = FILEERROR_CANNOTFOUNDSRC;
		}
		else	c_pBucket->iRespond = res;

		unlink(INSP_PATHT);
	}
	else	c_pBucket->iRespond = FILEERROR_NOTFOUNDUSB;

	if (c_nDebug)	MSGLOG("[BEIL]Inspect download result is %d.\r\n", c_pBucket->iRespond);
}

void CBeil::TraceToUsb()
{
	c_pBucket->iRespond = FILEWORK_WORKING;
	if (c_nDebug)	MSGLOG("[BEIL]Begin trace downloading.\r\n");

	if (c_pBucket->wState & BEILSTATE_BEENUSB) {
		SHORT res = BuildTargetCamp(FALSE);
		if (res == FILEERROR_NON) {
			c_sizes = 0;
			int leng = (int)GetFileLength(TATC_PATHT);
			if (leng > 0)	c_sizes += leng;
			leng = (int)GetFileLength(TATO_PATHT);
			if (leng > 0)	c_sizes += leng;
			for (UCURV n = 0; n < MAX_TRAIN_LENGTH; n ++) {
				sprintf(c_src, "%s%c", TSIV_PATHT, 'a' + n);
				leng = (int)GetFileLength(c_src);
				if (leng > 0)	c_sizes += leng;
				sprintf(c_src, "%s%c", TV3F_PATHT, 'a' + n);
				leng = (int)GetFileLength(c_src);
				if (leng > 0)	c_sizes += leng;
				sprintf(c_src, "%s%c", TECU_PATHT, 'a' + n);
				leng = (int)GetFileLength(c_src);
				if (leng > 0)	c_sizes += leng;
			}
			if (c_nDebug)	MSGLOG("[BEIL]Trace data size = %lld.\r\n", c_sizes);
			if (c_sizes > 0) {
				c_pBucket->src.s.size = (DWORD)(c_sizes & 0xffffffff);
				c_pBucket->src.s.free = (DWORD)(c_sizes >> 32);
				c_pBucket->wProgress = 0;
				int64_t free = GetFreeSpace("/dos/c");
				if (c_sizes < free) {
					c_tsize = 0;
					int fs, fd;
					BOOL bStop = FALSE;
					time_t cur = time(NULL);
					struct tm* lc = localtime(&cur);
					int year = (lc->tm_year + 1900) % 100;
					BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 1 : 0;
					BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
					//sprintf(c_dest, "/dos/c/TC%d%04X/insp%02d%02d%02d", id, c_pBucket->drv.wFno, lc->tm_hour, lc->tm_min, lc->tm_sec);
					for (WORD fileID = 0; fileID < 5; fileID ++) {
						for (WORD devID = 0; devID < MAX_TRAIN_LENGTH; devID ++) {
							switch (fileID) {
							case 0 :
								sprintf(c_src, "%s", TATO_PATHT);
								sprintf(c_dest, "/dos/c/TC%d%04X%d/ato%02d%02d%02d", id, c_pBucket->drv.wFno, liu, year, lc->tm_mon + 1, lc->tm_mday);
								break;
							case 1 :
								sprintf(c_src, "%s", TATC_PATHT);
								sprintf(c_dest, "/dos/c/TC%d%04X%d/atc%02d%02d%02d", id, c_pBucket->drv.wFno, liu, year, lc->tm_mon + 1, lc->tm_mday);
								break;
							case 2 :
								sprintf(c_src, "%s%c", TSIV_PATHT, 'a' + devID);
								sprintf(c_dest, "/dos/c/TC%d%04X%d/siv%c%02d%02d%02d", id, c_pBucket->drv.wFno, liu, 'a' + devID, year, lc->tm_mon + 1, lc->tm_mday);
								break;
							case 3 :
								sprintf(c_src, "%s%c", TV3F_PATHT, 'a' + devID);
								sprintf(c_dest, "/dos/c/TC%d%04X%d/v3f%c%02d%02d%02d", id, c_pBucket->drv.wFno, liu, 'a' + devID, year, lc->tm_mon + 1, lc->tm_mday);
								break;
							default :
								sprintf(c_src, "%s%c", TECU_PATHT, 'a' + devID);
								sprintf(c_dest, "/dos/c/TC%d%04X%d/ecu%c%02d%02d%02d", id, c_pBucket->drv.wFno, liu, 'a' + devID, year, lc->tm_mon + 1, lc->tm_mday);
								break;
							}
							if ((fs = open(c_src, O_RDONLY)) >= 0) {
								if (c_nDebug)	MSGLOG("[BEIL]Target = %s\r\n", c_dest);
								if ((fd = open(c_dest, O_RDWR | O_CREAT | O_TRUNC,
										S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) >= 0) {
									int size;
									do {
										size = read(fs, c_buf, MAX_BUF);
										if (size > -1) {
											write(fd, c_buf, size);
											c_tsize += size;
										}
										if (c_sizes > 0)
											c_pBucket->wProgress = (WORD)(c_tsize * 100L / c_sizes);
									} while (size >= MAX_BUF && !IsExitCmd());
									close(fd);
								}
								else {
									if (c_nDebug)	MSGLOG("[BEIL]ERROR:write - %s.\r\n", strerror(errno));
									//c_pBucket->iRespond = (SHORT)errno;-----
									c_pBucket->iRespond = FILEERROR_CANNOTCREATERACEFILE;
									bStop = TRUE;
								}
								close(fs);
								unlink(c_src);
							}
							if (fileID < 2 || IsExitCmd())	break;		// atc ato has not device id
						}
						if (bStop || IsExitCmd())	break;
					}
					if (!bStop) {
						c_pBucket->iRespond = FILEWORK_END;
						c_pBucket->wProgress = 100;
					}
					else if (IsExitCmd())	c_pBucket->iRespond = FILEERROR_EXITCMD;
				}
				else	c_pBucket->iRespond = FILEERROR_NOTENOUGHSPACE;
			}
			else	c_pBucket->iRespond = FILEERROR_CANNOTFOUNDDATA;
		}
		else	c_pBucket->iRespond = res;
	}
	else	c_pBucket->iRespond = FILEERROR_NOTFOUNDUSB;

	if (c_nDebug)	MSGLOG("[BEIL]Trace download result is %d.\r\n", c_pBucket->iRespond);
}

void CBeil::TrialToUsb()
{
	c_pBucket->iRespond = FILEWORK_WORKING;
	if (c_nDebug)	MSGLOG("[BEIL]Begin trial downloading.\r\n");

	if (c_pBucket->wState & BEILSTATE_BEENUSB) {
		SHORT res = BuildTargetCamp(FALSE);
		if (res == FILEERROR_NON) {
			sprintf(c_src, "%s", TRIAL_PATHT);
			struct stat st;
			if (stat(c_src, &st) > -1) {
				c_pBucket->src.s.length = 1;
				c_pBucket->src.s.size = c_sizes = st.st_size;
				c_pBucket->wProgress = 0;
				int64_t free = GetFreeSpace("/dos/c");
				if (c_sizes < free) {
					c_tsize = 0;
					int fs, fd;
					if ((fs = open(c_src, O_RDONLY)) >= 0) {
						time_t cur = time(NULL);
						struct tm* lc = localtime(&cur);
						int year = (lc->tm_year + 1900) % 100;
						BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 1 : 0;
						BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
						// Modified 2013/11/02
						//sprintf(c_dest, "/dos/c/TC%d%04X%d/trial%02d%02d%02d", id, c_pBucket->drv.wFno, liu, lc->tm_hour, lc->tm_min, lc->tm_sec);
						sprintf(c_dest, "/dos/c/TC%d%04X%d/trial%02d%02d%02d", id, c_pBucket->drv.wFno, liu, year, lc->tm_mon + 1, lc->tm_mday);
						if (c_nDebug)	MSGLOG("[BEIL]Target = %s\r\n", c_dest);
						if ((fd = open(c_dest, O_RDWR | O_CREAT | O_TRUNC,
								S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) >= 0) {
							int size;
							do {
								size = read(fs, c_buf, MAX_BUF);
								if (size > -1) {
									write(fd, c_buf, size);
									c_tsize += size;
								}
								if (c_sizes > 0)
									c_pBucket->wProgress = (WORD)(c_tsize * 100L / c_sizes);
							} while (size >= MAX_BUF && !IsExitCmd());
							close(fd);
							if (IsExitCmd())	c_pBucket->iRespond = FILEERROR_EXITCMD;
							else	c_pBucket->iRespond = FILEWORK_END;
						}
						else {
							if (c_nDebug)	MSGLOG("[BEIL]ERROR:write - %s.\r\n", strerror(errno));
							c_pBucket->iRespond = FILEERROR_CANNOTCREATEINSPFILE;	//FILEERROR_CANNOTCREATETRIALFILE;
							//c_pBucket->iRespond = (SHORT)errno;-----
						}
						close(fs);
					}
					else	c_pBucket->iRespond = FILEERROR_CANNOTREADINSPFILE;	//FILEERROR_CANNOTREADTRIALFILE
						// c_pBucket->iRespond = fs;-----
				}
				else	c_pBucket->iRespond = FILEERROR_NOTENOUGHSPACE;
			}
			else	c_pBucket->iRespond = FILEERROR_CANNOTFOUNDSRC;
		}
		else	c_pBucket->iRespond = res;
		unlink(TRIAL_PATHT);
	}
	else	c_pBucket->iRespond = FILEERROR_NOTFOUNDUSB;

	if (c_nDebug)	MSGLOG("[BEIL]Trial download result is %d.\r\n", c_pBucket->iRespond);
}

void CBeil::SectQuantity()
{
	c_pBucket->iRespond = FILEWORK_WORKING;
	char fn[256];
	sprintf(fn, "D%02d%02d%02d", c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day);
	DIRINFO di;
	GetFileQuantity(BYARCH_PATH, fn, &di);
	c_pBucket->tar.s.size = (DWORD)di.size;
	c_pBucket->tar.s.length = (WORD)di.quantity;
	c_pBucket->iRespond = FILEWORK_END;
	if (c_nDebug)	MSGLOG("[BEIL]\"%s\" size is %ld.\r\n", fn, c_pBucket->tar.s.size);
}

void CBeil::ReadSect()
{
	c_pBucket->iRespond = FILEWORK_WORKING;
	unlink(SECT_PATHT);
	if (c_pBucket->wFileID < 4) {
		char* fnp;
		switch (c_pBucket->wFileID) {
		case 0 :	fnp = (char*)DRV_FILE;	break;
		case 1 :	fnp = (char*)ARM_FILE;	break;
		case 2 :	fnp = (char*)BDRV_FILE;	break;
		default :	fnp = (char*)BARM_FILE;	break;
		}
		char fn[256];
		sprintf(fn, "%s/D%02d%02d%02d%02d%02d%02d/%s", BYARCH_PATH,
				c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day,
				c_pBucket->src.dt.hour, c_pBucket->src.dt.minute, c_pBucket->src.dt.sec, fnp);
		//int64_t size = GetFileLength(fn);
		int64_t target = (int64_t)c_pBucket->dwOffset * SIZE_MATETEXT;
		FILE* fp = fopen(fn, "r");
		if (fp != NULL) {
			fseek(fp, target, SEEK_SET);
			int rsize = fread((PVOID)&c_buf, sizeof(char), SIZE_MATETEXT, fp);
			fclose(fp);
			//MSGLOG("[BEIL]sect read=%d offset=%lld.\r\n", rsize, target);
			fp = fopen(SECT_PATHT, "w");
			if (fp != NULL) {
				fwrite((PVOID)&c_buf, sizeof(char), rsize, fp);	// tmp
				fclose(fp);
			}
			else {
				if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not open target file(%s).\r\n", SECT_PATHT);
			}
		}
		else {
			if (c_nDebug)	MSGLOG("[BEIL]ERROR:can not open source file(%s).\r\n", fn);
		}
	}
	c_pBucket->iRespond = FILEWORK_END;
}

void CBeil::WriteSectToUsb()
{
	c_pBucket->iRespond = FILEWORK_WORKING;

	if (c_pBucket->wState & BEILSTATE_BEENUSB) {
		FILE* fp = fopen(SECT_PATHT, "r");
		if (fp != NULL) {
			int size = fread((PVOID)&c_buf, sizeof(char), SIZE_MATETEXT, fp);
			fclose(fp);

			int64_t free = GetFreeSpace("/dos/c");
			if ((int64_t)size < free) {
				char* fnp;
				switch (c_pBucket->wFileID) {
				case 0 :	fnp = (char*)DRV_FILE;	break;
				case 1 :	fnp = (char*)ARM_FILE;	break;
				case 2 :	fnp = (char*)BDRV_FILE;	break;
				default :	fnp = (char*)BARM_FILE;	break;
				}
				char fn[256];
				sprintf(fn, "D%02d%02d%02d%02d%02d%02d",
						c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day,
						c_pBucket->src.dt.hour, c_pBucket->src.dt.minute, c_pBucket->src.dt.sec);
				SHORT res = BuildTargetDir(fn, TRUE);
				if (res == FILEERROR_NON) {
					BYTE id = (c_pBucket->wState & BEILSTATE_TC1) ? 0 : 1;		// Opposite
					BYTE liu = (c_pBucket->wState & BEILSTATE_LIU2) ? 1 : 0;
					sprintf(fn, "/dos/c/TC%d%04X%d/D%02d%02d%02d%02d%02d%02d/%s", id, c_pBucket->drv.wFno, liu,
							c_pBucket->src.dt.year, c_pBucket->src.dt.month, c_pBucket->src.dt.day,
							c_pBucket->src.dt.hour, c_pBucket->src.dt.minute, c_pBucket->src.dt.sec, fnp);
					fp = fopen(fn, "a");
					if (fp != NULL) {
						fwrite((PVOID)&c_buf, sizeof(char), size, fp);	// usb
						fclose(fp);
						c_pBucket->iRespond = FILEWORK_END;
					}
					else {
						c_pBucket->iRespond = FILEERROR_CANNOTOPENDIR;
						MSGLOG("[BEIL]ERROR:can not open target to usb(%s)%s.\r\n", fn, strerror(errno));
					}
				}
				else {
					c_pBucket->iRespond = res;
					MSGLOG("[BEIL]ERROR:can not create target to usb.\r\n");
				}
			}
			else {
				c_pBucket->iRespond = FILEERROR_NOTENOUGHSPACE;
				MSGLOG("[BEIL]ERROR:not enough to usb.\r\n");
			}
		}
		else {
			c_pBucket->iRespond = FILEERROR_CANNOTFOUNDSRC;
			MSGLOG("[BEIL]ERROR:can not open source(%s).\r\n", SECT_PATHT);
		}
	}
	else {
		c_pBucket->iRespond = FILEERROR_NOTFOUNDUSB;
	}
}

void CBeil::Working(WORD msg)
{
	switch (msg) {
	case BEILCMD_RTC :
		system("rtc-men -s hw");
		break;
	case BEILCMD_SEARCHUSB :
		if (SearchUsb()) {
			if (!(c_pBucket->wState & BEILSTATE_BEENUSB)) {
				char cmd[256];
				sprintf(cmd, "mount -tdos /dev/%s /dos/c", c_usbName);
				system(cmd);
				DIR* pDir = opendir("/dos/c");
				if (pDir != NULL)	c_pBucket->wState |= BEILSTATE_BEENUSB;
				closedir(pDir);
				if (c_nDebug)	MSGLOG("[BEIL]Attached %s.\r\n", c_usbName);
				GetFreeSpace("/dos/c");
			}
		}
		else if (c_pBucket->wState & BEILSTATE_BEENUSB) {
			c_pBucket->wState &= ~BEILSTATE_BEENUSB;
			if (c_nDebug)	MSGLOG("[BEIL]Detach usb.\r\n");
		}
		break;
	case BEILCMD_WRITEENV :
		c_pBucket->nMainAck = BEILACK_WORKING;
		WriteEnv();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_LOGBOOKTOMEM :
		c_pBucket->nMainAck = BEILACK_WORKING;
		if ((c_pBucket->wState & BEILSTATE_BEENMEM))	WorkLogbook();
		else {
			WriteLogbook();
			if (c_nDebug)	MSGLOG("[BEIL]Save logbook result is %d.\r\n", c_pBucket->iRespond);
		}
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_LOGBOOKTOUSB :
	case BEILCMD_LOGBOOKTOUSBNDELETE :
		c_pBucket->nMainAck = BEILACK_WORKING;
		SelectToUsb(TRUE);
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_INSPECTTOUSB :
		c_pBucket->nMainAck = BEILACK_WORKING;
		InspectToUsb();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_TRACETOUSB :
		c_pBucket->nMainAck = BEILACK_WORKING;
		TraceToUsb();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_TRIALTOUSB :
		c_pBucket->nMainAck = BEILACK_WORKING;
		TrialToUsb();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_DELETEARCHIVES :
		c_pBucket->nMainAck = BEILACK_WORKING;
		DeleteArchives();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_TICKER :
		//if (c_pBucket->wState & BEILSTATE_BEENMEM)
		//	Ticker();
		break;
	case BEILCMD_MAKEDIRECTORYLIST :
		c_pBucket->nMainAck = BEILACK_WORKING;
		MakeDirectoryList();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_LOADALARM :
		c_pBucket->nMainAck = BEILACK_WORKING;
		LoadAlarm();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_APPENDDRV :
		c_pBucket->nPeriodAck = BEILACK_WORKING;
		if (c_pBucket->wTicker & BEILTICKER_DRV) {
			AppendDrv();		// result to c_pBucket->iRespond
			c_pBucket->wTicker &= ~BEILTICKER_DRV;
		}
		if (c_pBucket->wState & BEILSTATE_BEENMEM)
			Tickering();
		if (c_pBucket->wTicker & BEILTICKER_ENV) {
			WriteEnv();
			c_pBucket->wTicker &= ~BEILTICKER_ENV;
		}
		c_pBucket->nPeriodAck = BEILACK_WORKEND;
		break;
	case BEILCMD_SELTOUSB :
		c_pBucket->nMainAck = BEILACK_WORKING;
		SelectToUsb(FALSE);
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_SELTODELETE :
		c_pBucket->nMainAck = BEILACK_WORKING;
		SelectToDelete();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_SECTQUANTITY :
		c_pBucket->nMainAck = BEILACK_WORKING;
		SectQuantity();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_READSECT :
		c_pBucket->nMainAck = BEILACK_WORKING;
		ReadSect();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	case BEILCMD_WRITESECT :
		c_pBucket->nMainAck = BEILACK_WORKING;
		WriteSectToUsb();
		c_pBucket->nMainAck = BEILACK_WORKEND;
		break;
	default :
		if (c_nDebug)	MSGLOG("[BEIL]ERROR:unknown command(%d).\r\n", msg);
		c_pBucket->nMainAck = BEILACK_WORKEND;
		c_pBucket->nPeriodAck = BEILACK_WORKEND;
		break;
	}
}

void CBeil::Shutoff()
{
	if (c_hThread != (pthread_t)NULL) {
		pthread_cancel(c_hThread);
		c_hThread = NULL;
	}
	if (IsValid(c_hMsg)) {
		mq_close(c_hMsg);
		c_hMsg = -1;
	}
	if (IsValid(c_hShm)) {
		close(c_hShm);
		c_hShm = -1;
	}
	mq_unlink(NAME_BEIL);
	shm_unlink(NAME_BUCKET);
}

BOOL CBeil::CreateQueue()
{
	mq_unlink(NAME_BEIL);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = BEIL_MAX;
	attr.mq_msgsize = sizeof(WORD);
	if (c_nDebug)
		MSGLOG("[BEIL]Queue size=%d length=%d.\r\n", attr.mq_msgsize, attr.mq_maxmsg);

	c_hMsg = mq_open(NAME_BEIL, O_RDWR | O_CREAT,
						S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (IsValid(c_hMsg)) {
		int res;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if ((res = pthread_attr_setinheritsched(&attr,
							PTHREAD_EXPLICIT_SCHED)) == 0) {
			struct sched_param param;
			param.sched_priority = PRIORITY_BEIL;
			if ((res = pthread_attr_setschedparam(&attr, &param)) == 0) {
				if ((res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO)) == 0) {
					if ((res = pthread_create(&c_hThread, &attr,
							&CBeil::Handler, (PVOID)this)) == 0) {
						if (c_nDebug)
							MSGLOG("[BEIL]Create message queue(%d).\r\n", c_hMsg);
						return TRUE;
					}
					else	MSGLOG("[BEIL]ERROR:can not create thread for msg(%s).\r\n",
								strerror(errno));
				}
				else	MSGLOG("[BEIL]ERROR:can not set sch policy(%d-%s).\r\n", res, strerror(errno));
			}
			else	MSGLOG("[BEIL]ERROR:can not set sch param(%d-%s).\r\n", res, strerror(errno));
		}
		else	MSGLOG("[BEIL]ERROR:can not set inherit(%d-%s).\r\n", res, strerror(errno));
		mq_close(c_hMsg);
		mq_unlink(NAME_BEIL);
		c_hMsg = -1;
	}
	else
		MSGLOG("[BEIL]ERROR:create mail queue(%s).\r\n",
					strerror(errno));
	return FALSE;
}

BOOL CBeil::CreateShmem()
{
	c_hShm = shm_open(NAME_BUCKET, O_RDWR | O_CREAT, 0777);
	if (c_hShm < 0) {
		MSGLOG("[BEIL]ERROR:open(%s).\r\n", strerror(errno));
		return FALSE;
	}
	if (ftruncate(c_hShm, sizeof(BUCKET)) < 0) {
		MSGLOG("[BEIL]ERROR:ftruncate(%s).\r\n", strerror(errno));
		close(c_hShm);
		shm_unlink(NAME_BUCKET);
		return FALSE;
	}
	c_pBucket = (PBUCKET)mmap(0, sizeof(BUCKET), PROT_READ | PROT_WRITE, MAP_SHARED, c_hShm, 0);
	if (c_pBucket == MAP_FAILED) {
		MSGLOG("[BEIL]ERROR:mmap(%s).\r\n", strerror(errno));
		close(c_hShm);
		shm_unlink(NAME_BUCKET);
		return FALSE;
	}
	if (c_nDebug)	MSGLOG("[BEIL]pBucket is %08X.\r\n", c_pBucket);
	memset((PVOID)c_pBucket, 0, sizeof(BUCKET));
	return TRUE;
}
