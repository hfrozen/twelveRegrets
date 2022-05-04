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

#define	MM_VERSION		210		// / 100

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
	memset(&c_cds, 0, sizeof(CURDIRSTATE));

	c_cEntryType = ENTRYTYPE_NON;
	c_dwEntryIndex = 0;
	c_dwEntryLength = 0;
	c_dwCurSize = 0;

	memset(&c_sectds, 0, sizeof(SECTDOSS));
	memset(c_szSrcDailyDir, 0, SIZE_DEFAULTPATH);
	memset(c_szSrcFileName, 0, SIZE_DEFAULTPATH);
	memset(c_szDestDailyPath, 0, SIZE_DEFAULTPATH);
	memset(c_szDestInspPath, 0, SIZE_DEFAULTPATH);
	memset(c_szDestFileName, 0, SIZE_DEFAULTPATH);
	//memset(c_szUsb, 0, SIZE_DEFAULTPATH);
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
		case AIDCMD_TIMESET :
		case AIDCMD_CLOSECURRENT :
		case AIDCMD_SAVEENVIRONS :
		case AIDCMD_SAVECAREERS :
		case AIDCMD_APPENDLOGBOOKTOCPM :
		case AIDCMD_APPENDLOGBOOKTOLTE :
		case AIDCMD_APPENDLOGBOOKTOSD :
		case AIDCMD_APPENDTROUBLE :
		case AIDCMD_SAVEINSPECT :
		case AIDCMD_SAVEECUTRACE :
		case AIDCMD_ERASEECUTRACE :
		case AIDCMD_DOWNLOADCANCEL :
		case AIDCMD_CREATENEWFILE :
			mq_send(c_hQueue[1], (char*)&msg, sizeof(AIDMSG), 0);
			break;
		case AIDCMD_ALIGNOLDTROUBLE :
		case AIDCMD_ARRAGEECUTRACE :
		case AIDCMD_SAVESIVTRACESPHERE :
		case AIDCMD_SAVEV3FTRACESPHERE :
		case AIDCMD_SAVEECUTRACESPHERE :
		case AIDCMD_EXTRACTLOGBOOKENTRY :
		case AIDCMD_EXTRACTINSPECTENTRY :
		case AIDCMD_DESTINATIONONUSB :
		case AIDCMD_EXTRACTLOGBOOKLIST :
		case AIDCMD_COPYLOGBOOK :
		case AIDCMD_EXTRACTLOGBOOK :
		case AIDCMD_EXTRACTINSPECT :
		case AIDCMD_EXTRACTOLDTROUBLE :
		case AIDCMD_INSERTIONLOGBOOK :
		case AIDCMD_INSERTIONINSPECT :
		case AIDCMD_USBSCAN :
		case AIDCMD_RESSECTIONDOSSIER :
		case AIDCMD_GETSECTIONDOSSIER :
		case AIDCMD_SENDEDBYWIRELESS :
		case AIDCMD_COMPRESSION :
		case AIDCMD_SENDEDBYWIRELESSOK :
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
		case AIDCMD_TIMESET :
			TimeSet();
			break;
		case AIDCMD_CLOSECURRENT :
			c_cds.bExistSd = false;
			c_cds.bExistCpm = false;
			memset(&c_pBulk->fts, 0, sizeof(FILEATS));
			if (!(c_pBulk->wState & (1 << BULKSTATE_INTCMS)))
				TRACK("MM:head info ! %02d/%02d/%02d %02d:%02d:%02d\n",
							c_pBulk->lbHead.dt.year, c_pBulk->lbHead.dt.mon, c_pBulk->lbHead.dt.day,
							c_pBulk->lbHead.dt.hour, c_pBulk->lbHead.dt.min, c_pBulk->lbHead.dt.sec);
			break;
		case AIDCMD_SAVEENVIRONS :
			MoveEnvirons();
			break;
		case AIDCMD_SAVECAREERS :
			MoveCareers();
			break;
		case AIDCMD_APPENDLOGBOOKTOSD :
			AppendLogbookToSd();
			c_wSeq = ((WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[1] << 8) | (WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0];
			c_pBulk->m[BPID_PERIOD].wSeq = ++ c_wSeq;
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			if (c_pBulk->wState & (1 << BULKSTATE_FILEWORK))	++ c_wLogbookMonitor;
			break;
		case AIDCMD_APPENDLOGBOOKTOCPM :		// AIDCMD_APPENDCPM
			c_wSeq = ((WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[1] << 8) | (WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0];
			if ((c_wSeq % 2) == 0)	AppendLogbookToCpm();		// for cpm
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			if (c_pBulk->wState & (1 << BULKSTATE_FILEWORK))	++ c_wCpmLogMonitor;
			break;
		case AIDCMD_APPENDLOGBOOKTOLTE :		// AIDCMD_APPENDLOGBOOKLTE
			AppendLogbookToLte();
			c_wSeq = ((WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[1] << 8) | (WORD)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0];
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			if (c_pBulk->wState & (1 << BULKSTATE_FILEWORK))	++ c_wLogbookMonitorLte;
			break;
		case AIDCMD_APPENDTROUBLE :
			AppendTroubleToSd();
			if (!(c_pBulk->wState & (1 << BULKSTATE_INTCMS))) {
				AppendTroubleToCpm();
				AppendTroubleToLte();
			}
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
			if (c_pBulk->wState & (1 << BULKSTATE_FILEWORK))	++ c_wTroubleMonitor;
			break;
		case AIDCMD_SAVEINSPECT :
			MoveInspect();
			c_pBulk->m[BPID_PERIOD].bBegin = false;		// !!! VERY IMPORTANT !!!
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
		case AIDCMD_CREATENEWFILE :			// AIDCMD_CREATE_NEW_FILE
			c_cds.bExistLte = false;
			TRACK("MM:LTE head info ! %02d/%02d/%02d %02d:%02d:%02d\n",
										c_pBulk->lbHead.dt.year, c_pBulk->lbHead.dt.mon, c_pBulk->lbHead.dt.day,
										c_pBulk->lbHead.dt.hour, c_pBulk->lbHead.dt.min, c_pBulk->lbHead.dt.sec);
			break;
		default :
			break;
		}
		//c_pBulk->wOrder = msg.wOrder;
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
		case AIDCMD_ALIGNOLDTROUBLE :
			c_tidy.AlignOldTrouble();
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
		case AIDCMD_EXTRACTOLDTROUBLE :
			ExtractOldTrouble();
			break;
		case AIDCMD_INSERTIONLOGBOOK :
			InsertLogbook();
			break;
		case AIDCMD_INSERTIONINSPECT :
			InsertInspect();
			break;
		case AIDCMD_USBSCAN :
			ScanUsb();
			break;
		case AIDCMD_RESSECTIONDOSSIER :
			RestoreSectionDossier();
			break;
		case AIDCMD_GETSECTIONDOSSIER :
			ReportSectionDossier();
			break;
		case AIDCMD_COMPRESSION :
			CompressionFile();
			break;
		case AIDCMD_SENDEDBYWIRELESS :
			SendDataByWifi();
			break;
		case AIDCMD_SENDEDBYWIRELESSOK :
			RemoveSendedFolder();
			break;
		default :
			break;
		}
		//c_pBulk->wOrder = msg.wOrder;
	}
	if (c_bDebug)	TRACK("MM:end mm assist.(%d)\n", c_hQueue[2]);
	mq_close(c_hQueue[2]);
	c_hQueue[2] = INVALID_HANDLE;
	mq_unlink((char*)MAQ_STR);
}

