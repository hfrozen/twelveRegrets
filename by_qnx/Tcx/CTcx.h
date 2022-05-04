/* CTcx.h
 *
 *  Created on: 2010. 12. 9
 *      Author: Che
 */

#ifndef CTCX_H_
#define CTCX_H_

#include <sys/syspage.h>
#include <sys/time.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <Draft.h>
#include <CAnt.h>
#include <CTimer.h>
#include <Msg.h>
#include <CProbe.h>
#include "CPanelTc.h"
#include "CLocal.h"
#include "CBusTrx.h"
#include "CScene.h"
#include "CSpy.h"

#include "CPath.h"
#include "CLook.h"
#include "CPack.h"
#include "CArtop.h"
#include "CPanto.h"
#include "CExt.h"
#include "CDoor.h"
#include "CDuDoc.h"
#include "CInsp.h"
#include "CChaser.h"
#include "CMate.h"

#define	VERSION_TCX		2.63
#define	REBUILD_TCX		131102

class CTcx	: public CAnt
{
public:
	CTcx();
	virtual ~CTcx();

	TCREFERENCE	c_tcRef;
	DUMBDEV		c_dumb;

protected:
	LCFIRM		c_lcFirm;
	TCDOZ		c_dozMain;
	TCDOZ		c_dozAux;
	TCDOZ		c_dozBkup;
	DUCINFO		c_ducInfo;
	TCPERT		c_pertSend;
	TCPERT		c_pertTake;

	PTCREFERENCE	c_pTcRef;
	PLCFIRM		c_pLcFirm;
	PTCDOZ		c_pDoz;

private:
	int			c_mail;
	BOOL		c_bMfb;
	pthread_mutex_t	c_mutex;
	pthread_mutex_t	c_mutexSig;
	pthread_mutex_t	c_mutexArm;

	TBTIME		c_tbTime;
	DEVTIME		c_devTime;
	ENVARCHEXTENSION	c_env;

	CTimer*		c_pTimer;
	CPanelTc	c_panel;
	CProbe		c_probe;

	typedef struct _tagBUSARCH {
		WORD		wState;		// not used, change to c_tcRef.real.cycle.nBusState[]
#define	BUSSTATE_ENABLE		1
#define	BUSSTATE_WAIT		2

		CBusTrx*	pLine;
	} BUSARCH;
	BUSARCH		c_busA;
	BUSARCH		c_busB;
public:
	CLocal*		c_pLocal;

private:
	CPath		c_path;
	CLook		c_look;
	CPack		c_pack;
	CArtop		c_artop;
	CPanto		c_panto;
	CExt		c_ext;
	CDoor		c_door;
	CInsp		c_insp;
	CChaser		c_chaser;
	CDuDoc		c_doc;
	CScene		c_scene;
	CSpy		c_spy;
	CMate		c_mate;

	PBUCKET		c_pBucket;
	int			c_hShm;
	int			c_iWatch;
	DWORD		c_dwRecvOrderID;
	BOOL		c_bLocalSync;
	BYTE		c_nLocalTimer;
	BYTE		c_nHoldSec;
	BYTE		c_nUsbScanCounter;
#define	COUNT_USBSCAN	10

	typedef struct _tagDINPARCH {
		WORD	v;
		WORD	wCur;
		BYTE	nDeb;
#define	DEBOUNCE_INP	3
		BYTE	nLeng;
	} DINPARCH;
	DINPARCH		c_din[SIZE_DIN];

	typedef struct _tagAINPARCH {
		DWORD	dwSum;
		WORD	v;
		WORD	wLeng;
	} AINPARCH;
	AINPARCH	c_ain[SIZE_AIN];

	typedef struct _tagDISTANCEFAIR {
		WORD	cycle;
		DWORD	pulse;
		double	speed;
	} DISTANCEFAIR;
	DISTANCEFAIR	c_df;

	TRIALINFO	c_trial;
	_TV_		c_vV3fTbe;
	long		c_syncCur, c_syncMax, c_syncMin;
	uint64_t	c_busTime;
	uint64_t	c_beilTriggerTime;
	uint64_t	c_drvTriggerTime;
	uint64_t	c_oppoDirTriggerTime;

