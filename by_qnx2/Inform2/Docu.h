/**
 * @file	Docu.h
 * @brief	운행 기록, 고장 기록 등의 양식
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"
#include "DevInfo2/DevInfo.h"
#include "Draft.h"

/* 저장 공간 /h/sd/arch...
 디렉토리 이름은 "dyymmdd"(d-년도-월-일)로 한다.
 파일이름은 "*hhmmss"(*-시-분-초)로 한다.
 *는 운행 기록에서는		'l',
     고장 기록에서는		't',
     고장 추적 기록에서는
     				siv는	's',
     				v3f는	'v',
     				ecu는	'e'
     시운전 기록에서는		'r'로 각각 표시하며
 고장 추적과 시운전 기록(파일 이름)은 각각의 시작 시간을 사용하여 일일이 시간대를 맞출 필요는 없으나
 운행 기록과 고장 기록(파일 이름)은 시간대를 맞추어 서로 참조가 가능하도록 한다.
 운행 기록과 고장 기록은 F/R이 투입되는 시간을 파일 이름으로 하고 전원이 차단될 때까지,
 또는 F/R이 재 투입될 때까지 같은 파일에 기록을 남긴다.
*/

#pragma pack(push, 1)

///	*** 운행 기록 양식 ***
// 파일 첫 부분에 열차 번호와 편성 번호, 길이등은 따로 남기고
// 운행 기록을 시작한다.

#define	MAX_FILEDATE		991231				// 파일 이름으로 남길 수 있는 최대치

//						 	 012345678901
#define	TITLE_LOGBOOK		"TCMS LOGBOOK"
// 180511
#define	LOGBOOKCTRLSIDE_VERSION	0x207			//
// 170911
#define	LOGBOOKCSUM_VERSION	0x203				// 이 번호부터 체크섬이 포함된다는 것이므로 수정하면 안된다.
// 171114
//#define LOGBOOK_VERSION		0x203				// 170620,	0x102					// 17/06/07,	0x101
// 171123
//#define LOGBOOK_VERSION		0x204				// siv io byte->word
//#define LOGBOOK_VERSION		0x205				// vvvf revive status
// 180219
//#define	LOGBOOK_VERSION		0x206				// wRealPwm은 비교나 환산하지 않은 엔코더 출력 값.
// 180828
#define	LOGBOOK_VERSION		0x207			// v3f.cBed삭제, pis notr, station code 추가

// 17/06/20
// OBLIGE가 기록되지 않아서 각 TC의 정확한 동작을 파악하기 힘들어서 추가한다.
// 늘어난 Paper Command를 추가한다.
// pres[PID_MAX]는 삭제한다.

typedef struct _tagLOGBOOKHEAD {
	BYTE	title[12];		// 0:11
	WORD	wVer;			// 12:13
	WORD	wPermNo;		// 14:15, 2Xnn
	BYTE	cLength;		// 16:0
	BYTE	cRate;			// 17:0
	//BYTE	cDummy[14];		// 18:31
	// 17/05/29
	// 디렉토리와 파일이름을 각각의 시간에 의존하다보니 1~2초 차이가 생긴다.
	// 헤드에 DEVTIME을 추가하여 마스터의 시간에 맞추도록 한다.
	DEVTIME	devt;			// 18:23
	// 171125
	//BYTE	cTimeValid;		// 24:0		// 안쓴다. 0x5a, 고정 값: dt가 유효함을 나타낸다. mm에서 이 값이 5a이면 dt를 참조하고 아니면 기존대로 한다.
	BYTE	cAux;			// 24:		// 보조 TC
	BYTE	cDummy[7];		// 25:31
} LOGBOOKHEAD;

#define	MAX_DTLYEAR			74
#define	MAX_DTLYEAR4		(MAX_DTLYEAR + 2000)
#define	MIN_DTLYEAR			10
#define	MIN_DTLYEAR4		(MIN_DTLYEAR + 2000)

typedef struct _tagTCSTATUSCMD {	// draft.h - SIGNATURE를 참조할 것.
	BYTE	gcmd;				//  6, 11, 16, 21
	BYTE	ecmd;				//  7, 12, 17, 22
	BYTE	scmd;				//  8, 13, 18, 23
	BYTE	hcmd;				//  9, 14, 19, 24
	BYTE	hcmdEx;				// 10, 15, 20, 25
} TCSCMD;

