/*
 * CArchAux.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once
#define	__EXT_LF64SRC

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/neutrino.h>

#include "Infer.h"
#include "Mm.h"

class CArchAux {
public:
	CArchAux();
	virtual ~CArchAux();

private:
	PVOID	c_pParent;

	QHND		c_hMm;
	int			c_hShared;
	PBULKPACK	c_pBulk;

	int			c_nChannel;
	int			c_nConnect;
	TRHND		c_hThread;

	ENVIRONS	c_environs;
	CAREERS		c_careers;				// !!! double type
	CMJOGB		c_cmjog;
	V3FBRKCNTF	c_vbs;

	BYTE		c_cMmState;				// 180105
	BYTE		c_cMs10;
	double		c_dbSegment;			// 구간 거리, m
	double		c_dbPrevDist;			// 정지 시 현재 거리와 이 거리를 비교해서 차이가 있으면 현재 거리를 저장한다.

// 170726, remove
//	typedef struct _tagHVACTEMPSYNC {
//		bool	bTain;
//		WORD	wPeriod;
//	} HTEMPSYNC;
//	HTEMPSYNC	c_hts;
//#define	PERIOD_HVACTEMPSYNC			REAL2PERIOD(3000)

	WORD		c_wAidCmd;
	WORD		c_wBulkCmd;
	WORD		c_wBulkState;
	BYTE		c_cEcuTraceLength;

	bool		c_bEcuTrace;
	bool		c_bSivDetect;
	bool		c_bRtdNewSheet;
	DWORD		c_dwMakeCdmaTime;
	FILEATS		c_fts;

	CDMAREPORT	c_cdmar;

	//typedef union _tagECUTRACETEXTS {
	//	BYTE	c1[LIMIT_TRACE * ECUTRACEPAGE_MAX * SIZE_TRACETEXT];
	//	BYTE	c2[LIMIT_TRACE][ECUTRACEPAGE_MAX * SIZE_TRACETEXT];
	//	BYTE	c3[LIMIT_TRACE][ECUTRACEPAGE_MAX][SIZE_TRACETEXT];
	//} ECUTRACETEXTS;

#define	READY_ARRANGEECUTRACE			0xfffe
	typedef struct _tagECUTRACEUNIT {		// TRACE 데이터를 마스터에 주기위해...
		int				iStep;				// arrange step for trace data from 'sd/trc' to here
		WORD			wLength;			// tds의 바이트 수 / 3100
		ECUTRACETEXTS	tds;
	} ECUTRACEUNIT;
	ECUTRACEUNIT	c_etu;

	typedef struct _tagSIVTRACECOMBO {		// 마스터가 각 TRACE 데이터를 모으기위해...
		WORD		wLeng;					// block size
		union {
			BYTE	c1[LIMIT_TRACE * SIVTRACEPAGE_MAX * SIZE_TRACETEXT];
			BYTE	c2[LIMIT_TRACE][SIVTRACEPAGE_MAX * SIZE_TRACETEXT];
			BYTE	c3[LIMIT_TRACE][SIVTRACEPAGE_MAX][SIZE_TRACETEXT];
		} ds;
	} SIVTRACECOMBO;

	typedef struct _tagSIVTRACECOMBOEX {		// 마스터가 각 TRACE 데이터를 모으기위해...
		WORD		wLeng;					// block size
		union {
			BYTE	c1[LIMIT_TRACE * SIVTRACEPAGEEX_MAX * SIZE_TRACETEXT];
			BYTE	c2[LIMIT_TRACE][SIVTRACEPAGEEX_MAX * SIZE_TRACETEXT];
			BYTE	c3[LIMIT_TRACE][SIVTRACEPAGEEX_MAX][SIZE_TRACETEXT];
		} ds;
	} SIVTRACECOMBOEX;

	typedef struct _tagSIVTRACESPHERE {
		bool			bExist;				// mm에게 TRACE SPHERE를 저장할 필요가 있다는 것을 표시...
		SIVTRACECOMBO	cb[SIV_MAX];
		SIVTRACECOMBOEX	cbEx[SIV_MAX];
	} SIVTRACESPHERE;
	SIVTRACESPHERE	c_sivsp;

	typedef struct _tagV3FTRACECOMBO {		// 마스터가 각 TRACE 데이터를 모으기위해...
		WORD		wLeng;					// block size
		union {
			BYTE	c1[LIMIT_TRACE * V3FTRACEPAGE_MAX * SIZE_TRACETEXT];
			BYTE	c2[LIMIT_TRACE][V3FTRACEPAGE_MAX * SIZE_TRACETEXT];
			BYTE	c3[LIMIT_TRACE][V3FTRACEPAGE_MAX][SIZE_TRACETEXT];
		} ds;
	} V3FTRACECOMBO;

	typedef struct _tagV3FTRACESPHERE {
		bool			bExist;
		V3FTRACECOMBO	cb[V3F_MAX];
	} V3FTRACESPHERE;
	V3FTRACESPHERE	c_v3fsp;

	typedef struct _tagECUTRACECOMBO {		// 마스터가 각 TRACE 데이터를 모으기위해...
		WORD			wLeng;
		ECUTRACETEXTS	ds;
	} ECUTRACECOMBO;

	typedef struct _tagECUTRACESPHERE {
		bool			bExist;
		ECUTRACECOMBO	cb[ECU_MAX];
	} ECUTRACESPHERE;
	ECUTRACESPHERE	c_ecusp;

	enum enTEMPORARYCMD {
		TEMPCMD_LOADECUTRACE = _PULSE_CODE_MINAVAIL,
		TEMPCMD_SAVESIVTRACESPHERE,
		TEMPCMD_SAVEV3FTRACESPHERE,
		TEMPCMD_SAVEECUTRACESPHERE,
		TEMPCMD_EXIT,
		TEMPCMD_MAX
	};

	static PVOID	TemporaryManagerEntry(PVOID pVoid);
	void	TemporaryManager();

	void	InitialTable();
	int		GetFileLength(const char* pPath, char cSign);
	bool	SaveFile(const char* pPath, PVOID pVoid, int iLength);
	bool	SaveFileA(const char* pPath, PVOID pVoid, int iLength);
	void	InitialEnvirons();
	bool	CheckEnvironsTime(DTSTIME dt, bool bTrack);
	int		CheckTimeScope(DTSTIME dt, bool bTrack);
	int		CheckDiaScope(BYTE* pDia, bool bTrack);
	int		CheckTempScope(BYTE* pTemp, BYTE cVentTime, bool bTrack);
	void	InitialCareers();
	void	AccordCmjog();
	void	AccordV3fBreaker();

	void	InitialBulkSyncro();
	void	GrowBulkSyncro();

	BYTE	ConvHdcToCdcA(BYTE ch);
	BYTE	ConvHdcToCdcB(BYTE ch);
	BYTE	ConvHdcToCdcC(BYTE ch);

	void	SetLogbookhead(bool bFrom);
	bool	IsValidAid();
	bool	Launch();

public:
	int64_t	GetFileSize(const char* pPath);
	void	Initial(int iFrom);
	bool	MirrorEnvirons(BYTE* pSrc);
	void	AccordEnvirons(int eid);
	void	SaveEnvirons();
	int		CheckEnvirons();
	void	AccordCareers(int rid);
	void	AddCareers(int nCrri, int di, double db);
	bool	MirrorCareers(BYTE* pSrc);
	double	GetCareers(int nCrri, int di);
	bool	MirrorCmjog(BYTE* pSrc);
	bool	CheckCmjog();
	bool	MirrorV3fBreaker(PV3FBRKCNTU pVbc);
	void	ClearV3fBreaker();
	PV3FBRKCNTF	GetV3fBreaker()				{ return &c_vbs; }

	bool	CullRequiredDoc();
	bool	CloseCurrent();
	bool	NewSheet();
	bool	SaveInspect(BYTE* pInsp);
	void	ClearTraceSphere(int nDevID);
	bool	GetTraceSphereDemand(int nDevID);
	void	ClearTraceSphereDemand(int nDevID);
	void	SetTraceLengthSphere(int nDevID, int nCID, WORD wLeng);
	void	LocateTraceSphere(int nDevID, int nID, WORD wChapterID, WORD wPageID, BYTE* p);
	void	ResultTraceSphere(int nDevID);
	void	SaveTraceSphere(int nDevID);
	void	ClearEcuTrace();
	int		GetEcuTraceStep()				{ return c_etu.iStep; }
	void	SetEcuTraceStep(int iStep)		{ c_etu.iStep = iStep; }
	WORD	GetEcuTraceLength()				{ return c_etu.wLength; }
	void	ArrangeEcuTrace();
	void	LoadEcuTraceFromTemporary();
	bool	TakeoutEcuTrace(WORD wChapID, WORD wPageID, BYTE* pBuf);
	int		SaveEcuTrace(BYTE* pTrace);
	void	EraseEcuTrace();
	int		SaveSivDetect(BYTE* pDetect);
	int		SaveSivDetect2(BYTE* pDetect);
	void	GetLogbookEntry(DWORD dwIndex, DWORD dwTotal);
	void	GetInspectEntry(DWORD dwIndex, DWORD dwTotal);
	void	GetLogbookList(char* pszLogbook, DWORD dwIndex, DWORD dwTotal);
	void	CopyLogbook(char* pszLogbook);
	void	GetLogbook(char* pszLogbook = NULL);
	void	GetInspect(char* pszInspect = NULL);
	// 170809
	//void	GetOldTrouble(bool bBegin);
	void	ContinueOldTrouble();
	void	MakeDestination(char* pszIndex, bool bLog);
	// 170809
	//void	RealignOldTrouble();
	void	PutLogbook(PRTDSPADE pSpd);
	void	PutInspect(PRTDSPADE pSpd);
	//void	GetSectionDossier();
	//void	RestoreSectionDossier();
	int		Bale(bool bType, BYTE* pMesh);

	PENVIRONS	GetEnvirons()				{ return &c_environs; }
	PCAREERS	GetCareers()				{ return &c_careers; }
	PCMJOGB		GetCmjog()					{ return &c_cmjog; }
	void	ClearSegment()					{ c_dbSegment = 0.f; }
	void	AddSegment(double dbDist)		{ c_dbSegment += dbDist; }
	double	GetSegment()					{ return c_dbSegment; }
	double	GetPrevDistance()				{ return c_dbPrevDist; }
	void	SetPrevDistance(double dbDist)	{ c_dbPrevDist = dbDist; }
	bool	GetRtdNewSheetState()			{ return c_bRtdNewSheet; }
	void	SetRtdNewSheetState(bool bStat)	{ c_bRtdNewSheet = bStat; }
	//bool	GetTain()						{ return c_hts.bTain; }
	bool	GetEcuTraceDemand()				{ return c_bEcuTrace; }
	void	ClearEcuTraceDemand()			{ c_bEcuTrace = false; }
	bool	GetSivDetectDemand()			{ return c_bSivDetect; }
	void	ClearSivDetectDemand()			{ c_bSivDetect = false; }
	WORD	GetMmVersion()					{ return c_pBulk->wMmVer; }
	BYTE	GetMmState()					{ return c_cMmState; }

	void	BuildCdmaReport();
	PCDMAREPORT	GetCdmaReport()				{ return &c_cdmar; }

	bool	OpenAid();
	void	AidCmd(WORD wCmd);
	bool	AidCmdOccasion(WORD cmd, DWORD dwIndex, DWORD dwTotal);
	bool	AidCmdPeriod(WORD cmd, BYTE* p, WORD wLength);
	bool	GetTimeSetStatus()				{ return (c_pBulk->wState & (1 << BULKSTATE_TIMESETTING)) ? true : false; }
	PBULKPACK	GetBulk()					{ return c_pBulk; }

	void	Arteriam();
	void	SetParent(PVOID pParent)		{ c_pParent = pParent; }

	PUBLICY_CONTAINER();
};
