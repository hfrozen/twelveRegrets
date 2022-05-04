/*
 * CProse.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma	once
#include "Arrange.h"
#include "Prefix.h"
#include "../Inform2/DevInfo2/PDevInfo.h"
#include "Draft.h"
#include "Infer.h"
#include "Mm.h"
#include "../Component2/CPop.h"

#define	GETPROSE(p)	CProse* p = CProse::GetInstance()

class CProse
{
public:
	CProse();
	virtual ~CProse();

private:
	WORD	c_wRed[REALDEV_MAX];
	WORD	c_wDebs[REALDEV_MAX];
	WORD	c_wLineState;

	// 200218
	BYTE	c_cLocalRedID;

	CMutex	c_mtx;
	CMutex	c_mtxRed;
	bool	c_bSiv;
	bool	c_bV3f;

	__BC(	c_inspCmdBuf,
				siv,	1,
				v3f,	1,
				ecub,	1,
				ecua,	1,
				hvac,	1,
				horc,	1,
				cancel,	1,
				sp,		1);

	bool	c_bEcuTraceUpload;			// CU에서 사용
	bool	c_bHvacInspectCancel;		// CU에서 사용
	bool	c_bFinalAlter;				// 200218

	int		c_iFinalDcuID[2];

	BYTE	c_cSivTraceLength;			// 해당 객차의 트레이스 길이를 보관한다.
	BYTE	c_cV3fTraceLength;
	BYTE	c_cEcuTraceLength;

	// 171019
	WORD	c_wEcuBlindCnt;
	// 171226
	typedef struct _tagECULOADRATESYNC {
		bool	bSync;
		WORD	wTimer;
	} ECULRSYNC;
	ECULRSYNC	c_eculr;
#define	TIME_ECULOADRATESYNC			REAL2PERIOD(3000)

	typedef struct _tagECUTRACENOTE {
		WORD	wPreID;
		WORD	wPreLength;
		WORD	wSufLength;
		SAECUS	cura;
		BYTE	cPreBuf[SIZE_PREECUTRACE][sizeof(SAECUS)];	// 중고장이 발생하기 전의 데이터들... 라운드 로빈 방식으로 최근 것을 겹쳐쓴다.
		BYTE	cBuf[SIZE_ECUTRACE][sizeof(SAECUS)];			// 중고장이 발생한 후의 데이터들을 포함한...
	} ECUTRACENOTE;
	ECUTRACENOTE	c_ecuNote;

	static const BYTE	c_cEcuMajfs1[sizeof(SAECUS)];
	static const BYTE	c_cEcuMajfs2[sizeof(SAECUS)];

	typedef struct _tagSIVDETECTIVENOTE {
		WORD		wPreID;
		WORD		wPreLength;
		WORD		wSufLength;
		SIVADETEX	cura;
		BYTE		cPreBuf[SIZE_PRESIVDETECT][sizeof(SIVADETEX)];	// 중고장이 발생하기 전의 데이터들... 라운드 로빈 방식으로 최근 것을 겹쳐쓴다.
		BYTE		cBuf[SIZE_SIVDETECT][sizeof(SIVADETEX)];		// 중고장이 발생한 후의 데이터들을 포함한...
	} SIVDETNOTE;
	SIVDETNOTE	c_sivNote;

	static const BYTE	c_cSivDets1[sizeof(SASIVEX)];

	// 200218
	typedef struct _tagSIVDETECTIVENOTE2 {
		WORD		wPreID;
		WORD		wPreLength;
		WORD		wSufLength;
		SIVADET2	cura;
		BYTE		cPreBuf[SIZE_PRESIVDETECT][sizeof(SIVADET2)];	// 중고장이 발생하기 전의 데이터들... 라운드 로빈 방식으로 최근 것을 겹쳐쓴다.
		BYTE		cBuf[SIZE_SIVDETECT][sizeof(SIVADET2)];		// 중고장이 발생한 후의 데이터들을 포함한...
	} SIVDETNOTE2;
	SIVDETNOTE2	c_sivNote2;

	static const BYTE	c_cSivDets2[sizeof(SASIV2)];

	// 180625
	// 190909
	//bool		c_bSivFound;
	//bool		c_bSivEx;
	//SIVFLAPTRC	c_sivflt[LI_MAX];		// 트레이스까지 포함한다.
	SIVFLAPTRCEX	c_sivfltEx[LI_MAX];
	// 200218
	SIVFLAPTRC2	c_sivflt2[LI_MAX];

	V3FFLAPTRC	c_v3fflt[LI_MAX];
	ECUFLAPS	c_ecufl[LI_MAX];
	HVACFLAP	c_hvacfl[LI_MAX];
	DCUFLAP		c_dcufl[8][LI_MAX];
	// 200218
	CMSBFLAP	c_cmsbfl[LI_MAX];

	static const WORD	c_wFlapLength1[DEVID_CM][LL_MAX];
	// 201116
	static const WORD	c_wFlapLength2[DEVID_CM][LL_MAX];

	// CPel, CUniv에서 관리하고 CStage에서 참조하는 변수들...
	typedef struct _tagSIVCURSTATE {
		bool		bTrace;					// trace data가 있으면 "지우기"를 가능하게 한다.
		WORD		wWarmUpTimer;
		WORD		wCorrectMap;			// SIVK 배열
		WORD		wDefectMap;				// SIVF 배열
		WORD		wManualDefectMap;		// SIVCN1, 2 배열
		BYTE		cLines[SIV_MAX];
		SIVFLAPEX	fl[SIV_MAX];			// 트레이스를 포함하지 않는다.
		SIVFLAP2	fl2[SIV_MAX];
	} SIVCURSTATE;
#define	TIME_SIVWARMUP				REAL2PERIOD(5000)

	typedef struct _tagV3FCURSTATE {
		bool		bTrace;
		WORD		wWarmUpTimer;
		WORD		wDefectMap;
		BYTE		cLines[V3F_MAX];
		V3FFLAP		fl[V3F_MAX];			// 단순히 HDU에 통신내용을 복사하기위한 버퍼이다.
	} V3FCURSTATE;
#define	TIME_V3FWARMUP				REAL2PERIOD(5000)

	typedef struct _tagECUCURSTATE {
		bool		bTrace;
		WORD		wWarmUpTimer;
		// 201016
		WORD		wNrbdDetectTimer[ECU_MAX];
		WORD		wNrbdDetectMap;
		WORD		wNrbdDetectedMap;
		WORD		wDefectMap;
		WORD		wSlideBitmap;	// slip & slide
		WORD		wHbReleaseWatch[ECU_MAX];
		BYTE		cLines[ECU_MAX];
		//BYTE		cTraceLength;
		ECUFLAPS	fl[ECU_MAX];
	} ECUCURSTATE;
#define	TIME_ECUWARMUP				REAL2PERIOD(15000)
#define	WATCH_HBRELEASE				REALBYSPREAD(3000)
#define	TIME_NRBDDETECT				REALBYSPREAD(5000)

//#define	TIME_HVACINSPECTCANCLE		REAL2PERIOD(2000)
	typedef struct _tagHVACCURSTATE {
		WORD		wWarmUpTimer;
		WORD		wDefectMap;
		BYTE		cLines[HVAC_MAX];
		HVACFLAP	fl[HVAC_MAX];
	} HVACCURSTATE;
#define	TIME_HVACWARMUP				REAL2PERIOD(5000)

	// 200218
	typedef struct _tagCMSBCURSTATE {
		WORD		wWarmUpTimer;
		WORD		wDefectMap;
		BYTE		cLines[CMSB_MAX];
		CMSBFLAP	fl[CMSB_MAX];
	} CMSBCURSTATE;
#define	TIME_CMSBWARMUP				REAL2PERIOD(5000)

	typedef struct _tagDCUCURSTATE {
		WORD	wWarmUpTimer;
		WORD	wDefectMap[CID_MAX];
		BYTE	cLines[CID_MAX][8];
		struct {
			__BC(	stat,
						cdi,	1,	// current di, CUDIB_LDx, CUDIB_RDx
						pdi,	1,	// previous di,
						xdi,	1,
						cci,	1,	// current sda.stat.b.fullo
						pci,	1,	// previous sda.stat.b.fullo
						xci,	1,
						xch,	1,
						vain,	1);	// fail on sda
			WORD	wXchTimer;
		} mon[CID_MAX][8];			// 171213
		DCUFLAP	fl[CID_MAX][8];		// cid는 C2F를 거친 것이므로 진행 방향을 고려한 객차 번호가 된다. 그러므로 이 객체를 사용할 때 다시 진행 방향을 고려해선 안된다!!!
	} DCUCURSTATE;
#define	TIME_DCUWARMUP				REAL2PERIOD(5000)
#define	TIME_DCUXCHSKIP				REALBYSPREAD(5000)		// 171226, (3000)

public:
	typedef struct _tagLSHAREVARS {
		SIVCURSTATE		sivcf;
		V3FCURSTATE		v3fcf;
		ECUCURSTATE		ecucf;
		HVACCURSTATE	hvaccf;
		DCUCURSTATE		dcucf;
		// 200218
		CMSBCURSTATE	cmsbcf;
		bool			bManualAlbo;	// Hdu를 통해 들어오는 사용자의 albo 등 가동
		bool			bAlbo;			// all lamp black out, 가선 전압이 얼마 이하로 떨어짐
		bool			bEnManualAlbo;	// Hdu albo 등을 보여주는...
		bool			bManualCmg;		// Hdu를 통해 들어오는 사용자의 cm 가동
		WORD			wManualCmgCounter;
#define	COUNT_MANUALCMG			10		// 1sec
		WORD			wCmgBitmap;		// CUDIB_CMG
		WORD			wCmkBitmap;
		// 180626
		//struct {
		//	WORD		wUnitDefectMap;	// 화재 발생 장치 고장
		//	WORD		wDefectMap;		// 화재 발생, 현재 값
		//	WORD		wSeq[CID_MAX];
		//	WORD		wUserCmd[CID_MAX];
		//} fire;
		struct {
			WORD		wUnitJamMap;
			WORD		wDefectMap;
			// 200629
			WORD		wDefectSync;
			WORD		wRecoverMap;
			WORD		wSeq;
			WORD		wUserCmd;
#define	TIME_FIREDETECTIONRELEASE	100			// 1sec
			WORD		wReleaseTimer;
			// 210806
			WORD		wFromMaster;	// fire from master
			bool		bAck;
			bool		bMbCancel;
		} fire;
		WORD			wPaMap;			// 승객 호출
		WORD			wInspectCancelTime;		// inspect cancel을 유지하는 시간
#define	TIME_INSPECTCANCEL			REAL2PERIOD(1000)

		BYTE			cLw[ECU_MAX];
		WORD			wLoadrSum;
		WORD			wFailedCCs;
		WORD			wNrbds;
		// 180626
		BYTE			cDiai[LENGTH_WHEELDIA];		// CC does not have Env.
		BYTE			cTempSv[CID_MAX];
		WORD			wReceivedRed[REALDEV_MAX][CID_MAX];
		// 200218
		//WORD			wSubjectiveRed[REALDEV_MAX][CID_MAX];
	} LSHARE, *PLSHARE;
	LSHARE		c_lsv;

	enum enFIRESEQ {
		FS_OFF,				// fire alarm sequence
		FS_WAITUSERCOMMAND,
		FS_ACTIVE,
		FS_IGNORE,
		FS_CLOSE,
	};

	enum enUSERCMDATFIRE {
		UCF_NORMAL,			// user command for fire alarm
		UCF_AGREE,
		UCF_IGNORE,
	};

	// 180511, same to du's animation flag
	enum enCTRLSIDE {
		HVACCTRL_LEFT	= 0x0401,
		HVACCTRL_RIGHT	= 0x0802,
		HLFSCTRL_LEFT	= 0x0004,
		HLFSCTRL_RIGHT	= 0x0008,
		ELFSCTRL_LEFT	= 0x0010,
		ELFSCTRL_RIGHT	= 0x0020,
		VFSCTRL_LEFT	= 0x0040,
		VFSCTRL_RIGHT	= 0x0080,
		APDSCTRL_LEFT	= 0x0100,
		APDSCTRL_RIGHT	= 0x0200,
		ACLPCTRL_LEFT	= 0x1000,
		ACLPCTRL_RIGHT	= 0x2000,
		DCLPCTRL_LEFT	= 0x4000,
		DCLPCTRL_RIGHT	= 0x8000,
	};

	// 201016
	enum enCTRLEXSIDE {
		APDSACTRL_LEFT	= 0x0001,
		APDSACTRL_RIGHT	= 0x0002,
	};

	CCouple <BYTE>	c_hvacMode;
	CCouple <bool>	c_hlfsSw;
	CCouple <bool>	c_elfsSw;
	CCouple <bool>	c_vfsSw;
	CCouple <bool>	c_apdsSw;
	// 201016
	CCouple <bool>	c_apdsASw;
	CCouple <bool>	c_acSw;
	CCouple <bool>	c_dcSw;

private:
	//WORD	GetDebounce(int ldi);
	int		GetSerialID(int ldi, int iSubi);
	void	Backup(int ldi, int iSubi, bool bTrace);

public:
	void	Initial();
	// 190909
	//WORD	GetSivType();
	//void	SetSivType(bool bType);
	PVOID	GetInfo(bool bDir, int ldi, int iSubi, int li);
	WORD	GetLength(bool bDir, int ldi);
	WORD	GetWarmup(int ldi);
	void	Closure(int ldi, int iSubi = 0);
	void	Success(int ldi, int iSubi);
	bool	Repair(int ldi, int iSubi, int recf);
	void	Reform(PRECIP pRecipWork, PRECIP pRecipHead);
	PVOID	Decorate(int ldi, int iSubi);
	void	Monitor();
	WORD	GetControlSide();
	// 201016
	WORD	GetControlExSide();
	// 200218
	void	ClearAllRed();
	void	CorralEcuTrace();
	void	CorralEcuTrace(BYTE* pSiva, WORD wStatus);
	void	CorralSivDetect();
	void	CorralSivDetect(BYTE* pSiva, WORD wStatus);
	// 200218
	void	CorralSivDetect2();
	void	CorralSivDetect2(BYTE* pSiva, WORD wStatus);
	int		Bale(bool bType, BYTE* pMesh);
	void	Arteriam();

	//void	SetEcuTraceLength(BYTE cLength)		{ c_ecuNote.cTotalLength = cLength & 0xf; }
	// 200218
	void	TriggerFireDetectionRelease();		//		{ c_lsv.fire.wReleaseTimer = TIME_FIREDETECTIONRELEASE; }
	// 210720
	bool	GetHvacInspCmdBuf()					{ return c_inspCmdBuf.b.hvac; }	// for display inspect sequence of hvac(on/off)
	bool	GetHorcInspCmdBuf()					{ return c_inspCmdBuf.b.horc; }	// for display inspect sequence of hvac(heat/cool)
	void	SetFinalAlter(bool bAlter)			{ c_bFinalAlter = bAlter; }
	void	SetFinalDculID(int id)				{ c_iFinalDcuID[0] = id; }
	void	SetFinalDcurID(int id)				{ c_iFinalDcuID[1] = id; }
	void	SetInspectCancel(bool bCancel)		{ c_lsv.wInspectCancelTime = bCancel ? TIME_INSPECTCANCEL : 0; }
	PLSHARE	GetShareVars()						{ return &c_lsv; }
	WORD	GetLineState()						{ return c_wLineState; }
	static CProse*	GetInstance()				{ static CProse prose; return &prose; }

	PUBLICY_CONTAINER();
};