// 170911
typedef struct _tagLOGCONTENTS {
	WORD	wSeq;				// 000:2, Sequence no
	DBFTIME	dbft;				// 002:4
	TCSCMD	tsc[LENGTH_TU];		// 006:20-5*4
	MOTIVE	motiv;				// 026:5, 026:6, recip[FID_HEAD].real.motiv
	__BC(	oppoLead,			// 후미차 pis 정보
				f,		1,			// 정 방향, 여기서는 의미 없음
				r,		1,			// 역 방향, 여기서는 의미 없음
				atom,	1,			// , 여기서는 의미 없음
				pism,	1,			// pis manual mode
				piss,	1,			// pis auto/manual status
				pisd,	1,			// manual from DU(0) / from pis(1)
				pisa,	1,			// pis trigger ack.
				pist,	1);			// pis auto/manual trigger
	DRIFT	drift;				// 032:6, recip[FID_HEAD].real.drift
	_OCTET	tui[LENGTH_TU][2];	// 038:64-c[8]-[4][2]
	BYTE	tuo[LENGTH_TU];		// 102:4
	_ANALOGV	avs[LENGTH_TU];	// 106:48-12*4
	_OCTET	cui[LENGTH_CU];		// 154:64-c[8]-[8]
	BYTE	cuo[LENGTH_CU];		// 218:8
	BYTE	ato[sizeof(SAATO)];	// 226:18, without crc
	WORD	wOtrn;				// 244:2, train number on the rail
	WORD	wRealPwm;			// 246:2, 180219	// wLogicPwm;			// 246:2,
	BYTE	cPoleLine[2];		// 248:2	// 488:2,
								// [0]은 head service device 통신 상태, [1]은 tail service device 통신상태
								//	b0:ato, b3:pau, b4:pis, b5:trs, 각각 1이면 정상 상태.
	BYTE	cSvcCtrlCode[2];	// 250:2,	//486:2
								// [0]은 head에서 판단하는 서비스 장치 제어코드, [1]은 tail에서...
								// b1-0:head sevice device code, b3-2:tail service device code
	DWORD	dwAvrSpeed;			// 252:4, average atc speed
	QWORD	qwDistance;			// 256:8, *****
	WORD	wSegment;			// 264:2, WORD wSegmentDistance, 역간 거리
	struct {
		BYTE	cEs;			// 266:1, 271:1, 276:1, siva.cEs
		BYTE	cVo;			// 267:1, 272:1, 277:1, siva.cVo
		BYTE	cFo;			// 268:1, 273:1, 278:1, siva.cFo
		WORD	wIo;			// 269:2, 274:2, 279:2, siva.ste.b.iom + siva.cIo
	} siv[SIV_MAX];				// 266~280:15-5*3
	// 180828
	//struct {
	//	BYTE	cIq;			// v3fa.cIq
	//	BYTE	cLv;			// v3fa.cLv
	//	BYTE	cTbe;
	//	BYTE	cBed;			//
	//	short	iApbr;			// v3fa.wApbr, note! big_endian -> little_endian
	//	WORD	wFc;			// v3fa.wFc, note! big_endian -> little_endian
	//} v3f[V3F_MAX];				// 281~320:40-8*5
	struct {
		BYTE	cIq;			// v3fa.cIq
		BYTE	cLv;			// v3fa.cLv
		BYTE	cTbe;			// v3fa.cTbe
		short	iApbr;			// v3fa.wApbr
		WORD	wFc;			// v3fa.wFc
	} v3f[V3F_MAX];				// 281~315
	struct {
		BYTE	cRef;			// 316:1,	pisa.ref.a
		BYTE	cCurrSt;		// 317:1,	pisa.cCurrSt
		BYTE	cNextSt;		// 318:1,	pisa.cNextSt
		WORD	wNotr;			// 319:2,	pisa.notr.a, wOtrn과 중복되는 것도 같다, 후에 더 필요한 데이터로 대체할 것.
	} pis;						//
	// 180511
	//WORD	wCtrlSide;			// 316:2
	//BYTE	cSpr318[3];			// 318:3
	struct {
		BYTE	cBe;
		BYTE	cAsp;			// ecua.cAsp
		BYTE	cLw;
		BYTE	cBcp;			// ecua.cbsp
		BYTE	cRef;			// BYTE cEcuRef[10];
	} ecu[ECU_MAX];				// 321~370:50-5 * 10
	__BC(	v3fst[V3F_MAX],		// 371~375
				regp,	1,
				pwrp,	1,
				ccos,	1,
				act,	1,
				cdron,	1,
				fout,	1,
				rgoff,	1,
				poff,	1);
	BYTE	cProperID;			// 171208, 376
	BYTE	cReason;			// 171212, 377
	//typedef struct _tagDEVPOWERS {
	//	QWORD	qwSivPower[SIV_MAX];	// 378:8, 386:8, 394:8
	//	QWORD	qwV3fPower[V3F_MAX];	// 402:8, 410:8, 418:8, 426:8, 434:8
	//	QWORD	qwV3fReviv[V3F_MAX];	// 442:8, 450:8, 458:8, 466:8, 474:8
	//} DEVPOWERS, *PDEVPOWERS;
	DEVPOWERS	dps;			// 378:104, *****
	DWORD	dwRoute;			// 482:4, DTB 통신 상태(b0:head DTB-A, b1:head DTB-B, b2:tail DTB-A, b3:tail DTB-B, b4:CC1 DTB-A, b5:CC1 DTB-B ~~~
	CMJOGTIMES	cjt;			// 486:16, *****
	// 170911
	WORD	wCSum;				// 502:2,
	// 체크섬으로서 각 블럭의 유효성을 검사한다. 전원 투입 후 거리, 전력등의 정보를 취할 때 유용하다.
} LOGCONTENTS;					// 504

