/**
 * @file	Draft.h
 * @brief	DTB data.
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#ifdef	__cplusplus
extern "C" {
#endif

#include "DuoTypes.h"
#include "../../../../Common/VarUnion.h"
#include "DevInfo2/DevInfo.h"
#include "Pio.h"
#include "Arrange.h"

#pragma pack(push, 1)

///	*** enviroment file ���� ***

#define	ENV_VERSION			0x202

#define	MAX_WHEELDIA		860		// mm
#define	MIN_WHEELDIA		780
#define	DEF_WHEELDIA		(MAX_WHEELDIA - MIN_WHEELDIA)

#define	MIN_TEMPSV			18
#define	MAX_TEMPSV			28
#define	DEF_TEMPSV			24

#define	MIN_VENTTIME		0
#define	MAX_VENTTIME		2
#define	DEF_VENTTIME		1

#define	LENGTH_WHEELDIA		7		// vvvf + tc	// 5
typedef struct _tagENVITEM {
	WORD	wVersion;						// 00-01
	DTSTIME	dt;								// 02-11
	BYTE	cWheelDiai[LENGTH_WHEELDIA];	// 12-18
	BYTE	cTempSv[CID_MAX];				// 19-28
	BYTE	cVentTime;						// 29
	BYTE	cCSum;							// 30
} ENVITEM;

#define	SIZE_ENVITEMS	(sizeof(BYTE) * (LENGTH_WHEELDIA + CID_MAX + 2))

typedef union _tagENVIRONS {
	ENVITEM		real;
	BYTE		c[sizeof(ENVITEM)];
} ENVIRONS, *PENVIRONS;

enum enMERROR {
	MERROR_NON = 0,
	MERROR_LOADENVIRONS,
	MERROR_OUTRANGEYEAR,		// HDU 580��
	MERROR_OUTRANGEMONTH,
	MERROR_OUTRANGEDAY,
	MERROR_OUTRANGEHOUR,
	MERROR_OUTRANGEMINUTE,
	MERROR_OUTRANGESECOND,
	MERROR_UNMATCHTIME,
	MERROR_OUTRANGEWHEELD0,
	MERROR_OUTRANGEWHEELD1,
	MERROR_OUTRANGEWHEELD2,
	MERROR_OUTRANGEWHEELD3,
	MERROR_OUTRANGEWHEELD4,
	MERROR_OUTRANGETEMPSV0,		// HDU 582��
	MERROR_OUTRANGETEMPSV1,
	MERROR_OUTRANGETEMPSV2,
	MERROR_OUTRANGETEMPSV3,
	MERROR_OUTRANGETEMPSV4,
	MERROR_OUTRANGETEMPSV5,
	MERROR_OUTRANGETEMPSV6,
	MERROR_OUTRANGETEMPSV7,
	MERROR_OUTRANGETEMPSV8,
	MERROR_OUTRANGETEMPSV9,
	MERROR_OUTRANGEVENTTIME,
	MERROR_MAX
};

///	*** enviroment file ���� ***

#define	CAREERS_VERSION		0x202

enum enCRRITEM {
	CRRITEM_DISTANCE = 0,
	CRRITEM_SIVPOWER,
	CRRITEM_V3FPOWER,
	CRRITEM_V3FREVIV,
	CRRITEM_MAX
};

typedef struct _tagCAREERITEM {
	WORD	wVersion;
	double	dbDistance;				// m
	double	dbSivPower[SIV_MAX];	// w/h
	double	dbV3fPower[V3F_MAX];	// w/h
	double	dbV3fReviv[V3F_MAX];	// w/h
} CRRITEM;

// 181012
// 180626
//typedef struct _tagTEMPDIAS {
//	BYTE	cDiai[LENGTH_WHEELDIA];		// wheel diameter, x - 780
//	BYTE	cTempSv[CID_MAX];			// DU�� ���� ����ڰ� �Է��� �� ������ ���� �µ�
//} TEMPDIAS;

typedef union _tagCAREERS {
	CRRITEM	real;
	BYTE	c[sizeof(CRRITEM)];
} CAREERS, *PCAREERS;

typedef struct _tagDEVPOWERS {
	QWORD	qwSivPower[SIV_MAX];
	QWORD	qwV3fPower[V3F_MAX];
	QWORD	qwV3fReviv[V3F_MAX];
} DEVPOWERS, *PDEVPOWERS;

typedef struct _tagCAREERITEMA {
	WORD		wVersion;
	QWORD		qwDistance;
	DEVPOWERS	dps;
} CRRITEMB;

typedef union _tagCAREERSB {
	CRRITEMB	real;
	BYTE		c[sizeof(CRRITEMB)];
} CAREERSB, *PCAREERSB;

#define	CMJOG_VERSION		0x202

typedef struct _tagSECOND179Y {
	WORD	wDay;				// 65535 / 365 day = 179 year
	DWORD	dwSec;				// 24 hour * 3600 sec = 86400 sec
} SEC179Y, *PSEC179Y;
#define	SECONDONEDAY		86400

typedef struct _tagCMJOGTIME {
	SEC179Y	sysrun;				// tcms ���� �ð�
	SEC179Y	cm[CM_MAX];			// cm ���� �ð�
} CMJOG, *PCMJOG;

typedef union _tagSECOND89Y {
	struct {
		DWORD	sec		: 17;	// 17bit, 1ffff = 131071 > 86440 sec
		DWORD	day		: 15;	// 15bit, 7fff = 32767 / 365 > 89 year
	} b;
	DWORD	a;
} SECOND89Y;

typedef struct _tagCMJOGTIMES {
	SECOND89Y	srt;				// system run time
	SECOND89Y	cm[CM_MAX];
} CMJOGTIMES;

typedef struct _tagCMJOGITEMB {
	WORD		wVersion;
	CMJOGTIMES	cjt;
} CMJOGITEMB;

typedef union _tagCMJOGB {			// 18bytes
	CMJOGITEMB	real;
	BYTE		c[sizeof(CMJOGITEMB)];
} CMJOGB, *PCMJOGB;

#define	BCAPTNESS_WORD		0x5a37

typedef struct _tagV3FBREAKERCNTS {	// 24bytes
	WORD	wAptness;
	WORD	wHbCnt[V3F_MAX];
	WORD	wLbCnt[V3F_MAX];
	WORD	wCSum;
} V3FBRKCNTS;

typedef union _tagV3FBREAKERCNTU {
	V3FBRKCNTS	v;
	BYTE		c[sizeof(V3FBRKCNTS)];
} V3FBRKCNTU, *PV3FBRKCNTU;

typedef struct _tagV3FBRKCNTF {		// CArchAux.h���� ����.
	bool		bHb[V3F_MAX];
	bool		bLb[V3F_MAX];
	V3FBRKCNTU	u;
} V3FBRKCNTF, *PV3FBRKCNTF;

typedef struct _tagPROPENSEECM {
	ENVIRONS	env;	// 31bytes
	CMJOGB		cmj;	// 18bytes
	V3FBRKCNTU	bcu;	// 24bytes
	// 210806
	WORD		wFire;	// for only 24
} PROPENSEECM;

enum enDEVTRACEITEM {
	TRACEITEM_SIV = 12,
	TRACEITEM_V3F,
	TRACEITEM_ECU,
	TRACEITEM_ALIGN
};

typedef struct _tagTRACENOTIFY {
	WORD	wDevIDMap;			// b0~b9:���õ� ������ ��Ʈ���� ǥ��, b12:siv, b13:v3f, b14:ecu, b15:align
	WORD	wChapID;
	WORD	wPageID;
} TRACENOTIFY;

typedef struct _tagTRACESCROLL {
	TRACENOTIFY	req;
	TRACENOTIFY	ans;
	BYTE	c[SIZE_TRACETEXT];
} TRACESCROLL;

typedef union _tagPROPENSITY {
	PROPENSEECM	ecm;
	CAREERS		crr;
	// 200218
	BMSCVS		bcv;		// byte[50], battery cell voltage
	//TEMPDIAS	tds;		// CC does not have Env.
	TRACESCROLL	trc;
} PROPENST, *PPROPENST;

typedef struct _tagCONSULTANT {
	BYTE		cID;
	PROPENST	roll;
} CONSULTANT, *PCONSULTANT;

enum enCONSULTID {
	CSTID_NON = 0,
	CSTID_ENVIRONS,
	CSTID_CAREERS,
	// 200218
	CSTID_BMSEX,
	//CSTID_TEMPDIA,	// 181012
	//CSTID_CMJOG,
	CSTID_TRACE
};

///	*** draft contents ***

#define	MAX_BATTERY		100
#define	LOW_BATTERY		70

// dtb head frame
typedef struct _tagPROLOGUE {
	WORD		wAddr;
	DWORD		dwOrder;		// dword greater than 0
	BYTE		cProperID;
	//BYTE		cFlow;			// use to master changing or something, �Ϸ������� �Ⱦ���.
	BYTE		cCarLength;		// lsb:10/8/6/4, msb:0x40(A)/0x80(B)
	BYTE		cPermNo;		// �⵿ �� �Է��� �����Ѵ�.
	// 170801
	//DEVTIME		dtm;
	DBFTIME		dbft;
	// 171019
	//WORD		wVersion;
	BYTE		cVersion;
	WORD		wOtrn;
} PROLOG, *PPROLOG;

enum enUNIFYFLAGS {
	UNIF_TIMERF = 0x04,
	UNIF_TIMEAF = 0x08,
	UNIF_ENVRF = 0x10,
	UNIF_ENVAF = 0x20,
	UNIF_CRRRF = 0x40,
	UNIF_CRRAF = 0x80
};
// 200218
enum enUNIFYFLAGSEX {
	UNIF_CLFCRF = 0x40,
	UNIF_CLFCAF = 0x80
};

typedef struct _tagSIGNATURE {
	__BC(	gcmd,
				mcd,	1,	// master change demand
				domin,	1,	// dominate control & monitor
				unifTr,	1,	// time set request
				unifTa,	1,	// ... agree
				unifEr,	1,	// unify environs
				unifEa,	1,	// ... agree
				unifCr,	1,	// unify careers
				unifCa,	1);	// ... agree
	__BC(	ecmd,
				dc,		1,	// dc����(UDOB_DCL)�� �Ѵ� ��Ʈ
				ac1,	1,	// ac����(UDOB_ACL1)�� �Ѵ� ��Ʈ
				ac2,	1,	// ac����(UDOB_ACL2)�� �Ѵ� ��Ʈ
				albo,	1,	// albo����(UDOB_ALBO)�� �Ѵ� ��Ʈ
				clrvb,	1,	// v3f breaker counter clear
				tdc,	1,	// erase trace data
				relblk,	1,	// release service device block
				fire,	1);	// ȭ�� ������ pis�� ����.
	__BC(	scmd,
				cm0,	1,	// 0ȣ�� cm�� �۵���Ű�� ��Ʈ
				cm1,	1,	// 5ȣ�� cm�� �۵���Ű�� ��Ʈ
				cm2,	1,	// 9ȣ�� cm�� �۵���Ű�� ��Ʈ
				eskp,	1,	// HVAC�� �����ϴ� esk
				esk0,	1,	// 2ȣ�� esk�� �۵���Ű�� ��Ʈ
				esk1,	1,	// 6ȣ�� esk�� �۵���Ű�� ��Ʈ
				lrr,	1,	// HVAC sdr's req-lrr(TEXT1:���� �氨)�� �����ϴ� ��Ʈ
				lor,	1);	// HVAC sdr's req-lor(TEXT1:���� ����)�� �����ϴ� ��Ʈ
	// 200218
	__BC(	scmdEx,
				// 210630
				//sp,		3,
				// 210720
				sp,		2,
				hicmd,	1,	// hvac inspect done(block) for du page
				lab,	1,	// 200218, GetLab(), 170914
				fireAck,	1,	// 210706
				fireMbc,	1,	// 210706
				unifLfcr,	1,	// line fail counter clear request
				unifLfca,	1);	// ... agreee
	__BC(	hcmd,			// control hvac(ctrl of sdr)
				cmd,	4,	// HVAC sdr's ctrl-cmd(TEXT2:cmd) - HVAC ���� ���
				ef,		1,	// HVAC sdr's ctrl-lf(TEXT2:��� ��)�� �����ϴ� ��Ʈ
				apdk,	1,	// HVAC sdr's ctrl-apdk(TEXT2:���� ��ȭ��)�� �����ϴ� ��Ʈ
				elff,	1,	// HVAC sdr's ctrl-elff(TEXT2:�ٱ� ȯ�� ��)�� �����ϴ� ��Ʈ
				lff,	1);	// HVAC sdr's ctrl-lff(TEXT2:�ٱ� ȯ�� ��)�� �����ϴ� ��Ʈ
	__BC(	hcmdEx,
				hts,	1,	// hvac temperature sync.
				vnt1,	1,	// HVAC sdr's ref-vnt1(TEXT3:��� �� 1)�� �����ϴ� ��Ʈ
				vnt2,	1,	// HVAC sdr's ref-vnt2(TEXT3:��� �� 2)�� �����ϴ� ��Ʈ
				// 200218
				//lab,	1,	// GetLab(), 170914
				// 201016
				//sp,		1,
				apdk2,	1,
				main,	1,	// oblige's clone, only use for logbook
				head,	1,
				active,	1,
				cab,	1);
	__BC(	insp,
				siv,	1,	// siv inspect, sivr.cmd.b.tsr = 1
				v3f,	1,
				ecub,	1,
				ecua,	1,
				hvac,	1,
				horc,	1,	// 1:heat, 0:cool
				cancel,	1,
				sp,		1);
	__BC(	pcmd,
				dt16,	1,	// msb of wDoorTest
				dt17,	1,
				dt18,	1,
				dt19,	1,
				dtest,	1,
				// 181019
				//spr,	2,
				// 200218
				//spr,	1,
				devEx,	1,	// device extension
				rtdtr,	1,	// timeset by rtd
				dow,	1);	// 171130, dow of main pis
	WORD	wDoorTest;		// door test, control reg.b.dlt of sdr
} SIGNATURE, *PSIGNATURE;

typedef struct _tagMOTIVE {	// control by CSteer
	BYTE	cMode;
	BYTE	cNotch;
	__BC(	lead,		// control only by CStick
				f,		1,	// �� ����
				r,		1,	// �� ����
				atom,	1,
				// 180704
				epaim,	1,	// pis auto(0)/manual(1) by tcms, each side
				epreq,	1,	// pis mode demand by tcms, each side, �� �κ�Ʈ�� ����̹Ƿ� ����ǰ� �������.
				pisaim,	1,	// pis auto(0)/manual(1) by tcms, judge
				pisuse,	1,	// pis control by head(0)/tail(1), �� �κ�Ʈ�� ���¸� ��Ÿ���Ƿ� ��� �����ȴ�.
				sp,		1);
	BYTE	cEffType;	// 0:�߸�,	1:����,	2:����
	BYTE	cEffort;	// 180105, ���ڴ� ����� ����� ��. 10~90 -> 0~100(%)
	// 171019
	//BYTE		cDoorMode;
} MOTIVE;

enum enMOTIVMODE {
	MOTIVMODE_NON = 0,
	MOTIVMODE_AUTORECURRENCE,
	MOTIVMODE_FULLAUTO,
	MOTIVMODE_AUTO,
	MOTIVMODE_MANUAL,
	MOTIVMODE_YARD,
	MOTIVMODE_EMERGENCY,
	MOTIVMODE_OVERLAP,
	MOTIVMODE_MAX
};
#define	IsAutoMotive(x)	(x == MOTIVMODE_AUTORECURRENCE || x == MOTIVMODE_FULLAUTO || x == MOTIVMODE_AUTO)
#define	IsManualMotive(x)	(x == MOTIVMODE_MANUAL || x == MOTIVMODE_YARD || x == MOTIVMODE_EMERGENCY || x == MOTIVMODE_NON)

enum enNOTCH_INDEX {
	NOTCHID_ERR = 0,
	NOTCHID_EB,
	NOTCHID_B7,
	NOTCHID_B6,
	NOTCHID_B5,
	NOTCHID_B4,
	NOTCHID_B3,
	NOTCHID_B2,
	NOTCHID_B1,
	NOTCHID_C,
	NOTCHID_P1,
	NOTCHID_P2,
	NOTCHID_P3,
	NOTCHID_P4
};

enum enEFFORTTYPE {
	EFFORT_COASTING,
	EFFORT_BRAKING,
	EFFORT_POWERING
};

// 181003
#define PWMHIGH_MIN		100		// 5%
#define	PWMHIGH_MAX		1900	// 95%

#define	PWMPERIOD_MAX	2040	// 2Khz + 40
#define	PWMPERIOD_MIN	1960	// 2Khz - 40
#define	PWMPERIOD_TOL	40		// 2%
#define	NOTCH_TOLERANCE	2.5

// 180219
//#define	PWMRATE_MAX		95
//#define	PWMRATE_MIN		5
#define	PWMRATE_MAX		90
#define	PWMRATE_MIN		10
#define	PWMRATE_MINP1	23		// 180219, 10~90 -> 0~100(28->22.5)	//24	// 180108, for v3f inspect

typedef struct _tagDRIFT {
	DWORD	dwSpeed;
	BYTE	cEbrk;
	BYTE	cNbrk;
} DRIFT;

enum enEBCAUSE {
	EBCAUSE_MC,
	EBCAUSE_ATP,
	EBCAUSE_HSW,
	EBCAUSE_TSW,
	EBCAUSE_MRPS,
	EBCAUSE_OFFRAIL,
	EBCAUSE_NONHCR,
};

enum enNORMALBRAKE {
	NBRAKE_PARKING,
	NBRAKE_SECURITY,
	NBRAKE_FSB,
	NDETECT_ZEROSPEED
};

#define	REVERSE_MAXSPEED	25000		// ���� �ְ� �ӵ�

typedef union _tagSVFFLAP {
	SIVFLAPEX	s;			// 180625
	SIVFLAP2	s2;			// 200218
	V3FFLAP		v;
} SVFFLAP, *PSVFFLAP;

typedef union _tagSVCFLAP {
	PAUFLAP		u;
	PISFLAP		i;
	TRSFLAP		t;
	FDUFLAPAB	f;
	BMSFLAPAB	m;
} SVCFLAP, *PSVCFLAP;

// 200218
typedef struct _tagCMSBBLADE {
	WORD	wSign;
	CMSBFLAP	c;
} CMSBBLADE, *PCMSBBLADE;
#define	SIGN_CMSBFLAP		0x375a

typedef union _tagHCBFLAP {
	HVACFLAP	hv;
	CMSBBLADE	cb;
} HCBFLAP, *PHCBFLAP;

enum enADCONVCH {
	ADCONVCH_BATTERY,
	ADCONVCH_PRESSURE,
	ADCONVCH_OUTTEMP,
	ADCONVCH_MAX
};

#define	OFFSETV_BATTERY		4		// ���� ���� �輱 ���̷� �̸�ŭ �������� �� ����.

//#define	CURVECH_MAX		3
typedef struct _tagANLOGV {
	DWORD	dwPwm;
	WORD	wTacho;
	WORD	wCurve[ADCONVCH_MAX];
} _ANALOGV;

enum enMMSTATE {
	MMSTATE_INVALIDQUEUE = 1,
	MMSTATE_INVALIDSHAREMEM,
	MMSTATE_INVALIDMAPPING,
	MMSTATE_INVALIDLOGAREA
};

// 200218
typedef struct _tagREDCNT {
	BYTE	cDevID;
	WORD	wCnt;
} REDCNT, *PREDCNT;

typedef struct _tagDTBCONTENTS {
	PROLOG			prol;
	DBFTIME			logMatch;					// tc0, tc9, rtd� ����� logbook, trouble file�� �̸��� �����ϰ� �ϱ����� �����Ѵ�.
	SIGNATURE		sign;
	MOTIVE			motiv;
	DRIFT			drift;
	// 180626
	//BYTE			cDiai[LENGTH_WHEELDIA];		// wheel diameter, x - 780
	signed char		cExtTemp;
	// 180626
	//BYTE			cTempSv[CID_MAX];			// DU�� ���� ����ڰ� �Է��� �� ������ ���� �µ�
	_OCTET			inp[2];						// ������ �Է�
	_DUET			outp;						// ������ ���
	_ANALOGV		avs;						// �Ƴ��α� �Է�
	ATOFLAP			ato;						// ato ��� ������
	SVCFLAP			svc;						// ���� ��ġ ��� ������
	SVFFLAP			svf;						// siv, vvvf ��� ������
	// 200218
	//ECUFLAP			ecu;						// ecu sdr/sda
	ECUFLAPS		ecu;
	//HVACFLAP		hvac;						// hvac sdr/sda
	HCBFLAP			hcb;
	DCUFLAP			dcul;						// dcu ��� ������
	DCUFLAP			dcur;
	// 170801
	//BYTE			cRtdState;
	//BYTE			cRtdServe;
	RTDSTATEA		rtd;
	WORD			wFpgaVersion;
	BYTE			cPoleLine;
	BYTE			cPoleBlock;
	BYTE			cSvcCtrlCode;				// ���� ��ġ �켱 ���� �ڵ�(enSERVICEDEVICECTRLCODEBIN), B0~3:head code, B4~7:tail code
	// 200218
	// ��ġ�� �þ�� ������ �����ϹǷ� �����Ѵ�.
	//WORD			wPoleRed[PDEVRED_MAX];		// ��� ���� ī����
	//WORD			wLocRed[DEVID_DCUL];		// siv-v3f-ecu-hvac������.. dcu�� land�� ó���Ѵ�.
	REDCNT			polef;
	REDCNT			localf;						// siv-v3f-ecu-hvac-cmsb������.. dcu�� CUniv���� ó���Ѵ�.
												// dtb ��� �ֱ�� DCU ��� �ֱⰡ ��ġ�ϹǷ�...
	BYTE			cMmVer;
	BYTE			cMmState;
	CONSULTANT		cst;
} DTBCONTENTS;

#define	SIZE_RECIP			512

typedef struct _tagRECIP {
	DTBCONTENTS	real;
	BYTE		cDummy[SIZE_RECIP - sizeof(DTBCONTENTS)];
} RECIP, *PRECIP;

typedef struct _tagDOZEN {
	RECIP	recip[FID_MAX];
} DOZEN, *PDOZEN;

#pragma pack(pop)

#ifdef	__cplusplus
}
#endif
