/*
 * CFlux.cpp
 *
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#define	__EXT_LF64SRC
#undef	__OFF_BITS__
#define	__OFF_BITS__	64

#include <inttypes.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/syspage.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/mount.h>
#include <sys/wait.h>

#include "Track.h"
#include "Prefix.h"
#include "slight.h"
#include "Insp.h"
#include "CBand.h"

#include "CFlux.h"

#define	MM_VERSION		231		// at 200, last version is 229

//#define	__FREE_SPACE__

CFlux::CFlux()
{
	c_hQueue[0] = c_hQueue[1] = c_hQueue[2] = INVALID_HANDLE;
	c_hThread[0] = c_hThread[1] = c_hThread[2] = INVALID_HANDLE;
	c_hShared = INVALID_HANDLE;
	c_wSeq = 0;
	c_wLogbookMonitor = c_wTroubleMonitor = 0;
	c_pBulk = NULL;

	c_bDebug = false;
	c_dwLogbookCnt = c_dwTroubleCnt = 0;
	memset(&c_cds, 0, sizeof(CURRENTDIRSTATE));
	c_dwEntryLength = 0;
	memset(&c_bcu, 0, sizeof(V3FBRKCNTU));
	memset(c_szSrcDailyDir, 0, SIZE_DEFAULTPATH);
	//memset(c_szSrcFileName, 0, SIZE_DEFAULTPATH);
	memset(c_szDestDailyPath, 0, SIZE_DEFAULTPATH);
	memset(c_szDestInspPath, 0, SIZE_DEFAULTPATH);
	//memset(c_szDestFileName, 0, SIZE_DEFAULTPATH);
	memset(c_tran, 0, sizeof(TRANSIT) * 2);
}

CFlux::~CFlux()
{
	Shutoff();
}

PVOID CFlux::WatchEntry(PVOID pVoid)
{
	int flow = 0;
	int res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (res == EOK) {
		++ flow;
		res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	}

	if (res != EOK)
		TRACK("MM>ERR:watch thread set cancel %d at %d!\n", res, flow);

	CFlux* pFlux = (CFlux*)pVoid;
	if (pFlux != NULL)	pFlux->Watch();
	pthread_exit(0);
	return NULL;
}

PVOID CFlux::PeriodicEntry(PVOID pVoid)
{
	int flow = 0;
	int res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (res == EOK) {
		++ flow;
		res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	}

	if (res != EOK)
		TRACK("MM>ERR:periodic thread set cancel %d at %d!\n", res, flow);

	CFlux* pFlux = (CFlux*)pVoid;
	if (pFlux != NULL)	pFlux->Periodic();
	pthread_exit(0);
	return NULL;
}

PVOID CFlux::OccasionEntry(PVOID pVoid)
{
	int flow = 0;
	int res = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	if (res == EOK) {
		++ flow;
		res = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	}

	if (res != EOK)
		TRACK("MM>ERR:occasion thread set cancel %d at %d!\n", res, flow);

	CFlux* pFlux = (CFlux*)pVoid;
	if (pFlux != NULL)	pFlux->Occasion();
	pthread_exit(0);
	return NULL;
}

void CFlux::Watch()
{
	if (c_bDebug)	TRACK("MM:start mm watch.(%d)\n", c_hQueue[0]);
	while (true) {
		AIDMSG msg;
		if (!Validity(mq_receive(c_hQueue[0], (char*)&msg, sizeof(AIDMSG), NULL))) {
			//if (c_bDebug)	TRACK("MM>ERR:mq_receive a!(%s)\n", strerror(errno));
			continue;
		}
		switch (msg.wCmd) {
		case AIDCMD_CLOSECURRENT :			case AIDCMD_NEWSHEET :		case AIDCMD_CULLREQUIREDDOC :
		case AIDCMD_SAVEENVIRONS :			case AIDCMD_SAVECAREERS :
		case AIDCMD_APPENDLOGBOOK :			case AIDCMD_APPENDTROUBLE :
		case AIDCMD_SAVEINSPECT :			case AIDCMD_SAVESIVDETECT :	case AIDCMD_SAVEECUTRACE :
		case AIDCMD_ERASEECUTRACE :			case AIDCMD_DOWNLOADCANCEL :
			mq_send(c_hQueue[1], (char*)&msg, sizeof(AIDMSG), 0);
			break;
		case AIDCMD_TIMESETBYUSER :			case AIDCMD_TIMESETBYRTD :	case AIDCMD_ARRAGEECUTRACE :
		case AIDCMD_SAVESIVTRACESPHERE :	case AIDCMD_SAVEV3FTRACESPHERE :	case AIDCMD_SAVEECUTRACESPHERE :
		case AIDCMD_EXTRACTLOGBOOKENTRY :	case AIDCMD_EXTRACTINSPECTENTRY :	case AIDCMD_DESTINATIONONUSB :
		case AIDCMD_EXTRACTLOGBOOKLIST :	case AIDCMD_COPYLOGBOOK :	case AIDCMD_EXTRACTLOGBOOK :
		case AIDCMD_EXTRACTINSPECT :	/*case AIDCMD_EXTRACTOLDTROUBLE :	*/case AIDCMD_INSERTIONLOGBOOK :
		case AIDCMD_INSERTIONINSPECT :	case AIDCMD_USBSCAN :
			mq_send(c_hQueue[2], (char*)&msg, sizeof(AIDMSG), 0);
			break;
		default :	break;
		}
		//c_pBulk->wOrder = msg.wOrder;
	}
	if (c_bDebug)	TRACK("MM:end mm watch.(%d)\n", c_hQueue[0]);
	mq_close(c_hQueue[0]);
	c_hQueue[0] = INVALID_HANDLE;
	mq_unlink((char*)MTQ_STR);
}

