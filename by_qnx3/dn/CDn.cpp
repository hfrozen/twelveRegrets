/**
 * @file	CDn.cpp
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
#include <errno.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "../Common/Refer.h"
#include "../Common/Infer.h"
#include "../Base/Track.h"
#include "../Base/CString.h"
#include "../Part/CTool.h"
#include "CDn.h"

#define	DN_VERSION		1.00

CDn::CDn()
{
	c_hThreadWatch = INVALID_HANDLE;
	c_hDtq = INVALID_HANDLE;
	c_hDtp = INVALID_HANDLE;
	c_pDtp = NULL;
}

CDn::~CDn()
{
	Destroy();
}

PVOID CDn::WatchEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CDn* pDn = (CDn*)pVoid;
	if (pDn != NULL)	pDn->Watch();
	pthread_exit(NULL);
	return NULL;
}

void CDn::Watch()
{
	while (true) {
		TASKCMD cmd;
		if (!Validity(mq_receive(c_hDtq, (char*)&cmd, sizeof(TASKCMD), NULL))) {
			TRACK("DN>ERR:mq_receive!(%s)\n", strerror(errno));
			continue;
		}
		switch (cmd.wCmd) {
		case TASKCMD_SCANUSB :
			ScanUsb();
			break;
		case TASKCMD_DOWNLOAD :
			c_pDtp->bail.wResult = 0;
			Download();
			break;
		default :
			TRACK("DN>ERR:Invalid task cmd!(%d)\n", cmd.wCmd);
			break;
		}
	}
}

void CDn::Destroy()
{
	if (Validity(c_hThreadWatch)) {
		int res = pthread_cancel(c_hThreadWatch);
		PVOID pResult;
		if (res == EOK)	res = pthread_join(c_hThreadWatch, &pResult);
		if (res != EOK)	TRACK("DN>ERR:thread shutdown!(%s)\n", strerror(errno));
		c_hThreadWatch = INVALID_HANDLE;
	}
	if (Validity(c_hDtq)) {
		mq_close(c_hDtq);
		mq_unlink(DTQ_STR);
		c_hDtq = INVALID_HANDLE;
	}
	if (Validity(c_hDtp)) {
		close(c_hDtp);
		shm_unlink(DTPACK_STR);
		c_hDtp = INVALID_HANDLE;
		c_pDtp = NULL;
	}
}

int CDn::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("DN>ERR:can not set inherit!(%s)\n", strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("DN>ERR:can not set sch param!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("DN>ERR:can not set sch policy!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("DN>ERR:can not create thread!(%s)\n", strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

bool CDn::ScanUsb()
{
	DIR* pDir;
	if (c_pDtp->bail.wState & (1 << DNSTATE_SEIZEUSB)) {
		pDir = opendir(USB_PATHA);
		if (pDir != NULL) {
			closedir(pDir);
			return true;
		}
		c_pDtp->bail.wState &= ~(1 << DNSTATE_SEIZEUSB);
	}

	_TWATCHBEGIN();

	pDir = opendir("/dev");
	if (pDir != NULL) {
		struct dirent* pEnt;
		while ((pEnt = readdir(pDir)) != NULL) {
			strncpy(c_cBuf, pEnt->d_name, 256);
			if (strlen(c_cBuf) > 4) {
				c_cBuf[4] = '\0';
				if (!strcmp(c_cBuf, "hd1t")) {
					strncpy(c_cBuf, pEnt->d_name, 256);
					closedir(pDir);

					TRACK("DN:find %s\n", c_cBuf);
					char cmd[256];
					sprintf(cmd, "mount -tdos /dev/%s %s", c_cBuf, USB_PATHA);
					c_pDtp->bail.wState |= (1 << DNSTATE_MOUNTING);
					system(cmd);
					c_pDtp->bail.wState &= ~(1 << DNSTATE_MOUNTING);
					DIR* pDiru = opendir(USB_PATHA);
					if (pDiru != NULL) {
						closedir(pDiru);
						c_pDtp->bail.wState |= (1 << DNSTATE_SEIZEUSB);
						TRACK("DN:mount usb\n");
						_TWATCHMEASURED(__sec_);
						TimeLog(&c_pDtp->bail.mi.sc, __sec_);
						return true;

					}
					TRACK("DN>ERR:usb mount failed!(%s)\n", strerror(errno));
				}
			}
		}
		closedir(pDir);
	}
	_TWATCHMEASURED(__sec_);
	TimeLog(&c_pDtp->bail.mi.sc, __sec_);
	return false;
}

void CDn::Download()
{
	c_pDtp->bail.wRatioT = 0;
	int iArcTotal = SearchOtherFile(ARCHIVE_PATH, CURTOD_SIGN);
	int iAnoTotal = SearchOtherFile(ANOTHER_PATH, '*');
	if ((iArcTotal + iAnoTotal) == 0) {
		TRACK("DN>ERR:no files to download!\n");
		c_pDtp->bail.wResult = DNRESULT_NOFILES;
		return;
	}

	_TWATCHBEGIN();

	int iFiles = 0;
	char fsbuf[256] = { 0 };
	char ffbuf[256] = { 0 };
	char fdbuf[256] = { 0 };
	TRACK("DN:begin -------------------------------------\n");
	if (iArcTotal > 0) {
		int res = MakeDirectory(USB_PATHB);
		if (res != 0 && res != EEXIST) {
			TRACK("DN>ERR:can not make download archive directory!(%s)\n", strerror(errno));
			TRACK("DN:fail1 -------------------------------------\n");
			_TWATCHMEASURED(__sec_);
			TimeLog(&c_pDtp->bail.mi.dn, __sec_);
			c_pDtp->bail.wResult = DNRESULT_CANNOTMAKEDIR1;
			return;
		}
		while (FindOtherFile(fsbuf, ARCHIVE_PATH, CURTOD_SIGN)) {
			CString strFileName(fsbuf);
			int pos = strFileName.Find('.');
			if (pos >= 5) {
				CString strExtName = strFileName.Mid(pos + 1);
				strFileName = strFileName.Left(pos);
				CString strDirName = strFileName.Mid(pos - 4);
				CString strDestName = strFileName.Left(pos - 4);
				sprintf(ffbuf, "%s/%s", USB_PATHB, (const char*)strDirName);
				res = MakeDirectory(ffbuf);
				if (res != 0 && res != EEXIST) {
					TRACK("DN>ERR:can not make download archive directory!(%s)\n", strerror(errno));
					TRACK("DN:fail2 -------------------------------------\n");
					_TWATCHMEASURED(__sec_);
					TimeLog(&c_pDtp->bail.mi.dn, __sec_);
					c_pDtp->bail.wResult = DNRESULT_CANNOTMAKEDIR2;
					return;
				}
				sprintf(fdbuf, "%s.%s", (const char*)strDestName, (const char*)strExtName);
			}
			else {
				sprintf(ffbuf, "%s", USB_PATHB);
				strncpy(fdbuf, fsbuf, 256);
			}
			if (!MoveFile(ffbuf, fdbuf, ARCHIVE_PATH, fsbuf)) {
				TRACK("DN>ERR:can not move file!%s/%s<-%s/%s\n", ffbuf, fdbuf, ARCHIVE_PATH, fsbuf);
				TRACK("DN:fail3 -------------------------------------\n");
				_TWATCHMEASURED(__sec_);
				TimeLog(&c_pDtp->bail.mi.dn, __sec_);
				c_pDtp->bail.wResult = DNRESULT_FAIL1;
				return;
			}
			++ iFiles;
			WORD w = (WORD)(iFiles ++ * 1000 / (iArcTotal + iAnoTotal));
			if (w >= 1000)	c_pDtp->bail.wRatioT = 1000;
			else	c_pDtp->bail.wRatioT = w;
		}
	}

	if (iAnoTotal > 0) {
		int res = MakeDirectory(USB_PATHC);
		if (res != 0 && res != EEXIST) {
			TRACK("DN>ERR:can not make download archive directory!(%s)\n", strerror(errno));
			TRACK("DN:fail4 -------------------------------------\n");
			_TWATCHMEASURED(__sec_);
			TimeLog(&c_pDtp->bail.mi.dn, __sec_);
			c_pDtp->bail.wResult = DNRESULT_CANNOTMAKEDIR3;
			return;
		}
		while (FindOtherFile(fsbuf, ANOTHER_PATH, '*')) {
			if (!MoveFile(USB_PATHC, fsbuf, ANOTHER_PATH, fsbuf)) {
				TRACK("DN>ERR:can not move file!%s/%s<-%s/%s\n", USB_PATHC, fsbuf, ANOTHER_PATH, fsbuf);
				TRACK("DN:fail5 -------------------------------------\n");
				_TWATCHMEASURED(__sec_);
				TimeLog(&c_pDtp->bail.mi.dn, __sec_);
				c_pDtp->bail.wResult = DNRESULT_FAIL2;
				return;
			}
			++ iFiles;
			WORD w = (WORD)(iFiles ++ * 1000 / (iArcTotal + iAnoTotal));
			if (w >= 1000)	c_pDtp->bail.wRatioT = 1000;
			else	c_pDtp->bail.wRatioT = w;
		}
	}

	_TWATCHMEASURED(__sec_);
	TimeLog(&c_pDtp->bail.mi.dn, __sec_);
	c_pDtp->bail.wResult = DNRESULT_SUCCESS;
}

int CDn::SearchOtherFile(PSZ pPath, char cDeselSign)
{
	if (pPath == NULL || pPath[0] == '\0')	return 0;

	int iLength = 0;
	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (cDeselSign == '*' || pEntry->d_name[0] != cDeselSign)
				++ iLength;
		}
		closedir(pDir);
	}
	return iLength;
}

bool CDn::FindOtherFile(char* pBuf, PSZ pPath, char cDeselSign)
{
	if (pPath == NULL || pPath[0] == '\0')	return false;

	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			if (cDeselSign == '*' || pEntry->d_name[0] != cDeselSign) {
				strncpy(pBuf, pEntry->d_name, 256);
				closedir(pDir);
				return true;
			}
		}
		closedir(pDir);
	}
	return false;
}

bool CDn::MoveFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile)
{
	char dbuf[256];
	sprintf(dbuf, "%s/%s", pDestPath, pDestFile);
	char sbuf[256];
	sprintf(sbuf, "%s/%s", pSrcPath, pSrcFile);

	int fs = open(sbuf, O_RDONLY);
	if (fs < 0) {
		TRACK("DN>ERR:move fail1!(%s-%s)\n", sbuf, strerror(errno));
		return false;
	}

	int fd = open(dbuf, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		TRACK("DN>ERR:move fail2!(%s-%s)\n", dbuf, strerror(errno));
		return false;
	}

	int rres = 0;
	char buf[8192];
	do {
		rres = read(fs, buf, 8192);
		if (rres < 0) {
			close(fs);
			close(fd);
			TRACK("DN>ERR:move fail3!(%s)\n", strerror(errno));
			return false;
		}
		int wres = write(fd, buf, rres);
		if (wres < 0) {
			close(fs);
			close(fd);
			TRACK("DN>ERR:move fail4!(%s)\n", strerror(errno));
			return false;
		}
	} while (rres >= 8192);
	close(fs);
	close(fd);

	rres = remove(sbuf);
	if (rres < 0) {
		TRACK("DN>ERR:move fail5!(%s)\n", strerror(errno));
		return false;
	}
	return true;
}

int CDn::MakeDirectory(PSZ pPath)
{
	DIR* pDir = opendir(pPath);
	if (pDir == NULL)	return (mkdir(pPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));
	else	closedir(pDir);
	return 0;
}

void CDn::TimeLog(PMSRTIMEDW pMtd, double sec)
{
	QWORD qw = (QWORD)(sec * 1e6);
	if (qw < 0xffffffff)	pMtd->dwNor = (DWORD)qw;
	else	pMtd->dwNor = 0xffffffff;
	if (pMtd->dwMin > pMtd->dwNor)	pMtd->dwMin = pMtd->dwNor;
	if (pMtd->dwMax < pMtd->dwNor)	pMtd->dwMax = pMtd->dwNor;
}

bool CDn::Launch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("dn begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	TRACK("dn ver. %.2f %s %s\n", DN_VERSION, __DATE__, __TIME__);

	mq_unlink(DTQ_STR);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_TASKCMD;
	attr.mq_msgsize = sizeof(TASKCMD);

	c_hDtq = mq_open(DTQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (!Validity(c_hDtq)) {
		TRACK("DN>ERR:can not open queue!(%s)\n", strerror(errno));
		return false;
	}

	if (CreateThread(&c_hThreadWatch, &CDn::WatchEntry, (PVOID)this, PRIORITY_TASKCMD) != EOK) {
		TRACK("DN>ERR:can not create watch thread!(%s)\n", strerror(errno));
		return false;
	}

	c_hDtp = shm_open(DTPACK_STR, O_RDWR | O_CREAT, 0777);
	if (!Validity(c_hDtp)) {
		TRACK("DN>ERR:can not open shared memory!(%s)\n", strerror(errno));
		return false;
	}
	if (ftruncate(c_hDtp, sizeof(DTPACK)) < 0) {
		TRACK("DN>ERR:can not truncate to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pDtp = (PDTPACK)mmap(0, sizeof(DTPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hDtp, 0);
	if (c_pDtp == MAP_FAILED) {
		TRACK("DN>ERR:can not map to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	memset((PVOID)c_pDtp, 0, sizeof(DTPACK));
	c_pDtp->bail.mi.dn.dwMin = c_pDtp->bail.mi.sc.dwMin = DEFAULT_MINDW;

	return true;
}