// ***** absolute position

typedef union _tagLOGINFO {
	LOGCONTENTS	r;
	BYTE		c[sizeof(LOGCONTENTS)];
} LOGINFO, *PLOGINFO;

#if	!defined(SIZE_LOGBOOK)
#define	SIZE_LOGBOOK	504		// RTDBRAND와 같은 크기로 할것!
#endif

typedef struct _tagLOGBOOK {
	LOGINFO	real;
	BYTE	cDummy[SIZE_LOGBOOK - sizeof(LOGCONTENTS)];
} LOGBOOK, *PLOGBOOK;

///	*** 고장 기록 양식 ***
typedef union _tagTROUBLEINFO {
	struct {
		BYTE	down	:  1;	// 지워진..
		// 171107
		//BYTE	alarm	:  1;	// type중에서 사용자가 참조하는 고장일 때 1이 된다. 코더의 고장은 감춘다.
		BYTE	saw		:  1;	// 사용자의 확인을 거친 코드, 고장 표시줄에 표시하지않는다.
		BYTE	type	:  4;	// see ALARMTYPE enum, 없앨 수 있지 않을까?
		BYTE	cid		:  4;	// 0~9, 객차 번호
		WORD	code	: 11;	// 고장 코드, 0x7ff-2047
		BYTE	mon		:  4;	// 0~11, 발생한 달(1~12)
		BYTE	day		:  5;	// 0~30, ...    날(1~31)
		BYTE	uhour	:  5;	// 0~23, ...    시(0~23)
		BYTE	umin	:  6;	// 0~59, ...    분(0~59)
		BYTE	usec	:  6;	// 0~59, ...    초(0~59)
		BYTE	dhour	:  5;	// 0~23, 소멸한 시(0~23)
		BYTE	dmin	:  6;	// 0~59, ...    분(0~59)
		BYTE	dsec	:  6;	// 0~59, ...    초(0~59)
	} b;
	QWORD	a;
} TROUBLEINFO, *PTROUBLEINFO;

#define	MAX_TROUBLE			1024		// 2048
#define	SIZE_TROUBLEINFO	4194304		// 4M * 8 = 32Mbytes, 0x400000
#define	TCODE_ONLYTU		200

enum enTROUBLETYPE {
	TROUBLE_NON = 0,
	// 180911
	//TROUBLE_TEXT,		// 001
	TROUBLE_EVENT,		// 001	// 010
	TROUBLE_NOTICE,		// 010
	TROUBLE_STATUS,		// 011
	TROUBLE_WARNING,	// 100
	TROUBLE_LIGHT,		// 101
	TROUBLE_SEVERE,		// 110
	TROUBLE_EMERGENCY	// 111
	// 171102
	//TROUBLE_PLATE		// 1000
};
#define	TROUBLETYPE_ONLY	7

#define	IsSevereType(x)		((x & TROUBLETYPE_ONLY) == TROUBLE_SEVERE)
#define	IsTroubleType(x)	((x & TROUBLETYPE_ONLY) > TROUBLE_STATUS)
#define	IsStatusType(x)		((x & TROUBLETYPE_ONLY) == TROUBLE_STATUS)
#define	IsListedType(x)		((x & TROUBLETYPE_ONLY) >= TROUBLE_STATUS)
// 180129
//#define	IsTuCode(x)			((x > 20 && x < 200) || (x >= 660 && x < 850))
#define	IsTuCode(x)			((x != 11 && x != 13 && x != 19 && x < 200) || (x >= 660 && x < 850))

#define	TROUBLEBUFINDEX_BEGIN	1	// 첫번째는 비운다. 부호가 없다보니 0으로 하면 반복되는 현상이 생길 수도...
#define	TROUBLELIST_LINE		12

typedef union _tagTROUBLECODE {
	struct {
		WORD	code	: 11;
		BYTE	spr		:  1;
		BYTE	cid		:  4;
	} b;
	WORD	a;
} TROUBLECODE, *PTROUBLECODE;	// for coding