void CFlux::Periodic()
{
	if (c_bDebug)	TRACK("MM:start mm periodic.(%d)\n", c_hQueue[1]);
	while (true) {
		AIDMSG msg;
		if (!Validity(mq_receive(c_hQueue[1], (char*)&msg, sizeof(AIDMSG), NULL))) {
			//if (c_bDebug)	TRACK("MM>ERR:mq_receive a!(%s)\n", strerror(errno));
			continue;
		}
		switch (msg.wCmd) {
		case AIDCMD_CLOSECURRENT :
			c_cds.bExist = false;
			memset(&c_pBulk->fts, 0, sizeof(FILEATS));
			TRACK("MM:head info ! %02d/%02d/%02d %02d:%02d:%02d\n",
						c_pBulk->lbHead.devt.year, c_pBulk->lbHead.devt.mon, c_pBulk->lbHead.devt.day,
						c_pBulk->lbHead.devt.hour, c_pBulk->lbHead.devt.min, c_pBulk->lbHead.devt.sec);

			if (c_pBulk->wState & (1 << BULKSTATE_INTCMS)) {
				c_pBulk->wState &= ~(1 << BULKSTATE_OLDTROUBLE);
				TRACK("MM:with TCMS ------------------------------------\n");
			}
			else if (c_pBulk->wState & (1 << BULKSTATE_INRTD))
				TRACK("MM:with RTD -------------------------------------\n");
			else	TRACK("MM:with ?????????????????????????????????????????\n");

			c_tidy.StirAlignOldTrouble();
			break;
		case AIDCMD_NEWSHEET :
			c_cds.bExist = false;
			break;
		case AIDCMD_CULLREQUIREDDOC :
			c_pBulk->wState &= ~(1 << BULKSTATE_CULLREQUIREDDOC);
			CullRequiredDoc();
			break;
		case AIDCMD_SAVEENVIRONS :
			MoveEnvirons();
			break;
		case AIDCMD_SAVECAREERS :	// not used
			MoveCareers();
			break;
		case AIDCMD_APPENDLOGBOOK :
			AppendLogbook();
			c_wSeq = ((WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[1] << 8) | (WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0];
			c_pBulk->m[BPID_PERIOD].wSeq = ++ c_wSeq;
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			if (c_pBulk->wState & (1 << BULKSTATE_FILEWORK))	++ c_wLogbookMonitor;
			break;
		case AIDCMD_APPENDTROUBLE :
			AppendTrouble();
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			if (c_pBulk->wState & (1 << BULKSTATE_FILEWORK))	++ c_wTroubleMonitor;
			break;
		case AIDCMD_SAVEINSPECT :
			MoveInspect();
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			break;
		case AIDCMD_SAVESIVDETECT :
			MoveSivDetect();
			break;
		case AIDCMD_SAVEECUTRACE :
			MoveEcuTrace();
			break;
		case AIDCMD_ERASEECUTRACE :
			c_pBulk->wState |= (1 << BULKSTATE_ERASEECUTRACE);
			EraseEcuTrace();
			c_pBulk->wState &= ~(1 << BULKSTATE_ERASEECUTRACE);
			break;
		case AIDCMD_DOWNLOADCANCEL :
			c_tidy.SetCancel(true);
			break;
		default :	break;
		}
		//c_pBulk->wOrder = msg.wOrder;
		if (!(c_pBulk->wState & (1 << BULKSTATE_OLDTROUBLE))) {
			FILE* fp;
			if ((fp = fopen(OLDTROUBLEEND_PATHTMP, "r")) != NULL) {
				WORD w;
				fread(&w, 1, sizeof(WORD), fp);
				fclose(fp);
				remove(OLDTROUBLEEND_PATHTMP);
				c_pBulk->wState |= (1 << BULKSTATE_OLDTROUBLE);
				c_pBulk->wOldLoadTime = w;
			}
		}
	}

	if (c_bDebug)	TRACK("MM:end mm periodic.(%d)\n", c_hQueue[1]);
	mq_close(c_hQueue[1]);
	c_hQueue[1] = INVALID_HANDLE;
	mq_unlink((char*)MMQ_STR);
}

void CFlux::Occasion()
{
	if (c_bDebug)	TRACK("MM:start mm occasion.(%d)\n", c_hQueue[2]);
	while (true) {
		AIDMSG msg;
		if (!Validity(mq_receive(c_hQueue[2], (char*)&msg, sizeof(AIDMSG), NULL))) {
			//if (c_bDebug)	TRACK("MM>ERR:mq_receive b!(%s)\n", strerror(errno));
			continue;
		}
		switch (msg.wCmd) {
		case AIDCMD_TIMESETBYUSER :
			TimeSet(false);
			break;
		case AIDCMD_TIMESETBYRTD :
			TimeSet(true);
			c_tidy.StirDeleteBadFurtureDir();
			break;
		case AIDCMD_ARRAGEECUTRACE :
			c_tidy.ArrangeEcuTrace();
			break;
		case AIDCMD_SAVESIVTRACESPHERE :
			c_tidy.MoveTraceSphere(DEVID_SIV);
			break;
		case AIDCMD_SAVEV3FTRACESPHERE :
			c_tidy.MoveTraceSphere(DEVID_V3F);
			break;
		case AIDCMD_SAVEECUTRACESPHERE :
			c_tidy.MoveTraceSphere(DEVID_ECU);
			break;
		case AIDCMD_EXTRACTLOGBOOKENTRY :
			ExtractEntryList(LOGDIR_SIGN);
			break;
		case AIDCMD_EXTRACTINSPECTENTRY :
			ExtractEntryList(PDTINSPECT_SIGN);
			break;
		case AIDCMD_DESTINATIONONUSB :
			MakeDestination();
			break;
		case AIDCMD_EXTRACTLOGBOOKLIST :
			ExtractLogbookList();
			break;
		case AIDCMD_COPYLOGBOOK :
			CopyLogbook();
			break;
		case AIDCMD_EXTRACTLOGBOOK :
			ExtractLogbook();
			break;
		case AIDCMD_EXTRACTINSPECT :
			ExtractInspect();
			break;
		//case AIDCMD_EXTRACTOLDTROUBLE :
		//	//ExtractOldTrouble();
		//	break;
		case AIDCMD_INSERTIONLOGBOOK :
			InsertLogbook();
			break;
		case AIDCMD_INSERTIONINSPECT :
			InsertInspect();
			break;
		case AIDCMD_USBSCAN :
			ScanUsb();
			break;
		default :	break;
		}
		//c_pBulk->wOrder = msg.wOrder;
	}
	if (c_bDebug)	TRACK("MM:end mm assist.(%d)\n", c_hQueue[2]);
	mq_close(c_hQueue[2]);
	c_hQueue[2] = INVALID_HANDLE;
	mq_unlink((char*)MAQ_STR);
}

bool CFlux::TimeSet(bool byRtd)
{
	if (c_pBulk == NULL)	return false;

	c_pBulk->wState |= (1 << BULKSTATE_TIMESETTING);
	_TWATCHCLICK();
	int res = system("rtc -s isl12020");
	_TWATCHMEASURED();
	c_pBulk->wState &= ~(1 << BULKSTATE_TIMESETTING);
	if (Validity(res)) {
		if (c_bDebug)	TRACK("MM:TimeSet%d() %f sec - %d\n", byRtd ? 1 : 0, __sec_, WEXITSTATUS(res));
		// 181011
		//c_tidy.StirDeleteBadFurtureDir();
	}
	else	TRACK("MM>ERR:TimeSet() %f sec - %s\n", __sec_, strerror(errno));
	return true;
}

bool CFlux::MoveEnvirons()
{
	int res = c_tidy.CopyFile(ENVIRONS_PATH, ENVIRONS_PATHTMP);
	if (res == BULKRES_ERRNON) {
		remove(ENVIRONS_PATHTMP);
		if (c_bDebug)	TRACK("MM:move environs.\n");
	}
	c_pBulk->m[BPID_PERIOD].iResult = res;
	return res == BULKRES_ERRNON ? true : false;
}

bool CFlux::MoveCareers()
{
	int res = c_tidy.CopyFile(CAREERS_PATH, CAREERS_PATHTMP);
	if (res == BULKRES_ERRNON) {
		remove(CAREERS_PATHTMP);
		if (c_bDebug)	TRACK("MM:save careers.\n");
	}
	c_pBulk->m[BPID_PERIOD].iResult = res;
	return res == BULKRES_ERRNON ? true : false;
}

bool CFlux::MakeCurrentDirectory(bool bFrom)
{
	//sprintf(c_szCurDir, "%s/d%06d", ARCHIVE_PATH, c_tidy.GetDateIndex(false));		// "/h/sd/arch/dyymmdd"
	DWORD dwDate, dwTime;
	bool bBy = true;
	if (c_pBulk != NULL && c_pBulk->lbHead.devt.year != 0 &&
		c_pBulk->lbHead.devt.mon > 0 && c_pBulk->lbHead.devt.mon < 13 &&
		c_pBulk->lbHead.devt.day > 0 && c_pBulk->lbHead.devt.day < 32 &&
		c_pBulk->lbHead.devt.hour < 24 && c_pBulk->lbHead.devt.min < 60 && c_pBulk->lbHead.devt.sec < 60) {
		dwDate = (DWORD)c_pBulk->lbHead.devt.year * 10000 + (DWORD)c_pBulk->lbHead.devt.mon * 100 + (DWORD)c_pBulk->lbHead.devt.day;
		dwTime = (DWORD)c_pBulk->lbHead.devt.hour * 10000 + (DWORD)c_pBulk->lbHead.devt.min * 100 + (DWORD)c_pBulk->lbHead.devt.sec;
	}
	else {
		if (c_pBulk == NULL)	TRACK("MM:bulk is null!\n");
		else	TRACK("MM:head info wrong! %02d/%02d/%02d %02d:%02d:%02d\n",
							c_pBulk->lbHead.devt.year, c_pBulk->lbHead.devt.mon, c_pBulk->lbHead.devt.day,
							c_pBulk->lbHead.devt.hour, c_pBulk->lbHead.devt.min, c_pBulk->lbHead.devt.sec);
		dwDate = c_tidy.GetDateIndex(false);
		dwTime = c_tidy.GetTimeIndex();
		bBy = false;
	}
	sprintf(c_cds.szCurDir, "%s/d%06d", ARCHIVE_PATH, dwDate);
	c_cds.iYear = dwDate / 10000;
	dwDate %= 10000;
	c_cds.iMonth = dwDate / 100;
	c_cds.iDay = dwDate % 100;

	int res = mkdir(c_cds.szCurDir, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_ISVTX);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make cur %s!(%s)\n", c_cds.szCurDir, strerror(errno));
		c_cds.bExist = false;
		return false;
	}
	sprintf(c_cds.szCurLogbook, "%s/%c%06d", c_cds.szCurDir, LOGBOOK_SIGN, dwTime);
	sprintf(c_cds.szCurTrouble, "%s/%c%06d", c_cds.szCurDir, TROUBLE_SIGN, dwTime);
	c_cds.bExist = true;
	c_cds.bLogbookHead = false;
	c_dwLogbookCnt = c_dwTroubleCnt = 0;
	if (c_bDebug)	TRACK("MM:%s dir. %02d/%02d/%02d %06d from %s at %s\n",
			errno == EEXIST ? "find" : "make", c_cds.iYear, c_cds.iMonth, c_cds.iDay, dwTime, bBy ? "ts" : "self", bFrom ? "logb" : "troub");
	return true;
}

