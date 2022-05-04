/*
 * CVerse.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once
#include "Arrange.h"
#include "Prefix.h"
#include "Slight.h"
#include "../Inform2/DevInfo2/PDevInfo.h"
#include "Draft.h"
#include "Infer.h"
#include "../Component2/CPop.h"

#define	GETVERSE(p)	CVerse* p = CVerse::GetInstance()

class CVerse
{
public:
	CVerse();
	virtual ~CVerse();

private:
	BYTE	c_cPermNo;				// CUDIB_CT01
	BYTE	c_cAtoSeqNo;			// 120719
	BYTE	c_cAtoChgNo;
	WORD	c_wCurOtrn;
	// 200218
	//WORD	c_wRed[PDEVID_MAX];
	typedef struct _tagREDITEM {
		WORD	wHour;
		BYTE	cMin;
		BYTE	cSec;
		BYTE	cMs;
		WORD	wRed[PDEVID_MAX];
	} REDITEM;
	REDITEM	c_ritem;
	WORD	c_wDebs[PDEVID_MAX];
	BYTE	c_cLineState;

#define	BLOCK_CYCLES		(10 - LINEDEB_POLE)
	WORD	c_wBlocks[PDEVID_MAX];	// TUDIB_SVCDF 대신에 사용되는 통신 오류 카운터
	BYTE	c_cLineBlocks;

	// 200218
	typedef struct _tagSEQCNTS {
		WORD	wFdu;
		WORD	wBms;
	} SEQCNTS;
	SEQCNTS	c_seqCnts;
	BYTE	c_cPoleRedID;

	WORD	c_wHduRecvLength;

	typedef struct _tagRTDREFERENCE {
		bool		bSdrHold;			// 1:RTD로 보냈던 데이터를 계속 보낸다.
		BYTE		cResult;
		WORD		wAddr;
		RTDSTATE	state;
	} RTDREF;
	RTDREF		c_rtdRef;

	CMutex		c_mtx;
	CMutex		c_mtxRed;
	int			c_iServicedID;

	typedef struct _tagPISINFO {
		bool	bAim;			// manual(1)/auto(0)
		bool	bUser;			// head(0)/tail(1)
		bool	bManual[2];		// 0/9
		BYTE	cCurCtrlCode;	// service device priority code;
		BYTE	cNorCtrlCode;
	} PISINFO;
	PISINFO		c_pisInfo;

	ATOFLAP		c_ato[LI_MAX];
	HDUFLAP		c_hdu[LI_MAX];
	RTDFLAP		c_rtd[LI_MAX];
	PAUFLAP		c_pau[LI_MAX];
	PISFLAP		c_pis[LI_MAX];
	TRSFLAP		c_trs[LI_MAX];
	// 200218
	FDUFLAP		c_fdu[LI_MAX];
	BMSFLAP		c_bms[LI_MAX];

	_BUNDLES	c_bund;
	_SHEAF		c_sheaf;
	BYTE		c_cDump[512];

	static const WORD	c_wFlapLength[PDEVID_MAX ][LL_MAX];	// SIZE_SR, SIZE_SA, de-bounce time
	static const WORD	c_wFlapLength2[PDEVID_MAX][LL_MAX];	// SIZE_SR, SIZE_SA, de-bounce time

	// CLide에서 관리하고 CStage에서 참조하는 변수...
	typedef struct _tagATOCURSTATE {
		WORD	wWarmUpTimer[2];	// 180511
		WORD	wDefectMap;
		BYTE	cLines[2];
		ATOFLAP	fl[2];			// 단순히 HDU에 통신내용을 복사하기위한 버퍼이다.
	} ATOCURSTATE;
#define	TIME_ATOWARMUP			REAL2PERIOD(60000)	// 190102, 45000)		// 171221, 40000)		// 171219, 35000)		// 170717, 30000)

	typedef struct _tagRTDCURSTATE {
		WORD	wWarmUpTimer;
		//WORD	wDefectMap;
		BYTE	cLines[2];
		RTDFLAP	fl[2];
	} RTDCURSTATE;
// 210803
//#define	TIME_RTDWARMUP			REAL2PERIOD(30000)	// 201207	0)
//#define	TIME_RTDWARMUPA			REAL2PERIOD(50000)	// 210218
//#define	TIME_RTDWARMUPA			REAL2PERIOD(0)  // 211112 delete
//#define	TIME_RTDWARMUPA			REAL2PERIOD(50000)  // 211112
//#define	TIME_RTDWARMUPA			REAL2PERIOD(100000)  // 211124
#define	TIME_RTDWARMUPA			REAL2PERIOD(10000)  // 220220 test
#define	TIME_RTDWARMUPB			REAL2PERIOD(30000)	// 201207	0)
#define	TIME_RTDWARMUPC			REAL2PERIOD(50000)	// 210218

	typedef struct _tagPAUCURSTATE {
		WORD	wWarmUpTimer;
		//WORD	wDefectMap;
		BYTE	cLines[2];
		PAUFLAP	fl[2];
	} PAUCURSTATE;
// 210803
#define	TIME_PAUWARMUP			REAL2PERIOD(10000)	// 210614	(3000)	// 201207	0)
#define	TIME_PAUWARMUPA			REAL2PERIOD(0)
#define	TIME_PAUWARMUPB			REAL2PERIOD(120000)	// 210614	(3000)	// 201207	0)

	typedef struct _tagPISCURSTATE {
		WORD	wWarmUpTimer;
		//WORD	wDefectMap;
		BYTE	cLines[2];
		PISFLAP	fl[2];
	} PISCURSTATE;
// 210803
#define	TIME_PISWARMUP			REAL2PERIOD(10000)	// 210614	(10000)
#define	TIME_PISWARMUPA			REAL2PERIOD(10000)
#define	TIME_PISWARMUPB			REAL2PERIOD(120000)	// 210614	(10000)

	typedef struct _tagTRSCURSTATE {
		WORD	wWarmUpTimer;
		WORD	wDefectMap;
		BYTE	cLines[2];
		BYTE	cDebMastChg[2];
		BYTE	cDebModeAck[2];
		TRSFLAP	fl[2];
	} TRSCURSTATE;
// 210803
#define	TIME_TRSWARMUP			REAL2PERIOD(3000)	// 201207	0)
#define	TIME_TRSWARMUPA			REAL2PERIOD(0)
#define	TIME_TRSWARMUPB			REAL2PERIOD(3000)	// 201207	0)
#define	DEB_PISMASTERCHG		4				// 3에서 바꾸고 4에서는 무시. 비트가 0이되어 수치가 0으로 될 때까지 4로 대기한다.

	// 200218
	// !!! CAUTION !!!
	// [0] has context of tc0, [1] has context of tc9, without direction !!!
	typedef struct _tagFDUCURSTATE {
		WORD	wWarmUpTimer;
		WORD	wDefectMap;
		BYTE	cLines[2];
		FDUFLAPAB	fl[2];
	} FDUCURSTATE;
#define	TIME_FDUWARMUP			REAL2PERIOD(30000)	// 201207	0)

	typedef struct _tagBMSCURSTATE {
		WORD	wWarmUpTimer;
		WORD	wDefectMap;
		BYTE	cLines[2];
		BMSFLAPAB	fl[2];
		BMSCVS		cvs[2];		// 2 * 50bytes
	} BMSCURSTATE;
#define	TIME_BMSWARMUP			REAL2PERIOD(0)

#define	WATCH_VALIDWEIGHTBYDIR		REALBYSPREAD(3000)
	typedef struct _tagVALIDWEIGHTBYDIR {
		bool	bStb;			// to ATO.sdr.valid.b.weight, stabilize
		WORD	wWatch;
	} VWEIGHTBYDIR;

public:
	enum enRTDCLOSURETYPE {
		RCLSTYPE_SUCCESS = 0,
		RCLSTYPE_FAILSEND,
		RCLSTYPE_FAILCRC,
		RCLSTYPE_FAILSIZE,
		RCLSTYPE_FAILADD,
		RCLSTYPE_FAILFLOW,
		RCLSTYPE_FAILFRAME,
		RCLSTYPE_FAILRESPOND,
		RCLSTYPE_TIMEOUT
	};

	typedef struct _tagPSHAREVARS {
		ATOCURSTATE		atocf;
		RTDCURSTATE		rtdcf;
		PAUCURSTATE		paucf;
		PISCURSTATE		piscf;
		TRSCURSTATE		trscf;
		// 200228
		FDUCURSTATE		fducf;
		BMSCURSTATE		bmscf;

		VWEIGHTBYDIR	vwbd;
		// 171129
		//BYTE			cRefSvcCode;
		//BYTE			cSvcCtrlCode;	// service device priority code;
										// b7-6:tail trs, b5-4:head trs, b3-2:tail pau-pis, b1-0:head pau-pis
		// 200218
		//WORD			wReceivedRed[PDEVRED_MAX][2];
		WORD			wReceivedRed[PDEVID_MAX][2];
	} PSHARE, *PPSHARE;
	PSHARE		c_psv;

private:
	void	Backup(int pdi);
	void	DecorAto(PATORINFO pAtor);
	void	DecorAis(PSRAIS pAisr);

public:
	void	Initial();
	void	InitAtoWarmup(int si);		// 180511
	int		GetValidPisSide();
	PVOID	GetInfo(bool bDir, int pdi, int li);	// bDir:true(sda)/false(sdr)
	WORD	GetLength(bool bDir, int pdi);
	//void	SetAtoWarmup();
	WORD	GetWarmup(int pdi, int si = 0);
	void	ClearAtoFlap();
	void	Closure(int pdi);
	void	Success(int pdi);
	bool	RtdClosure(int iType);
	bool	Repair(int pdi, int recf);
	void	Reform(PRECIP pRecip);
	PVOID	Decorate(int pdi);
	void	ReleaseServiceBlock();
	void	SetServiceControlCode();
	void	ClearAllRed();

	// 200218
	//WORD	GetServiceDebTime();
	void	ClearSheaf();
	void	CopyBundle(PVOID pSrc, WORD wLength);
	void	Arteriam();

	void	SetPisManual(bool bSide, bool bManual)	{ c_pisInfo.bManual[bSide ? 1 : 0] = bManual; }
	bool	GetPisManual(bool bSide)				{ return c_pisInfo.bManual[bSide ? 1 : 0]; }
	void	GetStationA(WORD* pwNext, WORD* pwDest);
	void	TurnPisCode();
	void	SetPisMode(bool bUser, bool bAim);
	PPSHARE	GetShareVars()						{ return &c_psv; }
	// 180917
	//void	SetCurOtrn(WORD wOtrn)				{ c_wCurOtrn = wOtrn; }		// c_bCurOtrXer = bXer; }
	WORD	GetPauWarmUpTimer()					{ return c_psv.paucf.wWarmUpTimer; }
	WORD	GetPisWarmUpTimer()					{ return c_psv.piscf.wWarmUpTimer; }
	void	SetCurOtrn(WORD wOtrn);
	WORD	GetCurOtrn()						{ return c_wCurOtrn; }
	void	SetPermNo(BYTE cPermNo)				{ c_cPermNo = cPermNo; }
	BYTE	GetPermNo()							{ return c_cPermNo; }
	BYTE	GetLineState()						{ return c_cLineState; }
	BYTE	GetLineBlocks()						{ return c_cLineBlocks; }
	void	SetCloseSvcID(int iSvcID)			{ c_iServicedID = iSvcID; }
	PBUNDLES	GetBundle()						{ return &c_bund; }
	WORD	GetHduRecvLength()					{ return c_wHduRecvLength; }
	void	SetHduRecvLength(WORD wLeng)		{ c_wHduRecvLength = wLeng; }
	PSHEAF	GetSheaf()							{ return &c_sheaf; }
	BYTE*	GetDump()							{ return &c_cDump[0]; }
	WORD	GetRedHour()						{ return c_ritem.wHour; }
	BYTE	GetRedMin()							{ return c_ritem.cMin; }
	BYTE	GetRedSec()							{ return c_ritem.cSec; }
	static CVerse*	GetInstance()				{ static CVerse verse; return &verse; }

	int		BaleHduRef(bool bType, BYTE* pMesh);
	int		BaleSvcRef(bool bType, BYTE* pMesh);

	PUBLICY_CONTAINER();
};