	DWORD		c_dwThumbWheel;
	OPERATETIME	c_opTimes[TTIME_MAXSHEET];
#define	SIZE_ATCSPEEDMAX	20
	BYTE		c_nAccpes[SIZE_ATCSPEEDMAX];
#define	SIZE_EACHDEV		24
	BYTE		c_nEachDevError[SIZE_EACHDEV];

	DWORD		c_dwUsbSize;
	DWORD		c_dwUsbFree;
	BYTE		c_nSelectedCid;
	BYTE		c_nDownloadPulse;
	BYTE		c_armTimeText[256];

#define	FLASH_MAX	TCDOB_DBZA
	WORD	c_wFlashT[FLASH_MAX];
	WORD	c_wFlashO[CCDOB_MAX];
#define	TIMETABLES_MAX	128
	WORD*	c_pTimeTables[TIMETABLES_MAX];

	static const WORD	c_wTrainCompose[3][12];
	static const double	c_dbAspBase[];
	static const BYTE	c_nAtcSample[2][ATCSPEED_UNITMAX];	// thing about this
	static const BYTE	c_nDuPages[4][5];
	static const BYTE	c_nJointOut[8];
#define	SIZE_LAMP	2
	static const WORD	c_wLamps[SIZE_LAMP];

protected:
	void	MaAccept(DWORD dwParam);
	void	MaSendBus(DWORD dwParam);
	void	MaReceiveBus(DWORD dwParam);
	void	MaReceivePert(DWORD dwParam);
	void	MaReceiveFault(DWORD dwParam);
	void	MaReceiveLocal(DWORD dwParam);
	void	MaReceiveDuc(DWORD dwParam);
	DECLARE_MAILMAP();

	static void	Interval(PVOID pVoid);

protected:
	BOOL	TakingOff();
	BOOL	Seed();
	BOOL	Trigger();
	BOOL	OpenPost();
	BOOL	OpenBeil();

	void	Opinion(BYTE id);
	void	OutputD();
	BOOL	CheckBusState();
	BOOL	IsValidFirm(UCURV id);
	BOOL	IsValidTcFirm(UCURV ti);
	BOOL	IsValidCcFirm(UCURV ci);
	void	ScanBucketStep();
	void	SearchBroadcastAddr(BYTE* pBca, BYTE sa);
	void	CalcAtcSpeed();
	void	ControlByMainDev();
	void	LocalErrorToBus();
	void	TipPartner(BYTE addr, BOOL stat);

	void	FormatDuc();
	//void	LatterDuc();
	//void	SetTimeFromDuc(BYTE* p);
public:
	void	TimeSet(WORD year, BYTE mon, BYTE day, BYTE hour, BYTE min);
	void	OpenAlarm(WORD index);
	void	MakeCurAlarmList();
	void	OpenAlarmList(WORD index);
	void	OpenAlarmPageToDu();
	void	DirectoryList();
protected:
	void	BeilTimeRecorder(const char* pMsg);
	void	ScanUsb();
	void	SetTimeByTb();
	void	DisjointTime(DWORD day, DWORD sec);

	void	ReadProbe();
	void	CalcTimes();
	double	TimeToSec(PDEVTIME pDt);
	void	LongToByte(BYTE* p, long v);
	int		PeriodicSnap();

	void	ReportBus()	{ c_spy.ReportBus(&c_tcRef, c_pLcFirm, &c_dozMain, &c_ducInfo.srDuc); }

public:
	void	SysTimer();
	UCURV	BusSetup();
	BOOL	ReceiveFrameZero(BOOL bCh);
	void	ReceiveAll();
	BOOL	GetBusCondition(UCURV id);
	BOOL	GetRealBusCondition(UCURV id);
	void	BusTrace(UCURV id, BOOL line);
	void	ClearBusTrace();
	BOOL	GetLocalCondition(UCURV uLocalID);
	PATCAINFO	GetActiveAtc(BOOL real);
	PATOAINFO	GetAto(BOOL real, BOOL type = TRUE);
	UCURV	CheckSivReply(UCURV nCcid, PSIVEINFO peSiv, BOOL log = TRUE);
	UCURV	CheckV3fReply(UCURV nCcid, PV3FEINFO peV3f, BOOL log = TRUE);
	UCURV	CheckEcuReply(UCURV nCcid, PECUAINFO paEcu, BOOL log = TRUE);
	UCURV	CheckCmsbReply(UCURV nCcid, PCMSBAINFO paCmsb, BOOL log = TRUE);
	BOOL	CheckDcuReply(PDCUAINFO paDcu, BYTE id, BYTE cid);
	BOOL	CheckSivAddr(PSIVEINFO peSiv);
	BOOL	CheckV3fAddr(PV3FEINFO peV3f);
	BOOL	CheckEcuAddr(PECUAINFO paEcu);
	BOOL	CheckCmsbAddr(PCMSBAINFO paCmsb);
	BOOL	CheckDcuAddr(PDCUAINFO paDcu, BYTE id, BYTE cid);