bool CFlux::AppendLogbook()
{
	if (!c_cds.bExist && !MakeCurrentDirectory(true)) {
		c_pBulk->wState &= ~(1 << BULKSTATE_LOGAREA);
		return false;
	}
	c_pBulk->wState |= (1 << BULKSTATE_LOGAREA);

	_TWATCHCLICK();
	// 181026
	c_tran[0].wLength = c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength;
	if (c_tran[0].wLength > SIZE_LOGBOOK)	c_tran[0].wLength = SIZE_LOGBOOK;
	memcpy(c_tran[0].buf, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], c_tran[0].wLength);

	FILE* fp = fopen(c_cds.szCurLogbook, "a");
	if (fp != NULL) {
		if (!c_cds.bLogbookHead) {
			size_t ret = fwrite(&c_pBulk->lbHead, 1, sizeof(LOGBOOKHEAD), fp);
			if (ret != sizeof(LOGBOOKHEAD)) {
				c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
				TRACK("MM:ERR:can not write log head!(%d-%s)\n", ret, strerror(errno));
				fclose(fp);
				return false;
			}
			c_cds.bLogbookHead = true;
			fseek(fp, 0L, SEEK_END);
		}
		//size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		size_t ret = fwrite(c_tran[0].buf, 1, c_tran[0].wLength, fp);
		fclose(fp);

		bool bRes;
		if (ret == c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;

			if (memcmp(&c_pBulk->bcu.c[0], &c_bcu.c[0], sizeof(V3FBRKCNTU))) {
				memcpy(&c_bcu.c[0], &c_pBulk->bcu.c[0], sizeof(V3FBRKCNTU));
				c_bcu.v.wAptness = BCAPTNESS_WORD;
				SaveFile(V3FBCNT_PATHB, &c_bcu.c[0], sizeof(V3FBRKCNTU));
			}

			bRes = true;
		}
		else {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:append log book!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			bRes = false;
		}

		++ c_dwLogbookCnt;
		_TWATCHMEASURE(WORD, c_pBulk->fts.logw.wCur, 3);
		if (c_pBulk->fts.logw.wCur > c_pBulk->fts.logw.wMax) {
			c_pBulk->fts.logw.wMax = c_pBulk->fts.logw.wCur;
			//TRACK("MM:logbook time %dms at %d %s.\n", c_pBulk->fts.logw.wMax, c_dwLogbookCnt, bDir ? "with dir" : "only");
		}
		return bRes;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;
	TRACK("MM>ERR:can not open logbook!(%s)\n", strerror(errno));
	return false;
}

