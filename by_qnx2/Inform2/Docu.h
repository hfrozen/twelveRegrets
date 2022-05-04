/**
 * @file	Docu.h
 * @brief	���� ���, ���� ��� ���� ���
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"
#include "DevInfo2/DevInfo.h"
#include "Draft.h"

/* ���� ���� /h/sd/arch...
 ���丮 �̸��� "dyymmdd"(d-�⵵-��-��)�� �Ѵ�.
 �����̸��� "*hhmmss"(*-��-��-��)�� �Ѵ�.
 *�� ���� ��Ͽ�����		'l',
     ���� ��Ͽ�����		't',
     ���� ���� ��Ͽ�����
     				siv��	's',
     				v3f��	'v',
     				ecu��	'e'
     �ÿ��� ��Ͽ�����		'r'�� ���� ǥ���ϸ�
 ���� ������ �ÿ��� ���(���� �̸�)�� ������ ���� �ð��� ����Ͽ� ������ �ð��븦 ���� �ʿ�� ������
 ���� ��ϰ� ���� ���(���� �̸�)�� �ð��븦 ���߾� ���� ������ �����ϵ��� �Ѵ�.
 ���� ��ϰ� ���� ����� F/R�� ���ԵǴ� �ð��� ���� �̸����� �ϰ� ������ ���ܵ� ������,
 �Ǵ� F/R�� �� ���Ե� ������ ���� ���Ͽ� ����� �����.
*/

#pragma pack(push, 1)

///	*** ���� ��� ��� ***
// ���� ù �κп� ���� ��ȣ�� �� ��ȣ, ���̵��� ���� �����
// ���� ����� �����Ѵ�.

#define	MAX_FILEDATE		991231				// ���� �̸����� ���� �� �ִ� �ִ�ġ

//						 	 012345678901
#define	TITLE_LOGBOOK		"TCMS LOGBOOK"
// 180511
#define	LOGBOOKCTRLSIDE_VERSION	0x207			//
// 170911
#define	LOGBOOKCSUM_VERSION	0x203				// �� ��ȣ���� üũ���� ���Եȴٴ� ���̹Ƿ� �����ϸ� �ȵȴ�.
// 171114
//#define LOGBOOK_VERSION		0x203				// 170620,	0x102					// 17/06/07,	0x101
// 171123
//#define LOGBOOK_VERSION		0x204				// siv io byte->word
//#define LOGBOOK_VERSION		0x205				// vvvf revive status
// 180219
//#define	LOGBOOK_VERSION		0x206				// wRealPwm�� �񱳳� ȯ������ ���� ���ڴ� ��� ��.
// 180828
#define	LOGBOOK_VERSION		0x207			// v3f.cBed����, pis notr, station code �߰�

// 17/06/20
// OBLIGE�� ��ϵ��� �ʾƼ� �� TC�� ��Ȯ�� ������ �ľ��ϱ� ���� �߰��Ѵ�.
// �þ Paper Command�� �߰��Ѵ�.
// pres[PID_MAX]�� �����Ѵ�.

typedef struct _tagLOGBOOKHEAD {
	BYTE	title[12];		// 0:11
	WORD	wVer;			// 12:13
	WORD	wPermNo;		// 14:15, 2Xnn
	BYTE	cLength;		// 16:0
	BYTE	cRate;			// 17:0
	//BYTE	cDummy[14];		// 18:31
	// 17/05/29
	// ���丮�� �����̸��� ������ �ð��� �����ϴٺ��� 1~2�� ���̰� �����.
	// ��忡 DEVTIME�� �߰��Ͽ� �������� �ð��� ���ߵ��� �Ѵ�.
	DEVTIME	devt;			// 18:23
	// 171125
	//BYTE	cTimeValid;		// 24:0		// �Ⱦ���. 0x5a, ���� ��: dt�� ��ȿ���� ��Ÿ����. mm���� �� ���� 5a�̸� dt�� �����ϰ� �ƴϸ� ������� �Ѵ�.
	BYTE	cAux;			// 24:		// ���� TC
	BYTE	cDummy[7];		// 25:31
} LOGBOOKHEAD;

