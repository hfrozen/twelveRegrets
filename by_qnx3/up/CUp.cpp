/**
 * @file	CUp.cpp
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../Common/Refer.h"
#include "../Common/Infer.h"
#include "../Base/Track.h"
#include "../Base/CString.h"
#include "../Part/CTool.h"
#include "CUp.h"

#define	UP_VERSION		1.01
#define	REPORTTCP_IP	"10.240.208.249"
//#define	REPORTTCP_IP	"192.168.10.200"
#define	REPORTTCP_PORT	50091

CUp::CUp()
{
	c_iSock = INVALID_HANDLE;
	c_hThreadWatch = INVALID_HANDLE;
	c_hUtq = INVALID_HANDLE;
	c_hUtp = INVALID_HANDLE;
	c_pUtp = NULL;
	c_wPortNo = REPORTTCP_PORT;
	strncpy(c_cIpBuf, REPORTTCP_IP, sizeof(c_cIpBuf));
}

CUp::~CUp()
{
	Destroy();
}

PVOID CUp::WatchEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CUp* pUp = (CUp*)pVoid;
	if (pUp != NULL)	pUp->Watch();
	pthread_exit(NULL);
	return NULL;		// ????? meaningless & non-executable code
}

void CUp::Watch()
{
	while (true) {
		TASKCMD cmd;
		if (!Validity(mq_receive(c_hUtq, (char*)&cmd, sizeof(TASKCMD), NULL))) {
			TRACK("UP>ERR:mq_receive!(%s)\n", strerror(errno));
			continue;
		}
		switch (cmd.wCmd) {
		// 200212
		case TASKCMD_CONNECT :
			if (!(c_pUtp->bail.wRepState & (1 << REPST_CONNECTING))) {
				c_pUtp->bail.wRepState |= (1 << REPST_CONNECTING);
				if (Bridge())
					c_pUtp->bail.wRepState |= (1 << REPST_CONNECTED);
				c_pUtp->bail.wRepState &= ~(1 << REPST_CONNECTING);
			}
			break;
		case TASKCMD_REPORT :
			if (!(c_pUtp->bail.wRepState & (1 << REPST_SENDING))) {
				c_pUtp->bail.wRepState |= (1 << REPST_SENDING);
				Report();
				c_pUtp->bail.wRepState &= ~(1 << REPST_SENDING);
			}
			break;
		case TASKCMD_UPLOAD :
			c_pUtp->bail.wResult = 0;
			Upload();
			break;
		default :
			TRACK("UP>ERR:Invalid task cmd!(%d)\n", cmd.wCmd);
			break;
		}
	}
}

void CUp::Destroy()
{
	if (Validity(c_hThreadWatch)) {
		int res = pthread_cancel(c_hThreadWatch);
		PVOID pResult;
		if (res == EOK)	res = pthread_join(c_hThreadWatch, &pResult);
		if (res != EOK)	TRACK("UP>ERR:thread shutdown!(%s)\n", strerror(errno));
		c_hThreadWatch = INVALID_HANDLE;
	}
	if (Validity(c_hUtq)) {
		mq_close(c_hUtq);
		mq_unlink(UTQ_STR);
		c_hUtq = INVALID_HANDLE;
	}
	if (Validity(c_hUtp)) {
		close(c_hUtp);
		shm_unlink(UTPACK_STR);
		c_hUtp = INVALID_HANDLE;
		c_pUtp = NULL;
	}
	// 200212
	if (Validity(c_iSock)) {
		close(c_iSock);
		c_iSock = INVALID_HANDLE;
	}
}

int CUp::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("UP>ERR:can not set inherit!(%s)\n", strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("UP>ERR:can not set sch param!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("UP>ERR:can not set sch policy!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("UP>ERR:can not create thread!(%s)\n", strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

// 200212
bool CUp::Connect()
{
	// 200212
	c_iSock = socket(AF_INET, SOCK_STREAM, 0);
	if (!Validity(c_iSock)) {
		TRACK("UP>ERR:can not open socket!(%s)\n", strerror(errno));
		return false;
	}

	//int reuse = 1;
	//int ret = setsockopt(c_iSock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	//if (!Validity(ret)) {
	//	TRACK("UP>ERR:setsockopt() reuse failed!(%s)\n", strerror(errno));
	//	return false;
	//}
	//
	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(c_cIpBuf);
	saddr.sin_port = htons(c_wPortNo);
	//TRACK("UP:connect to %#x %d\n", saddr.sin_addr.s_addr, saddr.sin_port);
	TRACK("UP:connect to %s [%d]\n", inet_ntoa(saddr.sin_addr), htons(saddr.sin_port));
	int ret = connect(c_iSock, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret >= 0)	return true;
	if (errno == EINPROGRESS) {
		do {
			fd_set wfd;
			FD_ZERO(&wfd);
			FD_SET(c_iSock, &wfd);

			timeval to;
			to.tv_sec = 1;
			to.tv_usec = 0;
			ret = select(c_iSock + 1, NULL, &wfd, NULL, &to);
			if (ret == 0) {
				TRACK("UP>ERR:connect timeout!\n");
				close(c_iSock);
				c_iSock = INVALID_HANDLE;
				return false;
			}
		} while (ret < 0 && errno == EINTR);
	}
	if (ret < 0) {
		TRACK("UP>ERR:connect failed!(%d-%s)\n", c_iSock, strerror(errno));
		close(c_iSock);
		c_iSock = INVALID_HANDLE;
		return false;
	}

	return true;
}

bool CUp::Bridge()
{
	//if (c_pUtp->bail.wRepState & (1 << REPST_CONNECTED))
		return true;

	//_TWATCHBEGIN();

	//bool bRes = Connect();
	//_TWATCHMEASURED(__sec_);
	//TimeLog(&c_pUtp->bail.mi.cp, __sec_);
	//return bRes;
}

bool CUp::Send()
{
	//if (!(c_pUtp->bail.wRepState & (1 << REPST_CONNECTED)))
	//	return false;

	if (c_iSock == INVALID_HANDLE) {
		c_iSock = socket(AF_INET, SOCK_DGRAM, 0);
		if (!Validity(c_iSock)) {
			TRACK("UP>ERR:can not open socket!(%s)\n", strerror(errno));
			return false;
		}
	}

	struct sockaddr_in saddr;
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(c_cIpBuf);
	saddr.sin_port = htons(c_wPortNo);

	ssize_t ret = (int)sendto(c_iSock, c_cTodBuf, SIZE_TOD, MSG_NOSIGNAL, (struct sockaddr*)&saddr, sizeof(saddr));
	if (ret < 0) {
		TRACK("UP>err:sendto failed %s [%d]!(%s)\n", inet_ntoa(saddr.sin_addr), htons(saddr.sin_port), strerror(errno));
		return false;
	}

	TRACK("UP:sendto %d to %s [%d]\n", (int)ret, inet_ntoa(saddr.sin_addr), htons(saddr.sin_port));
	return true;
}

bool CUp::Report()
{
	_TWATCHBEGIN();

	memcpy(c_cTodBuf, c_pUtp->todt.c, SIZE_TOD);
	bool bRes = Send();
	if (!bRes) {
		c_pUtp->bail.wRepState &= ~(1 << REPST_CONNECTED);
		close(c_iSock);
		TRACK("UP:close socket!\n");
	}

	_TWATCHMEASURED(__sec_);
	TimeLog(&c_pUtp->bail.mi.rp, __sec_);

	return bRes;
}

void CUp::Upload()
{
	if (c_pUtp->env.szDip[c_pUtp->bail.wCurIP][0] == '\0') {
		TRACK("UP>ERR:non destination ip!\n");
		c_pUtp->bail.wResult = UPRESULT_NONIP;
		return;
	}

	c_pUtp->bail.wRatioT = 0;
	int iTotal = FindFile(ARCHIVE_PATH, ZIPTOD_SIGN);
	if (iTotal == 0) {
		TRACK("UP>ERR:no files to upload!\n");
		c_pUtp->bail.wResult = UPRESULT_NOFILES;
		return;
	}

	_TWATCHBEGIN();

	int iFiles = 0;
	char fnbuf[256] = { 0, };
	TRACK("UP:begin ---------------------------------------\n");
	while (FindFile(fnbuf, ARCHIVE_PATH, 19, ZIPTOD_SIGN)) {
		if (c_pUtp->bail.wCmd & (1 << UPCMD_ABORT)) {
			TRACK("UP:abort during upload!\n");
			c_pUtp->bail.wResult = UPRESULT_ABORT;
			return;
		}
		if (!Transfer(ARCHIVE_PATH, fnbuf)) {
			TRACK("UP:fail ----------------------------------------\n");
			_TWATCHMEASURED(__sec_);
			TimeLog(&c_pUtp->bail.mi.up, __sec_);
			c_pUtp->bail.wResult = UPRESULT_FAIL;
			return;
		}
		WORD w = (WORD)(iFiles ++ * 1000 / iTotal);
		if (w >= 1000)	c_pUtp->bail.wRatioT = 1000;
		else	c_pUtp->bail.wRatioT = w;
	}

	TRACK("UP:end -----------------------------------------\n");
	_TWATCHMEASURED(__sec_);
	TimeLog(&c_pUtp->bail.mi.up, __sec_);
	c_pUtp->bail.wResult = UPRESULT_SUCCESS;
}

bool CUp::Transfer(PSZ pPath, char* pName)
{
	int ssize = (int)GetFileSize(pPath, pName);
	if (ssize <= 0) {
		MoveFile(ANOTHER_PATH, pName, pPath, pName);
		TRACK("UP>ERR:file size!(%s)\n", pName);
		return true;
	}

	CString strFileName(pName);
	int pos = strFileName.Find('.');
	if (pos < 5) {
		MoveFile(ANOTHER_PATH, pName, pPath, pName);
		TRACK("UP>ERR:file name unmatch!(%s)\n", pName);
		return true;
	}
	CString strExtName = strFileName.Mid(pos + 1);
	strFileName = strFileName.Left(pos);
	CString strDirName = strFileName.Mid(pos - 4);
	CString strDestName = strFileName.Left(pos - 4);

	char dname[256] = { 0, };
	sprintf(dname, "%s.%s", (const char*)strDestName, (const char*)strExtName);

	// upload
	memset(c_cBuf, 0, 1024);
	sprintf(c_cBuf, "ftp -u ftp://anonymous:@%s/%s/%s %s/%s > %s",
			c_pUtp->env.szDip[c_pUtp->bail.wCurIP],
			(const char*)strDirName, dname, pPath, pName, UPLOADRESULT_FILE);
	TRACK("UP:%s\n", c_cBuf);
	system(c_cBuf);

	// re- download
	sprintf(c_cBuf, "ftp -o %s ftp://anonymous:@%s/%s/%s > %s",
			CMP_FILE, c_pUtp->env.szDip[c_pUtp->bail.wCurIP],
			(const char*)strDirName, dname, UPLOADRESULT_FILE);
	TRACK("UP:%s\n", c_cBuf);
	system(c_cBuf);

	bool bRes = true;
	// compare size
	int dsize = (int)GetFileSize(CMP_FILE);
	if (dsize == ssize) {
		TRACK("UP:%s upload complete\n", pName);
		char sname[256];
		sprintf(sname, "%s/%s", pPath, pName);
		pName[0] = UPETOD_SIGN;
		sprintf(dname, "%s/%s", pPath, pName);
		RenameFile(dname, sname);
	}
	else {
		TRACK("UP>%s upload failed!\n", pName);
		bRes = false;		// ????? if the file size error persists ?????
	}
	remove(CMP_FILE);

	return bRes;
}

bool CUp::MoveFile(PSZ pDestPath, PSZ pDestFile, PSZ pSrcPath, PSZ pSrcFile)
{
	char dbuf[256];
	sprintf(dbuf, "%s/%s", pDestPath, pDestFile);
	char sbuf[256];
	sprintf(sbuf, "%s/%s", pSrcPath, pSrcFile);

	int fs = open(sbuf, O_RDONLY);
	if (fs < 0) {
		TRACK("UP>ERR:move fail1!(%s-%s)\n", sbuf, strerror(errno));
		return false;
	}

	int fd = open(dbuf, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (fd < 0) {
		close(fs);
		TRACK("UP>ERR:move fail2!(%s-%s)\n", dbuf, strerror(errno));
		return false;
	}

	int rres = 0;
	char buf[8192];
	do {
		rres = read(fs, buf, 8192);
		if (rres < 0) {
			close(fs);
			close(fd);
			TRACK("UP>ERR:move fail3!(%s)\n", strerror(errno));
			return false;
		}
		int wres = write(fd, buf, rres);
		if (wres < 0) {
			close(fs);
			close(fd);
			TRACK("UP>ERR:move fail4!(%s)\n", strerror(errno));
			return false;
		}
	} while (rres >= 8192);
	close(fs);
	close(fd);

	rres = remove(sbuf);
	if (rres < 0) {
		TRACK("UP>ERR:move fail5!(%s)\n", strerror(errno));
		return false;
	}
	return true;
}

bool CUp::RenameFile(PSZ pNew, PSZ pOld)
{
	if (pNew == NULL || pOld == NULL)	return false;

	if (rename(pOld, pNew) >= 0)	return true;
	return false;
}

int CUp::FindFile(PSZ pPath, char cSign1, char cSign2)
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

bool CUp::FindFile(char* pBuf, PSZ pPath, int iFileNameLength, char cSign1, char cSign2)
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

int64_t CUp::GetFileSize(PSZ pPath, PSZ pFile)
{
	if (pPath == NULL || pFile == NULL)	return -1;

	char buf[256];
	sprintf(buf, "%s/%s", pPath, pFile);
	return GetFileSize(buf);
}

int64_t CUp::GetFileSize(PSZ pPath)
{
	if (pPath == NULL)	return -1;

	struct stat st;
	if (stat(pPath, &st) >= 0)	return st.st_size;
	return -1;
}

void CUp::TimeLog(PMSRTIMEDW pMtd, double sec)
{
	QWORD qw = (QWORD)(sec * 1e6);
	if (qw < 0xffffffff)	pMtd->dwNor = (DWORD)qw;
	else	pMtd->dwNor = 0xffffffff;
	if (pMtd->dwMin > pMtd->dwNor)	pMtd->dwMin = pMtd->dwNor;
	if (pMtd->dwMax < pMtd->dwNor)	pMtd->dwMax = pMtd->dwNor;
}

bool CUp::Launch(int argc, char* argv1, char* argv2)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("up begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	TRACK("up ver. %.2f %s %s\n", UP_VERSION, __DATE__, __TIME__);

	if (argc > 1)	strncpy(c_cIpBuf, argv1, sizeof(c_cIpBuf));
	else	strncpy(c_cIpBuf, REPORTTCP_IP, sizeof(c_cIpBuf));
	if (argc > 2)	c_wPortNo = (WORD)atoi(argv2);
	else	c_wPortNo = REPORTTCP_PORT;
	TRACK("UP:destination IP: %s [%d]\n", c_cIpBuf, c_wPortNo);

	mq_unlink(UTQ_STR);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_TASKCMD;
	attr.mq_msgsize = sizeof(TASKCMD);

	c_hUtq = mq_open(UTQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (!Validity(c_hUtq)) {
		TRACK("UP>ERR:can not open queue!(%s)\n", strerror(errno));
		return false;
	}

	if (CreateThread(&c_hThreadWatch, &CUp::WatchEntry, (PVOID)this, PRIORITY_TASKCMD) != EOK) {
		TRACK("UP>ERR:can not create watch thread!(%s)\n", strerror(errno));
		return false;
	}

	c_hUtp = shm_open(UTPACK_STR, O_RDWR | O_CREAT, 0777);
	if (!Validity(c_hUtp)) {
		TRACK("UP>ERR:can not open shared memory!(%s)\n", strerror(errno));
		return false;
	}
	if (ftruncate(c_hUtp, sizeof(UTPACK)) < 0) {
		TRACK("UP>ERR:can not truncate to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pUtp = (PUTPACK)mmap(0, sizeof(UTPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hUtp, 0);
	if (c_pUtp == MAP_FAILED) {
		TRACK("UP>ERR:can not map to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	memset((PVOID)c_pUtp, 0, sizeof(UTPACK));
	c_pUtp->bail.mi.up.dwMin = DEFAULT_MINDW;
	c_pUtp->bail.mi.cp.dwMin = DEFAULT_MINDW;
	c_pUtp->bail.mi.rp.dwMin = DEFAULT_MINDW;

	return true;
}