bool CFlux::AppendTrouble()
{
	//bool bDir = false;
	if (!c_cds.bExist && !MakeCurrentDirectory(false)) {
		c_pBulk->wState &= ~(1 << BULKSTATE_LOGAREA);
		return false;
		//bDir = true;
	}
	c_pBulk->wState |= (1 << BULKSTATE_LOGAREA);

	TROUBLEINFO ti;
	memcpy(&ti, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], sizeof(TROUBLEINFO));
	ti.b.type &= TROUBLETYPE_ONLY;		// remove TROUBLE_PLATE
	if (/*ti.b.mon != c_iCurMonth || ti.b.day != c_iCurDay || */ti.b.cid > 9 || ti.b.code > 999)
		TRACK("MM>ERR:mon %d day %d cid %d code %d\n", ti.b.mon, ti.b.day, ti.b.cid, ti.b.code);
	_TWATCHCLICK();
	//FILE* fp = fopen(c_cds.szCurTrouble, "a");
	//if (fp != NULL) {
	//	size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
	//	fclose(fp);
	//	_TWATCHMEASURE(WORD, c_pBulk->fts.trbw.wCur, 3);
	//	if (c_pBulk->fts.trbw.wCur > c_pBulk->fts.trbw.wMax) {
	//		c_pBulk->fts.trbw.wMax = c_pBulk->fts.trbw.wCur;
	//		printf("FLUX:trouble time %dms at %d\n", c_pBulk->fts.trbw.wMax, c_wLogbCnt);
	//	}
	//	if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
	//		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
	//		TRACK("MM>ERR:append trouble!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
	//		return false;
	//	}
	//	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;
	//	return true;
	//}
	// 18/10/26
	c_tran[1].wLength = c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength;
	memcpy(c_tran[1].buf, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], c_tran[1].wLength);

	FILE* fp = fopen(c_cds.szCurTrouble, "a");
	if (fp != NULL) {
		//size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		size_t ret = fwrite(c_tran[1].buf, 1, c_tran[1].wLength, fp);
		fclose(fp);
	//}
	//int fd = open(c_cds.szCurTrouble, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	//if (fd >= 0) {
	//	int res = write(fd, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength);
	//	close(fd);
		++ c_dwTroubleCnt;
		_TWATCHMEASURE(WORD, c_pBulk->fts.trbw.wCur, 3);
		if (c_pBulk->fts.trbw.wMax < c_pBulk->fts.trbw.wCur) {
			c_pBulk->fts.trbw.wMax = c_pBulk->fts.trbw.wCur;
			//TRACK("MM:trouble time %dms at %d %s.\n", c_pBulk->fts.trbw.wMax, c_dwTroubleCnt, bDir ? "with dir" : "only");
		}
		if ((WORD)ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:append trouble!(%d:%d-%s)\n", (int)ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			return false;
		}
		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;
		return true;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;
	TRACK("MM>ERR:can not open trouble!(%s)\n", strerror(errno));
	return false;
}

void CFlux::MoveInspect()
{
	_QUARTET qu;
	qu.c[0] = c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0];			// wItem
	qu.c[1] = c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[1];

	char cSign;
	if (qu.w[0] & (1 << INSPITEM_MONTHLY))	cSign = MONTHLYINSPECT_SIGN;
	else if (qu.w[0] & (1 << INSPITEM_DAILY))	cSign = DAILYINSPECT_SIGN;
	else	cSign = PDTINSPECT_SIGN;

	for (int n = 0; n < (int)sizeof(DWORD); n ++)
		qu.c[n] = c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[n + 2];	// DTLTIME

	DBFTIME dbft;
	dbft.dw = qu.dw;
	char buf[256];
					//  yy  mm  dd  hh  mm  ss
	sprintf(buf, "%s/%c%02d%02d%02d%02d%02d%02d", INSPECT_PATH, cSign,
			dbft.t.year + MIN_DTLYEAR, dbft.t.mon, dbft.t.day, dbft.t.hour, dbft.t.min, dbft.t.sec);
	int res = c_tidy.CopyFile(buf, INSPECT_PATHTMP);
	if (res == BULKRES_ERRNON) {
		remove(INSPECT_PATHTMP);
		if (cSign == PDTINSPECT_SIGN)	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_PDT, PDTINSPECT_SIGN);
		else if (cSign == DAILYINSPECT_SIGN)	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_DAILY, DAILYINSPECT_SIGN);
		else	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_MONTHLY, MONTHLYINSPECT_SIGN);
		if (c_bDebug)	TRACK("MM:save inspect.\n");
	}
	c_pBulk->m[BPID_PERIOD].iResult = res;
}

void CFlux::MoveSivDetect()
{
	char buf[256];
	DWORD dwDate = c_tidy.GetDateIndex(false) % 1000000;
	DWORD dwTime = c_tidy.GetTimeIndex() % 1000000;
	sprintf(buf, "%s/%c%06d%06d", TRACE_PATH, TRACESIV_SIGN, dwDate, dwTime);

	_TWATCHCLICK();
	int res = c_tidy.CopyFile(buf, SIVDETECT_PATHTMP);
	_TWATCHMEASURED();
	if (res == BULKRES_ERRNON) {
		remove(SIVDETECT_PATHTMP);
		c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACESIV_SIGN);
		if (c_bDebug)	TRACK("MM:save siv detect.(%f sec)\n", __sec_);
	}
	c_pBulk->m[BPID_PERIOD].iResult = res;
}

void CFlux::MoveEcuTrace()
{
	char buf[256];
	DWORD dwDate = c_tidy.GetDateIndex(false) % 1000000;
	DWORD dwTime = c_tidy.GetTimeIndex() % 1000000;
	sprintf(buf, "%s/%c%06d%06d", TRACE_PATH, TRACEECU_SIGN, dwDate, dwTime);

	_TWATCHCLICK();
	int res = c_tidy.CopyFile(buf, ECUTRACE_PATHTMP);
	_TWATCHMEASURED();
	if (res == BULKRES_ERRNON) {
		remove(ECUTRACE_PATHTMP);
		c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACEECU_SIGN);
		c_pBulk->cEcuTraceLength = c_tidy.GetFilesLength(TRACE_PATH, TRACEECU_SIGN);
		if (c_bDebug)	TRACK("MM:save ecu trace.(%d, %f sec)\n", c_pBulk->cEcuTraceLength, __sec_);
	}
	c_pBulk->m[BPID_PERIOD].iResult = res;
}