//typedef struct _tagTROUBLEREFERENCE {
//	DWORD	dwIndex;
//	DWORD	dwReduce;
//	DWORD	dwSevere;
//	DWORD	dwCrash;
//	DWORD	dwState;
//	DWORD	dwOther;
//	//WORD	wRecentCode;
//	//WORD	wRecentCid;
//	TROUBLEINFO	cellar[SIZE_TROUBLEINFO];
//} TROUBLEREF, *PTROUBLEREF;		// for coding

enum {
	TTRAY_TROUBLE,
	TTRAY_STATUS,
	TTRAY_OTHER,
	TTRAY_MAX
};

typedef struct _tagTROUBLECATALOG {
	DWORD	dwIndex;
	DWORD	dwReduce;
	DWORD	dwHeavy;
	TROUBLEINFO	cellar[SIZE_TROUBLEINFO];
} TROUBLETRAY, *PTROUBLETRAY;

#define	MAX_OLDTROUBLE		100
#define	MAX_TROUBLELIST		1024
#define	SIZE_TROUBLELIST	(DEFAULT_CARLENGTH + 2)	// 객차별 고장 목록 수, tu는 따로...

typedef struct _tagTROUBLEBYCAR {
	WORD		wMin;			// MAX_OLDTROUBLE - old trouble size
	WORD		wSlash;			// revoke limit
	WORD		wCur;			// increase from MAX_OLDTROUBLE
	TROUBLEINFO	ts[MAX_TROUBLELIST];
} TROUBLEBYCAR, *PTROUBLEBYCAR;		// 객차별 고장 목록

// 190923
//typedef struct _tagOLDTROUBLELIST {
//	short		iLength[SIZE_TROUBLELIST];
//	TROUBLEINFO	ts[SIZE_TROUBLELIST][MAX_OLDTROUBLE];
//} OLDTROUBLELIST, *POLDTROUBLELIST;
//
typedef struct _tagOLDTROUBLES {
	short		iSize;
	TROUBLEINFO	ls[MAX_OLDTROUBLE];
} OLDTROUBLES;

typedef struct _tagOLDTROUBLELIST {
	WORD		wFileCount;
	OLDTROUBLES	s[SIZE_TROUBLELIST];
} OLDTROUBLELIST, *POLDTROUBLELIST;

///	**** CDMA 자료 양식 ***

// 누적 주행 거리...
// 100Km/h * 24시간 = 2400Km, 하루 기록되는 양이 2400(0x960)일 때
// DWORD 최대치는 4294967295(0xffffffff)이므로
// 4294967295 / 2400 = 1789596(일) -> / 365 = 4902(년)
// 그래서 km단위에선 DWORD 32비트로도 충분하다.

