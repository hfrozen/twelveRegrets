/**
 * @file	CSm.cpp
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
#include <sys/statvfs.h>

#include "../Common/Refer.h"
#include "../Common/Infer.h"
#include "../Part/CTool.h"
#include "../Base/Track.h"
#include "CSm.h"

#define	SM_VERSION		1.00

CSm::CSm()
{
	c_hThreadWatch = c_hThreadAlign = INVALID_HANDLE;
	c_sema.Init();
	c_hStq = INVALID_HANDLE;
	c_hStp = INVALID_HANDLE;
	c_pStp = NULL;

	c_cPrevSec = 0;
	c_curFid.qw = 0;
	c_alignFid.qw = 0;
	c_bAligning = false;
}

CSm::~CSm()
{
	Destroy();
}

PVOID CSm::WatchEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CSm* pSm = (CSm*)pVoid;
	if (pSm != NULL)	pSm->Watch();
	pthread_exit(NULL);
	return NULL;		// ????? meaningless & non-executable code
}

PVOID CSm::FileAlignEntry(PVOID pVoid)
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	CSm* pSm = (CSm*)pVoid;
	if (pSm != NULL)	pSm->FileAlign();
	pthread_exit(NULL);
	return NULL;		// ????? meaningless & non-executable code
}

void CSm::Watch()
{
	while (true) {
		TASKCMD cmd;
		if (!Validity(mq_receive(c_hStq, (char*)&cmd, sizeof(TASKCMD), NULL))) {
			TRACK("SM>ERR:mq_receive!(%s)\n", strerror(errno));
			continue;
		}
		switch (cmd.wCmd) {
		case TASKCMD_TIMESET :
			break;
		case TASKCMD_APPENDTOD :
			c_pStp->bail.wStatus |= (1 << SMSTATUS_APPEND);
			AppendTod();
			c_pStp->bail.wStatus &= ~(1 << SMSTATUS_APPEND);
			break;
		default :
			TRACK("SM>ERR:Invalid task cmd!(%d)\n", cmd.wCmd);
			break;
		}
	}
}

void CSm::Destroy()
{
	if (Validity(c_hThreadWatch)) {
		int res = pthread_cancel(c_hThreadWatch);
		PVOID pResult;
		if (res == EOK)	res = pthread_join(c_hThreadWatch, &pResult);
		if (res != EOK)	TRACK("SM>ERR:thread shutdown!(%s)\n", strerror(errno));
		c_hThreadWatch = INVALID_HANDLE;
	}
	if (Validity(c_hThreadAlign)) {
		c_bAligning = false;
		int res = pthread_cancel(c_hThreadAlign);
		PVOID pResult;
		if (res == EOK)	res = pthread_join(c_hThreadAlign, &pResult);
		if (res != EOK)	TRACK("SM>ERR:thread shutdown!(%s)\n", strerror(errno));
		c_hThreadAlign = INVALID_HANDLE;
	}
	if (Validity(c_hStq)) {
		mq_close(c_hStq);
		mq_unlink(STQ_STR);
		c_hStq = INVALID_HANDLE;
	}
	if (Validity(c_hStp)) {
		close(c_hStp);
		shm_unlink(STPACK_STR);
		c_hStp = INVALID_HANDLE;
		c_pStp = NULL;
	}
}

int CSm::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set inherit!(%s)\n", strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set sch param!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set sch policy!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not create thread!(%s)\n", strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

int CSm::CreateThreadA(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set inherit!(%s)\n", strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set sch param!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set sch policy!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not set detachstate!(%s)\n", strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("SM>ERR:can not create thread!(%s)\n", strerror(errno));
		return res;
	}
	pthread_attr_destroy(&attr);
	return EOK;
}

bool CSm::AppendTod()
{
	_TWATCHBEGIN();

	GETTOOL(pTool);
	BYTE sec = pTool->ToHex(c_pStp->todt.real.td.t.sec.a);

	TODFINDEX ti = ToddBcd2Hex(c_pStp->todt.real.td);
	ti.i.t.r.min /= 5;
	ti.i.t.r.min *= 5;

	char buf[256];
	memset(buf, 0, 256);
	sprintf(buf, "%s/%c%02d%02d%02d%02d%02d%04d", ARCHIVE_PATH, CURTOD_SIGN,
			ti.i.t.r.year, ti.i.t.r.mon, ti.i.t.r.day, ti.i.t.r.hour, ti.i.t.r.min, ti.i.wTrNo);

	FILE* fp = fopen(buf, "a");
	if (fp != NULL) {
		size_t wsize = fwrite(c_pStp->todt.c, 1, SIZE_TOD, fp);
		if (wsize != SIZE_TOD) {
			c_pStp->bail.wResult = PACKERR_CANNOTWRITETOD;
			TRACK("SM>ERR:can not append tod!(%s)\n", strerror(errno));
		//	fclose(fp);
		//	return false;
		}
		fclose(fp);
	}

	if (c_pStp != NULL)	c_pStp->env.wTrNo = ti.i.wTrNo;

	if (c_curFid.qw != 0) {
		BYTE ofs = c_cPrevSec > sec ? 60 : 0;
		BYTE tol = (sec + ofs) - c_cPrevSec;
		if (tol == 0) {
			INCWORD(c_pStp->bail.wRecvErr[0]);
			memset(buf, 0, 256);
			TodtBcd2Str(c_pStp->todt.real.td, buf);
			TRACK("SM>ERR:received same time!(%s-%d)\n", buf, c_pStp->bail.wRecvErr[0]);
		}
		else if (tol > 1) {
			INCWORD(c_pStp->bail.wRecvErr[1]);
			memset(buf, 0, 256);
			TodtBcd2Str(c_pStp->todt.real.td, buf);
			TRACK("SM>ERR:do not receive one or more frame!(%s-%d)\n", buf, c_pStp->bail.wRecvErr[1]);
		}

		if (c_curFid.qw != ti.qw) {
			c_alignFid.qw = c_curFid.qw;
			TRACK("SM:current file changed to %02d%02d%02d%02d%02d%04d.\n",
					ti.i.t.r.year, ti.i.t.r.mon, ti.i.t.r.day, ti.i.t.r.hour, ti.i.t.r.min, ti.i.wTrNo);
			c_sema.Post();
		}
		else {
			struct timeval tv;
			gettimeofday(&tv, NULL);
			struct tm* pTm = localtime(&tv.tv_sec);
			int year = (pTm->tm_year + 1900) % 100;
			int mon = pTm->tm_mon + 1;
			int day = pTm->tm_mday;
			int hour = pTm->tm_hour;
			if (year != ti.i.t.r.year || mon != ti.i.t.r.mon || day != ti.i.t.r.day || hour != ti.i.t.r.hour) {
				pTm->tm_year = (2000 + ti.i.t.r.year) - 1900;
				pTm->tm_mon = ti.i.t.r.mon - 1;
				pTm->tm_mday = ti.i.t.r.day;
				pTm->tm_hour = ti.i.t.r.hour;
				pTm->tm_min = ti.i.t.r.min;
				pTm->tm_sec = 0;
				time_t t = mktime(pTm);
				if (t >= (time_t)0) {
					struct timespec ts;
					ts.tv_sec = t;
					ts.tv_nsec = 0;
					if (clock_settime(CLOCK_REALTIME, &ts) >= 0) {
						system("rtc -s isl12020");
						TRACK("SM:Time set.\n");
					}
				}
			}
		}
	}
	c_cPrevSec = sec;
	c_curFid.qw = ti.qw;

	_TWATCHMEASURED(__sec_);
	TimeLog(&c_pStp->bail.mi.wrt, __sec_);

	return true;
}

int CSm::MakeDirectory(PSZ pPath)
{
	DIR* pDir = opendir(pPath);
	if (pDir == NULL)	return (mkdir(pPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH));
	else	closedir(pDir);
	return 0;
}

int CSm::DeleteFile(PSZ pPath, PSZ pFile)
{
	char buf[256];
	sprintf(buf, "%s/%s", pPath, pFile);
	return (remove(buf));
}

TODFINDEX CSm::ToddBcd2Hex(TODDLM tdl)
{
	GETTOOL(pTool);
	TODFINDEX ti;
	ti.qw = 0;
	ti.i.t.r.year = pTool->ToHex(tdl.t.year.a);
	ti.i.t.r.mon = pTool->ToHex(tdl.t.mon.a);
	ti.i.t.r.day = pTool->ToHex(tdl.t.day.a);
	ti.i.t.r.hour = pTool->ToHex(tdl.t.hour.a);
	ti.i.t.r.min = pTool->ToHex(tdl.t.min.a);

	ti.i.wTrNo = (WORD)pTool->ToHex(tdl.n.th.a) * 100;
	ti.i.wTrNo += (WORD)pTool->ToHex(tdl.n.to.a);
	ti.i.wNull = 0;
	return ti;
}

TODFINDEX CSm::ToddStr2Hex(char* pStr)
{
	int leng = strlen((PSZ)pStr);
	TODFINDEX ti;
	ti.qw = 0;
	if (leng > 10)
		ti.i.wTrNo = (WORD)atoi(pStr + 10);
	if (leng >= 10) {
		*(pStr + 10) = 0;
		ti.i.t.r.min = (BYTE)atoi(pStr + 8);
	}
	if (leng >= 8) {
		*(pStr + 8) = 0;
		ti.i.t.r.hour = (BYTE)atoi(pStr + 6);
	}
	if (leng >= 6) {
		*(pStr + 6) = 0;
		ti.i.t.r.day = (BYTE)atoi(pStr + 4);
	}
	if (leng >= 4) {
		*(pStr + 4) = 0;
		ti.i.t.r.mon = (BYTE)atoi(pStr + 2);
	}
	if (leng >= 2) {
		*(pStr + 2) = 0;
		ti.i.t.r.year = (BYTE)atoi(pStr);
	}
	return ti;
}

void CSm::TodtBcd2Str(TODDLM tdl, char* pStr)
{
	GETTOOL(pTool);
	sprintf(pStr, "%02d/%02d/%02d %02d:%02d:%02d",
			pTool->ToHex(tdl.t.year.a), pTool->ToHex(tdl.t.mon.a), pTool->ToHex(tdl.t.day.a),
			pTool->ToHex(tdl.t.hour.a), pTool->ToHex(tdl.t.min.a), pTool->ToHex(tdl.t.sec.a));
}

void CSm::TimeLog(PMSRTIMEDW pMtd, double sec)
{
	QWORD qw = (QWORD)(sec * 1e6);
	if (qw < 0xffffffff)	pMtd->dwNor = (DWORD)qw;
	else	pMtd->dwNor = 0xffffffff;
	if (pMtd->dwMin > pMtd->dwNor)	pMtd->dwMin = pMtd->dwNor;
	if (pMtd->dwMax < pMtd->dwNor)	pMtd->dwMax = pMtd->dwNor;
}

int64_t CSm::GetSpace(PSZ pPath, bool bSpace)
{
	struct statvfs64 st;
	int64_t size = 0;
	if (!Validity(statvfs64(pPath, &st)))
		TRACK("SM>ERR:can not found path!(%s) %s\n", pPath, strerror(errno));
	else {
		if (bSpace)	size = st.f_bsize * st.f_blocks;
		else	size = st.f_bsize * st.f_bfree;
	}
	return size;
}

bool CSm::ZipFile(PSZ pPath)
{
	char sbuf[256];
	char dbuf[256];
	char buf[256];
	sprintf(sbuf, "%s/%s", ARCHIVE_PATH, pPath);
	strncpy(buf, pPath, 256);
	buf[0] = ZIPTOD_SIGN;
	sprintf(dbuf, "%s/%s", ARCHIVE_PATH, buf);
	sprintf(buf, "zip -m %s %s", dbuf, sbuf);
	int res = system(buf);
	if (res < 0)	TRACK("SM>ERR:%s! (%s)\n", buf, strerror(errno));
	else	TRACK("SM:%s\n", buf);
	return true;
}

void CSm::FileAlign()
{
	char buf[256];

	while(c_bAligning) {
		c_sema.Wait();
		c_pStp->bail.wStatus |= (1 << SMSTATUS_ALIGN);
		_TWATCHBEGIN();

		TRACK("SM:align start %02d%02d%02d%02d%02d%04d ------------------\n",
				c_alignFid.i.t.r.year, c_alignFid.i.t.r.mon, c_alignFid.i.t.r.day,
				c_alignFid.i.t.r.hour, c_alignFid.i.t.r.min, c_alignFid.i.wTrNo);

		TODTHEX todtOld;
		todtOld.dw = c_alignFid.i.t.dw;
		if (todtOld.r.year == 0) {
			TRACK("SM>ERR:wrong year at file align!\n");
			return;
		}

		if (todtOld.r.mon > 1)	-- todtOld.r.mon;
		else {
			todtOld.r.mon = 12;
			-- todtOld.r.year;
		}

		TODFINDEX ti;
		DIR* pDir = opendir(ARCHIVE_PATH);
		if (pDir != NULL) {
			struct dirent* pEntry;
			while ((pEntry = readdir(pDir)) != NULL) {
				if (pEntry->d_name[0] == '.')	continue;

				strncpy(buf, &pEntry->d_name[1], 256);
				ti = ToddStr2Hex(buf);
				//if (dwIndex > c_dwAlignFileDate) {
				//	TRACK("SM>ERR:Found wrong file %s\n", pEntry->d_name);
				//	DeleteFile(pEntry->d_name);
				//}
				//else
				//if (pEntry->d_name[0] == ZIPTOD_SIGN) {
				if (ti.i.t.dw < todtOld.dw) {
					if (DeleteFile(ARCHIVE_PATH, pEntry->d_name) == 0)	TRACK("SM:remove %s\n", pEntry->d_name);
					else	TRACK("SM>ERR:can not remove %s!(s)\n", pEntry->d_name, strerror(errno));
				}
				//}
				else if (pEntry->d_name[0] == CURTOD_SIGN) {
					if (ti.i.t.dw <= c_alignFid.i.t.dw)	ZipFile(pEntry->d_name);
				}
			}
			closedir(pDir);
		}
		TRACK("SM:align end -----------------------------------\n");
		_TWATCHMEASURED(__sec_);
		TimeLog(&c_pStp->bail.mi.zip, __sec_);

		c_pStp->bail.wStatus &= ~(1 << SMSTATUS_ALIGN);
	}
}

void CSm::FileCounter(PSZ pPath)
{
	int cur, zip, uped, log, unknown;
	cur = zip = uped = log = unknown = 0;

	DIR* pDir = opendir(pPath);
	if (pDir != NULL) {
		struct dirent* pEntry;
		while ((pEntry = readdir(pDir)) != NULL) {
			if (pEntry->d_name[0] == '.')	continue;

			switch (pEntry->d_name[0]) {
			case CURTOD_SIGN :	++ cur;		break;
			case ZIPTOD_SIGN :	++ zip;		break;
			case UPETOD_SIGN :	++ uped;	break;
			case LOG_SIGN :		++ log;		break;
			default :			++ unknown;	break;
			}
		}
		closedir(pDir);
	}

	char buf[256];
	memset(buf, 0, 256);
	char* p = buf;
	sprintf(p, "%s", pPath);
	while (*p != 0)	++ p;
	if ((cur + zip + uped + log + unknown) != 0) {
		if (cur != 0) {
			sprintf(p, " cur %d", cur);
			while (*p != 0)	++ p;
		}
		if (zip != 0) {
			sprintf(p, " zip %d", zip);
			while (*p != 0)	++ p;
		}
		if (uped != 0) {
			sprintf(p, " uploaded %d", uped);
			while (*p != 0)	++ p;
		}
		if (log != 0) {
			sprintf(p, " log %d", log);
			while (*p != 0)	++ p;
		}
		if (unknown != 0) {
			sprintf(p, " unknown %d", unknown);
			while (*p != 0)	++ p;
		}
	}
	else	sprintf(p, " non files.");
	TRACK("SM:%s\n", buf);
}

bool CSm::Launch()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("sm begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
	TRACK("sm ver. %.2f %s %s\n", SM_VERSION, __DATE__, __TIME__);

	mq_unlink(STQ_STR);
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_TASKCMD;
	attr.mq_msgsize = sizeof(TASKCMD);

	c_hStq = mq_open(STQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (!Validity(c_hStq)) {
		TRACK("SM>ERR:can not open queue!(%s)\n", strerror(errno));
		return false;
	}
	if (CreateThread(&c_hThreadWatch, &CSm::WatchEntry, (PVOID)this, PRIORITY_TASKCMD) != EOK) {
		TRACK("SM>ERR:can not create watch thread!(%s)\n", strerror(errno));
		return false;
	}

	c_bAligning = true;
	if (CreateThread(&c_hThreadAlign, &CSm::FileAlignEntry, (PVOID)this, PRIORITY_DEFAULT) != EOK) {
		TRACK("SM>ERR:can not create align thread!(%s)\n", strerror(errno));
		return false;
	}

	c_hStp = shm_open(STPACK_STR, O_RDWR | O_CREAT, 0777);
	if (!Validity(c_hStp)) {
		TRACK("SM>ERR:can not open shared memory!(%s)\n", strerror(errno));
		return false;
	}
	if (ftruncate(c_hStp, sizeof(STPACK)) < 0) {
		TRACK("SM>ERR:can not truncate to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	c_pStp = (PSTPACK)mmap(0, sizeof(STPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hStp, 0);
	if (c_pStp == MAP_FAILED) {
		TRACK("SM>ERR:can not map to shared memory!(%s)\n", strerror(errno));
		return false;
	}
	memset((PVOID)c_pStp, 0, sizeof(STPACK));
	//c_pStp->bail.mi.wrt.dbMin = c_pStp->bail.mi.zip.dbMin = 3600.f;
	c_pStp->bail.mi.wrt.dwMin = DEFAULT_MINDW;

	int64_t space = GetSpace(STORAGE_PATH, true);
	TRACK("SM:find memory %lld.\n", space);

	int res = MakeDirectory(ARCHIVE_PATH);
	if (res != 0 && res != EEXIST) {
		TRACK("SM>ERR:can not make archive directory!(%s)\n", strerror(errno));
		return false;
	}
	res = MakeDirectory(ANOTHER_PATH);
	if (res != 0 && res != EEXIST) {
		TRACK("SM>ERR:can not make another directory!\n");
		return false;
	}
	res = MakeDirectory(LOG_PATH);
	if (res != 0 && res != EEXIST) {
		TRACK("SM>ERR:can not make log directory!\n");
		return false;
	}

	space = GetSpace(STORAGE_PATH, false);
	TRACK("SM:free space is %lld.\n", space);
	FileCounter(ARCHIVE_PATH);
	FileCounter(ANOTHER_PATH);
	FileCounter(LOG_PATH);

	return true;
}