void CFlux::EraseEcuTrace()
{
	c_tidy.DeleteFiles(TRACE_PATH, TRACEECU_SIGN);
	c_pBulk->cEcuTraceLength = c_tidy.GetFilesLength(TRACE_PATH, TRACEECU_SIGN);
	if (c_bDebug)	TRACK("MM:delete ecu trace.(%d)\n", c_pBulk->cEcuTraceLength);
}

WORD CFlux::ReportList(WORD wIndex)
{
	WORD w = 0;
	PENTRYSHAPE pEntrys = c_tidy.GetEntrys();
	//if (c_bDebug)	TRACK("MM:list begin %d\n", wIndex);
	while (w < SIZE_ENTRYBLOCK) {
		memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[w], (const void*)&pEntrys[wIndex], sizeof(ENTRYSHAPE));
		//memcpy(&c_pBulk->m[BPID_OCCASN].bt.rs.s.text.f[w], &c_entrys[wIndex], sizeof(ENTRYSHAPE));
		//if (c_bDebug)	TRACK("MM:listb%d %s\n", wIndex, c_pBulk->m[BPID_OCCASN].bt.rs.s.text.f[w].szName);
		++ w;			// for 문을 쓰면 아래 문에서 break 할 때 빠져나가 마지막 w ++을 하지 못한다...
		if (++ wIndex >= (WORD)c_dwEntryLength)	break;
	}
	//if (c_bDebug)	TRACK("MM:list end %d\n", wIndex);
	return w;
}

void CFlux::ExtractEntryList(BYTE cSign)
{
	//TRACK("MM:extract entry list\n");
	BYTE sign = (cSign == PDTINSPECT_SIGN || cSign == DAILYINSPECT_SIGN || cSign == MONTHLYINSPECT_SIGN) ? 1 : 0;
	if (c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex == 0) {
		// 처음에는 dwTotal에 TU가 원하는 최대 길이가 있다.
		_QUARTET qu;
		if (sign == 1) {
			qu.dw = c_tidy.MakeInspectEntry(c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal);	// 395280(0x60810)
			// return:dwTotal - b7:b0:pdt length, b15~b8:daily length, b23~b16:monthly length
			c_dwEntryLength = (DWORD)(qu.c[0] + qu.c[1] + qu.c[2]);
		}
		else	c_dwEntryLength = c_tidy.MakeLogbookEntry(NULL, c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal);

		if (c_dwEntryLength > 0) {			// 버퍼에 담긴 엔트리 수.
			c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = sign == 1 ? qu.dw : c_dwEntryLength;
			// inspect:처음에는 각 항목의 숫자를 따로 보내서 TU가 알게한다.
			// inspect:다음 절차에 TU가 이 항목을 보낼 때는 모두 합산한 숫자를 보내 MM이 참조하도록 한다.
			// inspect:그러나 305줄에서 하고있는데...
		}
		else	c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = 0;
		//if (c_bDebug)	TRACK("MM:index %d, total %d\n", c_pBulk->m[BPID_OCCASN].bt.rs.s.dwIndex, c_pBulk->m[BPID_OCCASN].bt.rs.s.dwTotal);
	}
	else	c_dwEntryLength = c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal;

	if (c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex < c_dwEntryLength)
		c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength = ReportList((WORD)c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex);
	else	c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength = 0;

	c_pBulk->m[BPID_OCCASN].bEnd = true;
	if (c_bDebug) {
		if (sign == 1)	TRACK("MM:insp end %d\n", c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength);
		else	TRACK("MM:log end %d\n", c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength);
	}
}

void CFlux::ExtractLogbookList()
{
	if (c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex == 0)
		c_tidy.MakeLogbookList(c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.h.f.szName);
	PFILELIST pFl = c_tidy.GetLists();
	c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwTotal = (DWORD)pFl->wLength;
	WORD wIndex = (WORD)c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex;
	memset(&c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe, 0, sizeof(RTDHOE));
	WORD w = 0;
	while (w < SIZE_ENTRYBLOCK) {
		memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.fs[w], (const void*)&pFl->list[wIndex], sizeof(ENTRYSHAPE));
		++ w;
		if (++ wIndex >= pFl->wLength)	break;
	}
	c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength = w;
	c_pBulk->m[BPID_OCCASN].bEnd = true;
	if (c_bDebug)	TRACK("MM:file list %d\n", w);
}

bool CFlux::MakeDirOnUsb()
{
	sprintf(c_szDestDailyPath, "%s/n%04x", USB_PATH, c_pBulk->lbHead.wPermNo);		// /dos/c/n2XYY
	if (c_pBulk->lbHead.cAux != 0)	strcat(c_szDestDailyPath, "a");
	strncpy(c_szDestInspPath, c_szDestDailyPath, SIZE_DEFAULTPATH);
	int res = mkdir(c_szDestDailyPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make usb %s!(%s)\n", c_szDestDailyPath, strerror(errno));
		return false;
	}
	TRACK("MM:make %s\n", c_szDestDailyPath);

	bool bDaily = c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.cItem == USBDIRITEM_DAILY ? true : false;
	if (bDaily)	strcat(c_szDestDailyPath, USBARCH_NAME);													// /dos/c/n2XYY/arch
	else	strcat(c_szDestInspPath, USBINSP_NAME);															// /dos/c/n2XYY/insp
	res = mkdir(bDaily ? c_szDestDailyPath : c_szDestInspPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make usb %s!(%s)\n", bDaily ? c_szDestDailyPath : c_szDestInspPath, strerror(errno));
		return false;
	}
	TRACK("MM:make %s\n", bDaily ? c_szDestDailyPath : c_szDestInspPath);

	if (bDaily) {
		sprintf(c_szSrcDailyDir, "%c%06d", LOGDIR_SIGN, c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.dwIndex);	// /dos/c/n2XYY/arch/dYYMMDD
		strcat(c_szDestDailyPath, "/");
		strcat(c_szDestDailyPath, c_szSrcDailyDir);
		res = mkdir(c_szDestDailyPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && errno != EEXIST) {
			TRACK("MM>ERR:can not make usb %s!(%s)\n", c_szDestDailyPath, strerror(errno));
			return false;
		}
		TRACK("MM:make %s\n", c_szDestDailyPath);
	}
	return true;
}

void CFlux::MakeDestination()
{
	if (MakeDirOnUsb())	c_pBulk->m[BPID_OCCASN].iResult = BULKRES_OK;
	else	c_pBulk->m[BPID_OCCASN].iResult = BULKRES_ERRCANNOTMAKEDIR;
	c_pBulk->m[BPID_OCCASN].bEnd = true;
}

