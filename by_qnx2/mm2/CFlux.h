/*
 * CFlux.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once
//#define	__EXT_LF64SRC
//#undef	__OFF_BITS__
//#define	__OFF_BITS__	64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "Mm.h"
#include "CTidy.h"

class CFlux
{
public:
	CFlux();
	virtual ~CFlux();

private:
	CTidy	c_tidy;

	QHND	c_hQueue[3];
	TRHND	c_hThread[3];
	int		c_hShared;
	bool	c_bDebug;

	WORD	c_wSeq;
	WORD	c_wLogbookMonitor;
	WORD	c_wTroubleMonitor;

	WORD	c_wCpmLogMonitor;
	WORD	c_wLogbookMonitorLte;
	BYTE	c_clogFileSn;

	PBULKPACK	c_pBulk;
	SECTDOSS	c_sectds;

	enum enENTRYTYPE {
		ENTRYTYPE_NON,
		ENTRYTYPE_LIST,
		ENTRYTYPE_DUMPLOGBOOK,
		ENTRYTYPE_DUMPINSPECT,
	};

	char		c_cEntryType;
	DWORD		c_dwEntryIndex;
	DWORD		c_dwEntryLength;
	DWORD		c_dwCurSize;

	typedef struct _tagCURDIRSTATE {
		int		iYear;
		int		iMonth;
		int		iDay;
		bool	bExistSd;
		bool	bLogbookHeadSd;
		bool	bExistCpm;
		bool	bLogbookHeadCpm;
		bool	bExistLte;
		bool	bLogbookHeadLte;
		bool	bHistoryDirExist;
		char	szCurDirSd[SIZE_DEFAULTPATH];
		char	szCurLogbookSd[SIZE_DEFAULTPATH];
		char	szCurTroubleSd[SIZE_DEFAULTPATH];
		char	szCurDirCpm[SIZE_DEFAULTPATH];
		char	szCurLogbookCpm[SIZE_DEFAULTPATH];
		char	szCurTroubleCpm[SIZE_DEFAULTPATH];
		char	szCurDirLte[SIZE_DEFAULTPATH];
		char	szCurLogbookLte[SIZE_DEFAULTPATH];
		char	szCurTroubleLte[SIZE_DEFAULTPATH];
		char	szDirForSending[SIZE_DEFAULTPATH];
		char	szZipName[SIZE_DEFAULTPATH];
	} CURDIRSTATE;
	CURDIRSTATE	c_cds;

	char	c_szSrcDailyDir[SIZE_DEFAULTPATH];
	char	c_szSrcFileName[SIZE_DEFAULTPATH];
	char	c_szDestDailyPath[SIZE_DEFAULTPATH];
	char	c_szDestInspPath[SIZE_DEFAULTPATH];
	char	c_szDestFileName[SIZE_DEFAULTPATH];
	//char	c_szUsb[SIZE_DEFAULTPATH];
	char	c_szCpm[SIZE_DEFAULTPATH];
	char	c_zipBuf[1024];

	LOGLIST	c_fl;
	LOGLIST	c_fh;

public:
	virtual void DoTimeSet()			{}
	virtual void DoCloseCurrent()		{}
	virtual void DoMoveEnvirons()		{}
	virtual void DoMoveCareers()		{}
	virtual void DoAppendLogbook()		{}
	virtual void DoAppendTrouble()		{}
	virtual void DoMoveInsect()			{}
	virtual void DoMoveEcuTrace()		{}
	virtual void DoEraseEcuTrace()		{}
	virtual void DoDownloadCancel()		{}
	virtual void DoPeriodic(WORD wMsg)	{}
	virtual void DoOccasion(WORD wMsg)	{}

private:
	static PVOID	WatchEntry(PVOID pVoid);
	static PVOID	PeriodicEntry(PVOID pVoid);
	static PVOID	OccasionEntry(PVOID pVoid);

	void	Watch();
	void	Periodic();
	void	Occasion();
	bool	TimeSet();
	bool	MoveEnvirons();
	bool	MoveCareers();
	int		MoveIndexFiles(char* tp, int ti, char* sp, int si);
	//void	MoveTraces();
	bool	MakeCurrentDirectoryLte();
	bool	MakeCurrentDirectoryCpm();
	bool	MakeCurrentDirectorySd();
	bool	AppendLogbookToLte();
	bool	AppendLogbookToCpm();
	bool	AppendLogbookToSd();
	bool	AppendTroubleToLte();
	bool	AppendTroubleToCpm();
	bool	AppendTroubleToSd();
	void	MoveInspect();
	void	MoveEcuTrace();
	void	EraseEcuTrace();
	WORD	ReportList(WORD wIndex);
	void	ExtractEntryList(BYTE cSign);
	void	ExtractLogbookList();
	bool	MakeDirOnUsb();
	void	MakeDestination();
	void	CopyLogbook();
	void	ExtractLogbook();
	void	ExtractInspect();
	void	ExtractOldTrouble();
	void	InsertLogbook();
	void	InsertInspect();
	void	EntryDump(BYTE cSign);
	bool	ScanUsb();
	void	ReportSectionDossier();
	bool	RestoreSectionDossier();
	BYTE	CompressionFile();
	void	SendDataByWifi();
	bool	MakeLogForRtdFile();
	void	RemoveSendedFolder();

	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), int priority);
	bool	CreateQueue();
	bool	CreateSharedMem();

public:
	void	FileWorkFlag(bool bCmd);
	void	SetBulkState(int index, bool bSet);
	bool	Launch(bool c_bDebug);
	void	Shutoff();

	//char*	GetSrcDailyDir()	{ return c_szSrcDailyDir; }
	//char*	GetDestDailyPath()	{ return c_szDestDailyPath; }
	PBULKPACK	GetBulk()		{ return c_pBulk; }
};