typedef struct _tagCDMACONTENTS {
	DBFTIME	dbft;
	BYTE	cProperNo;			// 004:1, 편성 번호
	__BC(	cab,				// 005:1
				ldl,	1,			// 왼쪽 위치에서 왼쪽 방향		(forward)
				rdl,	1,			// 왼쪽 위치에서 오른쪽 방향	(reverse)
				rdr,	1,			// 오른쪽 위치에서 오른쪽 방향	(forward)
				ldr,	1,			// 오른쪽 위치에서 왼쪽 방향	(reverse)
				lm,		1,			// 왼쪽이 마스터
				rm,		1,			// 오른쪽이 마스터
				spr,	2);
	__BC(	lamp,				// 006:1
				pb,		1,			// 주차 제동 등
				sb,		1,			// 보안 제동 등
				dir,	1,			// 17/11/27, TUDIB_DIR
				eb,		1,			// 비상 제동 등
				psdc,	1,			// psd close
				adbs,	1,			// all door bypass
				fire,	1,			// ! 화재 등 ???
				err,	1);			// ! 경보 등 ???
	BYTE	cMode;				// 007:1, 운행 방식
	BYTE	cPowerRate;			// 008:1, 0~100%
	BYTE	cBrakeRate;			// 009:1, 0~100%
	BYTE	cNextStation;		// 010:1, 다음 역
	BYTE	cDestStation;		// 011:1, 종착 역
	WORD	wCurSpeed;			// 012:2, 속도 (km/h)
	DWORD	dwDistance;			// 014:4, 누적 주행 거리(km)
	WORD	wOtrn;				// 018:2, 열차 번호
	BYTE	cDoor[CID_MAX];		// 020:10, 0~9호차 출입문 상태, 0:닫힘/1:열림
	BYTE	cDbp;				// 030:1, 0~7호차 출입문 바이패스 상태, 0:정상/1:바이패스
	BYTE	cDeh;				// 031:1, 0~7호차 출입문 비상 핸들 상태, 0:정상/1:비상
	__BC(	dbeh,				// 032:1
				bp8,	1,			// 8호차 바이패스 상태
				bp9,	1,			// 9호차 바이패스 상태
				eh8,	1,			// 8호차 비상 핸들 상태
				eh9,	1,			// 9호차 비상 핸들 상태
				spr4,	4);
	__BC(	panto,				// 033:1
				c2,		2,			// 2호차 팬터그래프 상태
				c4,		2,			// 4호차 팬터그래프 상태
				c8,		2,			// 8호차 팬터그래프 상태
				spr,	2);
	__BW(	svf,				// 034:2
				siv0,	2,			// 0호차 siv 상태, 0:NON/1:정상/2:통신/3:고장
				siv5,	2,			// 5호차 siv 상태
				siv9,	2,			// 9호차 siv 상태
				v3f1,	2,			// 1호차 v3f 상태
				v3f2,	2,			// 2호차 v3f 상태
				v3f4,	2,			// 4호차 v3f 상태
				v3f7,	2,			// 7호차 v3f 상태
				v3f8,	2);			// 8호차 v3f 상태
	WORD	wEcu;				// 036:2, 0~7호차 ecu 상태
	__BW(	etc,				// 038:2
				ecu8,	2,			// 8호차 ecu 상태
				ecu9,	2,			// 9호차 ecu 상태
				cm0,	2,			// 0호차 cmsb 상태, 0:NON/1:정지/2:가동/3:바이패스
				cm5,	2,			// 5호차 cmsb 상태
				cm9,	2,			// 9호차 cmsb 상태
				ato0,	2,			// 0호차 ato 상태
				ato9,	2,			// 9호차 ato 상태
				esk3,	1,			// 3호차 esk 상태, 0:OFF/1:ON
				esk6,	1);			// 6호차 esk 상태
	WORD	wFire;				// 040:2, 0~7호차 화재 상태, 0:정상/1:화재/2:고장
	__BW(	fph,				// 042:2
				f8,		2,			// 8호차 화재 상태
				f9,		2,			// 9호차 화재 상태
				p0,		1,			// 0호차 객실 폰 상태, 0:정상/1:호촐
				p1,		1,
				p2,		1,
				p3,		1,
				p4,		1,
				p5,		1,
				p6,		1,
				p7,		1,
				p8,		1,
				p9,		1,			// 9호차 객실 폰 상태
				spr,	2);

	// 베이스 110 서비스 기기
	__BW(	svc,				// 044:2,
				pau0,	2,			// 0호차 pau 상태, 0:NON/1:정상/2:통신/3:고장
				pau9,	2,			// 9호차 pau 상태
				pis0,	2,			// 0호차 pis 상태
				pis9,	2,			// 9호차 pis 상태
				rtd0,	2,			// 0호차 rtd 상태
				rtd9,	2,			// 9호차 rtd 상태
				trs0,	2,			// 0호차 trs 상태
				trs9,	2);			// 9호차 trs 상태
	__BC(	tus,				// 046:1
				liu01,	1,			// 0호차 liu1 상태, 0:정상/1통신
				liu02,	1,			// 0호차 liu2 상태
				liu91,	1,			// 9호차 liu1 상태
				liu92,	1,			// 9호차 liu2 상태
				spr,	4);
	BYTE	cCus;				// 047:1, 1~8호차 cc 상태, 0:정상/1통신

	// 베이스 108 고압/제동
	BYTE	cBc;				// 048:1, 0~7호차 bc 상태, 0:off/1:on
	__BC(	hbc,				// 049:1
				hb1,	1,			// 1호차 v3f hb 상태
				hb2,	1,			// 2호차 v3f hb 상태
				hb4,	1,			// 4호차 v3f hb 상태
				hb7,	1,			// 7호차 v3f hb 상태
				hb8,	1,			// 8호차 v3f hb 상태
				bc8,	1,			// 8호차 bc 상태
				bc9,	1,			// 9호차 bc 상태
				spr,	1);
	BYTE	cAs;				// 050:1, 0~7호차 as 상태
	__BC(	lba,				// 051:1
				lb1,	1,			// 1호차 v3f lb 상태
				lb2,	1,			// 2호차 v3f lb 상태
				lb4,	1,			// 4호차 v3f lb 상태
				lb7,	1,			// 7호차 v3f lb 상태
				lb8,	1,			// 8호차 v3f lb 상태
				as8,	1,			// 8호차 as 상태
				as9,	1,			// 9호차 as 상태
				spr,	1);
	BYTE	cV3fIq[5];			// 052:5, v3f "모터 전류", 추진 장치 Im항목과 같다.
	BYTE	cSpr57;				// 057:1
	BYTE	cEcuBcp[ECU_MAX];	// 058:10, ecu "bc 압"

	// 베이스 120 추진 장치
	WORD	wV3fFc[V3F_MAX];	// 068:10, v3f "fc" 전압
	__BW(	v3fd,				// 078:2
				d1,		2,			// 1호차 v3f, 0:C/1:NON/2:P/3:B
				d2,		2,			// 2호차 v3f
				d4,		2,			// 4호차 v3f
				d7,		2,			// 7호차 v3f
				d8,		2,			// 8호차 v3f
				spr,	6);
	BYTE	cV3fTbe[V3F_MAX];	// 080:5, v3f "pwm"

	// 베이스 122 보조 전원
	BYTE	cSivVo[SIV_MAX];	// 085:3, siv "출력 전압"
	// 171114
	//BYTE	cSivIo[SIV_MAX];	// 088:3, siv "출력 전류"
	BYTE	cSivNull[SIV_MAX];
	BYTE	cSivFo[SIV_MAX];	// 091:3, siv "출력 주파수"
	WORD	wBatV[SIV_MAX];		// 094:6, "배터리 전압"

	// 베이스 124 제동
	WORD	wBcv[ECU_MAX];		// 100:20, "BC 압"
	WORD	wV3fAbr[V3F_MAX];	// 120:10, "회생 제동력"
	BYTE	cEcuAsp[ECU_MAX];	// 130:10, ecu "as 압"

	// 베이스 126 냉난방
	WORD	wHeat;				// 140:2, 0~7호차 난방, 0:non/1:1/2:2/3:3
	__BW(	hf,					// 142:2
				h8,		2,			// 8호차 난방
				h9,		2,			// 9호차 난방
				f0,		1,			// 0호차 송풍, 0:off/1:on
				f1,		1,
				f2,		1,
				f3,		1,
				f4,		1,
				f5,		1,
				f6,		1,
				f7,		1,
				f8,		1,
				f9,		1,			// 9호차 송풍, 0:off/1:on
				spr,	2);
	__BW(	fan2,				// 144:2, lffk2
				f0,		1,
				f1,		1,
				f2,		1,
				f3,		1,
				f4,		1,
				f5,		1,
				f6,		1,
				f7,		1,
				f8,		1,
				f9,		1,
				spr,	6);
	WORD	wCool1;				// 146:2, 0~7호차 냉방기 1, 0:non/1:fan/2:half/3:full
	__BW(	cov,				// 148:2
				c8,		2,			// 8호차 냉방기 1
				c9,		2,			// 9호차 냉방기 1
				v0,		1,			// 0호차 환기, 0:off/1:on
				v1,		1,
				v2,		1,
				v3,		1,
				v4,		1,
				v5,		1,
				v6,		1,
				v7,		1,
				v8,		1,
				v9,		1,			// 9호차 환기, 0:off/1:on
				spr,	2);
	WORD	wCool2;				// 150:2, 0~7호차 냉방기 2, 0:non/1:fan/2:half/3:full
	__BW(	ctr,				// 152:2
				c8,		2,			// 8호차 냉방기 2
				c9,		2,			// 9호차 냉방기 2
				r0,		1,			// 0호차 정화기, 0:off/1:on
				r1,		1,
				r2,		1,
				r3,		1,
				r4,		1,
				r5,		1,
				r6,		1,
				r7,		1,
				r8,		1,
				r9,		1,			// 9호차 정화기, 0:off/1:on
				spr,	2);
	//BYTE	cLoadr[CID_MAX];	// 154:10, 탑승율 -> cEcuAsp로 계산할 것
	BYTE	cIntTemp[CID_MAX];	// 154:10, 내부 온도
	WORD	wExtTemp[2];		// 164:4, 외부 온도

	// 베이스 128 등구류
	BYTE	cAc1;				// 168:1,
	BYTE	cAc2;				// 169:1
	BYTE	cDc;				// 170:1
	__BC(	rem,				// 171:1
				ac18,	1,
				ac19,	1,
				ac28,	1,
				ac29,	1,
				dc8,	1,
				dc9,	1,
				spr,	1);
	BYTE	cLength;			// 172:1, 열차 길이

	// 170929, 추가
	BYTE	cSivEs[SIV_MAX];
	BYTE	cEcuLw[ECU_MAX];
	// 171114
	WORD	wSivIo[SIV_MAX];	// siv "출력 전류"
WORD	wCode;
} CDMACONTENTS;

