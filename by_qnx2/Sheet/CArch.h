/*
 * CArch.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Slight.h"
#include "Prefix.h"
#include "Draft.h"
#include "../Inform2/DevInfo2/RtdInfo.h"
#include "Mm.h"
#include "Infer.h"
#include "../Component2/CPop.h"
#include "CArchAux.h"

#define	GETARCHIVE(p)	CArch* p = CArch::GetInstance()

class CArch
{
private:
	CArch();
	virtual ~CArch();

	PVOID		c_pCarrier;

	CMutex		c_mtt;		// 시간 설정 때...
	CMutex		c_mtx;		// 시간에 대한...
	CMutex		c_mty;		// 공간에 대한...
	CMutex		c_mtz;		// film...

	DTSTIME		c_dtsTime;
	DEVTIME		c_devTime;
	bool		c_bLogArea;
	bool		c_bChangeEnvirons;
	WORD		c_wFutile;
	bool		c_bDelayPrevious;
	WORD		c_wLastPeriodCmd;
	WORD		c_wClosureInsideSeq;	// 171206

	// 171101
#define	TPERIOD_WARMUP		REAL2PERIOD(5000)
	WORD		c_wWarmupTimer;			// 이시간 동안은 고장 표시를 하지않는다. 171226-시간을 다시 생각해 볼 것

	typedef struct _MMMONOTOR {
		bool	bUsbScan;
		WORD	wUsbScanTime;
		bool	bCmp;
		WORD	wState;
		WORD	wSeq;
		WORD	wDeb;
		WORD	wSkip;
	} MMMONITOR;
	MMMONITOR	c_mmm;
#define	MMMONITOR_USBSCANTIME	10			// 1sec
#define	MMMONITOR_DEBOUNCE		10
#define	MMMONITOR_DEBOUNCE2		100

	WORD		c_wShotCnt;
	WORD		c_wCutCnt;
	WORD		c_wAbacus[MAX_TROUBLE];		// c_wAbacus[1234] : 고장 코드(ex:1234)가 발생한 객차 번호 비트
											// b9:9번 객차 ~ b0:0번 객차
	//TROUBLEREF	c_troubleRef;
	TROUBLETRAY	c_troubleTray[TTRAY_MAX];

#define	SIZE_FILM		4096
	typedef struct _tagFILM {
		WORD	ri;
		WORD	wi;
		WORD	stack;
		TROUBLEINFO	cellar[SIZE_FILM];
	} FILM, *PFILM;
	FILM		c_film[TFILMID_MAX];		// 2

	// 181001
	typedef struct _tagRTDTROUBLECHECK {
		DWORD	dwReserve;
		DWORD	dwSend;
		DWORD	dwConfirm;
		DWORD	dwSection;
	} RTDTROUBLECHECK;
	RTDTROUBLECHECK	c_rtdtc;

#define	SIZE_RECORD		14400				// 2 hour
	typedef struct _tagLRECORD {
		WORD	wSeq;
		WORD	ri;
		WORD	wi;
		WORD	stack;
		LOGBOOK	cur;
		LOGBOOK	cellar[SIZE_RECORD];
	} LRECORD;
	LRECORD		c_rec;
	SPARKLE		c_fspk[SPARKLE_MAX][2];		// 짧은 펄스라도 램프에 표시되도록 시간 지연을 한다.
	DWORD		c_dwMakeLogbookTime;

	bool		c_bInsprInside;
	bool		c_bInsprBridge;
	INSPREPORT	c_inspr;

	// 171101
#define	DEB_SDBITFAIL		3
#define	DEB_SIVSDBITFAIL	10				// SIV SD FRAME에서 발생되는 trouble debounce
#define	DEB_CMSBSDBITFAIL	5

	// 190523
	typedef struct _tagSDAFAILDEB {
		WORD	wDeb[CID_MAX][MAX_TROUBLE];
	} SDAFAILDEB;
	SDAFAILDEB	c_sdafd;

	BYTE		c_cFullSdaFlow;
	bool		c_bFstrBridge;

#define	CYCLE_FULLSDASTACK		10
	BYTE		c_cFullSdaStack;
	enum enFSTID {
		FSTID_WRITE,
		FSTID_REPORT,
		FSTID_MAX
	};
	FSDATONE	c_fst[FSTID_MAX];

	CArchAux	c_auxi;

	WORD		c_wOldtMap;
	TROUBLEBYCAR	c_tbc[SIZE_TROUBLELIST];

	static const BYTE c_cTroubleTypes1[1025];
	static const BYTE c_cTroubleTypes2[1025];

	void	InitialTable();
	int		TakeOrderBit(WORD wbf);
	WORD	BitfFromCID(int iCID);
	void	SortAdd(int iCID, WORD wCode);
	void	AddEach(int iCID, WORD wCode);
	//void	Add(int iCID, WORD wCode);
	//void	Revoke(int iCID, WORD wCode);
	void	SortRevoke(int iCID, WORD wCode);
	void	RevokeEach(int iCID, WORD wCode);	// 180522
	void	SetStillOnRecord();
	bool	GetStillOnRecord(PLOGBOOK pLogbook);
	void	SetStillOnFilm(PTROUBLEINFO pTroub);
	void	SetStillOnFilm(PTROUBLEINFO pTroub, int to);
	void	BuildLogbook();
	//190523
	void	BuildFullSda();
	void	BuildAvsSda(int cid);
	void	BuildSivSda(int cid);
	void	BuildV3fSda(int cid);
	void	BuildUnivSda(int cid);

	bool	SetSysTime(struct tm st, bool byRtd);

public:
	void	Initial(int iFrom);
	void	AddRtdEdge();
	//void	Hide(int iCID, WORD wCode);
	void	SortHide(int iCID, WORD wCode);
	void	Shot(int iCID, WORD wCode);
	void	Cut(int iCID, WORD wBegin, WORD wEnd = 0, WORD wFlesh = 0);
	void	Closeup(int iCID, WORD wShot, WORD wCutBegin, WORD wCutEnd);
	void	Enumerate(int iCID, WORD wOccur, WORD wErase = 0);
	void	Conceal(int iCID, WORD wErase);
	void	ConcealAtoBcf();
	BYTE	GetTroubleType(WORD wCode);		// 210803						{ return c_cTroubleTypes[wCode]; }
	void	Prompt(bool bSign, int iCID, WORD wCode);
	PTROUBLEBYCAR	GetTroubleListForEachCar(WORD wTcid);
	void	ShiftCount(WORD* pwShot, WORD* pwCut);
	bool	GetStillOnFilm(PTROUBLEINFO pTroub, int from);
	bool	GetSparkle(int id, bool bCh);
	TROUBLECODE	GetRecentTrouble();
	void	ReformT(PRECIP pRecip);
	void	ReformC(PRECIP pRecip);
	void	Arteriam();
	bool	SetSysTimeByBcdDts(DTSTIME dtst);
	void	GetSysTimeToBcdDts(PDTSTIME pDtst);
	void	GetSysTimeToDbf(PDBFTIME pDbft);
	bool	SetSysTimeByDbf(DBFTIME dbft, bool byRtd);
	void	Devt2Dbft(PDBFTIME pDbft, DEVTIME devt);
	void	Dbft2Devt(PDEVTIME pDevt, DBFTIME dbft);
	void	Dbft2BcdDevt(PDEVTIME pDevt, DBFTIME dbft);
	bool	CheckDbft(DBFTIME dbft);
	bool	CheckDbft(DBFTIME dbfta, DBFTIME dbftb);
	bool	Evacuate();
	bool	ClosureInside(bool bLogbook);
	void	ClosureInside();
	void	ClosureBridge(PRTDBINFO pRtdr);
	void	PreInspect(WORD wItem);
	void	SuffInspect(WORD wRequired);
	void	SdaPrompt(bool bSign, int iCID, WORD wCode, WORD wDeb = DEB_SDBITFAIL);
	int		Bale(bool bType, BYTE* pMesh);

	void	SetWarmup(WORD wTime)							{ c_wWarmupTimer = wTime; }
	void	SetWarmup()										{ c_wWarmupTimer = TPERIOD_WARMUP; }
	PINSPREPORT	GetInspectReport()							{ return &c_inspr; }
	void	UsbScaning(bool bScan);
	bool	GetUsbState();
	bool	GetUsbScanState()								{ return c_mmm.bUsbScan; }
	DWORD	GetSevereLength()								{ return c_troubleTray[TTRAY_TROUBLE].dwHeavy; }

	int64_t	GetFileSize(const char* pPath)					{ return c_auxi.GetFileSize(pPath); }
	bool	CullRequiredDoc()								{ return c_auxi.CullRequiredDoc(); }
	bool	CloseCurrent();
	bool	NewSheet();
	void	ReserveSaveEnvirons()							{ c_bChangeEnvirons = true; }
	void	SetRtdNewSheetState(bool bStat)					{ c_auxi.SetRtdNewSheetState(bStat); }
	bool	GetRtdNewSheetState()							{ return c_auxi.GetRtdNewSheetState(); }
	void	RtdTroubleSuccess()								{ c_rtdtc.dwConfirm += c_rtdtc.dwSection; }

	void	ClearTraceSphere(int nDevID)					{ c_auxi.ClearTraceSphere(nDevID); }
	void	SetTraceLengthSphere(int nDevID, int nCID, WORD wLeng)	{ c_auxi.SetTraceLengthSphere(nDevID, nCID, wLeng); }
	void	LocateTraceSphere(int nDevID, int nCID, WORD wIndex, WORD wBlockID, BYTE* p)	{ c_auxi.LocateTraceSphere(nDevID, nCID, wIndex, wBlockID, p); }
	void	ResultTraceSphere(int nDevID)					{ c_auxi.ResultTraceSphere(nDevID); }
	void	SaveTraceSphere(int nDevID)						{ c_auxi.SaveTraceSphere(nDevID); }
	void	ClearEcuTrace()									{ c_auxi.ClearEcuTrace(); }
	int		GetEcuTraceStep()								{ return c_auxi.GetEcuTraceStep(); }
	void	SetEcuTraceStep(int iStep)						{ c_auxi.SetEcuTraceStep(iStep); }
	WORD	GetEcuTraceLength()								{ return c_auxi.GetEcuTraceLength(); }
	void	ArrangeEcuTrace()								{ c_auxi.ArrangeEcuTrace(); }
	void	LoadEcuTraceFromTemporary()						{ c_auxi.LoadEcuTraceFromTemporary(); }
	bool	TakeoutEcuTrace(WORD wIndex, WORD wBlockID, BYTE* pBuf)		{ return c_auxi.TakeoutEcuTrace(wIndex, wBlockID, pBuf); }
	int		SaveEcuTrace(BYTE* p)							{ return c_auxi.SaveEcuTrace(p); }
	void	EraseEcuTrace()									{ c_auxi.EraseEcuTrace(); }
	int		SaveSivDetect(BYTE* p)							{ return c_auxi.SaveSivDetect(p); }
	int		SaveSivDetect2(BYTE* p)							{ return c_auxi.SaveSivDetect2(p); }

	bool	MirrorEnvirons(BYTE* pSrc)						{ return c_auxi.MirrorEnvirons(pSrc); }
	void	AccordEnvirons(int eid)							{ c_auxi.AccordEnvirons(eid); }
	void	SaveEnvirons()									{ c_auxi.SaveEnvirons(); }
	int		CheckEnvirons()									{ return c_auxi.CheckEnvirons(); }
	PENVIRONS	GetEnvirons()								{ return c_auxi.GetEnvirons(); }
	bool	MirrorCareers(BYTE* pSrc)						{ return c_auxi.MirrorCareers(pSrc); }
	void	AccordCareers(int rid)							{ c_auxi.AccordCareers(rid); }
	void	AddCareers(int nCrri, int di, double db)		{ c_auxi.AddCareers(nCrri, di, db); }
	double	GetCareers(int nCrri, int di)					{ return c_auxi.GetCareers(nCrri, di); }
	PCAREERS	GetCareers()								{ return c_auxi.GetCareers(); }

	bool	MirrorCmjog(BYTE* pSrc)							{ return c_auxi.MirrorCmjog(pSrc); }
	//void	AccordCmjog()									{ c_auxi.AccordCmjog(); }
	PCMJOGB	GetCmjog()										{ return c_auxi.GetCmjog(); }
	void	ClearSegment()									{ c_auxi.ClearSegment(); }
	bool	MirrorV3fBreaker(PV3FBRKCNTU pVbc)				{ return c_auxi.MirrorV3fBreaker(pVbc); }
	void	ClearV3fBreaker()								{ c_auxi.ClearV3fBreaker(); }
	PV3FBRKCNTF	GetV3fBreaker()								{ return c_auxi.GetV3fBreaker(); }
	void	AddSegment(double dbDist)						{ c_auxi.AddSegment(dbDist); }
	double	GetSegment()									{ return c_auxi.GetSegment(); }
	double	GetPrevDistance()								{ return c_auxi.GetPrevDistance(); }
	void	SetPrevDistance(double dbDist)					{ c_auxi.SetPrevDistance(dbDist); }

	bool	OpenAid()										{ return c_auxi.OpenAid(); }
	void	AidCmd(WORD wCmd)								{ c_auxi.AidCmd(wCmd); }
	bool	AidCmdOccasion(WORD cmd, DWORD dwIndex, DWORD dwTotal)	{ return c_auxi.AidCmdOccasion(cmd, dwIndex, dwTotal); }
	bool	AidCmdPeriod(WORD cmd, BYTE* p, WORD wLength)			{ return c_auxi.AidCmdPeriod(cmd, p, wLength); }
	bool	GetTimeSetStatus()								{ return c_auxi.GetTimeSetStatus(); }
	PBULKPACK	GetBulk()									{ return c_auxi.GetBulk(); }

	void	DownloadCancel()								{ c_auxi.AidCmd(AIDCMD_DOWNLOADCANCEL); }
	void	GetLogbookEntry(DWORD dwIndex, DWORD dwTotal)	{ c_auxi.GetLogbookEntry(dwIndex, dwTotal); }
	void	GetInspectEntry(DWORD dwIndex, DWORD dwTotal)	{ c_auxi.GetInspectEntry(dwIndex, dwTotal); }
	void	GetLogbookList(char* pszLogbook, DWORD dwIndex, DWORD dwTotal)		{ c_auxi.GetLogbookList(pszLogbook, dwIndex, dwTotal); }
	void	CopyLogbook(char* pszLogbook)					{ c_auxi.CopyLogbook(pszLogbook); }
	void	GetLogbook(char* pszLogbook = NULL)				{ c_auxi.GetLogbook(pszLogbook); }
	void	GetInspect(char* pszInspect = NULL)				{ c_auxi.GetInspect(pszInspect); }
	void	PutLogbook(PRTDSPADE pSpd)						{ c_auxi.PutLogbook(pSpd); }
	void	PutInspect(PRTDSPADE pSpd)						{ c_auxi.PutInspect(pSpd); }
	void	MakeDestination(char* pszIndex, bool bLog)		{ c_auxi.MakeDestination(pszIndex, bLog); }
	WORD	GetMmVersion()									{ return c_auxi.GetMmVersion(); }

	PLOGBOOK	GetCurrentLogbook()							{ return &c_rec.cur; }
	PTROUBLETRAY	GetTroubleTray(int ttype)				{ return &c_troubleTray[ttype]; }
	PDTSTIME	GetDtsTime()								{ return &c_dtsTime; }
	PDEVTIME	GetDevTime()								{ return &c_devTime; }
	PVOID		GetCarrier()								{ return c_pCarrier; }
	void		SetCarrier(PVOID pCarrier)					{ c_pCarrier = pCarrier; }
	static CArch*	GetInstance()							{ static CArch arch; return &arch; }

	PUBLICY_CONTAINER();
};