void CFlux::CopyLogbook()
{
	if (c_szSrcDailyDir[0] != 0 && c_szDestDailyPath[0] != 0)
		c_tidy.CopyLogbook(&c_pBulk->m[BPID_OCCASN].bt.pad.sp, c_szDestDailyPath, c_szSrcDailyDir);
	c_pBulk->m[BPID_OCCASN].bEnd = true;
}

void CFlux::ExtractLogbook()
{
	c_tidy.ExcerptFile(&c_pBulk->m[BPID_OCCASN].bt.pad.sp, c_szSrcDailyDir);
	c_pBulk->m[BPID_OCCASN].bEnd = true;
}

void CFlux::ExtractInspect()
{
	c_tidy.ExcerptFile(&c_pBulk->m[BPID_OCCASN].bt.pad.sp, NULL);
	c_pBulk->m[BPID_OCCASN].bEnd = true;
}

void CFlux::InsertLogbook()
{
	if (c_szDestDailyPath[0] != 0)
		c_tidy.BindFileToUsb(&c_pBulk->m[BPID_OCCASN].bt.pad.sp, c_szDestDailyPath);
	c_pBulk->m[BPID_OCCASN].bEnd = true;
}

void CFlux::InsertInspect()
{
	if (c_szDestInspPath[0] != 0)
		c_tidy.BindFileToUsb(&c_pBulk->m[BPID_OCCASN].bt.pad.sp, c_szDestInspPath);
	c_pBulk->m[BPID_OCCASN].bEnd = true;
}

bool CFlux::ScanUsb()
{
	DIR* pDir;
	if (c_pBulk->wState & (1 << BULKSTATE_SEIZEUSB)) {
		pDir = opendir("/dos/c");
		if (pDir != NULL) {
			closedir(pDir);
			return true;
		}
		c_pBulk->wState &= ~(1 << BULKSTATE_SEIZEUSB);
	}
	char buf[256];
	pDir = opendir("/dev");
	if (pDir != NULL) {
		struct dirent* pEnt;
		while ((pEnt = readdir(pDir)) != NULL) {
			strncpy(buf, pEnt->d_name, 256);
			if (strlen(buf) > 4) {
				buf[4] = 0;
				if (!strcmp(buf, "hd1t")) {
					strncpy(buf, pEnt->d_name, 256);
					closedir(pDir);

					if (c_bDebug)	TRACK("MM:find %s\n", buf);
					char cmd[256];
					sprintf(cmd, "mount -tdos /dev/%s %s", buf, USB_PATH);
					c_pBulk->wState |= (1 << BULKSTATE_MOUNTING);
					_TWATCHCLICK();
					int res = system(cmd);
					_TWATCHMEASURED();
					c_pBulk->wState &= ~(1 << BULKSTATE_MOUNTING);
					if (Validity(res)) {
						pDir = opendir(USB_PATH);
						if (pDir != NULL) {
							closedir(pDir);
							c_pBulk->iUsbSize = c_tidy.GetSpace(USB_PATH, false);
							c_pBulk->wState |= (1 << BULKSTATE_SEIZEUSB);
							if (c_bDebug)	TRACK("MM:capture usb.(%lld - %f sec)\n", c_pBulk->iUsbSize, __sec_);
							return true;
						}
						if (c_bDebug)	TRACK("MM>ERR:usb mount failure A!\n");
					}
					if (c_bDebug)	TRACK("MM>ERR:usb mount failure B!\n");
					c_pBulk->wState &= ~(1 << BULKSTATE_SEIZEUSB);
					return false;
				}
			}
		}
		closedir(pDir);
	}
	c_pBulk->wState &= ~(1 << BULKSTATE_SEIZEUSB);
	return false;
}

bool CFlux::SaveFile(const char* pPath, PVOID pVoid, int iLength)
{
	FILE* fp;
	if ((fp = fopen(pPath, "w")) != NULL) {
		size_t leng = fwrite(pVoid, 1, (size_t)iLength, fp);
		fclose(fp);
		if (leng == (size_t)iLength)	return true;
	}
	return false;
}

void CFlux::FileWorkFlag(bool bCmd)
{
	if (!bCmd) {
		c_pBulk->wState &= ~(1 << BULKSTATE_FILEWORK);
		if (c_bDebug)	TRACK("MM:log %d trb %d at filework.\n", c_wLogbookMonitor, c_wTroubleMonitor);
	}
	else {
		c_wLogbookMonitor = c_wTroubleMonitor = 0;
		c_pBulk->wState |= (1 << BULKSTATE_FILEWORK);
	}
}

void CFlux::SetBulkState(int index, bool bSet)
{
	if (bSet)	c_pBulk->wState |= (1 << index);
	else	c_pBulk->wState &= ~(1 << index);
}