#define	MAX_DTLYEAR			74
#define	MAX_DTLYEAR4		(MAX_DTLYEAR + 2000)
#define	MIN_DTLYEAR			10
#define	MIN_DTLYEAR4		(MIN_DTLYEAR + 2000)

typedef struct _tagTCSTATUSCMD {	// draft.h - SIGNATURE�� ������ ��.
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
	__BC(	oppoLead,			// �Ĺ��� pis ����
				f,		1,			// �� ����, ���⼭�� �ǹ� ����
				r,		1,			// �� ����, ���⼭�� �ǹ� ����
				atom,	1,			// , ���⼭�� �ǹ� ����
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
								// [0]�� head service device ��� ����, [1]�� tail service device ��Ż���
								//	b0:ato, b3:pau, b4:pis, b5:trs, ���� 1�̸� ���� ����.
	BYTE	cSvcCtrlCode[2];	// 250:2,	//486:2
								// [0]�� head���� �Ǵ��ϴ� ���� ��ġ �����ڵ�, [1]�� tail����...
								// b1-0:head sevice device code, b3-2:tail service device code
	DWORD	dwAvrSpeed;			// 252:4, average atc speed
	QWORD	qwDistance;			// 256:8, *****
	WORD	wSegment;			// 264:2, WORD wSegmentDistance, ���� �Ÿ�
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
		WORD	wNotr;			// 319:2,	pisa.notr.a, wOtrn�� �ߺ��Ǵ� �͵� ����, �Ŀ� �� �ʿ��� �����ͷ� ��ü�� ��.
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
	DWORD	dwRoute;			// 482:4, DTB ��� ����(b0:head DTB-A, b1:head DTB-B, b2:tail DTB-A, b3:tail DTB-B, b4:CC1 DTB-A, b5:CC1 DTB-B ~~~
	CMJOGTIMES	cjt;			// 486:16, *****
	// 170911
	WORD	wCSum;				// 502:2,
	// üũ�����μ� �� ���� ��ȿ���� �˻��Ѵ�. ���� ���� �� �Ÿ�, ���µ��� ������ ���� �� �����ϴ�.
} LOGCONTENTS;					// 504

// ***** absolute position

typedef union _tagLOGINFO {
	LOGCONTENTS	r;
	BYTE		c[sizeof(LOGCONTENTS)];
} LOGINFO, *PLOGINFO;

#if	!defined(SIZE_LOGBOOK)
#define	SIZE_LOGBOOK	504		// RTDBRAND�� ���� ũ��� �Ұ�!
#endif

typedef struct _tagLOGBOOK {
	LOGINFO	real;
	BYTE	cDummy[SIZE_LOGBOOK - sizeof(LOGCONTENTS)];
} LOGBOOK, *PLOGBOOK;

///	*** ���� ��� ��� ***
typedef union _tagTROUBLEINFO {
	struct {
		BYTE	down	:  1;	// ������..
		// 171107
		//BYTE	alarm	:  1;	// type�߿��� ����ڰ� �����ϴ� ������ �� 1�� �ȴ�. �ڴ��� ������ �����.
		BYTE	saw		:  1;	// ������� Ȯ���� ��ģ �ڵ�, ���� ǥ���ٿ� ǥ�������ʴ´�.
		BYTE	type	:  4;	// see ALARMTYPE enum, ���� �� ���� ������?
		BYTE	cid		:  4;	// 0~9, ���� ��ȣ
		WORD	code	: 11;	// ���� �ڵ�, 0x7ff-2047
		BYTE	mon		:  4;	// 0~11, �߻��� ��(1~12)
		BYTE	day		:  5;	// 0~30, ...    ��(1~31)
		BYTE	uhour	:  5;	// 0~23, ...    ��(0~23)
		BYTE	umin	:  6;	// 0~59, ...    ��(0~59)
		BYTE	usec	:  6;	// 0~59, ...    ��(0~59)
		BYTE	dhour	:  5;	// 0~23, �Ҹ��� ��(0~23)
		BYTE	dmin	:  6;	// 0~59, ...    ��(0~59)
		BYTE	dsec	:  6;	// 0~59, ...    ��(0~59)
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

#define	TROUBLEBUFINDEX_BEGIN	1	// ù��°�� ����. ��ȣ�� ���ٺ��� 0���� �ϸ� �ݺ��Ǵ� ������ ���� ����...
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
#define	SIZE_TROUBLELIST	(DEFAULT_CARLENGTH + 2)	// ������ ���� ��� ��, tu�� ����...

typedef struct _tagTROUBLEBYCAR {
	WORD		wMin;			// MAX_OLDTROUBLE - old trouble size
	WORD		wSlash;			// revoke limit
	WORD		wCur;			// increase from MAX_OLDTROUBLE
	TROUBLEINFO	ts[MAX_TROUBLELIST];
} TROUBLEBYCAR, *PTROUBLEBYCAR;		// ������ ���� ���

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

///	**** CDMA �ڷ� ��� ***

// ���� ���� �Ÿ�...
// 100Km/h * 24�ð� = 2400Km, �Ϸ� ��ϵǴ� ���� 2400(0x960)�� ��
// DWORD �ִ�ġ�� 4294967295(0xffffffff)�̹Ƿ�
// 4294967295 / 2400 = 1789596(��) -> / 365 = 4902(��)
// �׷��� km�������� DWORD 32��Ʈ�ε� ����ϴ�.

typedef struct _tagCDMACONTENTS {
	DBFTIME	dbft;
	BYTE	cProperNo;			// 004:1, �� ��ȣ
	__BC(	cab,				// 005:1
				ldl,	1,			// ���� ��ġ���� ���� ����		(forward)
				rdl,	1,			// ���� ��ġ���� ������ ����	(reverse)
				rdr,	1,			// ������ ��ġ���� ������ ����	(forward)
				ldr,	1,			// ������ ��ġ���� ���� ����	(reverse)
				lm,		1,			// ������ ������
				rm,		1,			// �������� ������
				spr,	2);
	__BC(	lamp,				// 006:1
				pb,		1,			// ���� ���� ��
				sb,		1,			// ���� ���� ��
				dir,	1,			// 17/11/27, TUDIB_DIR
				eb,		1,			// ��� ���� ��
				psdc,	1,			// psd close
				adbs,	1,			// all door bypass
				fire,	1,			// ! ȭ�� �� ???
				err,	1);			// ! �溸 �� ???
	BYTE	cMode;				// 007:1, ���� ���
	BYTE	cPowerRate;			// 008:1, 0~100%
	BYTE	cBrakeRate;			// 009:1, 0~100%
	BYTE	cNextStation;		// 010:1, ���� ��
	BYTE	cDestStation;		// 011:1, ���� ��
	WORD	wCurSpeed;			// 012:2, �ӵ� (km/h)
	DWORD	dwDistance;			// 014:4, ���� ���� �Ÿ�(km)
	WORD	wOtrn;				// 018:2, ���� ��ȣ
	BYTE	cDoor[CID_MAX];		// 020:10, 0~9ȣ�� ���Թ� ����, 0:����/1:����
	BYTE	cDbp;				// 030:1, 0~7ȣ�� ���Թ� �����н� ����, 0:����/1:�����н�
	BYTE	cDeh;				// 031:1, 0~7ȣ�� ���Թ� ��� �ڵ� ����, 0:����/1:���
	__BC(	dbeh,				// 032:1
				bp8,	1,			// 8ȣ�� �����н� ����
				bp9,	1,			// 9ȣ�� �����н� ����
				eh8,	1,			// 8ȣ�� ��� �ڵ� ����
				eh9,	1,			// 9ȣ�� ��� �ڵ� ����
				spr4,	4);
	__BC(	panto,				// 033:1
				c2,		2,			// 2ȣ�� ���ͱ׷��� ����
				c4,		2,			// 4ȣ�� ���ͱ׷��� ����
				c8,		2,			// 8ȣ�� ���ͱ׷��� ����
				spr,	2);
	__BW(	svf,				// 034:2
				siv0,	2,			// 0ȣ�� siv ����, 0:NON/1:����/2:���/3:����
				siv5,	2,			// 5ȣ�� siv ����
				siv9,	2,			// 9ȣ�� siv ����
				v3f1,	2,			// 1ȣ�� v3f ����
				v3f2,	2,			// 2ȣ�� v3f ����
				v3f4,	2,			// 4ȣ�� v3f ����
				v3f7,	2,			// 7ȣ�� v3f ����
				v3f8,	2);			// 8ȣ�� v3f ����
	WORD	wEcu;				// 036:2, 0~7ȣ�� ecu ����
	__BW(	etc,				// 038:2
				ecu8,	2,			// 8ȣ�� ecu ����
				ecu9,	2,			// 9ȣ�� ecu ����
				cm0,	2,			// 0ȣ�� cmsb ����, 0:NON/1:����/2:����/3:�����н�
				cm5,	2,			// 5ȣ�� cmsb ����
				cm9,	2,			// 9ȣ�� cmsb ����
				ato0,	2,			// 0ȣ�� ato ����
				ato9,	2,			// 9ȣ�� ato ����
				esk3,	1,			// 3ȣ�� esk ����, 0:OFF/1:ON
				esk6,	1);			// 6ȣ�� esk ����
	WORD	wFire;				// 040:2, 0~7ȣ�� ȭ�� ����, 0:����/1:ȭ��/2:����
	__BW(	fph,				// 042:2
				f8,		2,			// 8ȣ�� ȭ�� ����
				f9,		2,			// 9ȣ�� ȭ�� ����
				p0,		1,			// 0ȣ�� ���� �� ����, 0:����/1:ȣ��
				p1,		1,
				p2,		1,
				p3,		1,
				p4,		1,
				p5,		1,
				p6,		1,
				p7,		1,
				p8,		1,
				p9,		1,			// 9ȣ�� ���� �� ����
				spr,	2);

	// ���̽� 110 ���� ���
	__BW(	svc,				// 044:2,
				pau0,	2,			// 0ȣ�� pau ����, 0:NON/1:����/2:���/3:����
				pau9,	2,			// 9ȣ�� pau ����
				pis0,	2,			// 0ȣ�� pis ����
				pis9,	2,			// 9ȣ�� pis ����
				rtd0,	2,			// 0ȣ�� rtd ����
				rtd9,	2,			// 9ȣ�� rtd ����
				trs0,	2,			// 0ȣ�� trs ����
				trs9,	2);			// 9ȣ�� trs ����
	__BC(	tus,				// 046:1
				liu01,	1,			// 0ȣ�� liu1 ����, 0:����/1���
				liu02,	1,			// 0ȣ�� liu2 ����
				liu91,	1,			// 9ȣ�� liu1 ����
				liu92,	1,			// 9ȣ�� liu2 ����
				spr,	4);
	BYTE	cCus;				// 047:1, 1~8ȣ�� cc ����, 0:����/1���

	// ���̽� 108 ���/����
	BYTE	cBc;				// 048:1, 0~7ȣ�� bc ����, 0:off/1:on
	__BC(	hbc,				// 049:1
				hb1,	1,			// 1ȣ�� v3f hb ����
				hb2,	1,			// 2ȣ�� v3f hb ����
				hb4,	1,			// 4ȣ�� v3f hb ����
				hb7,	1,			// 7ȣ�� v3f hb ����
				hb8,	1,			// 8ȣ�� v3f hb ����
				bc8,	1,			// 8ȣ�� bc ����
				bc9,	1,			// 9ȣ�� bc ����
				spr,	1);
	BYTE	cAs;				// 050:1, 0~7ȣ�� as ����
	__BC(	lba,				// 051:1
				lb1,	1,			// 1ȣ�� v3f lb ����
				lb2,	1,			// 2ȣ�� v3f lb ����
				lb4,	1,			// 4ȣ�� v3f lb ����
				lb7,	1,			// 7ȣ�� v3f lb ����
				lb8,	1,			// 8ȣ�� v3f lb ����
				as8,	1,			// 8ȣ�� as ����
				as9,	1,			// 9ȣ�� as ����
				spr,	1);
	BYTE	cV3fIq[5];			// 052:5, v3f "���� ����", ���� ��ġ Im�׸�� ����.
	BYTE	cSpr57;				// 057:1
	BYTE	cEcuBcp[ECU_MAX];	// 058:10, ecu "bc ��"

	// ���̽� 120 ���� ��ġ
	WORD	wV3fFc[V3F_MAX];	// 068:10, v3f "fc" ����
	__BW(	v3fd,				// 078:2
				d1,		2,			// 1ȣ�� v3f, 0:C/1:NON/2:P/3:B
				d2,		2,			// 2ȣ�� v3f
				d4,		2,			// 4ȣ�� v3f
				d7,		2,			// 7ȣ�� v3f
				d8,		2,			// 8ȣ�� v3f
				spr,	6);
	BYTE	cV3fTbe[V3F_MAX];	// 080:5, v3f "pwm"

	// ���̽� 122 ���� ����
	BYTE	cSivVo[SIV_MAX];	// 085:3, siv "��� ����"
	// 171114
	//BYTE	cSivIo[SIV_MAX];	// 088:3, siv "��� ����"
	BYTE	cSivNull[SIV_MAX];
	BYTE	cSivFo[SIV_MAX];	// 091:3, siv "��� ���ļ�"
	WORD	wBatV[SIV_MAX];		// 094:6, "���͸� ����"

	// ���̽� 124 ����
	WORD	wBcv[ECU_MAX];		// 100:20, "BC ��"
	WORD	wV3fAbr[V3F_MAX];	// 120:10, "ȸ�� ������"
	BYTE	cEcuAsp[ECU_MAX];	// 130:10, ecu "as ��"

	// ���̽� 126 �ó���
	WORD	wHeat;				// 140:2, 0~7ȣ�� ����, 0:non/1:1/2:2/3:3
	__BW(	hf,					// 142:2
				h8,		2,			// 8ȣ�� ����
				h9,		2,			// 9ȣ�� ����
				f0,		1,			// 0ȣ�� ��ǳ, 0:off/1:on
				f1,		1,
				f2,		1,
				f3,		1,
				f4,		1,
				f5,		1,
				f6,		1,
				f7,		1,
				f8,		1,
				f9,		1,			// 9ȣ�� ��ǳ, 0:off/1:on
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
	WORD	wCool1;				// 146:2, 0~7ȣ�� �ù�� 1, 0:non/1:fan/2:half/3:full
	__BW(	cov,				// 148:2
				c8,		2,			// 8ȣ�� �ù�� 1
				c9,		2,			// 9ȣ�� �ù�� 1
				v0,		1,			// 0ȣ�� ȯ��, 0:off/1:on
				v1,		1,
				v2,		1,
				v3,		1,
				v4,		1,
				v5,		1,
				v6,		1,
				v7,		1,
				v8,		1,
				v9,		1,			// 9ȣ�� ȯ��, 0:off/1:on
				spr,	2);
	WORD	wCool2;				// 150:2, 0~7ȣ�� �ù�� 2, 0:non/1:fan/2:half/3:full
	__BW(	ctr,				// 152:2
				c8,		2,			// 8ȣ�� �ù�� 2
				c9,		2,			// 9ȣ�� �ù�� 2
				r0,		1,			// 0ȣ�� ��ȭ��, 0:off/1:on
				r1,		1,
				r2,		1,
				r3,		1,
				r4,		1,
				r5,		1,
				r6,		1,
				r7,		1,
				r8,		1,
				r9,		1,			// 9ȣ�� ��ȭ��, 0:off/1:on
				spr,	2);
	//BYTE	cLoadr[CID_MAX];	// 154:10, ž���� -> cEcuAsp�� ����� ��
	BYTE	cIntTemp[CID_MAX];	// 154:10, ���� �µ�
	WORD	wExtTemp[2];		// 164:4, �ܺ� �µ�

	// ���̽� 128 ���
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
	BYTE	cLength;			// 172:1, ���� ����

	// 170929, �߰�
	BYTE	cSivEs[SIV_MAX];
	BYTE	cEcuLw[ECU_MAX];
	// 171114
	WORD	wSivIo[SIV_MAX];	// siv "��� ����"
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
	DBFTIME	dbft;			// ���� �ð�
	WORD	wSubItem;		//
	WORD	wMsg;
	WORD	wRequired;		// �ҿ� �ð�, ��
	//WORD	wProperNo;		// 004:1, �� ��ȣ
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