	void	ResetDuc();
	BYTE	GetPscIDByBus(UCURV id, BYTE head);
	SHORT	GetPscIDByAddr(UCURV id);
	WORD	GetCarDevices(WORD carID);
	BYTE	GetV3fQuantity();
	BYTE	GetEcuQuantity();
	BYTE	GetTrainDir(BOOL bSide);
	double	GetRealAsp(UCURV cid);
	double	GetRealAcp(UCURV cid);
	double	GetRealBcp(UCURV cid);
	double	GetPassengerWeightRate(UCURV cid);
	BYTE	GetHeadCarID();
	void	Powering(BOOL bP, BOOL bB, BYTE nDir, _TV_ vTbeB, BOOL bMask248 = FALSE);
	void	PoweringMask();
	SHORT	TbeToPercent(_TV_ v);
	void	RecoverAllBrake();
	void	LcdCtrl(BOOL side, BOOL cmd);
	void	AnnalsMrp(BOOL state, BYTE nCid, WORD wMsg);
	void	Annals(BOOL state, BYTE nCid, WORD wMsg);
	void	ShotMrp(BYTE nCid, WORD wMsg);
	void	ShotMainDev(BYTE nCid, WORD wMsg);
	void	ShotAidDev(BYTE nCid, WORD wMsg);
	void	ScanNuetralAtDriverless(BOOL bStop);
	void	EntryDriverless(BOOL mode);
	// Modified 2012/12/10
	void	Primary();
	// Appended 2013/11/02
	void	PrintEnv(UCURV from, BOOL rw);
	// Modified 2013/03/25
	void	RemoveRescueOutput();
	//void	PanelActive();
	void	ClearDistance();
	void	ObsCtrl(BOOL bState, UCURV uID, WORD wCarMap);
	void	ToFlash(WORD to, BOOL cmd);
	void	CoFlash(WORD co, BOOL cmd);

	void	SaveArchives();
	void	SaveArchives(DWORD dwArmID);
	SHORT	SaveTrace(UCURV di, UCURV ci, BYTE* pTrace, DWORD dwLength);
	// Modified 2013/11/02
	//SHORT	SaveEnv();
	SHORT	SaveEnv(UCURV from);
	void	LoadEnv();
	void	CampUsb();
	void	BeilCmd(WORD cmd);
	WORD	GetBucketProgress();
	void	SaveDrv(PDRV_ARCH pDrv);
	DWORD	PickAlarm();
	void	SectQuantity(DWORD dwDay);
	void	ReadSect(BYTE nDirectoryID, BYTE nFileID, DWORD dwOffset);
	void	WriteSect(BYTE nDirectoryID, BYTE nFileID, DWORD dwOffset);
	void	CopyEachDevError(BYTE* p);