void CFlux::CullRequiredDoc()
{
	// 180308
	c_pBulk->cCullState = 0;
	memset(c_pBulk->m[BPID_OCCASN].bt.c, 0, sizeof(BULKTEXT));

	FILE* fp;
	size_t leng;
	if ((fp = fopen(ENVIRONS_PATHA, "r")) != NULL) {
		leng = fread(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[0], 1, sizeof(ENVIRONS), fp);
		fclose(fp);
	}
	if (fp == NULL || leng < (sizeof(ENVIRONS) - 2)) {	//!= sizeof(ENVIRONS)) {
		if ((fp = fopen(ENVIRONS_PATH, "r")) != NULL) {
			ENVIRONS env;
			fread(&env.c[0], 1, sizeof(ENVIRONS), fp);
			fclose(fp);
			BYTE sum = 0;
			for (int n = 0; n < (int)(sizeof(ENVIRONS) - 1); n ++)	sum += env.c[n];
			env.real.wVersion = ENV_VERSION;
			memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[0], &env.c[0], sizeof(ENVIRONS));
		//	fread(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[0], 1, sizeof(ENVIRONS), fp);
		}
		else	c_pBulk->cCullState |= (1 << CULLERR_CANNOTOPENENV);
	}
	else	c_pBulk->cCullState |= (1 << CULLERR_UNMATCHSIZEENV);

	if (!access(ENVIRONS_PATHA, F_OK))	remove(ENVIRONS_PATHA);
	if (!access(SECTDOSS_PATH, F_OK))	remove(SECTDOSS_PATH);

	LOGINFO li;
	// 180308
	//if (c_tidy.GetLatestLogbookBlock(&la)) {
	BYTE cRes = c_tidy.GetLatestLogbookBlock(&li);
	if (cRes == 0) {
		CAREERSB crr;
		crr.real.wVersion = CAREERS_VERSION;
		crr.real.qwDistance = li.r.qwDistance;
		for (int n = 0; n < V3F_MAX; n ++) {
			if (n < SIV_MAX)	crr.real.dps.qwSivPower[n] = li.r.dps.qwSivPower[n];
			crr.real.dps.qwV3fPower[n] = li.r.dps.qwV3fPower[n];
			crr.real.dps.qwV3fReviv[n] = li.r.dps.qwV3fReviv[n];
		}
		memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[100], &crr.c[0], sizeof(CAREERSB));

		CMJOGB cmj;
		cmj.real.wVersion = CMJOG_VERSION;
		cmj.real.cjt.srt.a = li.r.cjt.srt.a;
		for (int n = 0; n < CM_MAX; n ++)
			cmj.real.cjt.cm[n].a = li.r.cjt.cm[n].a;

		memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[300], &cmj.c[0], sizeof(CMJOGB));

		// 180511
		//WORD w = 0;
		//if (c_tidy.GetCurrentLogbookVersion() >= LOGBOOKCTRLSIDE_VERSION) {
		//	w = li.r.wCtrlSide;
		//	TRACK("MM:control side flag %04x.\n", w);
		//}
		//memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[400], &w, sizeof(WORD));

		if (!access(CAREERS_PATHB, F_OK))	remove(CAREERS_PATHB);
		if (!access(CAREERS_PATHA, F_OK))	remove(CAREERS_PATHA);
		if (!access(CAREERS_PATH, F_OK))	remove(CAREERS_PATH);
		if (!access(CMJOG_PATHB, F_OK))	remove(CMJOG_PATHB);
		if (!access(CMJOG_PATHA, F_OK))	remove(CMJOG_PATHA);
		if (!access(CMJOG_PATH, F_OK))	remove(CMJOG_PATH);
	}
	else {
		c_pBulk->cCullState |= cRes;

		CAREERS crr0;
		if ((fp = fopen(CAREERS_PATHA, "r")) != NULL) {
			leng = fread(&crr0, 1, sizeof(CAREERS), fp);
			fclose(fp);
		}
		if (fp == NULL || leng != sizeof(CAREERS)) {
			if ((fp = fopen(CAREERS_PATH, "r")) != NULL) {
				leng = fread(&crr0, 1, sizeof(CAREERS), fp);
				fclose(fp);
			}
		}
		if (fp == NULL || leng != sizeof(CAREERS)) {
			if ((fp = fopen(CAREERS_PATHB, "r")) != NULL) {
				fread(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[100], 1, sizeof(CAREERSB), fp);
				fclose(fp);
			}
		}
		else {
			CAREERSB crr;
			crr.real.wVersion = CAREERS_VERSION;
			crr.real.qwDistance = (QWORD)crr0.real.dbDistance;
			for (int n = 0; n < V3F_MAX; n ++) {
				if (n < SIV_MAX)	crr.real.dps.qwSivPower[n] = (QWORD)crr0.real.dbSivPower[n];
				crr.real.dps.qwV3fPower[n] = (QWORD)crr0.real.dbV3fPower[n];
				crr.real.dps.qwV3fReviv[n] = (QWORD)crr0.real.dbV3fReviv[n];
			}
			memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[100], &crr.c[0], sizeof(CAREERSB));
		}
		if (!access(CAREERS_PATHA, F_OK))	remove(CAREERS_PATHA);
		if (!access(CAREERS_PATH, F_OK))	remove(CAREERS_PATH);

		CMJOG cmj0;
		if ((fp = fopen(CMJOG_PATHA, "r")) != NULL) {
			leng = fread(&cmj0, 1, sizeof(CMJOG), fp);
			fclose(fp);
		}
		if (fp == NULL || leng != sizeof(CMJOG)) {
			if ((fp = fopen(CMJOG_PATH, "r")) != NULL) {
				leng = fread(&cmj0, 1, sizeof(CMJOG), fp);
				fclose(fp);
			}
		}
		if (fp == NULL || leng != sizeof(CMJOG)) {
			if ((fp = fopen(CMJOG_PATHB, "r")) != NULL) {
				fread(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[300], 1, sizeof(CMJOGB), fp);
				fclose(fp);
			}
		}
		else {
			CMJOGB cmj;
			cmj.real.wVersion = CMJOG_VERSION;
			cmj.real.cjt.srt.b.day = cmj0.sysrun.wDay;
			cmj.real.cjt.srt.b.sec = cmj0.sysrun.dwSec;
			for (int n = 0; n < CM_MAX; n ++) {
				cmj.real.cjt.cm[n].b.day = cmj0.cm[n].wDay;
				cmj.real.cjt.cm[n].b.sec = cmj0.cm[n].dwSec;
			}
			memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.tp.tin.c[300], &cmj.c[0], sizeof(CMJOGB));
		}
		if (!access(CMJOG_PATHA, F_OK))	remove(CMJOG_PATHA);
		if (!access(CMJOG_PATH, F_OK))	remove(CMJOG_PATH);
	}

	if ((fp = fopen(V3FBCNT_PATHB, "r")) != NULL) {
		leng = fread(&c_bcu.c[0], 1, sizeof(V3FBRKCNTU), fp);
		fclose(fp);
		if (c_bcu.v.wAptness != BCAPTNESS_WORD || leng < (sizeof(V3FBRKCNTU) - 2)) {	//!= sizeof(V3FBRKCNTU)) {
			memset(&c_bcu, 0, sizeof(V3FBRKCNTU));
			c_bcu.v.wAptness = BCAPTNESS_WORD;
		}
	}
	else {
		memset(&c_bcu, 0, sizeof(V3FBRKCNTU));
		c_bcu.v.wAptness = BCAPTNESS_WORD;
	}
	memcpy(&c_pBulk->bcu.c[0], &c_bcu.c[0], sizeof(V3FBRKCNTU));

	c_pBulk->wState |= ((1 << BULKSTATE_CULLREQUIREDDOC) | (1 << BULKSTATE_LOGAREA));
}

