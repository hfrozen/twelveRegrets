/**
 * @file	Slight.h
 * @brief	일반 매크로들을 정한다.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"
#include "Arrange.h"

#define M_PI			3.14159265358979323846
#define	TACHO_PULSE		90.f
#define	Validity(v)		((v) >= 0)
//#define	IsText(p)	(p != NULL && *p != 0)
#define	KILL(p)\
	do {\
		if (p != NULL) {\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	KILLC(p)\
	do {\
		if (p != NULL) {\
			p->Close();\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	KILLR(p)\
	do {\
		if (p != NULL) {\
			if (p->GetRhythm())	p->Release();\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	KILLM(p)\
	do {\
		if (p != NULL) {\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	MAKEDWORD(low, high)	((((DWORD)high << 16) & 0xffff0000) | (DWORD)low)
#define	HIGHWORD(dw)			((WORD)((dw >> 16) & 0xffff))
#define	LOWWORD(dw)				((WORD)(dw & 0xffff))
#define	MAKEWORD(low, high)		((((WORD)high << 8) & 0xff00) | ((WORD)low & 0xff))
#define	HIGHBYTE(w)				((BYTE)((w >> 8) & 0xff))
#define	LOWBYTE(w)				((BYTE)(w & 0xff))

#define	INCWORD(w)\
	do {\
		if (w < 0xffff) ++ w;\
	} while (0)

#define	INCBYTE(c)\
	do {\
		if (c < 0xff)	++ c;\
	} while (0)

#define	TAGPAR(lw, lc, hc)	MAKEDWORD(lw, MAKEWORD(lc, hc))
#define	TAGUNPAR(dw, lw, lc, hc)\
	WORD lw = LOWWORD(dw);\
	WORD __h = HIGHWORD(dw);\
	BYTE lc = LOWBYTE(__h);\
	BYTE hc = HIGHBYTE(__h)

//===========================================================================
#define	ISTU()			(CAR_FSELF() < FID_PAS)
#define	ISCU()			(CAR_FSELF() >= FID_PAS)
#define	IsCorrAddr(w)	((w & 0xff00) == DTB_BASEADDR)

#define	GETTI(ti, bi)	(pDoz->recip[ti].real.inp[1].c[bi >> 3] & (1 << (bi & 7)))
#define	GETCI(ci, bi)	(pDoz->recip[ci].real.inp[0].c[bi >> 3] & (1 << (bi & 7)))
#define	GETTIS(ti, bi)	(pDoz->recip[ti].real.inp[1].c[bi >> 3])
#define	GETCIS(ci, bi)	(pDoz->recip[ci].real.inp[0].c[bi >> 3])

#define	GETUI(id, bi)	(pDoz->recip[id].real.outp.w & (1 << bi))

#define	PTI2B(ti, bi)	((bool)((pPaper->GetRouteState(ti) &&  GETTI(ti, bi)) ? true : false ))
#define	NTI2B(ti, bi)	((bool)((pPaper->GetRouteState(ti) && !GETTI(ti, bi)) ? true : false ))
#define	PCI2B(ci, bi)	((bool)((pPaper->GetRouteState(ci) &&  GETCI(ci, bi)) ? true : false ))
#define	NCI2B(ci, bi)	((bool)((pPaper->GetRouteState(ci) && !GETCI(ci, bi)) ? true : false ))

#define	CUCUTOFF()\
	do {\
		if (ISCU()) {\
			TRACK("ERR:%s() is not process executed by the CU!\n", __PRETTY_FUNCTION__);\
			return;\
		}\
	} while (0)

#define	CUCUTOFFA()\
	do {\
		if (ISCU()) {\
			TRACK("ERR:%s() is not process executed by the CU!\n", __PRETTY_FUNCTION__);\
			return false;\
		}\
	} while (0)

#define	TUCUTOFF()\
	do {\
		if (ISTU()) {\
			TRACK("ERR:%s() is not process executed by the CU!\n", __PRETTY_FUNCTION__);\
			return;\
		}\
	} while (0)

/// === 기동 종류 ===
enum enFROMINTRO {
	INTROFROM_KEY = 0,	// first.. power on
	INTROFROM_STICK,	// turn direction
	INTROFROM_OTRN		// change otrno
};

enum enLOCALFSTATE {
	LCF_SEND = 0,
	LCF_ALL,			// 0xff
	LCF_RTIMEOUT,		// 0xfe
	LCF_RADDR,			// 0xfd
	LCF_RECHO,			// 0xfc
	LCF_RSIZE,			// 0xfb
	LCF_RCRC,			// 0xfa
	LCF_RFRAME,			// 0xf9
	LCF_MAX		// 8
};

//#define	REACTTIME_ATO	80
//#define	REACTTIME_RTD	80
//#define	REACTTIME_SVC	200

//#define	COUNT_DEADOPPO		5		// 카운터의 오버플로우 캐리는 리셋되지않으므로 별로 의미는 없지만 노이즈가 있을 수 있다고 보고...
#define	COUNT_DEADOPPO		50		// 카운터의 오버플로우 캐리는 리셋되지않으므로 별로 의미는 없지만 노이즈가 있을 수 있다고 보고...  (211210)
	typedef struct _tagSYSTEMCHECK {
		bool	bIoFail;
		BYTE	cVmes;
		BYTE	cDebs;
		BYTE	cPairCtrl;
		BYTE	nDeadOppoCnt;
		BYTE	cPast;
		WORD	wIntroTimer;		// reset에서 전의 프로세스가 남긴 상태를 3초동안 운행기록에 확실히 기록하기위해서...
		//WORD	wShutoffTimer;		// log 파일을 sd에 남기기위한...
	} SYSTEMCHECK, *PSYSTEMCHECK;
#define	TIME_SYSCHKINTRO		(3000 / 500);
//#define	TIME_SHUTOFF			REAL2PERIOD(500);
#define	DEBOUNCE_VME			10

#define	SIZE_DEVPOS		DEFAULT_CARLENGTH
typedef struct _tagDEVPOS {
	BYTE	cLength;
	BYTE	cPos[SIZE_DEVPOS];
	WORD	wMap;
} DEVPOS, *PDEVPOS;

typedef struct _tagDEVCONF {
	DEVPOS	siv;
	DEVPOS	v3f;
	DEVPOS	pan;
	DEVPOS	esk;
} DEVCONF, *PDEVCONF;

enum enDEVSTATE {
	DEVSTATE_BYPASS = 4,
	DEVSTATE_FAIL,
	DEVSTATE_BLINDUNIT,
	DEVSTATE_BLINDDTB
};

enum enDOORSTATE {
	DOORSTATE_OPEN = 0,
	DOORSTATE_OBSTACLE,
	DOORSTATE_EMERGENCYHANDLE,
	DOORSTATE_MAJF
};

enum enPOPINDEX {
	POPI_SEND = 0,
	POPI_WORK,
	POPI_HEAD,		// 181012, use to slave, storage broadcast
	POPI_MAX
};

// 200716
//#define	IsFreeze(x)		(x * ((1 << DEVSTATE_BLINDDTB) | (1 << DEVSTATE_BLINDUNIT)))
#define	IsFreeze(x)		(x & ((1 << DEVSTATE_BLINDDTB) | (1 << DEVSTATE_BLINDUNIT)))

// communication buffer usage
enum enREQUESTSERVICEID {
	REQ_NORMAL = 0,		// current
	REQ_TRACE,			// trace buffer
	REQ_BACKUP,			// backup for de-bounce, cause communication fault
	REQ_REPORT,			// for debug to monit
	REQ_MAX
};

enum enANSWERSERTVICEID {
	ANS_NORMAL = 0,
	ANS_TRACE,
	ANS_BACKUP,
	ANS_REPORT,
	ANS_MAX
};

enum enTROUBLEFILMID {
	TFILMID_MM = 0,
	TFILMID_RTD,
	TFILMID_MAX
};

enum enSPARKLE {
	SPARKLE_TROUBLEMM = TFILMID_MM,
	SPARKLE_TROUBLERTD = TFILMID_RTD,
	SPARKLE_LOGBOOK = TFILMID_MAX,
	SPARKLE_MAX
};

typedef struct _tagSPARKLE {
	bool	bFlash;
	bool	bCounter;
	WORD	wCounter;
} SPARKLE;
#define	TPERIOD_SPARKLE		REAL2PERIOD(1000)

typedef struct _tagLINEFABRIC {
	union {
		int		fd;			// for /dev/ser1...5
		WORD	wAddr;
	} domi;
	int		iCh;
	bool	bAddrMatch;
	DWORD	dwBps;
	struct {
		WORD	wResp;
		WORD	wCycle;
	} t;
} LFABRIC, *PLFABRIC;
