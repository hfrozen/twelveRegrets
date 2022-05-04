/**
 * @file	CPi.cpp
 * @brief
 * @details
 * @author
 * @data
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>

#include "../Common/Refer.h"
#include "../Common/Infer.h"
#include "../Base/Track.h"
#include "../Part/CTool.h"
#include "CPi.h"

#define	PI_VERSION		1.00

CPi::CPi()
{
	c_hThreadWatch = INVALID_HANDLE;
	c_hPtq = INVALID_HANDLE;
	c_hPtp = INVALID_HANDLE;
	c_pPtp = NULL;
}

CPi::~CPi()
{
	Destroy();
}

PVOID CPi::WatchEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CPi* pPi = (CPi*)pVoid;
	if (pPi != NULL)	pPi->Watch();
	pthread_exit(NULL);
	return NULL;		// ????? meaningless & non-executable code
}

void CPi::Watch()
{
	while (true) {
		TASKCMD cmd;
		if (!Validity(mq_receive(c_hPtq, (char*)&cmd, sizeof(TASKCMD), NULL))) {
			TRACK("PI>ERR:mq_receive!(%s)\n", strerror(errno));
			continue;
		}
		switch (cmd.wCmd) {
		case TASKCMD_WIDEPING :
		case TASKCMD_NARROWPING :
			c_pPtp->bail.wPiState |= (1 << PISTATE_PINGING);
			Ping(cmd.wCmd == TASKCMD_WIDEPING ? true : false);
			c_pPtp->bail.wPiState &= ~(1 << PISTATE_PINGING);
			break;
		default :
			TRACK("PI>ERR:Invalid task cmd!(%d)\n", cmd.wCmd);
			break;
		}
	}
}

void CPi::Destroy()
{
	if (Validity(c_hThreadWatch)) {
		int res = pthread_cancel(c_hThreadWatch);
		PVOID pResult;
		if (res == EOK)	res = pthread_join(c_hThreadWatch, &pResult);
		if (res != EOK)	TRACK("PI>ERR:thread shutdown!(%s)\n", strerror(errno));
		c_hThreadWatch = INVALID_HANDLE;
	}
	if (Validity(c_hPtq)) {
		mq_close(c_hPtq);
		mq_unlink(PTQ_STR);
		c_hPtq = INVALID_HANDLE;
	}
	if (Validity(c_hPtp)) {
		close(c_hPtp);
		shm_unlink(PTPACK_STR);
		c_hPtp = INVALID_HANDLE;
		c_pPtp = NULL;
	}
}

int CPi::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("PI>ERR:can not set inherit!(%s)\n", strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("PI>ERR:can not set sch param!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("PI>ERR:can not set sch policy!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("PI>ERR:can not create thread!(%s)\n", strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

void CPi::Ping(bool bWide)
{
	_TWATCHBEGIN();

	remove(PINGRESULT_FILE);
	memset(c_cBuf, 0, 1024);

	GETTOOL(pTool);
	if (!bWide && c_pPtp->bail.sr.qwShape != 0)	c_pPtp->bail.sr.wCurID = pTool->GetFirstBit(c_pPtp->bail.sr.qwShape, true);
	sprintf(c_cBuf, "/proc/boot/ping -c1 %s > %s", c_pPtp->env.szDip[c_pPtp->bail.sr.wCurID], PINGRESULT_FILE);
	system(c_cBuf);
	int ret = 0;
	FILE* fp = fopen(PINGRESULT_FILE, "r");
	if (fp != NULL) {
		int size = fread(c_cBuf, 1, 1024, fp);
		fclose(fp);
		if (size > 0) {
			GETTOOL(pTool);
			int pos = pTool->FindStr(c_cBuf, (char*)"packets received");
			if (pos > 2 && c_cBuf[pos - 2] == '1')	ret = 1;
		}
	}

	if (ret)	c_pPtp->bail.sr.qwShape |= (QWORD)(1 << c_pPtp->bail.sr.wCurID);
	else	c_pPtp->bail.sr.qwShape &= (QWORD)~(1 << c_pPtp->bail.sr.wCurID);
	if (bWide || c_pPtp->bail.sr.qwShape == 0) {
		if (c_pPtp->env.szDip[++ c_pPtp->bail.sr.wCurID][0] == '\0')	c_pPtp->bail.sr.wCurID = 0;
	}

	_TWATCHMEASURED(__sec_);
	TimeLog(&c_pPtp->bail.mi.ping, __sec_);
	//WORD ms = __sec_ * 1000;
	//c_pPtp->bail.mi.ping.dbNor = __sec_;
	//if (c_pPtp->bail.mi.ping.dbMin > __sec_)	c_pPtp->bail.mi.ping.dbMin = __sec_;
	//if (c_pPtp->bail.mi.ping.dbMax < __sec_)	c_pPtp->bail.mi.ping.dbMax = __sec_;
}

void CPi::TimeLog(PMSRTIMEDW pMtd, double sec)
{
	QWORD qw = (QWORD)(sec * 1e6);
	if (qw < 0xffffffff)	pMtd->dwNor = (DWORD)qw;
	else	pMtd->dwNor = 0xffffffff;
	if (pMtd->dwMin > pMtd->dwNor)	pMtd->dwMin = pMtd->dwNor;
	if (pMtd->dwMax < pMtd->dwNor)	pMtd->dwMax = pMtd->dwNor;
}

bool CPi::Launch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("pi begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	TRACK("pi ver. %.2f %s %s\n", PI_VERSION, __DATE__, __TIME__);

	mq_unlink(PTQ_STR);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_TASKCMD;
	attr.mq_msgsize = sizeof(TASKCMD);

	c_hPtq = mq_open(PTQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (!Validity(c_hPtq)) {
		TRACK("PI>ERR:can not open queue!(%s)\n", strerror(errno));
		return false;
	}

	if (CreateThread(&c_hThreadWatch, &CPi::WatchEntry, (PVOID)this, PRIORITY_TASKCMD) != EOK) {
		TRACK("PI>ERR:can not create watch thread!(%s)\n", strerror(errno));
		return false;
	}

	c_hPtp = shm_open(PTPACK_STR, O_RDWR | O_CREAT, 0777);
	if (!Validity(c_hPtp)) {
		TRACK("PI>ERR:can not open shared memory!(%s)\n", strerror(errno));
		return false;
	}
	if (ftruncate(c_hPtp, sizeof(PTPACK)) < 0) {
		TRACK("PI>ERR:can not truncate to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pPtp = (PPTPACK)mmap(0, sizeof(PTPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hPtp, 0);
	if (c_pPtp == MAP_FAILED) {
		TRACK("PI>ERR:can not map to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	memset((PVOID)c_pPtp, 0, sizeof(PTPACK));
	c_pPtp->bail.mi.ping.dwMin = DEFAULT_MINDW;

	return true;
}
