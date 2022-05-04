/**
 * @file	HduInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../Arrange.h"
#include "../../../../Common/VarUnion.h"

#define	XWORD(x)	((((x) & 0xff00) >> 8) | (((x) & 0xff) << 8))
//#define	XDWORD(x)	((((x) & 0xff000000) >> 8) | (((x) & 0xff0000) << 8) | (((x) & 0xff00) >> 8) | (((x) & 0xff) << 8))
#define	XDWORD(x)	((((x) & 0xff000000) >> 24) | (((x) & 0xff0000) >> 8) | (((x) & 0xff00) << 8) | (((x) & 0xff) << 24))
#define	GLSB(x)		((BYTE)(x & 0xff))
#define	GMSB(x)		((BYTE)((x >> 8) & 0xff))

#define	XWORDF(x, y)\
	do {\
		WORD w = y;\
		x = XWORD(w);\
	} while (0)

enum enLAMPSTATE {
	LPS_EMPTY = 0,
	LPS_OFF,		// "OFF"
	LPS_ON,			// "ON"
	LPS_STOP,		// "정지"

	LPS_RUN,		// "가동",		180124:서비스:"정상"(수동)
	LPS_NORMAL,		// "정상"
	LPS_BYPASS,		// "BYPASS",	180123:ATO:"대기"
	LPS_LERR,		// "통신"

	LPS_CALL,		// "호출"
	LPS_FAIL,		// "고장"
	LPS_FIRE,		// "화재"
	LPS_EB,			// "EB",		180123:ATO:"개방"

	// 171101
	//LPS_CALLF,		// "호출", flash
	LPS_BLOCK,		// "-/-", DTB block
	LPS_FAILF,		// "고장", flash
	LPS_FIREF,		// "화재", flash
	LPS_EBF			// "EB", flash
};

// 200218
enum enCMSBSTATE {
	CMS_STOPF = LPS_ON,	// yellow,	"정지"
	CMS_STOPN,			// green,	"정지"
	CMS_RUNF,			// yellow,	"가동"
	CMS_RUNN,			// green,	"가동"
	CMS_BYPASSF,		// yellow,	"BYPASS"
	CMS_BYPASSN			// green,	"BYPASS"
};

// 170717
enum enDEVSTATEA {
	LPS_CYLCUT = LPS_EBF			// "CUT", no flash
};
//#define	LPS_CUT		LPS_EBF		// "CUT", no flash

// 180123
enum enDEVSTATEB {
	LPS_ATOWAIT = LPS_BYPASS,	// "대기", no flash
	LPS_ATOOPEN = LPS_EB		// "개방, no flash
};
//#define	LPS_ATOWAIT	LPS_BYPASS	// "대기", no flash
//#define	LPS_ATOOPEN	LPS_EB		// "개방, no flash

// 180312
enum enDEVSTATEC {
	LPS_SIVREADY = LPS_BYPASS,	// "대기", no flash
};

// environ을 읽어서 항목에 문제가 있을 때 편집 화면으로 넘어가는 시간
#define	TPERIOD_TOEDITATBOOT		REAL2PERIOD(3000)	// boot 때
#define	TPERIOD_TOEDITATTURN		REAL2PERIOD(2000)	// 전환 때

#pragma pack(push, 1)

typedef struct _tagSENTENCETAIL {
	BYTE	cSum;
	BYTE	cCr;
	BYTE	cLf;
} SENTENCETAIL, *PSENTENCETAIL;

typedef	struct _tagTABLEFABRIC {
	WORD	wCID;
	WORD	wLow;
	WORD	wHigh;
	WORD	wReal;
} TBLFAB, *PTBLFAB;

#define	ADDR_OPENPAGESENTENCE	15
typedef struct _tagOPENPAGESENTENCE {
	WORD	wPage;
} OPENPAGESENTENCE;

typedef struct _tagOPENPAGESENTENCETAIL {
	OPENPAGESENTENCE	s;
	SENTENCETAIL		t;
} OPENPAGESTCTAIL;

#define ADDR_MBLANKSENTENCE		100
typedef struct _tagMBLANKSENTENCE {
	WORD	wBlankCmd;
} MBLANKSENTENCE;

typedef struct _tagMBLANKSENTENCETAIL {
	MBLANKSENTENCE	s;
	SENTENCETAIL	t;
} MBLANKSTCTAIL;

#define	ADDR_BASESENTENCE		101
typedef struct _tagBASESENTENCE {
	WORD	wLength;		// 101, car length, 10/08/06/04
	WORD	wLineNo;		// 102, 2 -> 2nnX
	WORD	wProperNo;		// 103, 2nnX -> X is 0 ~ 9
} BASESENTENCE;

typedef struct _tagBASESENTENCETAIL {
	BASESENTENCE	s;
	SENTENCETAIL	t;
} BASESTCTAIL;

//#define	ADDR_GROUNDSENTENCE		103
// D001
#define	ADDR_HEADLINESENTENCE		101
typedef struct _tagHEADLINESENTENCE {
	WORD	wLength;		// 101, car length, 10/08/06/04
	WORD	wLineNo;		// 102, 2 -> 2nnX
	WORD	wProperNo;		// 103, 2Xnn -> X is 0 ~ 9
	WORD	wProgress;		// 104, 받기 진행 율, 0~100%
	WORD	wOtrn;			// wTranNoOTR;		// 105, train number on the rail from ato
	WORD	wYear;			// 106, (bcd)
	WORD	wDate;			// 107,
	WORD	wTime;			// 108,
	WORD	wSecond;		// 109,
	WORD	wPatt[CID_MAX];	// 110, car form pattern
	WORD	wMode;			// 120, atp mode
							// 0:NON,	1:ATP-AR,	2:ATP-FA,	3:ATP-ATO
							// 4:ATP-MCS,	5:ATP-YARD,	6:FMC,	7:OVERLAP
	WORD	wPowerRate;		// 121, power 0~100%(bcd)
	WORD	wBrakeRate;		// 122, brake 0~100%(bcd)
	// 18/10/23
	//WORD	wAtcCode;		// 123, atc speed code, ???
	WORD	wOdoMeter;		// 123, qwOdoMeter % 1000 = 0~1000m
	WORD	wSpeed;			// 124, current speed(bcd)
	DWORD	dwOdoMeter;		// 125-x/0x10000, 126-x&0xffff, km
	WORD	wNextStation;	// 127, Next station code
	WORD	wDestStation;	// 128, destination
	// CAUTION LITTLE_ENDIAN <-> BIG_ENDIAN
	__BW(	cabEx,			// 129
				// msb at hdu
				ldl,	1,		// left direction in left (forward)
				rdl,	1,		// right direction in left (reverse)
				rdr,	1,		// right direction in right (forward)
				ldr,	1,		// left direction in right (reverse)
				lm,		1,		// left master
				rm,		1,		// right master
				ls,		1,
				rs,		1,
				// lsb at hdu
				zs,		1,
				uscan,	1,		// usb scan
				usb,	1,		// 받기 단추 보이기
				eb,		1,		// master controller eb
				otrv,	1,		// OtrNo 보이기, FMC를 제외하고는 열번 표시
				hcr,	1,		// HCR -> 출고전 검사 애니메이션...
				//initiative,	1,	// Initiative mode 표시, 운전모드 옆 "- 수동"글자 보이기
				releaseblk,	1,	// 서비스 장치 백업 해제 단추 보이기
				lab,	1);		// laboratory mode
	__BW(	lampEx,			// 130
				// msb at hdu
				pb,		1,		// parking brake lamp
				sb,		1,		// security brake lamp
				//fsb,	1,		// full service brake lamp
				eb,		1,		// emergency brake lamp
				psdc,	1,		// platform screen door close
				adbs,	1,		// all door bypass switch
				trb,	1,		// 오른쪽 위 "고장" 보이기
				severe,	1,		// 중고장 다이얼로그 보이기
				oldt,	1,		// 0 = "고장 목록을 준비중입니다. 잠시후 다시 사용하십시오"
				// lsb at hud
				fire,	1,		// fire alarm
				// 171129
				//mindi,	1,		// 171114 pis manual mode	// err,	1,		// error lamp
				// 200218
				//spr,	1,
				devEx,	1,		// device extension
				domin,	1,
				dling,	1,		// 받기 중, 취소 단추와 진행 막대 보이기, 검수 화면 단추 가리기
				fdlg,	1,		// 화재 감지 다이얼로그
				mas,	1,		// 검수 화면에서 수동 ALBO 램프 상태
				mav,	1,		// 검수 화면에서 수동 ALBO 램프 보이기
				mcm,	1);		// 검수 화면에서 수동 CM가동 단추 보이기
	__BW(	ctnp,			// 131, car trouble bitmap & pis inform
				// msb at hdu
				fc8,	1,
				fc9,	1,
				// 180704
				//spr,	6,
				pismm,	1,		// pis manual mode
				pisnl,	1,		// left pis 0x31
				pisxl,	1,		// left pis 0x34
				pisnr,	1,		// right pis 0x31
				pisxr,	1,		// right pis 0x34
				sp,		1,
				// lsb at hud
				fc0,	1,		//
				fc1,	1,
				fc2,	1,
				fc3,	1,
				fc4,	1,
				fc5,	1,
				fc6,	1,
				fc7,	1);
	__BW(	insp,			// 132:1, inspection state
				// msb at hdu
				spr,	7,
				ros,	1,		// view retry/skip button
				// lsb at hud
				stop,	1,		// stop
				pb,		1,		// parking brake
				cmi,	1,		// compressor
				em,		1,		// emergency mode or ats cut
				adc,	1,		// all door close
				ebr,	1,		// emergency brake release
				mb7,	1,		// master controller position B7
				sb,		1);		// security brake
	// 210630
	//WORD	wMsgInitiative;	// 133:1, 171209, 지울 것...
	WORD	wFireBuzz;		// 133:1, B0:	1:멀티 버저 송출, 0:멀티 버저 취소
	WORD	wTroubleID;		// 134:1
	WORD	wTroubleCode;	// 135:1, current trouble
	WORD	wMsg;			// 136:1, message for download
	WORD	wFireDlgSeq;	// 137:1
	WORD	wFireDlgAgree;	// 138:1
	WORD	wFireDlgCancel;	// 139:1
	DWORD	dwDistance;		// 140:2
	WORD	wOldLoadTime;	// 142:1
	WORD	wSevereLength;	// 143:1
	__BW(	ref,			// 144:1
				// msb
				spr8,	8,
				// lsb
				aux,	1,
				waux,	1,		// with aux
				// 180910
				//spr6,	6);		// 7); 180406 find bug...
				psim,	1,		// simulated driving by pis
				spr,	5);
	WORD	wMomentum;		// 145:1	// 180405
	WORD	wPowerTime;		// 146:1	// 180405
	WORD	wPowerDistance;	// 147:1	// 180405
	WORD	wBrakeTime;		// 148:1	// 180405
	WORD	wBrakeDistance;	// 149:1	// 180405
} HEADLINESENTENCE, *PHEADLINESENTENCE;

typedef struct _tagHEADLINESENTENCETAIL {
	HEADLINESENTENCE	s;
	SENTENCETAIL		t;
} HDLSTCTAIL;

#define	ADDR_UNITAPSENTENCE		300
typedef struct _tagUNITAPSENTENCE {
	WORD	wDoor[10][2];	// 300:20-t, door state
								// b15~12:d4, b11~08:d3, b07~04:d2, b03~00:d1
								// [n][0]:left side, [n][1]:right side
								// at port on,  0:close, 1:open, 2:fail, 3:bypass
								// at port off, 4:close, 5:open, 6:fail, 7:bypass
								// 8~15: reserve
	WORD	wBypass[2];		// 320:2-t, door bypass state
								// b31~20:resv., b19-18:car9, ... b01-00:car0
								// 0:off, 1:on, 2-3:reserve
	WORD	wEmhand[2];		// 322:2-t, emergency handle state
								// b31~20:resv., b19-18:car9, ... b01-00:car0
								// 0:off, 1:on, 2-3:reserve
	//WORD	wSpr160[2];		// 324:2, gate state ???
								// b03-02:gate 1 right, b01-00:gate 1 left,
								// b07-06:gate 2 right, b05-04:gate 2 left,
								// b15~8:resv.,
								// 0:off, 1:on, 2-3:reserve
	WORD	wDoorMode[2];	// 324:2, ???
	WORD	wSiv;			// 326:1-3, siv status
								// b15~12:resv., b11~08:siv2, b07~04:siv1, b03~00:siv0
								// define each status...
	WORD	wV3f[2];		// 327:2-5, v3f status
								// b15~12:v3f3, b11~08:v3f2, b07~04:v3f1, b03~00:v3f0
								// b31~20:resv., b19~16:v3f4,
	WORD	wCmsb;			// 329:1-3, cmsb status
								// b15~12:resv., b11~08:cmsb2, b07~04:cmsb1, b03~00:cmsb0
								// define each status...
	WORD	wEsk;			// 330:1-3, esk status
								// b15~12:resv., b11~08:esk2, b07~04:esk1, b03~00:esk0
								// 0:off, 1:on, 2-3:reserve
	WORD	wAto;			// 331:1, ato status
								// b15~08:resv., b07~04:ato1, b03~00:ato0
								// define each status...
	WORD	wTrsRtd;		// 332:1
								// b15~12:rtd1, b11~08:rtd0, b07~04:trs1, b03~00:trs0
	WORD	wPisPau;		// 333:1
								// b15~12:pis1, b11~08:pis0, b07~04:pau1, b03~00:pau0
	WORD	wTu;			// 334:1
								// b15~12:tc1/liu2, b11~08:tc1/liu1, b07~04:tc0/liu2, b03~00:tc0/liu1
	DWORD	dwCu;			// 335:2
								// [0] b15~12:cc3, b11~08:cc4, b07~04:cc2, b03~00:cc1
								// [1] b15~12:cc7, b11~08:cc8, b07~04:cc5, b03~00:cc6
	WORD	wHb;			// 337:1-5
	WORD	wLb;			// 338:1-5
	WORD	wSivV[3];		// 339:3-3, siv output voltage,	cVo
	WORD	wSivI[3];		// 342:3-3, siv output current,	cIo
	WORD	wSivF[3];		// 345:3-3, siv freq.,			cFo
	WORD	wBatV[3];		// 348:3-3
	WORD	wImv[5];		// 351:5-5
	WORD	wV3fV[5];		// 356:5-5, v3f fc voltage,		wFc
	WORD	wV3fI[5];		// 361:5-5, v3f motor current,	cIq
	WORD	wV3fP[5];		// 366:5-5, v3f pwm,			cTbe
	WORD	wV3fA[5];		// 371:5-5, v3f regen. pwr, 회생 제동력 - Apbr
	WORD	wV3fD;			// 376:1-5,	v3f p/b
	WORD	wSivEV[3];		// 377:3, siv line voltage, cFs
	WORD	wV3fEs[5];		// 380:5
	WORD	wV3fIdc[5];		// 385:5
	WORD	wCmRunrate[3];	// 390:3
	WORD	wHbCnt[5];		// 393:5
	WORD	wLbCnt[5];		// 398:5
	// 180626
	WORD	wSivVer[3];		// 403:3
	WORD	wV3fVer[5];		// 406:5
	// 180806
	WORD	wAtoVer;		// 411:1
	// 200218
	WORD	wBms;			// 412:1-3, bms status
								// b15~12:resv., b11~08:bms2, b07~04:bms1, b03~00:bms0
								// define each status...
} UNITAPSENTENCE, *PUNITAPSENTENCE;

typedef struct _tagUNITAPSENTENCETAIL {
	UNITAPSENTENCE	s;
	SENTENCETAIL	t;
} UNASTCTAIL;

#define	ADDR_UNITBPSENTENCE		600
typedef struct _tagUNITBPSENTENCE {
	WORD	wSpr218;		// 600:1
	WORD	wEcu[3];		// 601:3-e, ecu status
								// 172:ecu0~ecu3, 173:ecu4~ecu7, 174:ecu8~ecu9
								// b15~12:ecu3, b11~08:ecu2, b07~04:ecu1, b03~00:ecu0
								// b31~28:ecu7, b27~24:ecu6, b23~20:ecu5, b19~16:ecu4
								// b47~40:resv., b39~36:ecu9, b35~32:ecu8,
	WORD	wHvac[3];		// 604:3-e
	WORD	wFire[3];		// 607:3-e, same ecu
	WORD	wPhone[3];		// 610:3-e, interphone status
								// b31~20:resv., b19-18:car9, ... b01-00:car0
								// 0:off, 1:on, 2-3:reserve
	WORD	wBcs[3];		// 613:3-e, same ecu
	WORD	wAss[3];		// 616:3-e, same ecu
	WORD	wBcp[10];		// 619:10-e,					cBcp, 10.1972Kg/cm^2
	WORD	wAsp[10];		// 629:10-e, as pressure,		cAsp, Kg/cm^2
	WORD	wBcv[10];		// 639:10-e,
	WORD	wLampAc1[3];	// 649:3-e
	WORD	wLampAc2[3];	// 652:3-e
	WORD	wLampDc[3];		// 655:3-e
	WORD	wHeat[3];		// 658:3-e
	WORD	wFan1[2];		// 661:2-e
	WORD	wVent[2];		// 663:2-e
	WORD	wClean[2];		// 665:2-e
	WORD	wCool1[3];		// 667:3-e
	WORD	wCool2[3];		// 670:3-e
	WORD	wIntTemp[10];	// 673:10-e
	WORD	wExtTemp[2];	// 683:2-e
	WORD	wLoadr[10];		// 685:10-e
	WORD	wFan2[2];		// 695:2-e
	WORD	wCtrlCab;		// 697, 두 운전실에서 어느 쪽이 공조기나 조명을 제어했는지를 표시한다.
	WORD	wCalcLw[10];	// 698:10-e
	WORD	wBrcs;			// 708:1, CUDIB_BRCS, DU BCP색을 바꾼다.
	// 200218
	WORD	wBccs[10];		// 709:10, appended 4~20ma
	WORD	wFireCtrl;		// 719:1
	// 201016
	WORD	wCtrlExCab;		// 720, ADPS AUTO MODE
} UNITBPSENTENCE, *PUNITBPSENTENCE;

typedef struct _tagUNITBPSENTENCETAIL {
	UNITBPSENTENCE	s;
	SENTENCETAIL	t;
} UNBSTCTAIL;

#define	ADDR_IOCDUMPSENTENCE	840
typedef struct _tagIOCDUMPSENTENCE {		// io & careers
	WORD	wTi[4][8];		// 840, tc0-liu1/liu2, tc9-liu1/liu2
	WORD	wCi[8][4];		// 872, cc1~cc8
	WORD	wTo[8];			// 904
	WORD	wCo[8];			// 912
	WORD	wAi[4][6];		// 920, 0~1:pwm, 2:tacho, 3:battery, 4:bc, 5:temp
	WORD	wPi[8];			// 944~951, bc of each cc
	DWORD	dwDistance;			// 952:2,	2424:2
	DWORD	dwSivPwr[SIV_MAX];	// 954:6,	2426:6
	DWORD	dwV3fPwr[V3F_MAX];	// 960:10,	2432:10
	DWORD	dwV3fRev[V3F_MAX];	// 970:10,	2442:10
	struct {
		WORD	wSiv[3];	// 980:3	//2696:3
		WORD	wV3f[5];	// 983:5	//2699:5
		WORD	wEcu[10];	// 988:10	//2704
	} trace;				// trace length
	// 200218
	WORD	wNull;			// 998
	WORD	wTcRight;		// 999
	WORD	wBatC[50];		// 1000~1049
} IOCDUMPSENTENCE;

typedef struct _tagIOCDUMPSENTENCETAIL {
	IOCDUMPSENTENCE	s;
	SENTENCETAIL	t;
} IOCDUMPSTCTAIL;

typedef struct _tagTROUBLEBLOCK {
	WORD	wMD;
	WORD	wHM;
	WORD	wST;
	WORD	wCidCode;
} TROUBLEBLOCK;

#define	ADDR_LKEDUMPSENTENCE		1300
typedef struct _tagLKEDUMPSENTENCE {		// link & env.
	WORD	wLine[100];
	WORD	wWheelDiam[7];		// 1400
	WORD	wTempSv[10];		// 1407
	WORD	wVentSv;			// 1417
	// 171129
	//WORD	wInitiativeMode;	// 1418
	WORD	wPisManual;			// 1418
	WORD	wVertc[12];			// 1419~1430
	WORD	wVermm[12];			// 1431~1442
	WORD	wVerFpga[12];		// 1443~1454
} LKEDUMPSENTENCE;

typedef struct _tagLKEDUMPSENTENCETAIL {
	LKEDUMPSENTENCE	s;
	SENTENCETAIL	t;
} LKEDUMPSTCTAIL;

#define	ADDR_LKSTATESENTENCE		1500
typedef struct _tagLKSTATESENTENCE {
	WORD	wDtb[12];			// 1500~1511
	WORD	wDtbas[2];			// 1512-1513
	WORD	wDtbbs[2];			// 1514-1515
	WORD	wEcu[10];			// 1516~1525
	WORD	wHvac[10];			// 1526~1535
	WORD	wDcu[10][8];		// 1536~1615
	WORD	wSiv[3];			// 1616~1618
	WORD	wV3f[5];			// 1619~1623
	WORD	wAto[2];			// 1624-1625
	WORD	wPau[2];			// 1626-1627
	WORD	wPis[2];			// 1628-1629
	WORD	wTrs[2];			// 1630-1631
	WORD	wRtd[2];			// 1632-1633
	WORD	wDtbat[13];			// 1634-1646
	WORD	wDtbbt[13];			// 1647-1659
	WORD	wUpdate;			// 1660
	// 200218
	WORD	wFdu[2];			// 1661-1662
	WORD	wBms[3];			// 1663-1665
	WORD	wCmsb[3];			// 1666-1668
	WORD	wHour;				// 1669
	WORD	wMin;				// 1670
	WORD	wSec;				// 1671
} LKSTATESENTENCE;

typedef struct _tagLKSTATESENTENCETAIL {
	LKSTATESENTENCE	s;
	SENTENCETAIL	t;
} LKSTATESTCTAIL;

#define ADDR_TROUBLELIST		2100
#define	ADDR_STATELIST			2260
typedef struct _tagEVENTLISTSENTENCE {
	//WORD	wUpdate;			// 2100
	__BW(	date[12],			// 2100~2111
				mon,	8,		// msb
				day,	8);		// lsb
	__BW(	occurrence[12],		// 2112~2123
				hour,	8,
				min,	8);
	__BW(	second[12],			// 2124~2135
				occur,	8,
				dest,	8);
	__BW(	destroy[12],		// 2136~2147
				hour,	8,
				min,	8);
	WORD	wCID[12];			// 2148~2159
	WORD	wReal[12];			// 2160~2171
	//WORD	wBound;				// 2172, 180219 ?????
	WORD	wCurPage;			// 2172
	WORD	wTotalPage;			// 2173
	WORD	wIndex;				// 2174
	WORD	wReduce;			// 2175
	WORD	wAlarms;			// 2176
	WORD	wStates;			// 2177
	WORD	wRefer;				// 2178
	WORD	wBound;				// 2179
	WORD	wFence;				// 2180
	//DWORD	dwIndex;			// 2233-2234
	//DWORD	dwReduce;			// 2235-2236
} ELISTSENTENCE;

typedef struct _tagEVENTLISTTAIL {
	ELISTSENTENCE	s;
	SENTENCETAIL	t;
} ELISTSTCTAIL, *PELISTSTCTAIL;

#define	ADDR_USERSETSENTENCE		2400
typedef struct _tagUSERSETCONTENTS {
	WORD	year;				// 2400
	WORD	month;				// 2401
	WORD	day;				// 2402
	WORD	hour;				// 2403
	WORD	minute;				// 2404
	WORD	second;				// 2405
	WORD	wheeldiam[7];		// 2406:7
	WORD	temp[10];			// 2413:10
	WORD	venttime;			// 2423
	WORD	pisManual;			// 2424
	// 171115
	//WORD	initiativeMode;		// 2425
	// 180717
	//WORD	wNull25;				// 2425
	WORD	pisRequest;			// 2425
	DWORD	dwDistance;			// 2426:2
	DWORD	dwSivPwr[SIV_MAX];	// 2428:6
	DWORD	dwV3fPwr[V3F_MAX];	// 2434:10
	DWORD	dwV3fRev[V3F_MAX];	// 2444:10
	// 171115
	//WORD	wTrainNo;			// 2454
	//WORD	wDepartCode;		// 2455, 출발역
	//WORD	wCurrentCode;		// 2456, 현재역
	//WORD	wDestinationCode;	// 2457, 도착역
	WORD	wNull1;				// 2454
	WORD	wNull2;				// 2455
	WORD	wNull3;				// 2456
	WORD	wNull4;				// 2457
} USERSETCONTENTS;

typedef union _tagUSERSETSENTENCE {
	USERSETCONTENTS	sv;
	WORD			ws[sizeof(USERSETCONTENTS) / sizeof(WORD)];
	BYTE			c[sizeof(USERSETCONTENTS)];
} USERSETSENTENCE;

typedef struct _tagUSERSETSTCTAIL {
	USERSETSENTENCE	s;
	SENTENCETAIL	t;
} USERSETSTCTAIL;

#define	NORMAL_PAGE				100
#define	OVERHAUL_PAGE			500
#define	TIMEWHEELEDIT_PAGE		580
#define	TEMPOTHEREDIT_PAGE		582
#define	INSPECTENTRY_PAGE		600
#define	DTBCHECK_PAGE			620
#define	POLECHECK_PAGE			622
#define	LOCALCHECK_PAGE			624
#define	POLEEXCHECK_PAGE		626
#define	LOCALEXCHECK_PAGE		628
#define	V3FCHECK_PAGE			630
#define	SIVCHECK_PAGE			640
#define	ECUCHECK_PAGE			650
#define	COOLCHECK_PAGE			660
#define	HEATCHECK_PAGE			670

#define	ADDR_DOORTESTMAP		2480
typedef struct _tagDOORTESTMAPSENTENCE {
	WORD	wDoorTestMap[2];
} DTMAPSENTENCE;

typedef struct _tagDTMAPSENTENCETAIL {
	DTMAPSENTENCE	d;
	SENTENCETAIL	t;
} DTMAPSTCTAIL;

#define	ADDR_INSPECTSELMAP		2490
typedef struct _tagINSPSELMAPSENTENCE {
	WORD	wInspSelMap;		// 2490, inspect item
} ISMAPSENTENCE;

typedef struct _tagISMAPSENTENCETAIL {
	ISMAPSENTENCE	s;
	SENTENCETAIL	t;
} ISMAPSTCTAIL;

typedef struct _tagECUINSPECTCOLUMNS {
	WORD	wEcuRes;	// 2531, b01~00(B09~08)-0:box/1:"시험 중"/2:"결과"/3:"",
						//			b03~02(B10~09) 0:box/1:"정상"/2:"고장/3:""
	WORD	wEcuL;		// 2532, ecu(0~7),		-0:box/1:"정상"/2:"고장"/3:"시험"
	WORD	wEcuH;		// 2533, b03~00 ecu(8~9),
	WORD	wBcp[10];	// 2534
	WORD	wPwme[10];	// 2544
} ECUINSPECTCOL, *PECUINSPECTCOL;

typedef struct _tagINSPECTCOLUMNS {
	// 620쪽
	WORD	wTcRes;		// 2502,	bXX(DU side)-XWORD-BXX(ts side)
						// DTB test
						// b01~00(B09~08)-0:box/1:"시험 진행 중"/2:"시험 완료"/3:""
						// b03~02(B11~10)-0:box/1:"정상"/2:"고장"/3:""
						// pole test,	b07~04(B15~12)
						// local test,	b11~08(B03~00)
	WORD	wTul;		// 2503
						// b01~00(B09~08)-tc0 liu0,	b03~02(B11~10)-tc0-liu1
						// b05~04(B13~12)-tc9 liu0,	b07~06(B15~14)-tc9 liu1
						//	-0:box/1:"정상"/2:"고장"/3:"시험"
	WORD	wCul;		// 2504
						// cu * 8 = 16bit
	// 622쪽
	WORD	wPole;		// 2505
						// b01~00(B09~08)-trs0,	b03~02(B11~10)-trs9,	b05~04(B13~12)-pis0,	b07~06(B15~14)-pis9
						// b09~08(B01~00)-pau0,	b11~10(B03~02)-pau9,	b13~12(B05~04)-rtd0,	b15~14(B07~06)-rtd9
	// 624쪽
	WORD	wSvf;		// 2506, b09~00(B01~00,15~08)-v3f * 5,	b15~10(B07~B02)-siv * 3
	WORD	wEcu;		// 2507, ecu * (0~7)
	WORD	wHvac;		// 2508, hvac * (0~7)
	WORD	wEhv;		// 2509, b03~00(B11~08)-ecu8~9,	b07~04(B15~12)-hvac8~9
	// 630쪽
	WORD	wV3fRes;	// 2510, b01~00(B09~08)	0:box/1:"시험 진행 중"/2:"시험 완료:3:""
						//		 b03~02(B11~10)	0:box/1:"정상"/2:"고장/3:""
	WORD	wV3f;		// 2511, b01~00-v3f0 ...., -0:box/1:"정상"/2:"고장"/3:"시험"
	WORD	wHb;		// 2512, b09~00 v3f * 5, 0:""/01:"OFF"/2:"ON"/3:"정지"
	WORD	wLb;		// 2513, b09~00 v3f * 5
	WORD	wCpb;		// 2514, b09~00 v3f * 5, 0:"C"/1:""/2"P"/3"B"
	WORD	wPwmv[5];	// 2515
	WORD	wFc[5];		// 2520
	// 640쪽
	WORD	wSivRes;	// 2525, b01~00(B09~08)	0:box/1:"시험 진행 중"/2:"시험 완료:3:""
						//		 b03~02(B11~10)	0:box/1:"정상"/2:"고장/3:""
	WORD	wSiv;		// 2526, b01~00-siv0 ...., -0:box/1:"정상"/2:"고장"/3:"시험"
	WORD	wEo[3];		// 2527
	WORD	wFreq[3];	// 2530
	// 650쪽
	ECUINSPECTCOL ecu;	// 2533 ~ 2555, 23 word
	//WORD	wEcuRes;	// 2533, b01~00(B09~08)-0:box/1:"시험 중"/2:"결과"/3:"",
	//					//			b03~02(B10~09) 0:box/1:"정상"/2:"고장/3:""
	//WORD	wEcuL;		// 2534, ecu(0~7),		-0:box/1:"정상"/2:"고장"/3:"시험"
	//WORD	wEcuH;		// 2535, b03~00 ecu(8~9),
	//WORD	wBcp[10];	// 2536
	//WORD	wPwme[10];	// 2546
	// 660쪽
	WORD	wCoolRes;	// 2556, b01~00(B09~08)	0:box/1:"시험 진행 중"/2:"시험 완료:3:""
						//		 b03~02(B11~10)	0:box/1:"정상"/2:"고장/3:""
	WORD	wCool1;		// 2557, hvac(0~7),		-0:box/1:"정상"/2:"고장"/3:"시험"
	WORD	wCool2;		// 2558, hvac(0~7)
	WORD	wCool12;	// 2559, b01~00(B09~08)-cool1 hvac8, b03~02(B11~10)-cool1 hvac9,
						//		 b05~04(B13~12)-cool2 hvac8, b07~06(B15~14)-cool2 hvac9,
	// 670쪽
	WORD	wHeatRes;	// 2560, b01~00(B09~08)	0:box/1:"시험 진행 중"/2:"시험 완료:3:""
						//		 b03~02(B11~10)	0:box/1:"정상"/2:"고장/3:""
	WORD	wHeatL;		// 2561, hvac(0~7),		-0:box/1:"정상"/2:"고장"/3:"시험"
	WORD	wHeatH;		// 2562, b04~00 hvac(8~9)
	// 200218
	WORD	wExDev;		// 2563,
						// b01~00(B09~08)-fdu0,	b03~02(B11~10)-fdu9,	b05~04(B13~12)-cmsb0,	b07~06(B15~14)-cmsb5,
						// b09~08(B01~00)-cmsb9,	b11~10(B03~02)-bms0,	b13~12(B05~04)-bms5,	b15~14(B07~06)-bms9
	WORD	wHvacMon[4];	// 210720, display inspect sequence of hvac
} INSPECTCOLUMNS, *PINSPECTCOLUMNS;

#define	ADDR_INSPECTSENTENCE	2497
typedef struct _tagINSPECTSENTENCE {
	WORD	wStep;		// 2497
	WORD	wTime;		// 2498
	WORD	wItem;		// 2499, // 01:PDT, 02:daliy, 03:mothly
	WORD	wSubItem;	// 2500, ecu 시험에서 각 항목 표시, 10 ~ 19
	WORD	wMsg;		// 2501,
	INSPECTCOLUMNS	ics;	// 2502~2560, 59word
} INSPECTSENTENCE;

typedef struct _tagINSPSENTENCETAIL {
	INSPECTSENTENCE	s;
	SENTENCETAIL	t;
} INSPSTCTAIL;

#define	SIZE_ENTRYSENTENCE	32
#define	ADDR_ENTRYSENTENCE	2600
typedef struct _tagENTRYSENTENCE {
	WORD	wYm[SIZE_ENTRYSENTENCE];	// 2600:32,	msb:year,	lsb:month
	WORD	wDh[SIZE_ENTRYSENTENCE];	// 2632:32,	msb:day,	lsb:hour
	WORD	wMs[SIZE_ENTRYSENTENCE];	// 2664:32,	msb:minute,	lsb:second
} ENTRYSENTENCE;						// 2696

typedef struct _tagENTRYSENTENCETAIL {
	ENTRYSENTENCE	s;
	SENTENCETAIL	t;
} ENTRYSTCTAIL;

#define	ADDR_ENTRYSELMAP		2714
typedef struct _tagENTRYSELMAPSENTENCE {
	WORD	wEntrySelMap[2];		// 2714, entry selected bitmap
	WORD	wSvfSelMap;				// 2716, b0~b2:siv, b3~7:v3f
	WORD	wEcuSelMap;				// 2717, b0~b9:ecu
} ESMAPSENTENCE;

typedef struct _tagESMAPSENTENCETAIL {
	ESMAPSENTENCE	s;
	SENTENCETAIL	t;
} ESMAPSTCTAIL;

typedef struct _tagBUNDLES {
	OPENPAGESTCTAIL	opp;
	MBLANKSTCTAIL	mbl;
	BASESTCTAIL		bas;
	HDLSTCTAIL		hdl;
	UNASTCTAIL		una;
	UNBSTCTAIL		unb;
	IOCDUMPSTCTAIL	icd;
	LKEDUMPSTCTAIL	led;
	LKSTATESTCTAIL	lks;
	ELISTSTCTAIL	tli;		// trouble list
	ELISTSTCTAIL	sli;		// status list
	USERSETSTCTAIL	uss;
	DTMAPSTCTAIL	dtm;
	ISMAPSTCTAIL	ism;
	INSPSTCTAIL		isp;
	ENTRYSTCTAIL	ent;
	ESMAPSTCTAIL	esm;
	SENTENCETAIL	rds;
} _BUNDLES, *PBUNDLES;

typedef struct _tagSHEAF {
	HEADLINESENTENCE	hdb;
	//TROUBLESSENTENCE	trb;
	UNITAPSENTENCE		una;
	UNITBPSENTENCE		unb;
	IOCDUMPSENTENCE		icd;
	LKEDUMPSENTENCE		led;
	LKSTATESENTENCE		lks;
	//SDOSSSENTENCE		sdf;
	ELISTSENTENCE		tli;
	ELISTSENTENCE		sli;
	USERSETSENTENCE		uss;
	INSPECTSENTENCE		isp;
	ENTRYSENTENCE		ent;
} _SHEAF, *PSHEAF;

typedef union _tagHDURTEXT {
	OPENPAGESTCTAIL	opp;
	MBLANKSTCTAIL	mbl;
	BASESTCTAIL		bas;
	HDLSTCTAIL		hdl;
	UNASTCTAIL		una;
	UNBSTCTAIL		unb;
	IOCDUMPSTCTAIL	icd;
	LKEDUMPSTCTAIL	led;
	LKSTATESTCTAIL	lks;
	ELISTSTCTAIL	tli;		// trouble list
	ELISTSTCTAIL	sli;		// status list
	USERSETSTCTAIL	uss;
	DTMAPSTCTAIL	dtm;
	ISMAPSTCTAIL	ism;
	INSPSTCTAIL		isp;
	ENTRYSTCTAIL	ent;
	ESMAPSTCTAIL	esm;
	SENTENCETAIL	rds;
} HDURTEXT;

// tcms -> hdu
typedef struct _tagSRHDUINFO {
	BYTE		cEsc;
	BYTE		cRW;
	WORD		wAddr;
	WORD		wLength;
	HDURTEXT	t;
} HDURINFO, *PHDURINFO;

#define	SIZE_HDUSR		sizeof(HDURINFO)

// hdu -> tcms
typedef struct _tagSAHDUINST {
	BYTE	cLeng;
	BYTE	cCmd;
	BYTE	cDumm[16];
} SAHDUINST;

typedef struct _tagSAHDUINFO {
	BYTE	cEsc;
	BYTE	cSeq;		// 'I' or 'A'
	union {
		SAHDUINST	inst;
		BYTE		cText[256];
	} cont;
} HDUAINFO, *PHDUAINFO;

typedef union _tagSAHDUHOARD {
	HDUAINFO	s;
	BYTE		c[sizeof(HDUAINFO)];
} HDUHOARD;

#define	SIZE_HDUSA		sizeof(HDUAINFO)

typedef struct _tagHDUFLAP {
	HDURINFO	r;
	HDUAINFO	a;
} HDUFLAP, *PHDUFLAP;

#pragma pack(pop)

#define	oppb	pBund->opp.s			// OPENPAGESTCTAIL	opp
#define	mblb	pBund->mbl.s			// MBLANKSTCTAIL	mbl
#define	basb	pBund->bas.s			// BASESTCTAIL		bas
#define	hdlb	pBund->hdl.s			// HDLSTCTAIL		hdl
#define	unab	pBund->una.s			// UNASTCTAIL		una
#define	unbb	pBund->unb.s			// UNBSTCTAIL		unb
#define	icdb	pBund->icd.s			// IOCDUMOSTCTAIL	icd
#define	ledb	pBund->led.s			// LKEDUMPSTCTAIL	led
#define	lksb	pBund->lks.s			// LKSTATESTCTAIL	lks
#define	tlib	pBund->tli.s			// ELISTSTCTAIL		tli
#define	slib	pBund->sli.s			// ELISTSTCTAIL		sli
#define	ussb	pBund->uss.s			// USERSETSTCTAIL	uss
#define	ismb	pBund->ism.s			// IBOOKSTCTAIL		ism
#define	ispb	pBund->isp.s			// INSPSTCTAIL		isp
#define	entb	pBund->ent.s			// ENTRYSTCTAIL		ent
#define	esmb	pBund->esm.s			// ESMAPSTCTAIL		esm

enum enHDUKEY {
	HK_WAKEUPTRIGGER = 3,	// HDU가 TCMS동작 중에 리셋되어 부팅될 때 5초를 기다린 후 보내는 값.
	HK_WAKEUPEDNORMAL,		// 사용자가 운행 로그인을 할 때 TCMS로 들어오는 값
	HK_WAKEUPEDOVERHAUL,	// 사용자가 관리 로그인을 할 때 TCMS로 들어오는 값
	HK_6,
	HK_7,
	HK_BKSPACE = 8,
	HK_9,
	HK_10,
	HK_11,
	HK_12,
	HK_ENTER = 13,
	HK_14,
	HK_15,
	HK_UP = 16,
	HK_DOWN,			// 17,
	HK_LEFT,			// 18,
	HK_RIGHT,			// 19,
	HK_PGUP,			// 20,
	HK_PGDOWN,			// 21,
	HK_22,
	HK_23,
	HK_24,
	HK_25,
	HK_26,
	HK_ESC,				// 27,
	HK_T0,				// 28, 고장 목록에서 객차를 선택할 때...
	HK_T1,				// 29
	HK_CAR0,			// 30, HK_0 ~ HK_DCU는 스크립트를 통해 들어온다.
	HK_CAR1,
	HK_CAR2,
	HK_CAR3,
	HK_CAR4,
	HK_CAR5,
	HK_CAR6,
	HK_CAR7,
	HK_CAR8,
	HK_CAR9,
	HK_SIDE = 40,
	HK_V3F,				// 41
	HK_ECU,				// 42
	HK_SIV,				// 43
	HK_DCU,				// 44

	HK_INSPRUN = 58,	// PDT, 일상, 월상 검사
	HK_INSPSTOP,		// 59
	HK_INSPRETRY,		// 60, 재 검사
	HK_INSPSKIP,		// 61, 다음 검사

	HK_TEXT = 65,		//0x41, 'A'는 메모리 읽기에서 HDU가 쓰는 글자이므로 따로 명령으로 쓰면 안된다.180626
	HK_BFUNC,			// 66
	HK_CANCEL,			// 67, 받기 취소
	HK_DOWNLOAD,		// 68, 받기
	HK_EXAMIN,			// 69, 검수 화면
	HK_CLEARV3FBCNT,	// 70, v3f's hb/lb count clear
	HK_GENERAL,			// 71, 일반 화면
	HK_DOORTEST,		// 72, 출입문 시험
	HK_SETENVIRONS,		// 73
	HK_CMCHARGE,		// 74, CM CHARGE
	HK_LAMPTEST,		// 75, LAMP TEST
	HK_LINELIST,		// 76	????? 찾을 것
	HK_LISTSORTTROUBLE,	// 77, 객차별 고장 목록
	HK_LOGBOOKENTRY,	// 78, 운행 고장 기록
	HK_INSPECTENTRY,	// 79, 차상 시험 기록
	HK_TRACEENTRY,		// 80, 고장 추적 기록
	HK_LOGBOOKENTRYAUX,	// 81, AUX-운행 고장 기록
	HK_EXITEDIT,		// 82, 편집에서 검수로 넘어가려할 때
	HK_LISTSTATUS,		// 83, 상태 정보
	HK_LISTTROUBLE,		// 84, 고장 정보
	HK_TRACECLEAR,		// 85
	HK_RELEASEBLOCK,	// 86, 방송 표시기 백업 해제
	HK_SETWHEEL,		// 87
	HK_SETCAREER,		// 88
	HK_SETTIME,			// 89
	// 200218
	//HK_90,				// 90,
	HK_CLEARLINEFAULTCOUNTS,	// 90
	HK_NEXTTROUBLE,		// 91,
	HK_AGREESEVERE,		// 92
	HK_FIREAGREE0 = 100,	// 화재 감지 확인
	HK_FIREAGREE1,
	HK_FIREAGREE2,
	HK_FIREAGREE3,
	HK_FIREAGREE4,
	HK_FIREAGREE5,
	HK_FIREAGREE6,
	HK_FIREAGREE7,
	HK_FIREAGREE8,
	HK_FIREAGREE9,
	HK_FIRECANCEL0 = 110,	// 화재 감지 취소
	HK_FIRECANCEL1,
	HK_FIRECANCEL2,
	HK_FIRECANCEL3,
	HK_FIRECANCEL4,
	HK_FIRECANCEL5,
	HK_FIRECANCEL6,
	HK_FIRECANCEL7,
	HK_FIRECANCEL8,
	HK_FIRECANCEL9,
	// 210630
	HK_FIREBUZZ = 120,		// multi-buzzer cancel
	HK_FIRECLOSE,
	//180124
	//HK_TLISTTU0 = 120,	// 고장 목록 TC0
	//HK_TLISTCU0,		// 121, 고장 목록 CU0
	//HK_TLISTCU1,		// 122
	//HK_TLISTCU2,		// 123
	//HK_TLISTCU3,		// 124
	//HK_TLISTCU4,		// 125
	//HK_TLISTCU5,		// 126
	//HK_TLISTCU6,		// 127
	//HK_TLISTCU7,		// 128
	//HK_TLISTCU8,		// 129
	//HK_TLISTCU9,		// 130
	//HK_TLISTTU9,		// 131
	HK_MAX
};
