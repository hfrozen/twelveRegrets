/*
 * CLand.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Draft.h"
#include "Infer.h"

#include "CPump.h"
#include "../Component2/CTimer.h"
#include "../Component2/CFio.h"
#include "../Component2/CDtb.h"
#include "../Component2/CLocal.h"
#include "../Component2/CPole.h"
#include "../Component2/CWicket.h"
#include "../Component2/CPop.h"

#include "CTurf.h"
#include "CStick.h"
#include "CSteer.h"
#include "CPel.h"
#include "CStage.h"
#include "CLide.h"
#include "CUniv.h"

#define	_ENWATCHDOG_

class CLand :	public CPump
{
public:
	CLand();
	virtual ~CLand();

private:
	// ===== 차량 설정 =====
#define	DEBOUNCE_CARLENGTH		5
	typedef struct _tagCARLENGTH {
		CCalm <BYTE>	stab;
		// 190909
		//BYTE			cActive;
		//BYTE			cPassive;
		// at first, has DEFAULT_CARLENGTH -> start bus cycle;
		BYTE			cCurrent;
	} CARLENGTH;	// master send 'active' by setting length and

	typedef struct _tagVEHICLE {
		BYTE	cProperID;		// TU:0~3, CU:4~11
		WORD	wPermNo;		// 2XNN
		WORD	wBusCycleTime;
		CARLENGTH	length;		// when all slave respond from passive,
								// then adjust bus cycle by car length
	} VEHICLE;
	VEHICLE		c_vehicle;

#define	TIME_OTRSKIP			REAL2PERIOD(60000);
#define	DEBOUNCE_OTRNO			5
	typedef struct _tagOTRCHANGER {
		CCalm <WORD>	no;			// 171212
		WORD			wSkipTimer;
	} OTRCHANGER;
	OTRCHANGER	c_otr;

	// ===== 동작에 관계되는 ... =====
	typedef struct _tagDTBORDERNO {
		DWORD	dwActive;
		DWORD	dwPassive[2];
	} DTBORDERNO;
	DTBORDERNO	c_order;

	RECIP		c_recip[POPI_MAX];	// 0:send,	1:work
									// 1:current work area,
									// broadcast 전송(마스터)/응답(슬레이브)
	bool		c_bColdWakeup;
	bool		c_bControlInitialed;

	bool		c_bAlone;

	BYTE		c_cDtbChk[PID_MAX];
	WORD		c_wFpgaVersion[3];
	short		c_vExtTemp;

	enum enINTROSEQUENCE {
		INTROSEQ_NON = 0,
		INTROSEQ_TIMEMATCH,
		INTROSEQ_UNFOLD
	} INTROSEQ;

	typedef struct _tagINTROSYNC {
		BYTE	cFrom;
		BYTE	cSeq;
	} INTROSYNC;

#define	TPERIOD_DTBERRORPREVENT		REALBYSPREAD(5000)
#define	TPERIOD_ECUBCFPREVENT		REALBYSPREAD(10000)
#define	TPERIOD_WAITREQUIREDOC		REAL2PERIOD(5000)
	typedef struct _tagTCCOMMANDS {
		INTROSYNC	intro;
		__BC(	state,
					waitRequiredDoc,	1,
					launched,			1,
					arteriam,			1,
					dtbActive,			1,	// 17/07/03, Appended
											// "17/07/03.가" 항 참조,
											// DTB를 setup하고 1이 된다.
					//domin,			1,	// 190110
					//alone,			1,
					//spr,				2);
					spr,				4);
		__BC(	user,
					v3fBreakCntClear,	1,
					releaseServiceBlk,	1,
					pisRequest,			1,
					//pisManualAck,		1,
					spr,				5);
		WORD	wWaitRequireDoc;	// 180308
									// 5초 이내로 mm의 CULLREQUIREDDOC를 기다린다.
		DBFTIME	logMatchTime;
		struct {
			WORD	wDtb;			// 13~18까지 trouble을 검사하는 시간을 설정해둔다.
			WORD	wEcuBcf;		// trouble 323을 검사하지 않는 시간
		} prevent;
	} TCCMDS;
	TCCMDS		c_cmd;

#define	MAX_CENTIPOP		10
#define	TPERIOD_ATO1		1		// period at 200
#define	TPERIOD_ATO2		5		// 201222, period at 24
#define	TPERIOD_CLOSURE		REALBYSPREAD(500)
	typedef struct _tagMILLSCALE {
		WORD	wDtb;
		WORD	wPop;
		WORD	wPopMax;
		WORD	wPopN;
		WORD	wAto;			// ATO 통신 사이클은 100ms이상이라서 100ms를 카운터한다.
		// 200218
		//bool	bHvac;			// HVAC 통신 사이클은 200ms이므로 100ms를 한번 건너뛴다.
		WORD	wClosureCounter;// 500ms(실제로는 DTB통신 5회)마다 운행 기록을 남긴다.
		// 181003
		//bool	bConsult;
		// papar consultant에 career와 temp/dia를 번갈아서 전송한다.
		BYTE	nConsult;
		//bool	bEnvConsult;	// 181003
		//bool	bCrrConsult;
	} MILLSCALE;
	MILLSCALE	c_mill;			// 1/100초를 카운터하여 정해진 작업을 하는 기본 카운터이다.

#define	TACHOREAD_CYCLE		10
#define	TACHOBUF_MAX		3	//10
	typedef struct _tagTACHOSTREAM {
		int			iCycle;
		int			i;
		WORD		w[TACHOBUF_MAX];
		WORD		wAvr;
		uint64_t	clk;		// debug for tacho calc.
	} TACHOSTREAM;

#define	ADCONVBUF_MAX		10
	typedef struct _tagADFIELD {
		bool	bFull;
		int		i;
		WORD	w[ADCONVBUF_MAX];
		WORD	wAvr;
	} ADFIELD;
	typedef struct _tagADSFIELD {
		int		ch;
		ADFIELD	a[ADCONVCH_MAX];
	} ADSFIELD;

	typedef struct _tagSMOOTH {
		TACHOSTREAM	tacho;
		ADSFIELD	ads;
	} SMOOTH;
	SMOOTH		c_smooth;

	SYSTEMCHECK	c_sysChk;
	WORD		c_wArterSeq;

	// ===== 마스터를 설정하기위한 ... =====
#define	TIME_DTBWAITEMPTY		REALBYSPREAD(500)
#define	TIME_DTBOBSERVEMAIN		REALBYSPREAD(500)
#define	TIME_DTBOBSERVEAUX		REALBYSPREAD(1500)
#define	TIME_DTBOBSERVESLAVE	REALBYSPREAD(20000)

	enum enDTBINITSEQ {
		DTBISEQ_NON,
		DTBISEQ_DEFER,			// 전원 투입에서 방향을 정할 때...
		DTBISEQ_WAITHCR,
		DTBISEQ_WAITEMPTY,		// HCR/TCR 전환일 때...
		DTBISEQ_OBSERVE,
	};
	typedef struct _tagCONVENTION {
		WORD	wSeq;
		WORD	wBroadcastReceived;	// 브로드 캐스트를 받은 횟수
		WORD	wGatherCycle;
		WORD	wWaitTimer;
		WORD	wTime4Wait;
		bool	bWicket;
	} CONVENTION;
	CONVENTION	c_convention;
	CMutex		c_mtCvt;			// 190923

public:
	// ===== TRACE... =====
	enum enTRACESEQ {
		TRACESEQ_NON,
		TRACESEQ_COLLECT,
		TRACESEQ_SAVE,				// not used, delete this
		TRACESEQ_MAX
	};

#define	TIME_WAITTRACEARRANGE		REALBYSPREAD(3000)
#define	DEB_TRACECLOSE				5
	typedef struct _tagTRACEINFO {
		bool	bClear;
		WORD	wSeq;
		WORD	wItem;
		WORD	wChapterID;
		WORD	wChapterMax;
		WORD	wPageID;
		WORD	wPageMax;
		WORD	wRespDevMap;
		WORD	wNullDevMap;
		WORD	wWaitArrangeTimer;
	} TRACEINFO, *PTRACEINFO;
	TRACEINFO	c_trcInfo;

private:
	// 180511, same to du's animation flag
	//typedef union _tagCTRLSIDE {
	//	struct {
	//		WORD	hvachl	: 1;
	//		WORD	hvachr	: 1;
	//		WORD	hlfsl	: 1;
	//		WORD	hlfsr	: 1;
	//		WORD	elfsl	: 1;
	//		WORD	elfsr	: 1;
	//		WORD	vfsl	: 1;
	//		WORD	vfsr	: 1;

	//		WORD	adpsl	: 1;
	//		WORD	adpsr	: 1;
	//		WORD	hvaccl	: 1;
	//		WORD	hvaccr	: 1;
	//		WORD	aclpl	: 1;
	//		WORD	aclpr	: 1;
	//		WORD	dclpl	: 1;
	//		WORD	dclpr	: 1;
	//	} b;
	//	WORD	a;
	//} CTRLSIDE;
	//CTRLSIDE	c_ctrlSide;

	// ===== monitor =====
	typedef struct _tagPULSECHK {
		bool		bInitial;
		uint64_t	clk;
		DWORD		dwPeriod;
		WORD		wDuty;
		WORD		wDutyMax;
		WORD		wExceed;
	} PULSECHK;
	PULSECHK	c_pulse;

	typedef struct _tagTASKWATCH {
		WORD	wArt[10][2];
		DWORD	dwSupervise[2];
		DWORD	dwOrderAtMax;
	} TASKWATCH;
	TASKWATCH	c_tWatch;

	typedef struct _tagBROADCASTMON {
		WORD	wTimer;				// 두 채널을 같이 감시한다.
									// 두 채널에서 모두 브로드캐스트가 들어오지 않으면
									// 적당한 횟 수에서 독립적인 제어를 한다.
		WORD	wWaiter[2];			// 어느 채널에서 브로드캐스트가 들어오지 않았는지 보여주기위한 카운터...
		WORD	wRed;
		WORD	wCounter;
	} BROADCMON;
	BROADCMON	c_bcMon;
#define	COUNTER_BROADCASTX		30		// 3sec	// 10

	typedef struct _tagDTBMON {
		//BYTE	cRxedBrc[2];
		//BYTE	cRxedCls[2];
		DWORD	dwRoute;		// 디버그를 위한 변수
		WORD	wWakeupEach[FID_MAX];
	} DTBMON;
	DTBMON		c_dtbMon;

	typedef struct _tagTROUBLECOUNTER {
		WORD	wShot;
		WORD	wCut;
	} TCOUNTER;
	TCOUNTER	c_tcnt;

	BYTE		c_cWickBuf[2048];
	BYTE		c_cFace[64];

	// 210805, for ATO debug, ClearAtoFlap()
	typedef struct _tagDBGATOHCR {
		int		i;
		BYTE	inp[10];
	} DBGATOHCR;
	DBGATOHCR	c_dbgAtoHcr;

	typedef struct _tagDOUBLETRAFFICBUS {
		//bool	bRxedBrc;
		//bool	bRxedCls;
		__BC(	state,
					open,	1,
					close,	1,
					ack,	1,
					spr,	5);
		CDtb*	p;
	} ACCURATEDTB;
	ACCURATEDTB	c_dtb[2];

	CTimer*		c_pTuner;
	CLocal*		c_pLoc;
	CPole*		c_pPol;
	CWicket		c_wick;
	CFio		c_fio;

	CTurf		c_turf;
	CStick		c_stick;
	CSteer		c_steer;
	CPel		c_pel;
	CStage		c_stage;
	CLide		c_lide;
	CUniv		c_univ;
	CIntvMon	c_itvm;

	void	Destroy();
	void	Initial();
	bool	Negotiate();
	bool	ModifyDtb(BYTE uOblige);
	void	Alone();
	bool	Spread();
	void	Supervise(bool bAlone = false);	// 190110
	void	Gather();
	bool	CollectTrace(int ldi);
	void	CaptureLines();
	void	Reform();
	bool	PrologConfirm(PPROLOG pProl, DWORD dwOrder = 0);
	bool	CheckCarLength(BYTE cCarLength);
	bool	AlignLocal(BYTE cCarLength);
	void	DtbControl();
	void	InitialAll(int iFrom);
	void	Roll(char* p, const char* fmt, ...);
	void	Intro(enFROMINTRO from)
	{ c_cmd.intro.cFrom = from; c_cmd.intro.cSeq = INTROSEQ_TIMEMATCH; }

public:
	void	Arteriam();
	void	TurnObserve();
	bool	LeadTraceSphere(int ldi, WORD* pwTraceLengthFromCID);
	void	SuspendTraceSphere();
	void	ReleaseServiceBlock();
	// 200218
	void	ClearAllRed();
	bool	Launch();
	WORD	GetStablePairCondition(bool bTrig);
	void	LeaveReason(int reason = 0);
	void	LeaveOnly(BYTE reason);
	void	CopySysChk(PSYSTEMCHECK pSysChk);

	BYTE	GetProperID()				{ return c_vehicle.cProperID; }
	bool	GetColdWakeup()				{ return c_bColdWakeup; }
	void	SetAtoLinePrevent();
	void	SetEcuBcfPrevent();
	WORD	GetEcuBcfPrevent()			{ return c_cmd.prevent.wEcuBcf; }
	void	SetTraceClear(bool bClear)	{ c_trcInfo.bClear = bClear; }
	PTRACEINFO	GetTraceInfo()			{ return &c_trcInfo; }
	WORD	GetTraceInfoSeq()			{ return c_trcInfo.wSeq; }
	CFio*	GetFio()					{ return &c_fio; }
	// 181011
	//void	StirTimeMatch()				{ c_turf.StirTimeMatch(); }
	void	StirTimeMatch(bool bByRtd);
	void	StirEnvironMatch()			{ c_turf.StirEnvironMatch(); }
	void	StirCareerMatch()			{ c_turf.StirCareerMatch(); }
	// 200218
	void	StirClearLfc()				{ c_turf.StirClearLfc(); }
	bool	GetTimeMatchStatus()		{ return c_turf.GetTimeMatchStatus(); }
	//bool	GetStableHcr()				{ return c_stick.GetStableHcr(); }
	void	GetSteerAvrLog(PLOGBOOK pLog)	{ c_steer.GetAvrLog(pLog); }
	BYTE	GetSteerLogicPwm()			{ return c_steer.GetLogicPwm(); }
	void	CopyMomentum(PMOMENTUMINFO pMoment)
			{ c_steer.CopyMomentum(pMoment); }		// 180405
	BYTE	GetPantoState(BYTE id)		{ return c_pel.GetPantoState(id); }
	// 181023
	void	ClearRtdTimeFetch()		{ c_lide.ClearRtdTimeFetch(); }
	void	SendHdu()			{ if (c_pPol != NULL)	c_pPol->SendHdu(); }
	void	SendHduA()			{ if (c_pPol != NULL)	c_pPol->SendHduA(); }
	void	SendHdu(BYTE* pBuf, WORD wLength)
			{ if (c_pPol != NULL)	c_pPol->SendHdu(pBuf, wLength); }
	void	SendHduA(BYTE* pBuf, WORD wLength)
			{ if (c_pPol != NULL)	c_pPol->SendHduA(pBuf, wLength); }
	//void	TurnFireDialog()		{ c_stage.TurnFireDialog(); }
	void	RegisterDuPage(WORD wPage)	{ c_stage.RegisterPage(wPage); }
	// 210805
	BYTE	GetAtoHcrs(int i)		{ return c_dbgAtoHcr.inp[i]; }

#if	defined(_DIRECT_CALL_)
	void	SendDtbBus(DWORD dw);
	void	ReceiveDtbBus(DWORD dw);
#endif

	int		Bale(bool bType, BYTE* pMesh);
	PRIVACY_CONTAINER();

protected:
	void	TakeReceiveBus(DWORD dw);
	void	TakeSendBus(DWORD dw);
	void	TakeReceivePole(DWORD dw);
	void	TakeReceiveLocal(DWORD dw);
	void	TakeSaveEcuTrace(DWORD dw);
	PUBLISH_TAGTABLE();
};