	void	PutDistanceFair(WORD pulse, double sp);
	void	GetDistanceFair(DWORD* pPulse, double* pSp);
	BOOL	TimeRegister(WORD* pTimer);
	void	InterTrigger(WORD* pTimer, WORD t);	// ????? rename
	void	GetTbTime(PTBTIME pTt);
	void	RecordTime(UCURV n);
	int		ConvS(WORD psc, int pscMin, int pscMax, int logMin, int logMax);
	WORD	Conv(WORD wPsc, WORD wPscMin, WORD wPscMax, WORD wLogMin, WORD wLogMax);
	double	Conv(WORD wPsc, WORD wPscMin, WORD wPscMax, double min, double max);
	DWORD	ToBcd(DWORD dw);
	WORD	ToBcd(WORD w);
	BYTE	ToBcd(BYTE n);
	WORD	ToBcd(BYTE* p, UCURV length);
	WORD	ToHex(WORD bcd);
	BYTE	ToHex(BYTE bcd);
	DWORD	DevideBy10(DWORD dw);
	double	DevideBy10(double db);
	BYTE	GetBitQuantity(WORD w, BOOL bState);
	void	ArmNameParse(WORD* pYearSec, WORD* pDate, WORD* pTime);
	BOOL	SendPert(BYTE cmd);
	void	HalfwayNeutral(BOOL cmd);

	BOOL	Run(int aich, int bkdis, int sdis);
	void	Destroy();

	CArtop*		GetArtop()				{ return &c_artop; }
	CLocal*		GetLocal()				{ return c_pLocal; }
	CScene*		GetScene()				{ return &c_scene; }
	CDoor*		GetDoor()				{ return &c_door; }
	CInsp*		GetInsp()				{ return &c_insp; }
	CChaser*	GetChaser()				{ return &c_chaser; }

	PTCREFERENCE	GetTcRef()			{ return &c_tcRef; }
	PLCFIRM		GetLcFirm()				{ return &c_lcFirm; }
	PDUCINFO	GetDucInfo()			{ return &c_ducInfo; }
	PTCDOZ		GetTcDoz()				{ return &c_dozMain; }
	PBUCKET		GetBucket()				{ return c_pBucket; }
	PTCPERT		GetSendPert()			{ return &c_pertSend; }
	PTCPERT		GetTakePert()			{ return &c_pertTake; }

	PENVARCHEXTENSION	GetEnv()		{ return &c_env; }
	PTBTIME		GetTbt()				{ return &c_tbTime; }
	PDEVTIME	GetDevt()				{ return &c_devTime; }
	PTRIALINFO	GetTrial()				{ return &c_trial; }

	void	Lock()						{ pthread_mutex_lock(&c_mutex); }
	void	Unlock()					{ pthread_mutex_unlock(&c_mutex); }
	void	LockSig()					{ pthread_mutex_lock(&c_mutexSig); }
	void	UnlockSig()					{ pthread_mutex_unlock(&c_mutexSig); }
	void	LockArm()					{ pthread_mutex_lock(&c_mutexArm); }
	void	UnlockArm()					{ pthread_mutex_unlock(&c_mutexArm); }
	void	LocalSync()					{ c_bLocalSync = TRUE; }
	WORD	GetBusState()				{ return c_tcRef.real.bm.wState; }
	void	ClearDumb()					{ memset((PVOID)&c_dumb, 0, sizeof(DUMBDEV)); }
	void	CountDumb(WORD* pV)			{ INCWORD(*pV); }
	PDUMBDEV	GetDumb()				{ return &c_dumb; }
	void	CountEachDevError(BYTE id)	{ INCBYTE(c_nEachDevError[id]); }
	DWORD	GetThumbWheel()				{ return c_dwThumbWheel; }
	_TV_	GetV3fTbe()					{ return c_vV3fTbe; }
	void	SetV3fTbe(_TV_ tbe)			{ c_vV3fTbe = tbe; }
	void	SetSelectedCid(BYTE n)		{ c_nSelectedCid = n; }
	BYTE	GetSelectedCid()			{ return c_nSelectedCid; }
	void	BeilTimeTrigger()			{ c_beilTriggerTime = ClockCycles(); }
	void	OppoDirTimeTrigger()		{ c_oppoDirTriggerTime = ClockCycles(); }
	//void	VestigesRecOwn()			{ INCBYTE(c_tcRef.real.nBusFaultCounter[1]); }
	//void	VestigesHcrInfoR()			{ INCBYTE(c_tcRef.real.nBusFaultCounter[2]); }
	//void	VestigesHcrInfoA(PVOID pV)	{ INCBYTE(c_tcRef.real.nBusFaultCounter[3]); memcpy((PVOID)&c_tcRef.dummy[0], pV, 8); }
	//void	VestigesSend(PVOID pV)		{ memcpy((PVOID)&c_tcRef.dummy[10], pV, 8); }
	//void	VestigesPertR()				{ ++ c_dozMain.tcs[OWN_SPACE].dummy[11]; }	// ++ c_tcRef.dummy[1]; }
	//void	VestigesPertN()				{ ++ c_dozMain.tcs[OWN_SPACE].dummy[12]; }	// ++ c_tcRef.dummy[2]; }
	//void	VestigesPertF()				{ ++ c_dozMain.tcs[OWN_SPACE].dummy[13]; }	// ++ c_tcRef.dummy[3]; }
	//void	VestigesPertA()				{ ++ c_dozMain.tcs[OWN_SPACE].dummy[14]; }	// ++ c_tcRef.dummy[4]; }