#define	CDMA_VERIFYCODE		0x5a37

#define	SIZE_CDMAREPORT		200	// 170929, 180
typedef struct _tagCDMAREPORT {
	CDMACONTENTS	real;
	BYTE			cDummy[SIZE_CDMAREPORT - sizeof(CDMACONTENTS)];
} CDMAREPORT, *PCDMAREPORT;

///	**** Inspect report ***

typedef struct _tagINSPECTREPORT {
	WORD	wItem;			//
	DBFTIME	dbft;			// 시작 시간
	WORD	wSubItem;		//
	WORD	wMsg;
	WORD	wRequired;		// 소요 시간, 초
	//WORD	wProperNo;		// 004:1, 편성 번호
	INSPECTCOLUMNS	ics;
	ECUINSPECTCOL	ecu[4];
	WORD	wEcui;
} INSPREPORT, *PINSPREPORT;				// 126 bytes ???

///	**** acceleration/deceleration ***
// 180405

typedef struct _tagTIMERANGE {
	uint64_t	begin;
	double		dbCurrent;
} TIMERANGE;

typedef struct _tagDISTANCERANGE {
	double	dbBegin;
	double	dbCurrent;
} DISTANCERANGE;

typedef struct _tagRANGEPATTERN {
	bool			bTrigger;
	TIMERANGE		time;			// reset !TUDIB_POWERING, only TUDIB_POWERING
	DISTANCERANGE	distance;		// reset !TUDIB_BRAKING, only TUDIB_BRAKING
} RANGEPATTERN;

