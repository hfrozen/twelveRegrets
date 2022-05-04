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

	//QHND	c_hMsg[2];
	QHND	c_hQueue[3];
	TRHND	c_hThread[3];
	int		c_hShared;
	WORD	c_wSeq;
	WORD	c_wLogbookMonitor;
	WORD	c_wTroubleMonitor;
	PBULKPACK	c_pBulk;

	bool	c_bDebug;
	DWORD	c_dwLogbookCnt;
	DWORD	c_dwTroubleCnt;
	V3FBRKCNTU	c_bcu;

	DWORD		c_dwEntryLength;

	typedef struct _tagCURRENTDIRSTATE {
		int		iYear;
		int		iMonth;
		int		iDay;
		bool	bLogbookHead;
		bool	bExist;
		char	szCurDir[SIZE_DEFAULTPATH];
		char	szCurLogbook[SIZE_DEFAULTPATH];
		char	szCurTrouble[SIZE_DEFAULTPATH];
	} CURRENTDIRSTATE;
	CURRENTDIRSTATE	c_cds;

	char	c_szSrcDailyDir[SIZE_DEFAULTPATH];
	char	c_szDestDailyPath[SIZE_DEFAULTPATH];
	char	c_szDestInspPath[SIZE_DEFAULTPATH];

	// 181026
	typedef struct _tagTRANSIT {
		WORD	wLength;
		char	buf[SIZE_LOGBOOK * 2];
	} TRANSIT;
	TRANSIT	c_tran[2];

	static PVOID	WatchEntry(PVOID pVoid);
	static PVOID	PeriodicEntry(PVOID pVoid);
	static PVOID	OccasionEntry(PVOID pVoid);

	void	Watch();
	void	Periodic();
	void	Occasion();
	bool	TimeSet(bool byRtd);
	bool	MoveEnvirons();
	bool	MoveCareers();
	bool	MakeCurrentDirectory(bool bFrom);
	bool	AppendLogbook();
	bool	AppendTrouble();
	void	MoveInspect();
	void	MoveSivDetect();
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
	void	InsertLogbook();
	void	InsertInspect();
	void	EntryDump(BYTE cSign);
	bool	ScanUsb();
	bool	SaveFile(const char* pPath, PVOID pVoid, int iLength);
	void	CullRequiredDoc();

	bool	CreateQueue();
	bool	CreateSharedMem();

public:
	void	FileWorkFlag(bool bCmd);
	void	SetBulkState(int index, bool bSet);
	bool	Launch(bool c_bDebug);
	void	Shutoff();
	int		CreateThread(TRHND* pThreadHandle, PVOID (*ThreadFunc)(PVOID), PVOID pVoid, int priority);

	PBULKPACK	GetBulk()		{ return c_pBulk; }
};