	// Inserted 2012/01/17
	UCURV	CheckAtcReply(PATCAINFO paAtc)	{ return c_pLocal->CheckAtcReply(paAtc); }
	UCURV	CheckAtoReply(PATOAINFO paAto)	{ return c_pLocal->CheckAtoReply(paAto); }
	//UCURV	CheckCscReply(PCSCAINFO paCsc)	{ return c_pLocal->CheckCscReply(paCsc); }
	//UCURV	CheckPisReply(PPISAINFO paPis)	{ return c_pLocal->CheckPisReply(paPis); }
	//UCURV	CheckTrsReply(PTRSAINFO paTrs)	{ return c_pLocal->CheckTrsReply(paTrs); }
	//UCURV	CheckHtcReply(PHTCAINFO paHtc)	{ return c_pLocal->CheckHtcReply(paHtc); }
	//UCURV	CheckPsdReply(PPSDAINFO paPsd)	{ return c_pLocal->CheckPsdReply(paPsd); }

	BOOL	GetEpcor()					{ return c_artop.GetEpcor(); }
	WORD	ConvPwb()					{ return c_artop.ConvPwb(); }
	// Modified 2012/03/05 ... begin
	// void	GetMascon(PINSTRUCTA pInst, BYTE nIndex)	{ c_look.GetMascon(pInst, nIndex); }
	// Modified 2012/11/29
	void	AbsoluteEmergency()			{ c_look.AbsoluteEmergency(); }
	WORD	GetNotchRescue(BYTE n)		{ return c_look.GetNotchRescue(n); }
	void	AuxCompReset()				{ c_panto.AuxCompReset(); }
	void	PrimaryPanto()				{ c_panto.Primary(); }
	WORD	XchAtZero(BYTE nCid, WORD wMsg)	{ return c_door.XchAtZero(nCid, wMsg); }
	void	ShutDoor()					{ c_door.Shut(); }
	BOOL	GetPartner()				{ return c_panel.GetPartner(); }
	char*	TcNamePlate(char* p)		{ return c_panel.TcNamePlate(p); }
	char*	CcNamePlate(char* p)		{ return c_panel.CcNamePlate(p); }

	WORD	GetInspectPage()			{ return c_insp.GetPage(); }
	PDUINSPECTINFO	GetInspectInfo()	{ return c_insp.GetInfo(); }
	PINSPECTSPEC	GetInspectSpec()	{ return c_insp.GetSpec(); }

	PSIVCHASE	GetSivChase(UCURV cid)	{ return c_chaser.GetSivChase(cid); }
	PV3FCHASE	GetV3fChase(UCURV cid)	{ return c_chaser.GetV3fChase(cid); }
	PECUCHASE	GetEcuChase(UCURV cid)	{ return c_chaser.GetEcuChase(cid); }