#define	SIZE_SPEEDSAMPLE	10
typedef struct _tagSPEEDCALCM {
	WORD			wIndex;
	DWORD			dws[SIZE_SPEEDSAMPLE];
} SPEEDCALCM;

#define	MOMENTUM_CYCLE		5
typedef struct _tagMOMENTUMINFO {
	bool			bInit;
	RANGEPATTERN	power;
	RANGEPATTERN	brake;
	short			vMomentum;
	DWORD			dwSpeed;
	DWORD			dwDistance;
	double			dbBeginDistance;
	uint64_t		clk;
	SPEEDCALCM		speed;
} MOMENTUMINFO, *PMOMENTUMINFO;

// 190523
// REPORT FORM
//	TC0[114]
//		pau[4]		// 4	0~3
//		pis[27]		// 31	4~30
//		trs[4]		// 35	31~34
//		siv[14]		// 49	35~48
//		dcu[8][4]	// 81	49~80
//		ecu[15]		// 96	81~95
//		hvac[18]	// 114	96~113
//	CC1[95]114
//		v3f[30]		// 144	114~143
//		dcu[8][4]	// 176	144~175
//		ecu[15]		// 191	176~190
//		hvac[18]	// 209	191~208
//	CC2[95]209
//		v3f[30]		// 239	209~238
//		dcu[8][4]	// 271	239~270
//		ecu[15]		// 286	271~285
//		hvac[18]	// 304	286~303
//	CC3[65]304
//		dcu[8][4]	// 336	304~335
//		ecu[15]		// 351	336~350
//		hvac[18]	// 369	351~368
//	CC4[95]369
//		v3f[30]		// 399	369~398
//		dcu[8][4]	// 431	399~430
//		ecu[15]		// 446	431~445
//		hvac[18]	// 464	446~463
//	CC5[79]464
//		siv[14]		// 478	464~477
//		dcu[8][4]	// 510	478~509
//		ecu[15]		// 525	510~524
//		hvac[18]	// 543	525~542
//	CC6[65]543
//		dcu[8][4]	// 575	543~574
//		ecu[15]		// 590	575~589
//		hvac[18]	// 608	590~607
//	CC7[95]608
//		v3f[30]		// 638	608~637
//		dcu[8][4]	// 670	638~669
//		ecu[15]		// 685	670~684
//		hvac[18]	// 703	685~702
//	CC8[95]703
//		v3f[30]		// 733	703~732
//		dcu[8][4]	// 765	733~764
//		ecu[15]		// 780	765~779
//		hvac[18]	// 798	780~797
//	TC9[114]798
//		pau[4]		// 802	798~801
//		pis[27]		// 829	802~828
//		trs[4]		// 833	829~832
//		siv[14]		// 847	833~846
//		dcu[8][4]	// 879	847~878
//		ecu[15]		// 894	879~893
//		hvac[18]	// 912	894~911

enum enSIZEFSDAPAU		{	SIZE_FSDAPAU = 4	};
enum enSIZEFSDAPIS		{	SIZE_FSDAPIS = 27	};
enum enSIZEFSDATRS		{	SIZE_FSDATRS = 4	};
enum enSIZEFSDASIV		{	SIZE_FSDASIV = 14	};
enum enSIZEFSDASIV2		{	SIZE_FSDASIV2 = 17	};
enum enSIZEFSDAV3F		{	SIZE_FSDAV3F = 30	};
enum enSIZEFSDAECU		{	SIZE_FSDAECU = 15	};
enum enSIZEFSDAHVAC		{	SIZE_FSDAHVAC = 18	};
enum enSIZEFSDADCU		{	SIZE_FSDADCU = 4	};		// 19/12/11	//5	};