bool CFlux::Launch(bool bDebug)
{
	c_bDebug = bDebug;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* pTm = localtime(&tv.tv_sec);
	TRACK("mm begin on %04d/%02d/%02d %02d:%02d:%02d -----------------\n",
			pTm->tm_year + 1900, pTm->tm_mon + 1, pTm->tm_mday, pTm->tm_hour, pTm->tm_min, pTm->tm_sec);

	TRACK("mm ver %.2f%s %s %s\n", (double)MM_VERSION / 100.f, c_bDebug != 0 ? "(debug mode)" : "", __DATE__, __TIME__);

	c_tidy.SetParent(this);
	c_tidy.SetDebug(c_bDebug);
	if (!CreateQueue()) {
		Shutoff();
		TRACK("MM>ERR:can not create queue!\n");
		return false;
	}
	if (!CreateSharedMem()) {
		Shutoff();
		TRACK("MM>ERR:can not create shared memory!\n");
		return false;
	}

	int64_t space = c_tidy.GetSpace(STORAGE_PATH, true);
	TRACK("MM:find memory %lld.\n", space);

	//c_pBulk->wState |= (1 << BULKSTATE_STORAGE);
	c_pBulk->wMmVer = MM_VERSION;

	int res;
	DIR* pDir = opendir(ARCHIVE_PATH);
	if (pDir == NULL) {
		res = mkdir(ARCHIVE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make archive directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	pDir = opendir(INSPECT_PATH);
	if (pDir == NULL) {
		res = mkdir(INSPECT_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make inspect directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	pDir = opendir(TRACE_PATH);
	if (pDir == NULL) {
		res = mkdir(TRACE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make trace directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	// 171219
	pDir = opendir(LOG_PATH);
	if (pDir == NULL) {
		res = mkdir(LOG_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make log directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	_TWATCHCLICK();
	int dleng = c_tidy.DeletePrev6(ARCHIVE_PATH);

	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_PDT, PDTINSPECT_SIGN);
	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_DAILY, DAILYINSPECT_SIGN);
	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_MONTHLY, MONTHLYINSPECT_SIGN);

	c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACESIV_SIGN);
	c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACEV3F_SIGN);
	c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACEECU_SIGN);
	_TWATCHMEASURED();
	if (c_bDebug)	TRACK("MM:makeup archive.(%d deleted, %fms)\n", dleng, __sec_ * 1e3);
	CullRequiredDoc();

	c_pBulk->cEcuTraceLength = c_tidy.GetFilesLength(TRACE_PATH, TRACEECU_SIGN);
	if (c_bDebug)	TRACK("MM:ecu trace length %d.\n", c_pBulk->cEcuTraceLength);

	space = c_tidy.GetSpace(STORAGE_PATH, false);
	if (c_bDebug)	TRACK("MM:free space is %lld.\n", space);

	return true;
}

void CFlux::Shutoff()
{
	PVOID pResult;
	for (int n = 0; n < 3; n ++) {
		if (Validity(c_hThread[n])) {
			int flow = 0;
			int res = pthread_cancel(c_hThread[n]);
			if (res == EOK) {
				++ flow;
				res = pthread_join(c_hThread[n], &pResult);
			}
			if (res != EOK)	TRACK("MM>ERR:thread %d shutoff %d at %d!\n", n, res, flow);
			c_hThread[n] = INVALID_HANDLE;
		}
	}
	for (int n = 0; n < 3; n ++) {
		if (Validity(c_hQueue[n])) {
			mq_close(c_hQueue[n]);
			c_hQueue[n] = INVALID_HANDLE;
			switch (n) {
			case 0 :	mq_unlink((char*)MTQ_STR);	break;
			case 1 :	mq_unlink((char*)MMQ_STR);	break;
			default :	mq_unlink((char*)MAQ_STR);	break;
			}
		}
	}
	if (Validity(c_hShared)) {
		close(c_hShared);
		c_hShared = INVALID_HANDLE;
		c_pBulk = NULL;
		shm_unlink((char*)BULK_STR);
	}
}

int CFlux::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	int res = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if (!Validity(res)) {
		TRACK("MM>ERR:can not set inherit!(%d-%s)\n", res, strerror(errno));
		return res;
	}

	struct sched_param param;
	param.sched_priority = priority;
	res = pthread_attr_setschedparam(&attr, &param);
	if (!Validity(res)) {
		TRACK("MM>ERR:can not set sch param!(%d-%s)\n", res, strerror(errno));
		return res;
	}

	res = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
	if (!Validity(res)) {
		TRACK("MM>ERR:can not set sch policy!(%d-%s)\n", res, strerror(errno));
		return res;
	}

	res = pthread_create(pThreadHandle, &attr, ThreadFunc, (PVOID)this);
	if (!Validity(res)) {
		TRACK("MM>ERR:can not create thread!(%d-%s)\n", res, strerror(errno));
		return res;
	}
	return EOK;
}

bool CFlux::CreateQueue()
{
	mq_unlink((char*)MTQ_STR);
	mq_unlink((char*)MMQ_STR);
	mq_unlink((char*)MAQ_STR);

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = MAX_MMMSG;
	attr.mq_msgsize = sizeof(AIDMSG);

	c_hQueue[0] = mq_open((char*)MTQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	c_hQueue[1] = mq_open((char*)MMQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	c_hQueue[2] = mq_open((char*)MAQ_STR, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, &attr);
	if (Validity(c_hQueue[0]) && Validity(c_hQueue[1]) && Validity(c_hQueue[2])) {
		if (CreateThread(&c_hThread[0], &CFlux::WatchEntry, (PVOID)this, PRIORITY_MSG) == EOK) {
			if (CreateThread(&c_hThread[1], &CFlux::PeriodicEntry, (PVOID)this, PRIORITY_MSG) == EOK) {
				if (CreateThread(&c_hThread[2], &CFlux::OccasionEntry, (PVOID)this, PRIORITY_MSG) == EOK)
					return true;
				else	TRACK("MM>ERR:can not create occasion thread!\n");
			}
			else	TRACK("MM>ERR:can not create periodic thread!\n");
		}
		else	TRACK("MM>ERR:can not create watch thread!\n");
	}
	else	TRACK("MM>ERR:can not open queue %d %d %d!\n", c_hQueue[0], c_hQueue[1], c_hQueue[2]);
	return false;
}

bool CFlux::CreateSharedMem()
{
	c_hShared = shm_open(BULK_STR, O_RDWR | O_CREAT, 0777);
	if (!Validity(c_hShared)) {
		TRACK("MM>ERR:shm_open(%s)\n", strerror(errno));
		return false;
	}
	if (ftruncate(c_hShared, sizeof(BULKPACK)) < 0) {
		TRACK("MM>ERR:ftruncate(%s)\n", strerror(errno));
		close(c_hShared);
		shm_unlink(BULK_STR);
		return false;
	}
	c_pBulk = (PBULKPACK)mmap(0, sizeof(BULKPACK), PROT_READ | PROT_WRITE, MAP_SHARED, c_hShared, 0);
	if (c_pBulk == MAP_FAILED) {
		TRACK("MM>ERR:mmap(%s)\n", strerror(errno));
		close(c_hShared);
		shm_unlink(BULK_STR);
		return false;
	}
	if (c_bDebug)	TRACK("MM:pBulk is %08X\n", c_pBulk);

	memset((PVOID)c_pBulk, 0, sizeof(BULKPACK));
	return true;
}