	void	Buzz(WORD wTime)								{ c_scene.Buzz(wTime); }
	void	ShutBuzz()										{ c_scene.ShutBuzz(); }
	void	Hide()											{ c_scene.Hide(); }
	PARM_ARCH	GetFilm()									{ return c_scene.GetFilm(); }
	int		LoadSect(PVOID pVoid)							{ return c_scene.LoadSect(pVoid); }
	int		SaveSect(PVOID pVoid, WORD wLength)				{ return c_scene.SaveSect(pVoid, wLength); }
	WORD	GetFilesByDayLength()							{ return c_scene.GetFilesByDayLength(); }
	DWORD	GetSelectedByDay(WORD sel)						{ return c_scene.GetSelectedByDay(sel); }
	DWORD*	GetFilesByDay()									{ return c_scene.GetFilesByDay(); }
	void	SortUnderDay(DWORD day)							{ c_scene.SortUnderDay(day); }
	void	SetFilesByDay(WORD length, DWORD* pDwDays)		{ c_scene.SetFilesByDay(length, pDwDays); }
	WORD	GetFilesUnderDayLength()						{ return c_scene.GetFilesUnderDayLength(); }
	DWORD	GetSelectedUnderDay(WORD sel)					{ return c_scene.GetSelectedUnderDay(sel); }
	DWORD*	GetFilesUnderDay()								{ return c_scene.GetFilesUnderDay(); }
	void	SetFilesUnderDay(WORD length, DWORD* pDwDays)	{ c_scene.SetFilesUnderDay(length, pDwDays); }

	void	Shot(BYTE nCid, WORD wMsg)	{ c_scene.Shot(nCid, wMsg, &c_devTime); }
	void	Cut(BYTE nCid, WORD wBegin, WORD wEnd)	{ c_scene.Cut(nCid, wBegin, wEnd, &c_devTime); }
	void	Edit(BYTE nCid, WORD wBegin, WORD wEnd, WORD wRest)
					{ c_scene.Edit(nCid, wBegin, wEnd, wRest, &c_devTime); }
	BOOL	CheckShot(BYTE nCid, WORD wMsg)	{ return c_scene.CheckShot(nCid, wMsg); }
	WORD	GetSignCode()				{ return c_scene.GetSignCode(); }
	WORD	GetAlarmCode()				{ return c_scene.GetAlarmCode(); }
	// Modified 2013/11/02
	DWORD	GetSevereLength()			{ return c_scene.GetSevereLength(); }
	DWORD	GetAlarmLength()			{ return c_scene.GetAlarmLength(); }
	DWORD	GetBellLength()				{ return c_scene.GetBellLength(); }

	void	Hide(BYTE nCid, WORD wMsg)	{ c_scene.Hide(nCid, wMsg); }
	WORD	GetSevereCode()				{ return c_scene.GetSevereCode(); }
	WORD	GetActiveAlarmCode(DWORD dwIndex)	{ return c_scene.GetActiveAlarmCode(dwIndex); }
	WORD	GetActiveAlarmCode()		{ return c_scene.GetActiveAlarmCode(); }
	PARM_ARCH	GetAlarmList()			{ return c_scene.GetAlarmList(); }

	BYTE	GetDoorBypass(UCURV id)		{ return c_doc.GetDoorBypass(id); }
	PVOID	GetDucSendInfo()			{ return c_doc.GetSendInfo(); }
	WORD	GetDucSendLength()			{ return c_doc.GetSendLength(c_tcRef.real.du.nCurPage); }
	WORD	GetDucRecvLength()			{ return c_doc.GetRecvLength(c_tcRef.real.du.nCurPage); }
	void	OsMsg(char* p)				{ c_doc.OsMsg((BYTE*)p); }
	WORD	Hangdown()					{ return c_doc.Hangdown(); }
	void	Hangup(WORD msg)			{ c_doc.Hangup(msg); }
	void	SetTraceLength(UCURV di, UCURV ci, WORD length)	{ c_doc.SetTraceLength(di, ci, length); }
	WORD	GetTraceLength(UCURV di, UCURV ci)		{ return c_doc.GetTraceLength(di, ci); }
	void	SetTraceProgress(UCURV di, UCURV ci, WORD progress)	{ c_doc.SetTraceProgress(di, ci, progress); }

	PTRACEARCH	GetTraceArch(UCURV di, UCURV ci)	{ return c_chaser.GetTraceArch(di, ci); }
	void	TraceBegin(UCURV di, UCURV ci)			{ c_chaser.TraceBegin(di, ci); }

	//void	FlagDoSend(BOOL bCh)	{ c_dozMain.tcs[OWN_SPACE].dummy[0] |= bCh ? 4 : 0x40; }
	void	HcrInfo(BYTE nMasterID, BYTE nHcrInfo, WORD at);
};

#endif /* CTCX_H_ */