bool CFlux::TimeSet()
{
	if (c_pBulk == NULL)	return false;

	c_pBulk->wState |= (1 << BULKSTATE_TIMESETTING);
	_TWATCHCLICK();
	int res = system("rtc -s isl12020");
	_TWATCHMEASURED();
	c_pBulk->wState &= ~(1 << BULKSTATE_TIMESETTING);
	if (Validity(res)) {
		if (c_bDebug)	TRACK("MM:TimeSet() %f sec - %d\n", __sec_, WEXITSTATUS(res));
	}
	else	TRACK("MM>ERR:TimeSet() %f sec - %s\n", __sec_, strerror(errno));
	return true;
}

bool CFlux::MoveEnvirons()
{
	int res = c_tidy.CopyFile(ENVIRONS_PATH, ENVIRONS_PATHTMP);
	if (res == BULKRES_ERRNON) {
		remove(ENVIRONS_PATHTMP);
		if (c_bDebug)	TRACK("MM:save environs.\n");
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

int CFlux::MoveIndexFiles(char* tp, int ti, char* sp, int si)
{
	int leng = 0;
	for (int n = 0; n < CID_MAX; n ++) {
		sp[si] = n + '0';
		if (access(sp, R_OK) == 0) {
			tp[ti] = n + '0';
			if (c_tidy.CopyFile(tp, sp) == BULKRES_ERRNON) {
				++ leng;
				//unlink(sp);
				remove(sp);
			}
		}
	}
	return leng;
}

bool CFlux::MakeCurrentDirectoryLte()
{
	//sprintf(c_szCurDir, "%s/d%06d", ARCHIVE_PATH, c_tidy.GetDateIndex(false));		// "/h/sd/arch/dyymmdd"
	DWORD dwDate, dwTime;
	bool bFrom = true;

	if (c_pBulk != NULL && c_pBulk->lbHead.dt.year != 0 &&
		c_pBulk->lbHead.dt.mon > 0 && c_pBulk->lbHead.dt.mon < 13 &&
		c_pBulk->lbHead.dt.day > 0 && c_pBulk->lbHead.dt.day < 32 &&
		c_pBulk->lbHead.dt.hour < 24 && c_pBulk->lbHead.dt.min < 50 && c_pBulk->lbHead.dt.sec < 60) {
		dwDate = (DWORD)c_pBulk->lbHead.dt.year * 10000 + (DWORD)c_pBulk->lbHead.dt.mon * 100 + (DWORD)c_pBulk->lbHead.dt.day;
		dwTime = (DWORD)c_pBulk->lbHead.dt.hour * 10000 + (DWORD)c_pBulk->lbHead.dt.min * 100 + (DWORD)c_pBulk->lbHead.dt.sec;

		TRACK("MM>bulk is exist\n");
	}
	else {
		if (c_pBulk == NULL)	TRACK("MM:bulk is null!\n");
		else	TRACK("MM:head info wrong! %02d/%02d/%02d %02d:%02d:%02d\n",
							c_pBulk->lbHead.dt.year, c_pBulk->lbHead.dt.mon, c_pBulk->lbHead.dt.day,
							c_pBulk->lbHead.dt.hour, c_pBulk->lbHead.dt.min, c_pBulk->lbHead.dt.sec);
		dwDate = c_tidy.GetDateIndex(false);
		dwTime = c_tidy.GetTimeIndex();
		bFrom = false;
	}

	sprintf(c_cds.szCurDirLte, "%s/d%06d_%02d", ARCHIVELTE_PATH, dwDate, c_clogFileSn);
	sprintf(c_cds.szDirForSending, "%s/d%06d_%02d", SENDHISTORY_PATH, dwDate, c_clogFileSn);

	TRACK("MM>lte make new dir %s\n", c_cds.szCurDirLte);

	c_cds.iYear = dwDate / 10000;
	dwDate %= 10000;
	c_cds.iMonth = dwDate / 100;
	c_cds.iDay = dwDate % 100;

	int res = mkdir(c_cds.szCurDirLte, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make %s!(%s)\n", c_cds.szCurDirLte, strerror(errno));
		c_cds.bExistLte = false;
		return false;
	}
	sprintf(c_cds.szCurLogbookLte, "%s/%c%06d", c_cds.szCurDirLte, LOGBOOK_SIGN, dwTime);
	sprintf(c_cds.szCurTroubleLte, "%s/%c%06d", c_cds.szCurDirLte, TROUBLE_SIGN, dwTime);
	c_cds.bExistLte = true;
	c_cds.bLogbookHeadLte = false;

	MakeLogForRtdFile();		// 신규폴더 생성된 내역 history 폴더에 남기기, 현재 함수가 호출이 안 될 경우도 고려
	TRACK("MM:lte make  history dir %s \n", c_cds.szDirForSending);
	c_clogFileSn ++;

	if (c_bDebug)	TRACK("MM:make LTE dir %02d/%02d/%02d %06d from %s\n", c_cds.iYear, c_cds.iMonth, c_cds.iDay, dwTime, bFrom ? "ts" : "self");
	return true;
}

bool CFlux::MakeCurrentDirectoryCpm()
{
	//sprintf(c_szCurDir, "%s/d%06d", ARCHIVE_PATH, c_tidy.GetDateIndex(false));		// "/h/sd/arch/dyymmdd"
	DWORD dwDate, dwTime;
	bool bFrom = true;
	if (c_pBulk != NULL && c_pBulk->lbHead.dt.year != 0 &&
		c_pBulk->lbHead.dt.mon > 0 && c_pBulk->lbHead.dt.mon < 13 &&
		c_pBulk->lbHead.dt.day > 0 && c_pBulk->lbHead.dt.day < 32 &&
		c_pBulk->lbHead.dt.hour < 24 && c_pBulk->lbHead.dt.min < 50 && c_pBulk->lbHead.dt.sec < 60) {
		dwDate = (DWORD)c_pBulk->lbHead.dt.year * 10000 + (DWORD)c_pBulk->lbHead.dt.mon * 100 + (DWORD)c_pBulk->lbHead.dt.day;
		dwTime = (DWORD)c_pBulk->lbHead.dt.hour * 10000 + (DWORD)c_pBulk->lbHead.dt.min * 100 + (DWORD)c_pBulk->lbHead.dt.sec;
	}
	else {
		if (c_pBulk == NULL)	TRACK("MM:bulk is null!\n");
		else	TRACK("MM:head info wrong! %02d/%02d/%02d %02d:%02d:%02d\n",
							c_pBulk->lbHead.dt.year, c_pBulk->lbHead.dt.mon, c_pBulk->lbHead.dt.day,
							c_pBulk->lbHead.dt.hour, c_pBulk->lbHead.dt.min, c_pBulk->lbHead.dt.sec);
		dwDate = c_tidy.GetDateIndex(false);
		dwTime = c_tidy.GetTimeIndex();
		bFrom = false;
	}
	sprintf(c_cds.szCurDirCpm, "%s/d%06d", CPM_PATH, dwDate);
	c_cds.iYear = dwDate / 10000;
	dwDate %= 10000;
	c_cds.iMonth = dwDate / 100;
	c_cds.iDay = dwDate % 100;

	int res = mkdir(c_cds.szCurDirCpm, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make CPM %s!(%s)\n", c_cds.szCurDirSd, strerror(errno));
		c_cds.bExistCpm = false;
		return false;
	}
	sprintf(c_cds.szCurLogbookCpm, "%s/%c%06d", c_cds.szCurDirCpm, LOGBOOK_SIGN, dwTime);
	sprintf(c_cds.szCurTroubleCpm, "%s/%c%06d", c_cds.szCurDirCpm, TROUBLE_SIGN, dwTime);
	c_cds.bExistCpm = true;
	c_cds.bLogbookHeadCpm = false;
	if (c_bDebug)	TRACK("MM:make CPM dir %02d/%02d/%02d %06d from %s\n", c_cds.iYear, c_cds.iMonth, c_cds.iDay, dwTime, bFrom ? "ts" : "self");
	return true;
}

bool CFlux::MakeCurrentDirectorySd()
{
	//sprintf(c_szCurDir, "%s/d%06d", ARCHIVE_PATH, c_tidy.GetDateIndex(false));		// "/h/sd/arch/dyymmdd"
	DWORD dwDate, dwTime;
	bool bFrom = true;
	if (c_pBulk != NULL && c_pBulk->lbHead.dt.year != 0 &&
		c_pBulk->lbHead.dt.mon > 0 && c_pBulk->lbHead.dt.mon < 13 &&
		c_pBulk->lbHead.dt.day > 0 && c_pBulk->lbHead.dt.day < 32 &&
		c_pBulk->lbHead.dt.hour < 24 && c_pBulk->lbHead.dt.min < 60 && c_pBulk->lbHead.dt.sec < 60) {
		dwDate = (DWORD)c_pBulk->lbHead.dt.year * 10000 + (DWORD)c_pBulk->lbHead.dt.mon * 100 + (DWORD)c_pBulk->lbHead.dt.day;
		dwTime = (DWORD)c_pBulk->lbHead.dt.hour * 10000 + (DWORD)c_pBulk->lbHead.dt.min * 100 + (DWORD)c_pBulk->lbHead.dt.sec;
	}
	else {
		if (c_pBulk == NULL)	TRACK("MM:bulk is null!\n");
		else	TRACK("MM:head info wrong! %02d/%02d/%02d %02d:%02d:%02d\n",
							c_pBulk->lbHead.dt.year, c_pBulk->lbHead.dt.mon, c_pBulk->lbHead.dt.day,
							c_pBulk->lbHead.dt.hour, c_pBulk->lbHead.dt.min, c_pBulk->lbHead.dt.sec);
		dwDate = c_tidy.GetDateIndex(false);
		dwTime = c_tidy.GetTimeIndex();
		bFrom = false;
	}
	sprintf(c_cds.szCurDirSd, "%s/d%06d", ARCHIVE_PATH, dwDate);
	c_cds.iYear = dwDate / 10000;
	dwDate %= 10000;
	c_cds.iMonth = dwDate / 100;
	c_cds.iDay = dwDate % 100;

	int res = mkdir(c_cds.szCurDirSd, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make %s!(%s)\n", c_cds.szCurDirSd, strerror(errno));
		c_cds.bExistSd = false;
		return false;
	}
	sprintf(c_cds.szCurLogbookSd, "%s/%c%06d", c_cds.szCurDirSd, LOGBOOK_SIGN, dwTime);
	sprintf(c_cds.szCurTroubleSd, "%s/%c%06d", c_cds.szCurDirSd, TROUBLE_SIGN, dwTime);
	c_cds.bExistSd = true;
	c_cds.bLogbookHeadSd = false;
	if (c_bDebug)	TRACK("MM:make dir %02d/%02d/%02d %06d from %s\n", c_cds.iYear, c_cds.iMonth, c_cds.iDay, dwTime, bFrom ? "ts" : "self");
	return true;
}

bool CFlux::AppendLogbookToLte()
{
	if (!c_cds.bExistLte && !MakeCurrentDirectoryLte())	return false;

	_TWATCHCLICK();
	FILE* fp = fopen(c_cds.szCurLogbookLte, "a");
	if (fp != NULL) {
		if (!c_cds.bLogbookHeadLte) {// loogbook head가 가 파일의 첫머리에 없다면
			size_t ret = fwrite(&c_pBulk->lbHead, 1, sizeof(LOGBOOKHEAD), fp);
			if (ret != sizeof(LOGBOOKHEAD)) {
				c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
				TRACK("MM:ERR:can not write log head!(%d-%s)\n", ret, strerror(errno));
				fclose(fp);
				return false;
			}
			c_cds.bLogbookHeadLte = true;// 파일에 logbook head를 write했다는 이력을 남김
			TRACK("MM-LTE: Write log head!\n");
			fseeko64(fp, 0L, SEEK_END);
		}
		size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		fclose(fp);
		_TWATCHMEASURE(WORD, c_pBulk->fts.logw.wCur, 3);
		if (c_pBulk->fts.logw.wCur > c_pBulk->fts.logw.wMax)	c_pBulk->fts.logw.wMax = c_pBulk->fts.logw.wCur;
		if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:can not append LTE log book!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			return false;
		}
		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;// 파일에 write한 결과 정상 상태를 결과에 설정함

		return true;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;// 파일 열기 실패 상태를 결과에 넣음
	TRACK("MM>ERR:can not open LTE log book!(%s) %s\n", strerror(errno),c_cds.szCurLogbookLte);
	return false;
}

bool CFlux::AppendLogbookToCpm()
{
	if (!c_cds.bExistCpm && !MakeCurrentDirectoryCpm())	return false;

	_TWATCHCLICK();
	FILE* fp = fopen(c_cds.szCurLogbookCpm, "a");
	if (fp != NULL) {
		if (!c_cds.bLogbookHeadCpm) {// loogbook head가 작성이되어있지 않으면
			size_t ret = fwrite(&c_pBulk->lbHead, 1, sizeof(LOGBOOKHEAD), fp);
			if (ret != sizeof(LOGBOOKHEAD)) {
				c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
				TRACK("MM:ERR:can not write log head!(%d-%s)\n", ret, strerror(errno));
				fclose(fp);
				return false;
			}
			c_cds.bLogbookHeadCpm = true;// 파일에 logbook head를 write했다는 이력을 남김
			fseeko64(fp, 0L, SEEK_END);
		}
		size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		fclose(fp);
		_TWATCHMEASURE(WORD, c_pBulk->fts.logw.wCur, 3);
		if (c_pBulk->fts.logw.wCur > c_pBulk->fts.logw.wMax)	c_pBulk->fts.logw.wMax = c_pBulk->fts.logw.wCur;
		if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:can not append CPM log book!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			return false;
		}
		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;// 파일에 write한 결과 정상 상태를 결과에 설정함
		PLOGBOOK pLog = (PLOGBOOK)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c;
		c_sectds.dwDistance = pLog->real.qwDistance / 1000;		// km...
		for (int n = 0; n < V3F_MAX; n ++) {
			if (n < SIV_MAX)	c_sectds.qwSivPower[n] = pLog->real.qwSivPower[n];
			c_sectds.qwV3fPower[n] = pLog->real.qwV3fPower[n];
			c_sectds.qwV3fReviv[n] = pLog->real.qwV3fReviv[n];
		}
		return true;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;// 파일 열기 실패 상태를 결과에 넣음
	TRACK("MM>ERR:can not open CPM logbook!(%s)\n", strerror(errno));
	return false;
}

bool CFlux::AppendLogbookToSd()
{
	// 정보저장폴더가 없으면  신규 저장 폴더 생성하고 결과값이 false 이면 return false, 결과 값이 true 이면 그냥 빠짐
	// 정보저장 폴어가 있으면 아래 루틴 진행
	if (!(c_pBulk->wState & (1 << BULKSTATE_INTCMS))) {	// at RTD
		if (c_cds.bExistSd && !c_cds.bHistoryDirExist) {
			 MakeLogForRtdFile();		// 신규폴더 생성된 내역 history 폴더에 남기기, 현재 함수가 호출이 안 될 경우도 고려
			TRACK("MM:AppendLogbook ->  make  history dir %s!\n", c_cds.szDirForSending);
		}
	}

	if (!c_cds.bExistSd && !MakeCurrentDirectorySd()) {
		c_pBulk->wState &= ~(1 << BULKSTATE_LOGAREA);
		return false;
	}
	c_pBulk->wState |= (1 << BULKSTATE_LOGAREA);

	_TWATCHCLICK();
	FILE* fp = fopen(c_cds.szCurLogbookSd, "a");
	if (fp != NULL) {
		if (!c_cds.bLogbookHeadSd) {
			size_t ret = fwrite(&c_pBulk->lbHead, 1, sizeof(LOGBOOKHEAD), fp);
			if (ret != sizeof(LOGBOOKHEAD)) {
				c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
				TRACK("MM:ERR:can not write log head!(%d-%s)\n", ret, strerror(errno));
				fclose(fp);
				return false;
			}
			c_cds.bLogbookHeadSd = true;
			fseeko64(fp, 0L, SEEK_END);
		}
		size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		fclose(fp);
		_TWATCHMEASURE(WORD, c_pBulk->fts.logw.wCur, 3);
		if (c_pBulk->fts.logw.wCur > c_pBulk->fts.logw.wMax)	c_pBulk->fts.logw.wMax = c_pBulk->fts.logw.wCur;
		if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:can not append log book!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			return false;
		}
		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;
		PLOGBOOK pLog = (PLOGBOOK)c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c;
		c_sectds.dwDistance = (DWORD)(pLog->real.qwDistance / 1000);		// km...
		for (int n = 0; n < V3F_MAX; n ++) {
			if (n < SIV_MAX)	c_sectds.qwSivPower[n] = pLog->real.qwSivPower[n];
			c_sectds.qwV3fPower[n] = pLog->real.qwV3fPower[n];
			c_sectds.qwV3fReviv[n] = pLog->real.qwV3fReviv[n];
		}
		return true;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;
	TRACK("MM>ERR:can not open logbook!(%s)\n", strerror(errno));
	return false;
}

bool CFlux::AppendTroubleToLte()
{
	if (!c_cds.bExistLte && !MakeCurrentDirectoryLte())	return false;

	TROUBLEARCH ta;
	memcpy(&ta, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], sizeof(TROUBLEARCH));
	if (/*ta.b.mon != c_iCurMonth || ta.b.day != c_iCurDay || */ta.b.cid > 9 || ta.b.code > 999)
	{
		TRACK("MM>ERR:mon %d day %d cid %d code %d\n", ta.b.mon, ta.b.day, ta.b.cid, ta.b.code);
	}
	_TWATCHCLICK();
	FILE* fp = fopen(c_cds.szCurTroubleLte, "a");
	if (fp != NULL) {
		size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		fclose(fp);
		_TWATCHMEASURE(WORD, c_pBulk->fts.trbw.wCur, 3);
		if (c_pBulk->fts.trbw.wCur > c_pBulk->fts.trbw.wMax)	c_pBulk->fts.trbw.wMax = c_pBulk->fts.trbw.wCur;
		if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:can not append trouble!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			return false;
		}
		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;
		return true;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;
	TRACK("MM>ERR:can not open LTE trouble!(%s)\n", strerror(errno));
	return false;
}

bool CFlux::AppendTroubleToCpm()
{
	if (!c_cds.bExistCpm && !MakeCurrentDirectoryCpm())	return false;

	TROUBLEARCH ta;
	memcpy(&ta, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], sizeof(TROUBLEARCH));
	if (/*ta.b.mon != c_iCurMonth || ta.b.day != c_iCurDay || */ta.b.cid > 9 || ta.b.code > 999)
	{
		TRACK("MM>ERR:mon %d day %d cid %d code %d\n", ta.b.mon, ta.b.day, ta.b.cid, ta.b.code);
	}
	_TWATCHCLICK();
	FILE* fp = fopen(c_cds.szCurTroubleCpm, "a");
	if (fp != NULL) {
		size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		fclose(fp);
		_TWATCHMEASURE(WORD, c_pBulk->fts.trbw.wCur, 3);
		if (c_pBulk->fts.trbw.wCur > c_pBulk->fts.trbw.wMax)	c_pBulk->fts.trbw.wMax = c_pBulk->fts.trbw.wCur;
		if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:can not append trouble!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
			return false;
		}
		c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRNON;
		return true;
	}
	c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTOPENDEST;
	TRACK("MM>ERR:can not open CPM trouble!(%s)\n", strerror(errno));
	return false;
}