typedef struct _tagFSDAVS {
	BYTE	pau[SIZE_FSDAPAU];
	BYTE	pis[SIZE_FSDAPIS];
	BYTE	trs[SIZE_FSDATRS];
} FSDAVS, *PFSDAAVS;

// 200218
//typedef struct _tagFSDASIV {
//	BYTE	siv[SIZE_FSDASIV];
//} FSDASIV, *PFSDASIV;
//
//typedef union _tagFSDASIV {
//	BYTE	siv[SIZE_FSDASIV];
//	BYTE	siv2[SIZE_FSDASIV2];	// at 10car, must remove this line!!!!!!!
//} FSDASIV, *PFSDASIV;
//
typedef struct _tagFSDASIV {
	BYTE	siv[SIZE_FSDASIV];
} FSDASIV, *PFSDASIV;

typedef struct _tagFSDASIV2 {
	BYTE	siv[SIZE_FSDASIV2];
} FSDASIV2, *PFSDASIV2;

typedef struct _tagFSDAV3F {
	BYTE	v3f[SIZE_FSDAV3F];
} FSDAV3F, *PFSDAV3F;

typedef struct _tagFSDAUNIV {
	BYTE	dcu[8][SIZE_FSDADCU];
	BYTE	ecu[SIZE_FSDAECU];
	BYTE	hvac[SIZE_FSDAHVAC];
} FSDAUNIV, *PFSDAUNIV;

// 200218
typedef struct _tagFSDASETT {
	struct {
		FSDAVS		h;
		FSDASIV		s;
		FSDAUNIV	u;
	} tc0;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc1;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc2;
	struct {
		FSDAUNIV	u;
	} cc3;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc4;
	struct {
		FSDASIV		s;
		FSDAUNIV	u;
	} cc5;
	struct {
		FSDAUNIV	u;
	} cc6;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc7;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc8;
	struct {
		FSDAVS		h;
		FSDASIV		s;
		FSDAUNIV	u;
	} tc9;
} FSDASETT, *PFSDASETT;		// 10

typedef struct _tagFSDASETE {
	struct {
		FSDAVS		h;
		FSDASIV		s;
		FSDAUNIV	u;
	} tc0;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc1;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc2;
	struct {
		FSDAUNIV	u;
	} cc3;
	struct {
		FSDAUNIV	u;
	} cc4;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc5;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc6;
	struct {
		FSDAVS		h;
		FSDASIV		s;
		FSDAUNIV	u;
	} tc7;
} FSDASETE, *PFSDASETE;		// 8:

typedef struct _tagFSDASETS {
	struct {
		FSDAVS		h;
		FSDASIV2	s;		// !!!!!
		FSDAUNIV	u;
	} tc0;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc1;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc2;
	struct {
		FSDAUNIV	u;
	} cc3;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc4;
	struct {
		FSDAVS		h;
		FSDASIV2	s;		// !!!!!
		FSDAUNIV	u;
	} tc5;
} FSDASETS, *PFSDASETS;		// 6:

typedef struct _tagFSDASETF {
	struct {
		FSDAVS		h;
		FSDASIV		s;
		FSDAUNIV	u;
	} tc0;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
	} cc1;
	struct {
		FSDAV3F		v;
		FSDAUNIV	u;
} cc2;
	struct {
		FSDAVS		h;
		FSDASIV		s;
		FSDAUNIV	u;
	} tc3;
} FSDASETF, *PFSDASETF;		// 4:

#define	SIZE_FULLSDASET		sizeof(FSDASETT)		// 912
//#if	(SIZE_FULLSDASET % 2)
//#error	"size fail full sda set"
//#endif

#define	SIZE_FULLSDABLOCK	(SIZE_FULLSDASET / 2)	// 456

// 200218
typedef union _tagFSDASETA {
	FSDASETT	x;
	FSDASETE	vi3;
	FSDASETS	vi;
	FSDASETF	iv;
} FSDASETA, *PFSDASETA;

typedef union _tagFSDATONE {
	FSDASETA	fss;
	BYTE		cbs[2][SIZE_FULLSDABLOCK];
} FSDATONE, *PFSDATONE;

#define	FULLSDA_TEXT		"TCMS FULL SDA SET #x             "		// 14byte + 2byte
//							 012345678901234567890123456789012
#define	SIZE_FULLSDATEXT	32

typedef struct _tagFULLSDAREPORT {
	BYTE	cHead[SIZE_FULLSDATEXT];
	BYTE	cContext[SIZE_FULLSDABLOCK];
} FULLSDAREPORT;

#pragma pack(pop)