bool CFlux::AppendTroubleToSd()
{
	if (!c_cds.bExistSd && !MakeCurrentDirectorySd()) {
		c_pBulk->wState &= ~(1 << BULKSTATE_LOGAREA);
		return false;
	}
	c_pBulk->wState |= (1 << BULKSTATE_LOGAREA);

	TROUBLEARCH ta;
	memcpy(&ta, &c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], sizeof(TROUBLEARCH));
	if (/*ta.b.mon != c_iCurMonth || ta.b.day != c_iCurDay || */ta.b.cid > 9 || ta.b.code > 999)
		TRACK("MM>ERR:mon %d day %d cid %d code %d\n", ta.b.mon, ta.b.day, ta.b.cid, ta.b.code);
	_TWATCHCLICK();
	FILE* fp = fopen(c_cds.szCurTroubleSd, "a");
	if (fp != NULL) {
		size_t ret = fwrite(&c_pBulk->m[BPID_PERIOD].bt.pad.tp.tin.c[0], 1, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, fp);
		fclose(fp);
		_TWATCHMEASURE(WORD, c_pBulk->fts.trbw.wCur, 3);
		if (c_pBulk->fts.trbw.wCur > c_pBulk->fts.trbw.wMax)	c_pBulk->fts.trbw.wMax = c_pBulk->fts.trbw.wCur;
		if (ret != c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength) {
			c_pBulk->m[BPID_PERIOD].iResult = BULKRES_ERRCANNOTWRITEDEST;
			TRACK("MM>ERR:can not append trouble!(%d:%d-%s)\n", ret, c_pBulk->m[BPID_PERIOD].bt.pad.tp.wLength, strerror(errno));
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

	DBFTIME dbt;
	dbt.dw = qu.dw;
	char buf[256];
					//  yy  mm  dd  hh  mm  ss
	sprintf(buf, "%s/%c%02d%02d%02d%02d%02d%02d", INSPECT_PATH, cSign,
			dbt.t.year + MIN_DTLYEAR, dbt.t.mon, dbt.t.day, dbt.t.hour, dbt.t.min, dbt.t.sec);
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
			c_cEntryType = ENTRYTYPE_LIST;
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
	sprintf(c_szDestDailyPath, "%s/n%04x", USB_PATH, c_pBulk->lbHead.wProperNo);		// /dos/c/n2XYY
	strncpy(c_szDestInspPath, c_szDestDailyPath, SIZE_DEFAULTPATH);
	int res = mkdir(c_szDestDailyPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make %s!(%s)\n", c_szDestDailyPath, strerror(errno));
		return false;
	}
	TRACK("MM:make %s\n", c_szDestDailyPath);

	bool bDaily = c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.cItem == USBDIRITEM_DAILY ? true : false;
	if (bDaily)	strcat(c_szDestDailyPath, USBARCH_NAME);													// /dos/c/n2XYY/arch
	else	strcat(c_szDestInspPath, USBINSP_NAME);															// /dos/c/n2XYY/insp
	res = mkdir(bDaily ? c_szDestDailyPath : c_szDestInspPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	if (res != 0 && errno != EEXIST) {
		TRACK("MM>ERR:can not make %s!(%s)\n", bDaily ? c_szDestDailyPath : c_szDestInspPath, strerror(errno));
		return false;
	}
	TRACK("MM:make %s\n", bDaily ? c_szDestDailyPath : c_szDestInspPath);

	if (bDaily) {
		sprintf(c_szSrcDailyDir, "%c%06d", LOGDIR_SIGN, c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.ucmd.dwIndex);	// /dos/c/n2XYY/arch/dYYMMDD
		strcat(c_szDestDailyPath, "/");
		strcat(c_szDestDailyPath, c_szSrcDailyDir);
		res = mkdir(c_szDestDailyPath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && errno != EEXIST) {
			TRACK("MM>ERR:can not make %s!(%s)\n", c_szDestDailyPath, strerror(errno));
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

void CFlux::ExtractOldTrouble()
{
	int ofs = c_pBulk->m[BPID_OCCASN].bt.pad.sp.dwIndex * SIZE_RTDCONTEXT;
	if (ofs < (int)sizeof(OLDTROUBLES)) {
		BYTE* p = c_tidy.GetOldTroubleBuf();
		WORD leng = SIZE_RTDCONTEXT;
		if ((ofs + leng) > (int)sizeof(OLDTROUBLES))	leng = (int)sizeof(OLDTROUBLES) - ofs;
		memcpy(&c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.c[0], p + ofs, leng);
		c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength = leng;
	}
	else	c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength = 0;
	c_pBulk->m[BPID_OCCASN].bEnd = true;
	//TRACK("MM:old end %d\n", c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength);
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

void CFlux::ReportSectionDossier()
{
	SECTDOSS prevsd;
	memset(&prevsd, 0, sizeof(SECTDOSS));
	bool bPrev = false;

	FILE* fp;
	if ((fp = fopen(SECTDOSS_PATH, "w")) != NULL) {
		size_t leng = fread(&prevsd, 1, sizeof(SECTDOSS), fp);
		fclose(fp);
		if (leng == sizeof(SECTDOSS))	bPrev = true;
	}

	SECTDOSS sd;
	memset(&sd, 0, sizeof(SECTDOSS));
	if (bPrev) {
		memcpy(&sd.dt, &prevsd.dt, sizeof(DEVTIME));
		sd.dwDistance = c_sectds.dwDistance - prevsd.dwDistance;
		for (int n = 0; n < V3F_MAX; n ++) {
			if (n < SIV_MAX)	sd.qwSivPower[n] = c_sectds.qwSivPower[n] - prevsd.qwSivPower[n];
			sd.qwV3fPower[n] = c_sectds.qwV3fPower[n] - prevsd.qwV3fPower[n];
			sd.qwV3fReviv[n] = c_sectds.qwV3fReviv[n] - prevsd.qwV3fReviv[n];
		}
	}
	else {
		sd.dt.year = 17;
		sd.dt.mon = 3;
		sd.dt.day = 1;
		sd.dt.hour = sd.dt.min = sd.dt.sec = 0;
		sd.dwDistance = c_sectds.dwDistance;
		for (int n = 0; n < V3F_MAX; n ++) {
			if (n < SIV_MAX)	sd.qwSivPower[n] = c_sectds.qwSivPower[n];
			sd.qwV3fPower[n] = c_sectds.qwV3fPower[n];
			sd.qwV3fReviv[n] = c_sectds.qwV3fReviv[n];
		}
	}
	memcpy(c_pBulk->m[BPID_OCCASN].bt.pad.sp.hoe.c, &sd, sizeof(SECTDOSS));
	c_pBulk->m[BPID_OCCASN].bt.pad.sp.wLength = sizeof(SECTDOSS);
	c_pBulk->m[BPID_OCCASN].bEnd = true;
	if (c_bDebug) {
		TRACK("MM:section dossier\n");
		//TRACK("MM:distance %d\n", sd.dwDistance);
		//TRACK("MM:siv pwr %d\t%d\t%d\n", sd.qwSivPower[0], sd.qwSivPower[1], sd.qwSivPower[2]);
		//TRACK("MM:v3f pwr %d\t%d\t%d\t%d\t%d\n", sd.qwV3fPower[0], sd.qwV3fPower[1], sd.qwV3fPower[2], sd.qwV3fPower[3], sd.qwV3fPower[4]);
		//TRACK("MM:v3f rev %d\t%d\t%d\t%d\t%d\n", sd.qwV3fPower[0], sd.qwV3fPower[1], sd.qwV3fPower[2], sd.qwV3fPower[3], sd.qwV3fPower[4]);
	}
}

bool CFlux::RestoreSectionDossier()
{
	FILE* fp;
	if ((fp = fopen(SECTDOSS_PATH, "w")) != NULL) {
		size_t leng = fwrite(&c_sectds, 1, sizeof(SECTDOSS), fp);
		fclose(fp);
		if (leng == sizeof(SECTDOSS)) {
			if (c_bDebug)	TRACK("MM:save section dossier.\n");
			return true;
		}
		TRACK("MM>ERR:save section dossier!(%d)\n", leng);
	}
	TRACK("MM>ERR:can not open section dossire!\n");
	return false;
}

BYTE CFlux::CompressionFile()
{
	int rc;
	char buf[256];
	char zipList[256];
	int listCounter;
	int len = 0;
	int i = 0;
	int debugpt = 0;

	c_pBulk->m[BPID_OCCASN].bBegin = true;			// 압축 시작 상태 set
	c_pBulk->m[BPID_OCCASN].bEnd = false;			// 압축 종료 생태 clear

	TRACK("MM:start zip E:B %02x, Result %i\n",((c_pBulk->m[BPID_OCCASN].bEnd)<<1) | (c_pBulk->m[BPID_OCCASN].bBegin), c_pBulk->m[BPID_OCCASN].iResult);

	memset(&c_fl, 0, sizeof(LOGLIST));		// 압축 폴더 리스트 초기화(실제 저장폴더)
	memset(&c_fh, 0, sizeof(LOGLIST));		// 압축 폴더 리스트 초기화(history 폴더)
	strcpy(buf,(PSZ)SENDHISTORY_PATH);		// "/h/sd/his"

	TRACK("MM:CompressionFile %i \n",debugpt++);


	DIR* pDir = opendir(buf);				// "/h/sd/his" 폴더 열기

	if (pDir != NULL) {						// 열기 성공
		struct dirent* pEntry;				// 폴더  구조체 생성
		while ((pEntry = readdir(pDir)) != NULL) {		// 폴더내의 폴더 읽기
			if (pEntry->d_name[0] == '.')	continue;	// "."은 pass
			strcpy(c_fl.list[i].szName, ARCHIVE_PATH);  // 압축 path 생성 arc 디렉토리  "/h/sd/arc"
			strcpy(buf, pEntry->d_name);
			strcat(c_fl.list[i].szName, "/");
			strcat(c_fl.list[i].szName, buf);

			strcpy(c_fh.list[i].szName,SENDHISTORY_PATH);  // 압축 path 생성 history 디렉토리
			strcat(c_fh.list[i].szName, "/");
			strcat(c_fh.list[i].szName, buf);

			TRACK("MM:arc folder %s, history folder %s\n",c_fl.list[i].szName,c_fh.list[i].szName);
			len = len + strlen(c_fl.list[i].szName);
			i++;
		}
		closedir(pDir);
	}
	listCounter = i;
	c_fl.wListNumber = listCounter;		// 리스트 폴더 갯수
	c_fh.wListNumber = listCounter;		// 리스트 폴더 갯수

	for (i = 0; i < listCounter;i ++) {
		pDir = opendir(c_fl.list[i].szName);		// history폴더에 있는 이름의 폴더를 arc폴더에서 열기
		if (pDir != NULL) {
			c_fl.list[i].valid = true;				// 동일한 폴더가 있음
			TRACK("MM: LOG DIR(%s) is exist! \n", c_fl.list[i].szName);
		}
		else {
			c_fl.list[i].valid = false;				// 동일한 폴더가 없음
			TRACK("MM: LOG DIR(%s) is not exist! \n",c_fl.list[i].szName);
		}
		closedir(pDir);
	}

	for (i = 0; i < listCounter; i ++) {
		if (c_fl.list[i].valid) {
			strcat(zipList, c_fl.list[i].szName);		// history 폴더에 있는 리스트를 기준으로 압축 폴더 리스트를 생성함
			strcat(zipList," ");
		}
	}
	TRACK("MM: LOG zip List(%s) is exist! \n", zipList);

	// MakeCurrentDirectory()가 호출이 한번도 안된 상태에서는  운행정보 및 고장정보 기록 폴더 path를 정함
	// 우선 오늘날짜의 운행기록폴더를 압축해서 보내도록 함
	if (strlen(c_cds.szCurDirSd) == 0) {
		DWORD dwCur = c_tidy.GetDateIndex(false);
		sprintf(c_cds.szCurDirSd, "%s/d%06d", ARCHIVE_PATH, dwCur);
	}

	TRACK("MM: Current Dir %s \n",c_cds.szCurDirSd);
	pDir = opendir(c_cds.szCurDirSd);
	if (pDir != NULL) {
		TRACK("MM:cur. dir is exist ! %02d/%02d/%02d\n", c_cds.iYear, c_cds.iMonth, c_cds.iDay);
		closedir(pDir);
	}
	else {
		TRACK("MM:LOG DIR is not exist !");
		return LOG_DIR_NOT_EXIST;
	}

	sprintf(c_zipBuf, (PSZ)"zip -r %s %s", (PSZ)c_cds.szCurDirSd, zipList);		// zipList에 있는 폴더 및 파일을 c_cds.szCurDir의 폴더 및 이름으로 압축함
	rc = system(c_zipBuf);
	if (rc == INVALID_HANDLE) {
		TRACK("MM:shell could not be run\n");
		c_pBulk->m[BPID_OCCASN].iResult = BULKRES_ERRUNKNOWN;
	}
	else {
		TRACK("MM:result of running command(%d)\n", WEXITSTATUS(rc));
		c_pBulk->m[BPID_OCCASN].iResult = BULKRES_OK;
	}

	c_pBulk->m[BPID_OCCASN].bBegin = false;
	c_pBulk->m[BPID_OCCASN].bEnd = true;
	c_pBulk->m[BPID_OCCASN].iResult = BULKRES_OK;
	TRACK("MM:end zip %x, result %i\n",((c_pBulk->m[BPID_OCCASN].bEnd)<<1) | (c_pBulk->m[BPID_OCCASN].bBegin),c_pBulk->m[BPID_OCCASN].iResult);
	return LOG_COMP_PASS;
}

void CFlux::SendDataByWifi()
{
	int rc;
	c_pBulk->m[BPID_OCCASN].bBegin = true;
	c_pBulk->m[BPID_OCCASN].bEnd = false;
	TRACK("MM:Start ftp Data. %x\n", ((c_pBulk->m[BPID_OCCASN].bEnd)<<1) | (c_pBulk->m[BPID_OCCASN].bBegin));
	sprintf(c_zipBuf, (PSZ)"ftp ftp://%s@%s < ftpcmd.txt", WIFISERVER_IDPW, WIFISERVER_IP);
	rc = system(c_zipBuf);
	if (rc == INVALID_HANDLE) {
		TRACK("MM:shell could not be run\n");
		c_pBulk->m[BPID_OCCASN].iResult = BULKRES_ERRUNKNOWN;
	}
	else {
		TRACK("MM:result of running command(%d)\n", WEXITSTATUS(rc));
		c_pBulk->m[BPID_OCCASN].iResult = BULKRES_OK;
	}

	c_pBulk->m[BPID_OCCASN].bEnd = true;
	c_pBulk->m[BPID_OCCASN].bBegin = false;
	TRACK("MM:end ftp %x\n",((c_pBulk->m[BPID_OCCASN].bEnd)<<1) | (c_pBulk->m[BPID_OCCASN].bBegin));
/*
	sprintf(g_cmdBuf,(PSZ)"rm %s.zip",c_cds.szCurDir);
	rc =system(g_cmdBuf);
	if (rc == INVALID_HANDLE)
	{
		TRACK("MM:shell could not be run\n");
	}
	else
	{
		TRACK("MM:result of running command(%d)\n", WEXITSTATUS(rc));
	}
	*/
}

bool CFlux::MakeLogForRtdFile()
{
	// 로그파일을 위한 디렉토리 생성시 무선전송을 해야 한다는 의미를 나타내는 파일 생성
	// 전송 완료 후 해당 파일 삭제
	// 해당 폴더에 업데이트 한 내용이 있으면 다시 파일 생성 시킴
//	DWORD dwCur = c_tidy.GetDateIndex(false);
//	sprintf(c_cds.szDirForSending, "%s/d%06d_%02d", SEND_HISTORY_PATH, dwDate,logFileSn);
	int res = mkdir(c_cds.szDirForSending, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
	// 에러가 있고, 이미 디렉토리가 있는 것이 아니라면
	if (res != 0 && errno != EEXIST) {			// 0: No error, EEXIST : The directory named by path already exists.
		TRACK("MM>ERR:can not make his dir %s!(%s)\n", c_cds.szDirForSending, strerror(errno));
		c_cds.bHistoryDirExist = false;
		return false;
	}
	c_cds.bHistoryDirExist = true;
	return true;
}

void CFlux::RemoveSendedFolder()
{
	//char folderName[10] ;
	//char *pStr = folderName;
	for (int i = 0 ; i < c_fl.wListNumber;i ++) {
		if (c_fl.list[i].valid) {		// arc 폴더가 유효 했으면 history 폴더에서 해당 폴더 삭제
			memset(c_zipBuf, 0, 1024);
			sprintf(c_zipBuf, (PSZ)"rmdir %s",c_fh.list[i].szName);
			TRACK("MM:COMMAND %s is executed !\n", c_zipBuf);
			int rc = system(c_zipBuf);
			if (rc == INVALID_HANDLE)	TRACK("MM:shell could not be run\n");
			else	TRACK("MM:result of running command(%d)\n", WEXITSTATUS(rc));
			c_cds.bHistoryDirExist = false;
		}
	}

	sprintf(c_zipBuf, (PSZ)"rm %s.zip", c_cds.szCurDirSd);
	int rc = system(c_zipBuf);
	if (rc == INVALID_HANDLE)	TRACK("MM:shell could not be run\n");
	else	TRACK("MM: zip file(%s.zip) is deleted!(%d)\n", c_cds.szCurDirSd, WEXITSTATUS(rc));

	/*
	TRACK("MM: remove sended folder in history folder\n");
	TRACK("MM : Removed folder name size is %i \n",c_pBulk->m[BPID_OCCASN].info.bt.pad.tp.wLength);
	strncpy(pStr, (char*)&c_pBulk->m[BPID_OCCASN].info.bt.pad.tp.tin.c[0],c_pBulk->m[BPID_OCCASN].info.bt.pad.tp.wLength);
	TRACK("MM : Removed folder name is %s\n",folderName);
	*/
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

bool CFlux::Launch(bool bDebug)
{
	c_bDebug = bDebug;
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

	c_pBulk->wState |= (1 << BULKSTATE_STORAGE);

	int res;
	DIR* pDir = opendir(ARCHIVE_PATH);	// logging file 저장 폴더 생성
	if (pDir == NULL) {
		res = mkdir(ARCHIVE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make archive directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	pDir = opendir(INSPECT_PATH);		// 타 고장 정보 file 저장 폴더 생성
	if (pDir == NULL) {
		res = mkdir(INSPECT_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make inspect directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	pDir = opendir(TRACE_PATH);			// 고장 추적 정보 파일 저장 폴더 생성
	if (pDir == NULL) {
		res = mkdir(TRACE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make trace directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	// for RTD begin ---
	// 파일 전송 이력 폴더 생성
	pDir = opendir(SENDHISTORY_PATH);
	if (pDir == NULL) {
		res = mkdir(SENDHISTORY_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make his directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);

	// log server 전송파일  폴더 생성
	pDir = opendir(ARCHIVELTE_PATH);
	if (pDir == NULL) {
		res = mkdir(ARCHIVELTE_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if (res != 0 && res != EEXIST)	TRACK("MM>ERR:can not make lte directory!(%s)\n", strerror(errno));
	}
	else	closedir(pDir);
	// for RTD end ---

	c_tidy.DeletePrev6(ARCHIVE_PATH);

	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_PDT, PDTINSPECT_SIGN);
	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_DAILY, DAILYINSPECT_SIGN);
	c_tidy.LimitPrev12(INSPECT_PATH, LIMIT_MONTHLY, MONTHLYINSPECT_SIGN);

	c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACESIV_SIGN);
	c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACEV3F_SIGN);
	c_tidy.LimitPrev12(TRACE_PATH, LIMIT_TRACE, TRACEECU_SIGN);

	c_pBulk->cEcuTraceLength = c_tidy.GetFilesLength(TRACE_PATH, TRACEECU_SIGN);
	if (c_bDebug)	TRACK("MM:ecu trace length %d.\n", c_pBulk->cEcuTraceLength);

	//c_tidy.AlignOldTrouble();
	//c_pBulk->wState |= (1 << BULKSTATE_OLDTROUBLE);
	//c_tidy.SaveOldTrouble();

	space = c_tidy.GetSpace(STORAGE_PATH, false);
	TRACK("MM:free space is %lld.\n", space);

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

int CFlux::CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), int priority)
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
		if (CreateThread(&c_hThread[0], &CFlux::WatchEntry, PRIORITY_MSG) == EOK) {
			if (CreateThread(&c_hThread[1], &CFlux::PeriodicEntry, PRIORITY_MSG) == EOK) {
				if (CreateThread(&c_hThread[2], &CFlux::OccasionEntry, PRIORITY_MSG) == EOK)
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
