/*
 * TrainArch.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	TRAINARCH_H_
#define	TRAINARCH_H_

#include <inttypes.h>
#include <Define.h>

#pragma pack(1)

#define	CYCLETIME_TBUS			6	// 6ms
#define	CYCLECLOSE_TBUS			10	// 100ms
#define	CYCLEOVER_TBUSPERIOD	15	// 150ms

#define	BROADCAST_ADDR	0xff
#define	INITIAL_ADDR	0xf0

#define	DEFAULT_WHEELDIA	860
#define	MIN_WHEELDIA		780
#define	MAX_WHEELDIA		860

enum  {	CAR_HEAD = 0,
		CAR_HEADBK,
		CAR_TAIL,
		CAR_TAILBK,
		CAR_TCFRONTORG,
		CAR_M1FRONTORG,
		CAR_M2FRONTORG,
		CAR_T1ORG,
		CAR_T2ORG,
		CAR_M1REARORG,
		CAR_M2REARORG,
		CAR_TCREARORG,
		CAR_MAX				// 12
};

#define	OPPO_TCDI			CAR_TAIL	// opposite tc input bit read index
#define	OPPO_TCDO			1			// opposite tc output bit write index
#define	BOTH_TCDO			2
#define	CAR_CC				CAR_TCFRONTORG
#define	ONLYCC(n)			(n - CAR_CC)
#define	ALLCAR_FLAGS		c_pTcRef->real.cid.wAll
#define	SCENE_ID_ALLCAR		17
#define	SCENE_ID_HEADCAR	16

#define	LENGTH_TC		CAR_TCFRONTORG
#define	LENGTH_CC		(CAR_MAX - LENGTH_TC)

#define	LENGTH_SIV		2
#define	LENGTH_V3F		4
#define	LENGTH_AUX		2
#define	LENGTH_CMSB		2
#define	LENGTH_PWM		1
#define	LENGTH_ESK		1

#define	BUSADDR_HIGH		0x5a00
#define	DEVADDR_HIGH		0x7300

#define	DEVFLOW_NORMALR		0x20
#define	DEVFLOW_NORMALA		0x30
#define	DEVFLOW_TRACER		0x21
#define	DEVFLOW_TRACEA		0x31

#define	DEVFLOW_STX			0x2
#define	DEVFLOW_ERR			0xff
#define	DEVFLOW_ETX			0x3
#define	LOCALFLOW_STX		0x2
#define	LOCALFLOW_ETX		0x3
#define	LOCALFLOW_NORMAL	0x20
#define	LOCALFLOW_TRACE		0x21
#define	LOCALFLOW_INSPECT	0x30
#define	LOCALCTRL			0x13

enum  {	LOCALID_ATCA = 0,	// 0, 1
		LOCALID_ATCB,		// 1, 2
		LOCALID_ATO,		// 2, 4
		LOCALID_TWC,		// 3, 8
		LOCALID_DUC,		// 4, 0x10
		LOCALID_CSC,		// 5, 0x20
		LOCALID_PIS,		// 6, 0x40
		LOCALID_TRS,		// 7, 0x80
        LOCALID_HTC,		// 8, 0x100
		LOCALID_PSD,		// 9, 0x200
		LOCALID_MAX
};

#define	LOCALID_SERVICE_BASE	LOCALID_CSC
#define	LOCALID_SERVICE_LENGTH	LOCALID_MAX - LOCALID_SERVICE_BASE
#define	_SERVICEID(n)	(n - LOCALID_SERVICE_BASE)

#define	LOCALADD_ATC	0xff30
#define	LOCALADD_ATO	0xff40
#define	LOCALADD_TWC	0xff50
#define	LOCALADD_DUC	0x7373
#define	LOCALADD_PIS	0x7304
#define	LOCALADD_TRS	0x7305
#define	LOCALADD_HTC	0x7306
#define	LOCALADD_PSD	0x7307
#define	LOCALADD_CSC	0x7308

#define	LOCALERROR_NON		0
#define	LOCALERROR_BCC		0x10
#define	LOCALERROR_STX		0x20
#define	LOCALERROR_ETX		0x30
#define	LOCALERROR_ADDR		0x40
#define	LOCALERROR_MFB		0x50
#define	LOCALERROR_LENGTH	0x60
#define	LOCALERROR_TIMEOUTA	0x70
#define	LOCALERROR_TIMEOUTB	0x80
#define	LOCALERROR(p, f)\
	do {\
		p = f | (p & 0xf);\
	} while (0)

#define	DEVADD_V3F		0x7301
#define	DEVADD_ECU		0x7302
#define	DEVADD_SIV		0x7303
#define	DEVADD_DCU		0x7309
#define	DEVADD_CMSB		0x730a

enum  {	DEVID_SIV = 0,
		DEVID_V3F,
		DEVID_ECU,
		DEVID_DCU,
		DEVID_DCU2,
		DEVID_CMSB,
		DEVID_MAX
};

#define	DEVBEEN_SIV		(1 << DEVID_SIV)						// 1
#define	DEVBEEN_V3F		(1 << DEVID_V3F)						// 2
#define	DEVBEEN_ECU		(1 << DEVID_ECU)						// 4
#define	DEVBEEN_DCU		((1 << DEVID_DCU) | (1 << DEVID_DCU2))	// 0x18
#define	DEVBEEN_CMSB	(1 << DEVID_CMSB)						// 0x20
#define	DEVBEEN_AUX		(1 << (DEVID_CMSB + 1))					// 0x40
#define	DEVBEEN_PWM		(1 << (DEVID_CMSB + 2))					// 0x80
#define	DEVBEEN_ESK		(1 << (DEVID_CMSB + 3))					// 0x100
//#define	DEVBEEN_PANTO	0x0080
#define	DEVBEEN_CC	DEVBEEN_DCU | DEVBEEN_ECU

enum  {	TRACEID_SIV = 0,
		TRACEID_V3F,
		TRACEID_ECU,
		TRACEID_ATC,
		TRACEID_ATO,
		TRACEID_MAX
};

#define	WHEELBYATC(p)	((double)DEFAULT_WHEELDIA - (double)((p->d.st.tdws.a & 0xf) * 5))

// BEGIN ENVIROMENT { ====================================================
//#define	SIZE_ENTIRE		4
//typedef struct _tagENTIREELEMENT {
//	double	dbDist;			// accumulated distance
//	double	dbTime;			// accumulated running time
//	double	dbPower;		// accumulated power consumption
//	double	dbRevival;		// accumulated recovery power
//	WORD	wTrainNo;
//	DEVTIME	devTime;
//} ENTIREELEM;
//
//typedef union _tagENTIREINFO {
//	ENTIREELEM	elem;
//	BYTE	n[sizeof(ENTIREELEM)];
//} ENTIREINFO;

#define	SIZE_WHEELCAR	4
#define	SIZE_WHEELAXEL	4

//typedef struct _tagENVINFO {
//	ENTIREINFO	entire;
//	WORD	w[SIZE_WHEELCAR][SIZE_WHEELAXEL];
//} ENVINFO, *PENVINFO;
//
//typedef union _tagENVARCH {
//	ENVINFO	real;
//	BYTE	nRoll[sizeof(ENVINFO)];
//} ENVARCH, *PENVARCH;		// save to "/nand/env" file

#define	SIZE_ENTIREEXTENSTION	8
typedef struct _tagENTIREELEMENTEXTENSION {
	double	dbDist;
	double	dbTime;
	double	dbSivPower;
	double	dbV3fPower;
	double	dbRevival;
	double	dbReserve1;
	double	dbReserve2;
	double	dbReserve3;
	WORD	wTrainNo;
	DEVTIME	devTime;
} ENTIRELEMENTEEXTENSION;

typedef union _tagENTIREINFOEXTENSION {
	ENTIRELEMENTEEXTENSION	elem;
	BYTE	n[sizeof(ENTIRELEMENTEEXTENSION)];
} ENTIREINFOEXTENSION;

typedef struct _tagENVINFOEXTENSION {
	ENTIREINFOEXTENSION	entire;
	WORD	w[SIZE_WHEELCAR][SIZE_WHEELAXEL];
	BYTE	dummy[16];
} ENVINFOEXTENSION, *PENVINFOEXTENSION;

typedef union _tagENVARCHEXTENSION {
	ENVINFOEXTENSION	real;
	BYTE	nRoll[sizeof(ENVINFOEXTENSION)];
} ENVARCHEXTENSION, *PENVARCHEXTENSION;

// END ENVIROMENT { ====================================================

typedef struct _tagCARINFO {
	BYTE	nID;			// car id(0-7)
	BYTE	nLength;		// car length(4,6,8)
	DWORD	dwThumbWheel;	// thumbwheel switch number at front panel
} CARINFO, *PCARINFO;

typedef struct _tagDRECVINFO {
	BYTE	nCh;		// MFB board channel no.
	BYTE	nState;		// received state
	WORD	wLength;	// received length(byte)
} DRECVINFO;

typedef union _tagDRECV {
	DRECVINFO	info;
	DWORD		dw;
} DRECV;				// used to CLocal, CDevice, CPocket

enum  {	DATATYPE_NORMAL = 0,
		DATATYPE_TRACE,
		DATATYPE_INSPECT,
		DATATYPE_MAX
};

typedef struct _tagTRLENGTH {
	WORD	t;
	WORD	r;
} TRLENGTH;			// transmission length by normal, trace, inspect

#define	FSBTERM_FAULTV3FN		2	// FSB conditions
#define	FSBTERM_FAULTECUN		2
#define	FSBTERM_FAULTBCCN		3
#define	MSBTERM_FAULTECUN		1

typedef struct _tagTIMERANGE {
	uint64_t	begin;
	WORD		wCurrent;
} TIMERANGE;

typedef struct _tagDISTRANGE {
	DWORD		dwBegin;
	WORD		wCurrent;
} DISTRANGE;

typedef struct _tagRANGEITEM {
	WORD		s;
	TIMERANGE	t;
	DISTRANGE	d;
} RANGEITEM;

typedef struct _tagTRIALINFO {
	RANGEITEM	pwr;
	RANGEITEM	brk;
	SHORT		vAccel;
	DWORD		dwSpeed;
	DWORD		dwDist;
	WORD		wIm[4];
	WORD		wBcp[4];
	double		dbBeginDist;
	uint64_t	sampleClk;
	WORD		wSampleIndex;
	DWORD		dwSampleSpeed;
	DWORD		dwSample[10];
} TRIALINFO, *PTRIALINFO;	// trial data

typedef struct _tagOPERATETIME {
	uint64_t	cur;
	double		avr;
	double		min;
	double		max;
	BOOL		bUse;
	BOOL		bNull;
} OPERATETIME, *POPERATETIME;	// operation time for debug monitor

typedef struct _tagDEVICESTATE {
	WORD	wFailCC;		// 1-2, bit form of failed cc
#define	FSBTERM_CC		2

	WORD	wFailSiv;		// 3-4, bit form of failed siv
	WORD	wFailV3f;		// 5-6, bit form of failed vvvf
	WORD	wFailEcu;		// 7-8, bit form of failed ecu
	WORD	wFailCmsb;		// 9-10, bit form of failed cmsb
	WORD	wFailBccs1;		// 11-12, bit form of failed bccs1
	WORD	wFailBccs2;		// 13-14, bit form of failed bccs2
	WORD	wBoucs;			// 15-16, bit form of boucs input at each cc
	WORD	wMrp;			// 17-18, bit form of mrps input(negate) at each cc
#define	LOW_MRP		60		// 8.0Kg/cm2
#define	HIGH_MRP	90

	WORD	wNrbi;			// 19-20, bit form of nrbd input at each cc
	WORD	wNrbc;			// 21-22, bit form of nnrbd serial at each cc
	WORD	wCprl;			// 23-24, bit form of cprl output of each cc
	WORD	wAscs1;			// 25-26, bit form of ascs1 input at each cc
	WORD	wAscs2;			// 27-28, bit form of ascs2 input at each cc
	// Modified 2013/11/02
	WORD	wK1;			// 29-30, bit form of k4 input at each cc
	WORD	wK4;			// 31-32, bit form of k4 input at each cc
	WORD	wK6;			// 33-34, bit form of k6 input at each cc
	BYTE	nSivkmon[2];	// 35-36, count, Ed over 1200V during 6sec.(60cycle) then 264 Error
#define	DEVMON_SIVK			60	// 0.1 * 60 = 6sec
#define	VALID_SIVED			1200

	WORD	wFmcco;			// 37-38
	WORD	wFmccm;			// 39-40, monitor after fmcco
//	acoreg	wFmccm	wFmcco
//	1		0		0		// step1, monitor vco, to 0-1
//	1		0		1		// step2, monitor vco to on, if 1 then 1-1
//	1		1		1		// step3, monitor vco to off,
//	0		X		1		// step1, monitor vco, to 1-0
//	0		1		0		// step2, monitor vco, if 0 then 0-0

	BYTE	nState;			// 41, opend any door, not used
#define	DEVSTATE_SOMEDOOR	1

	BYTE	nEcuNrDetect;	// 42
#define	NRDETECTFROM_DI		1	// not release detect from input
#define	NRDETECTFROM_SDI	2	// not release detect from serial input

	_TV_	vV3fTbeL;		// 43-44, tebe for send to ATO
	// Modified 2013/11/02
	WORD	wV3fSqts;		// 45-46, collection of aV3f.cmd.sqts
	// Modified 2013/03/12
	WORD	wV3fRstReq;		// 47-48, collection of aV3f.cmd.rstreq
	WORD	wV3fRstCmd;		// 49-50, v3f reset command
} DEVICESTATE, *PDEVICESTATE;

typedef struct _tagCARFORMATION {
	BYTE	nLength;		// car length, 5th unit of thumbwheel at front panel
	BYTE	nSerialNo;		// car no., 4th and 3th unit
	BYTE	nOrgNo;			// original no. 2nd and 1st unit
	BYTE	nNull;
} CARFORMATION, *PCARFORMATION;

typedef struct _tagFSBTERM {
	BYTE	nNull;
	BYTE	nV3f;		// failed vvvf length, for fsb
	BYTE	nEcu;		// failed ecu length
	BYTE	nBcc;		// failed bcc length
} FSBTERM;

typedef struct _tagDEVESID {
	BYTE	nSivI[2];	// car id which has siv
	BYTE	nV3fI[4];	// car id which has vvvf
	BYTE	nAuxI[2];	// car id which has aux compressor
	BYTE	nCmsbI[2];	// car id which has cmsb
	BYTE	nPwmI;		// car id which has pwm
	BYTE	nEskI;		// car id which has esk relay
} DEVESID;

typedef union _tagCARNAME {
	struct {
		BYTE	nTcFront;
		BYTE	nM1Front;
		BYTE	nM2Front;
		BYTE	nT1;
		BYTE	nT2;
		BYTE	nM1Rear;
		BYTE	nM2Rear;
		BYTE	nTcRear;
	} each;
	BYTE	seq[8];
} CARNAME;	// car id (04-11), used to car combination(4, 6, 8)

typedef struct _tagCARID {
	//CARNAME	cn;
	WORD	wAll;	// bit form of car combination
	DEVESID	oc;		// never use ONLYCC()!!!
	FSBTERM	fsbTerm;	// fsb conditions
	BYTE	nTcAddr[LENGTH_TC];
} CARID, *PCARID;

#define	__SIVA		(c_pTcRef->real.cid.oc.nSivI[0])	// 04 to 00, 11 to 7
#define	__SIVB		(c_pTcRef->real.cid.oc.nSivI[1])
#define	__V3FA		(c_pTcRef->real.cid.oc.nV3fI[0])
#define	__V3FB		(c_pTcRef->real.cid.oc.nV3fI[1])
#define	__V3FC		(c_pTcRef->real.cid.oc.nV3fI[2])
#define	__V3FD		(c_pTcRef->real.cid.oc.nV3fI[3])
#define	__AUXA		(c_pTcRef->real.cid.oc.nAuxI[0])
#define	__AUXB		(c_pTcRef->real.cid.oc.nAuxI[1])
#define	__CMSBA		(c_pTcRef->real.cid.oc.nCmsbI[0])
#define	__CMSBB		(c_pTcRef->real.cid.oc.nCmsbI[1])
#define	__PWM		(c_pTcRef->real.cid.oc.nPwmI)
#define	__ESK		(c_pTcRef->real.cid.oc.nEskI)
#define	_ISOCC(n)	(n < 0x10)
#define	_REALCC(n)	(n + CAR_CC)
#define	_SHIFTV(n)	(_ISOCC(n) ? 1 << n : 0)

typedef struct _tagHCRINFO {
	BYTE	nState;
#define	HCRSTATE_DEBOUNCING			1

	BYTE	nBuf;
	BYTE	nMasterID;
#define	HCR_ONLYID			0xf
#define	HCR_NOACTIVE		0x10
#define	HCR_DIFFERENT		0x20
#define	REQ_PERTDIR			0x80
#define	ACK_PERTDIR			0x81
#define	REQ_PERTFILE		0x82
#define	ACK_PERTFILE		0x84
#define	ABORT_PERT			0x85

	BYTE	nDeb;
#define	DEBOUNCE_EXCHANGEHCRSMALL	5
#define	DEBOUNCE_EXCHANGEHCRLARGE	20	// 500

	WORD	wMainWarningInhibitTimer;
// Modified 2013/11/02
#define	TIME_MAINWARNINGINHIBIT		7000/ TIME_INTERVAL	//5000 / TIME_INTERVAL	// 3000 / TIME_INTERVAL
#define	IsMainWarn()	(c_pTcRef->real.hcr.wMainWarningInhibitTimer == 0)

	WORD	wAidWarningInhibitTimer;
#define	TIME_AIDWARNINGINHIBIT	60000 / TIME_INTERVAL	//5000 / TIME_INTERVAL

} HCRINFO, *PHCRINFO;

#define	IsPermitMain()\
	((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&\
	(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&\
	c_pTcRef->real.hcr.wMainWarningInhibitTimer == 0)

#define	IsPermitMainB()\
	((c_tcRef.real.nObligation & OBLIGATION_HEAD) &&\
	(c_tcRef.real.nObligation & OBLIGATION_ACTIVE) &&\
	c_tcRef.real.hcr.wMainWarningInhibitTimer == 0)

#define	IsPermitAid()\
	((c_pTcRef->real.nObligation & OBLIGATION_HEAD) &&\
	(c_pTcRef->real.nObligation & OBLIGATION_ACTIVE) &&\
	c_pTcRef->real.hcr.wAidWarningInhibitTimer == 0)

#define	IsPermitAidB()\
	((c_tcRef.real.nObligation & OBLIGATION_HEAD) &&\
	(c_tcRef.real.nObligation & OBLIGATION_ACTIVE) &&\
	c_tcRef.real.hcr.wAidWarningInhibitTimer == 0)

enum  {	TDIR_UNKNOWN,
		TDIR_FIXCONTINUE,	// No change stick at head and tail car
		TDIR_TURNOVER,		// change stick
		TDIR_TURNOVERD,		// change in driverless
		TDIR_CLASH,			// Invalid stick by opponent
		TDIR_NOTOPPOFR,		// Not found tail car
		TDIR_NOTOPPON
};							// use to CPath, control master car

typedef struct _tagTDIRSTATE {
	BYTE	nDir;			// TDIR_XXX
	BYTE	nDeb;			// debounce
#define	DEBOUNCE_TDIR	3

	BYTE	nDirVerify;
	BYTE	nCallID;
} TDIRSTATE, *PTDIRSTATE;

typedef struct _tagBUSCYCLE {
	WORD	wState;				// 0-1
#define	TCSTATE_RUN			1		// not use
#define	TCSTATE_MSRTIME		2		// measure each process time, view by Tcm program
#define	TCSTATE_READINP		4		// not use
#define	TCSTATE_FRAMEOPEN	8		// received tcfirm frame at slave tc
#define	TCSTATE_FRAMEEND	0x10	// received all firm
#define	TCSTATE_LOCALING	0x20	// enable transmission to local device of tc
#define	TCSTATE_LOCALPACK	0x40	// do local device frame packing
#define	TCSTATE_ERROR		0x80	// error at train bus
#define	TCSTATE_XCHDRVLESS	0x100
#define	TCSTATE_XCHMASTER	0x200	// change master tc

	WORD	wCur;				// 2-3, counter of interval, compare to wClose
	WORD	wClose;				// 4-5, constant 10, 10ms(interval time) * 10 = 100ms -> process sync. time
	WORD	wTime;				// 6-7, constant 6, train bus respond time
	WORD	wPeriodOverTime;	// 8-9, bus monitor time, 150ms
	WORD	wPeriodOverLampOnTimer;	// 10-11, bus over lamp on timer
	WORD	wXch;				// 12-13
#define	CYCLE_XCHMASTER1	500 / TIME_INTERVAL
#define	CYCLE_XCHMASTER0	1000 / TIME_INTERVAL

	WORD	wMasterRecoveryCounter;	// 14-15, tc backup time
#define	COUNT_MASTERRECOVERY	5

	WORD	wCtrlDisable;	// 16-17, control disable time at program starting, wait valid of electronic signal
#define	TIME_CTRLDISABLE	1000 / TIME_INTERVAL;

	BYTE	nLetterCycle;	// 18, driving record time
#define	CYCLE_LETTER_HCR	5	// 500ms
#define	CYCLE_LETTER_NORMAL	10

	BYTE	nLetterCount;	// 19
	WORD	wReportTime;	// 20-21
#define	TIME_REPORT			200 / TIME_INTERVAL

	BYTE	nBusState[2];
	WORD	wDrvSaveTime;	// 22-23
	WORD	wDrvSaveMaxTime;	// 24-25
} BUSCYCLE, *PBUSCYCLE;

typedef struct _tagREVOLT {
	BYTE	nCandidate[LENGTH_TC];
	BYTE	nWin;
	BYTE	nLength;
	BYTE	nCondition;
#define	CYCLE_REVOLTCONDITION	20	// 0.15 * 20 = 3sec

	BYTE	nNull;
} REVOLT, *PREVOLT;

//typedef struct _tagSUPPORT {
//	BYTE	nCandidate[LENGTH_TC];
//	BYTE	nSuccess;
//	BYTE	nLength;
//} SUPPORT, *PSUPPORT;
//
typedef struct _tagOPERATESTATE {
	BYTE	nMode;				// 0, OPMODE_XXX
	BYTE	nState;				// 1,
//#define	OPERATESTATE_WITHPWM		1
#define	OPERATESTATE_DETECTPSM6		8
#define	OPERATESTATE_INHIBITATO		0x10
#define	OPERATESTATE_READYATCON		0x20
#define	OPERATESTATE_TRIGGERATCON	0x40
#define	OPERATESTATE_ATCON			0x80

	BYTE	nCabm;				// 2, mode input switch buffer at cabin
	BYTE	nCabmDeb;			// 3, its debounce
	BYTE	nAtcMode;			// 4, mode input buffer at atc
	BYTE	nAtcModeDeb;		// 5, its debounce
	BYTE	nAtcSw;				// 6, swi input buffer at atc
	BYTE	nAtcSwDeb;			// 7, its debounce
#define	DEB_OPMODE	10

	BYTE	nAutoStep;			// 8, use auto mode & driverless mode
// AUTO MODE
#define	AUTOSTEP_INIT			0
#define	AUTOSTEP_WAITATCCMD		1
#define	AUTOSTEP_WAITATOR		2
#define	AUTOSTEP_SCANADP		3
#define	AUTOSTEP_SCANDBP		4

// DRIVERLESS MODE
#define	DRIVERLESSSTEP_INIT			0
#define	DRIVERLESSSTEP_WAITNUETRAL	1
#define	DRIVERLESSSTEP_WAITMODE		2
#define	DRIVERLESSSTEP_WAITKUP		4
#define	DRIVERLESSSTEP_INTRO		(DRIVERLESSSTEP_WAITNUETRAL | DRIVERLESSSTEP_WAITMODE | DRIVERLESSSTEP_WAITKUP)
#define	DRIVERLESSSTEP_INTROB		(DRIVERLESSSTEP_WAITMODE | DRIVERLESSSTEP_WAITKUP)
#define	DRIVERLESSSTEP_WAITATOR		8
#define	DRIVERLESSSTEP_HOLDINHIBIT	0x10
#define	DRIVERLESSSTEP_ATCONATTAIL	0x20

	BYTE	nAutoDeb;			// 9, kup signal debounce at atc
#define	DEBOUNCE_KUP			10
#define	DEBOUNCE_KDN			20
#define	DEBOUNCE_PSM6			5

	WORD	wWaitAtcValid;		// 10-11, for Alarm 606
#define	TIME_WAITATCVALID		7000 / TIME_INTERVAL

	WORD	wWaitAtcMode;		// 12-13, for Alarm 607
#define	TIME_WAITATCMODE		5000 / TIME_INTERVAL

	WORD	wWaitSignalAutoA;	// 14-15
#define	TIME_WAITATCDMBIT		10000 / TIME_INTERVAL

	WORD	wWaitSignalAutoB;	// 16-17
#define	TIME_WAITATCKUP			110000 / TIME_INTERVAL
#define	TIME_WAITATCAUTO		3000 / TIME_INTERVAL
#define	TIME_WAITATCCMD			5000 / TIME_INTERVAL
#define	TIME_WAITATOREADY		3000 / TIME_INTERVAL
#define	TIME_ATOINHIBIT			1000 / TIME_INTERVAL	//15000 / TIME_INTERVAL

	WORD	wWaitAtcOn;			// 18-19
#define	TIME_WAITATCON			4500 / TIME_INTERVAL
// !!!!!WARNING TIME_WAITATCON should not be less than bus exchange time !!!!!
// bus exchange time(3sec) + atc delay time(5sec)

	BYTE	nKdnState;			// 20
#define	KDNSTATE_SETTLE		1

	BYTE	nRescueMode;		// 21, RESCUEMODE_XXX
	BYTE	nLightMonitor;		// 22, light monitor of each cc
#define	LIGHT_MONITOR	1
#define	LIGHT_ON		2

	BYTE	nCmsbMonitor;		// 23
#define	CMSBMON_READYA		1
#define	CMSBMON_READYB		2
#define	CMSBMON_POWERA		0x10
#define	CMSBMON_POWERB		0x20
#define	CMSBMON_CTRLB		0x40	// control by rescue mode
#define	CMSBMON_CTRLA		0x80	// control by normal mode

	WORD	wLightMonTimer;		// 24-25
#define	TIME_LIGHTMON			1000 / TIME_INTERVAL

	WORD	wCmsbTimer[2];		// 26-27, 28-29
#define	TIME_CMSBRUNREADY		5000 / TIME_INTERVAL
#define	TIME_CMSBONMONITOR		5000	//50000 / TIME_INTERVAL
#define	TIME_CMSBOFFMONITOR		3000	//30000 / TIME_INTERVAL

//	WORD	wRescueTimer;		// 30-31, not use
#define	TIME_RESCUEWAIT			5000 / TIME_INTERVAL
} OPERATESTATE;

enum  {
	OPMODE_NON = 0,
	OPMODE_EMERGENCY,			// 1
	OPMODE_MANUAL,				// 2
	OPMODE_YARD,				// 3
	OPMODE_DRIVERLESSREADY,		// 4
	OPMODE_DRIVERLESS,			// 5
	OPMODE_DRIVERLESSEXIT,		// 6
	OPMODE_AUTOREADY,			// 7
	OPMODE_AUTO,				// 8
	OPMODE_MAX
};

enum  {
	RESCUEMODE_NON = 0,
	RESCUEMODE_PASSIVETRAIN,	// 1
	RESCUEMODE_PASSIVEDRIVE,	// 2
	RESCUEMODE_ACTIVEUNDERGO,	// 3
	RESCUEMODE_PASSIVEUNDERGO,	// 4
	RESCUEMODE_ACTIVEDRIVE,		// 5
	RESCUEMODE_MAX
};
//==========================================================================================
//				/ trainwreck | <---	/ rescue car |	|	/ trainwreck | <--- / rescue car |
//				| mascon pos.						|						| mascon pos.
//	rescue sw	-		tail		head		-	|	head	tail		-		-
//	mode		emer/manu			emer			|	emer				emer/manu
//	F/R			F					F				|	F					F
//	pwm			out					in				|	in(brake)			out
//	name		ACTIVEUNDERGO		PASSIVEDRIVE		PASSIVEUNDERGO		ACTIVEDRIVE
//==========================================================================================
//	rescue				ROS	ROS	basic	dead	exten.	pwm-in	pwm-out	device	ROPR
//	mode				HCR	TCR	ctrl.	ctrl.	ctrl.	P	B	P	B	P	B	ROBR
//	PASSIVE TRAIN		-	-	O		X		X		X	O	X	X	X	O	X(only BR)
//	PASSIVE DRIVE		O	X	O		X		O		O	O	X	X	O	O	from HCR
//	ACTIVE UNDERGO		X	O	O		O		X		X	X	O	O	O(a)O	to TCR
//	PASSIVE UNDERGO		O	O	O		X		X		X	O	X	X	X	O	X(only BR)
//	ACTIVE DRIVE		X	X	O		O		O		X	X	O	O	O	O
//	non rescue mode		X	X	A		O		O		X	X	X	O	O	O	to HCR
// (a)if powering to vvvf, vvvf not working cause not supplied high voltage.
//==========================================================================================

// Modified 2013/02/05... append PoscuePos()
#define	RescuePos()					(!GETTBIT(OWN_SPACEA, TCDIB_R))
#define	IsRescueTrain()				(c_pTcRef->real.op.nRescueMode == RESCUEMODE_PASSIVETRAIN && RescuePos())
#define	IsRescuePassiveDrive()		(c_pTcRef->real.op.nRescueMode == RESCUEMODE_PASSIVEDRIVE && RescuePos())
#define	IsRescueActiveUndergo()		(c_pTcRef->real.op.nRescueMode == RESCUEMODE_ACTIVEUNDERGO && RescuePos())
#define	IsRescuePassiveUndergo()	(c_pTcRef->real.op.nRescueMode == RESCUEMODE_PASSIVEUNDERGO && RescuePos())
#define	IsRescueActiveDrive()		(c_pTcRef->real.op.nRescueMode == RESCUEMODE_ACTIVEDRIVE && RescuePos())
#define	IsRescueCar()	(IsRescuePassiveDrive() || IsRescueActiveUndergo() || IsRescuePassiveUndergo() || IsRescueActiveDrive())
#define	IsRescue()		(IsRescueTrain() || IsRescueCar())
#define	IsNotRescue()	(c_pTcRef->real.op.nRescueMode == RESCUEMODE_NON)

typedef struct _tagINSPECTSTATE {
	BYTE	nMode;			// INSPECTMODE_XXX
	BYTE	nRefMode;
	BYTE	nNull;
	BYTE	nReqCount;		// inspect request count to ato
#define	COUNT_INSPECTREQ	4

	WORD	wItem;			// inspect item
#define	INSPECTITEM_SND		1	// signal device & door
#define	INSPECTITEM_SIV		2
#define	INSPECTITEM_EXT		4
#define	INSPECTITEM_ECU		8
#define	INSPECTITEM_BCP		0x10
#define	INSPECTITEM_V3F		0x20
#define	INSPECTITEM_PWR		0x40
#define	INSPECTITEM_COOL	0x80
#define	INSPECTITEM_HEAT	0x100
#define	INSPECTITEM_CMSB	0x200
#define	INSPECTITEM_DOORB	0x400
#define	INSPECTITEM_TCMS	0x800
#define	INSPECTITEM_LAMP	0x1000
#define	INSPECTPAGE_MAX		21

	BYTE	nId;			// inspect process reference
	BYTE	nStep;
	BYTE	nSubStep;
	BYTE	nAuxStep;
	BYTE	nAtoCmd;		// inspect command to ato
	BYTE	nAbsLamp;		// on all lamps
	WORD	wTimer;			// wait timer for inspect process excuting at device
	WORD	wDelay;			// delay timer for tc
	WORD	wTimeOuter;
#define	TIMEOUT_INSPECT		30000 / TIME_INTERVAL

	WORD	wPage;
	WORD	wPrevPage;
} INSPECTSTATE;

enum  {
	INSPECTMODE_NON = 0,
	INSPECTMODE_PDT,
	INSPECTMODE_DAILY,
	INSPECTMODE_MONTHLY,
	INSPECTMODE_EACHDEV,
	INSPECTMODE_VIEW,
	INSPECTMODE_MAX
};

typedef struct _tagPOWERSTATE {
	_TV_	vTbeL;		// 1-2, little-endian
#define	TEBEMAX_POWER	1024	//1023
#define	TEBEMAX_BRAKE	836		//835

	PARCEL(	inst,		// 3
				spr7, spr6, spr5,
				p,		// Powering selecton feedback
				b,		// Braking selection feedback
				spr2,
				f,		// forward feedback
				r );	// reverse feedback
	BYTE	nCond;
#define	COND_POWERSTART		1

	WORD	wTimer;		// powering monitor time, for Alarm 51
// Modified 2012/09/17 ... begin
#define	TIME_POWERMONIT		10000 / TIME_INTERVAL	// 5000 / TIME_INTERVAL
// end
} POWERSTATE;		// real state to device(v3f, ecu) from stick or ato

typedef struct _tagPWMARCH {
	WORD	high;
	WORD	period;
	BYTE	nMode;
} PWMARCH;

typedef struct _tagPWMBACKUP {
	WORD	high;
	WORD	period;
	WORD	v;
	BYTE	nDebFreq;
	BYTE	nDebDuty;
#define	DEB_PWM				10

} PWMBACKUP;

#define	PWMPERIOD_NORMAL	2000
#define	PWMPERIOD_MAX		2500
#define	PWMPERIOD_MIN		1500

#define	PWMMODE_INPUTA		0
#define	PWMMODE_INPUT		1
#define	PWMMODE_OUTPUT		2

#define	PWMOFFSET_ZERO		0	// 40
#define	PWMOFFSET_SPAN		100	// 103	// 1.03 * 100

typedef struct _tagATOCMD {
	BYTE	nCmd;
#define	INSTCMD_INHIBIT		1		// ato inhibit by fsb
#define	INSTCMD_INHIBITA	2		// ato inhibit by ato kup
#define	INSTCMD_CABIN		0x10	// ato cabin signal
#define	INSTCMD_ACTIVE		0x20	// active ato signal

	BYTE	nState;
#define	INSTSTATE_VALID		1	// valid tebe signal from ato
#define	INSTSTATE_SIGN		2
#define	INSTSTATE_DISABLEEDITTN	0x10	// train no. edit disable, not use

	_TV_	v;
	_TV_	vTbeB;		// big-endian
	_TV_	vAto;		// tebe or pwb value from ato
	BYTE	nAtc1FaultCount;
	BYTE	nAtc2FaultCount;
	BYTE	nAtoFaultCount;
#define	FSBTERM_FAULTATCCOUNT	30	//60
	BYTE	nDsn;
} ATOCMD;

typedef struct _tagINSTRUCTA {
	//BOOL	bValid;
	//BYTE	nWork;		// at rescue, indicate train or car process
	WORD	v;
	_TV_	vTbeB;		// big-endian, tebe value
	WORD	percent;
} INSTRUCTA, *PINSTRUCTA;

typedef struct _tagMCDSTICK {
	BOOL		bValid;
	BYTE		nIndex;		// index of cabin stick
#define	MCDSV_B7POS			1
#define	MCDSV_NPOS			8
	WORD		wFailSafeDeb;
// Modified 2012/09/17 ... begin
#define	DEB_FAILSAFE		50	// 20	// 10
// end

	INSTRUCTA	inst;		// tebe value by nIndex
} MCDSTICK, *PMCDSTICK;

typedef struct _tagSPEEDCONDITION {
	BYTE	nZv;
#define	SPEEDDETECT_ZVF	1	// zero velocity detect
#define	SPEEDCOUNT_ZVF	2	// zero velocity counting
	BYTE	nCurPulse;		// for monit atc pulse, must erase
	WORD	wZvTimer;
#define	TIME_DETECTZV	1000 / TIME_INTERVAL

	WORD	wMotorCurrent;	// max current of Im from vvvf
	UCURV	uLength;		// length of valid vvvf
	DWORD	dwV3fs;			// speed from vvvf tacho
#define	SPEED_ZERO	2000	// speed is zero under 2Km

	BYTE	nLocalCycle;	// local data packing counter, seem to
	BYTE	nAtcCur;		// atc speed buffer index for calc. it
	DWORD	dwAtcs;			// speed from atc
} SPEEDCONDITION, *PSPEEDCONDITION;

enum  {						// collect.	calc-T
	ATCSPEED_UNIT50MS,		// 1		20
	ATCSPEED_UNIT100MS,		// 2		10
	ATCSPEED_UNIT200MS,		// 4		5
	ATCSPEED_UNIT250MS,		// 5		4
	ATCSPEED_UNIT500MS,		// 10		2
	ATCSPEED_UNIT1SEC,		// 20		1
	ATCSPEED_UNITMAX
};

#define	ATCSPEED_UNIT	ATCSPEED_UNIT500MS

typedef struct _tagINTERGRALDIST {
	BYTE	nNull;			// 0, nBuzzReq;		// not use

	BYTE	nDistPulse;		// 1, distance pulse during 25
#define	DISTPULSE_PUT		1
#define	DISTPULSE_PERIOD	10.f

	WORD	wDistTimer;		// 2-3
#define	TIME_DISTPERIOD		200 / TIME_INTERVAL	//500 / TIME_INTERVAL

	double	dbIntg;			// 4, distance for 25 counter
	double	dbDist;			// 12, distance between stations
	double	dbDistLive;		// 20, total distance
	double	dbSivPowerLive;	// 28
	double	dbSivPower;		// 36
	double	dbV3fPowerLive;	// 44, total electric power
	double	dbV3fPower;		// 52
	double	dbRevival;		// 60
} INTERGRAL, *PINTERGRAL;

typedef struct _tagDMANSTATE {
	BYTE	nActive;
#define	DEADMAN_POWERMASK	1
#define	DEADMAN_FSB			2
	BYTE	nNull;
	WORD	wTimer;			// dead man switch counter
#define	TIME_DEADMANWARNING	3000 / TIME_INTERVAL
#define TIME_DEADMANFAULT	13000 / TIME_INTERVAL
} DMANSTATE;

enum  {	DOORMODE_UNKNOWN, DOORMODE_AOAC, DOORMODE_AOMC, DOORMODE_MOMC };
enum  {	DOORSTATE_CLOSED, DOORSTATE_OPENING, DOORSTATE_OPENED, DOORSTATE_CLOSING,
		DOORSTATE_NOTCLOSED };
enum  { DOORREF_CLOSE, DOORREF_LEFT, DOORREF_RIGHT, DOORREF_ALL,
		DOORREF_LEFTMON = 4, DOORREF_RIGHTMON = 8};

typedef struct _tagDOORSTATE {
	BYTE	nMode;			// 0, DOORMODE_XXX
	BYTE	nState;			// 1
#define	DOORSTATE_ONLY				0xf		// DOORSTATE_CLOSED<->DOORSTATE_NOTCLOSED
#define	DOORSTATE_ENABLEREOPEN		0x10	//
#define	DOORSTATE_REOPENING			0x20	// reopen command
#define	DOORSTATE_WAITODNEGATE		0x40	// wait atc odl odr low
#define	DOORSTATE_RIGHT				0x80	// if high then right door command

	BYTE	nCuri;			// 2, door mode switch buffer at cabin
	BYTE	nDeb;			// 3, its debounce
#define	DEB_DOORMODE	10

	WORD	wAdscrDeb;		// 4-5, all door close signal debounce
#define	DEB_ADSCR		6

	BYTE	nRefer;			// 6, door reference for local service device
	BYTE	nBackup;		// 7, door backup state
	// Modified 2013/01/10
	//WORD	wTimer;			// 8-9, door moving time, after this time, detect door alarm
	WORD	wTimerL;		// 8-9, door moving time, after this time, detect door alarm
	WORD	wTimerR;		// 10-11, door moving time, after this time, detect door alarm
#define	TIME_WAITDOOR	10000 / TIME_INTERVAL	//8000 / TIME_INTERVAL	//5000 / TIME_INTERVAL

//	WORD	wOpenRescanTimer;	// 10-11, this time must be zero, scan door open command
//#define	TIME_OPENRESCAN	3000 / TIME_INTERVAL

	WORD	wInhibitAlarmTimer;	// 12-13, disable detect alarm at reopening
#define	TIME_INHIBITALARM	10000 / TIME_INTERVAL

	BYTE	nAtcOdDeb;		// 14
#define	DEBOUNCE_ATCODNEGATE	5

	BYTE	nOppo;			// 15
	BYTE	nOwn;			// 16
	BYTE	nSum;			// 17, logical or of all door, compare to all door close signal
	BYTE	nCur[LENGTH_CC];	// door state of each cc
} DOORSTATE;

typedef struct _tagPTSTATE {
	BYTE	nState;			// PTSTEP_XXX, AUXSTATE_XXX
	BYTE	nCtrl;			// control byte
	WORD	wTimer;
	WORD	wRunTimer;
} PTSTATE, *PPTSTATE;

typedef struct _tagPTTYPE2 {
	BYTE	nState;			// PTSTEP_XXX, AUXSTATE_XXX
	BYTE	nCtrl;			// control byte
	WORD	wTimer;
	WORD	wRunTimer;
	WORD	wMonit;
} PTTYPE2, *PPTTYPE2;

typedef struct _tagPANSHEET {
	BYTE	enable;
	BYTE	cmd;
	BYTE	state;
} PANSHEET;

typedef struct _tagPANTOFLOW {
	PTSTATE	cur;			// 0-5,		panto control
	PTTYPE2	auxf;			// 6-13,	front aux. comp. control
	PTTYPE2 auxr;			// 14-21,	rear aux. comp. control
	WORD	wMonitCatenary;	// 22-23
	PANSHEET	sheet;		// 24-26
	BYTE	nState;			// 27
#define	PS_PANFRONTLINE		1
#define	PS_PANREARLINE		2
#define	PS_SIVFRONTLINE		4
#define	PS_SIVREARLINE		8
#define	PS_LINES			0xf
#define	PS_MONITCATENARY	0x40
#define	PS_ENABLEALARM		0x80

#define	IsPanFront()	(c_pTcRef->real.ptflow.nState & PS_PANFRONTLINE)
#define	IsPanRear()		(c_pTcRef->real.ptflow.nState & PS_PANREARLINE)
#define	IsSivFront()	(c_pTcRef->real.ptflow.nState & PS_SIVFRONTLINE)
#define	IsSivRear()		(c_pTcRef->real.ptflow.nState & PS_SIVREARLINE)

	WORD	wSkipTimer;		// 28-29
#define	TIME_PANTOALARMSKIP	9000	// 900sec = 15min, trigger by bus cycle time

	BYTE	nPos;			// 30
#define	FRONT_PANPS1		1
#define	FRONT_PANPS2		2
#define	REAR_PANPS1			4
#define	REAR_PANPS2			8

	BYTE	nIn;			// 31
	BYTE	nDeb;			// 32
#define	DEB_PANIN		10

	BYTE	nNull;			// 34
} PANTOFLOW, *PPANTOFLOW;

/*//#define	IsPantoUp()\
//	(GETCBIT(__AUXA, CCDIB_PANPS1) || GETCBIT(__AUXA, CCDIB_PANPS2) ||\
//	 GETCBIT(__AUXB, CCDIB_PANPS1) || GETCBIT(__AUXB, CCDIB_PANPS2))*/
#define	IsPantoUp()		(c_pTcRef->real.ptflow.nPos != 0)
#define	IsPantoUpB()	(c_tcRef.real.ptflow.nPos != 0)

enum  {
		PTSTEP_NON,
		PTSTEP_READY,
		PTSTEP_MOVE,
		PTSTEP_MONIT
};

#define	PTFLOW_STEP			3
#define	PTFLOW_DOWN			4
#define	PTFLOW_UP			8
#define	PTFLOW_ABSDOWN		0x10	// forced to down cause voltage alarm
// Modified 2013/11/02
//#define	PTFLOW_RUP			0x20
#define	PTFLOW_MDOWN		0x20	// when emergency pan down

#define	TIME_PTDOWNREADY	1000 / TIME_INTERVAL
#define	TIME_PTDOWNHOLD		3000 / TIME_INTERVAL
#define	TIME_PTUPHOLD		3000 / TIME_INTERVAL
#define	TIME_PTMONIT		100 / TIME_INTERVAL
#define	TIME_PTUPMONIT		30000	// 5 min
//#define	TIME_PTMONIT		4 * 60 * 1000 / TIME_INTERVAL

#define	TIME_PTCATENARYMONITUP		1000	// 10sec
#define	TIME_PTCATENARYMONITDOWN	1000	// 10sec
#define	CATENARY_PTON		100		// 1200V
#define	CATENARY_PTOFF		50		// 600V

enum  {
		AUXSTATE_PTNON = 0,
		AUXSTATE_PTREADY,
		AUXSTATE_PTWAITACMK,		// 1
		AUXSTATE_PTWAITUNACMG,		// 2
		AUXSTATE_PTWAITUNACMK,		// 3
		AUXSTATE_PTWAITUNACMKFAIL,
		AUXSTATE_PTFAILED
};

#define	TIME_PTAUXCOMPRELEASE	30000 / TIME_INTERVAL
#define	TIME_PTAUXCOMPCONTACT	30000 / TIME_INTERVAL
// Modified 2012/01/11	... begin
#define	TIME_PTAUXCOMPPRESSURE	24000	// 240000 / TIME_INTERVAL = 4minute	;30000 / TIME_INTERVAL
// ... end
#define	TIME_PTAUXMONITSW		30000 / TIME_INTERVAL
#define	TIME_PTAUXCOMPONMAX		30000	// 300000 / TIME_INTERVAL = 5minute	;6000	// 1minute

typedef struct _tagHOLDBRAKE {
	WORD	wState;
#define	HOLDBRAKE_ACTIVE	1

	WORD	wTimer;
#define	TIME_HOLDBRAKERELEASEATC		500 / TIME_INTERVAL	//1000 / TIME_INTERVAL
#define	TIME_HOLDBRAKERELEASEEMERGENCY	650 / TIME_INTERVAL
#define	TIME_HOLDBRAKECAPTUREEMERGENCY	1000 / TIME_INTERVAL

} HOLDBRAKE;

typedef struct _tagCTRLSTICK {
	BOOL	bValid;
	BYTE	nState;			// train direction(cabin stick) buffer
	WORD	wDeb;			// its debounce
#define	DEBOUNCE_STICK		3
} CTRLSTICK;

typedef struct _tagMOVESTATE {
	WORD	wMask;			// mask security brake at device inspect process
#define	MOVESTATE_MASKSHB	1

	WORD	wBrake;			// brake state
#define	MSTATE_EMERGENCYBRAKE	1
#define	MSTATE_BCCOOVERNINE		2
#define	MSTATE_NORMALFSB		4
#define	MSTATE_FSBATWRECKDIR	8
#define	MSTATE_FSBATMWB			0x10
#define	MSTATE_HOLDINGBRAKE		0x20
#define	MSTATE_SECURITYBRAKE	0x40
#define	MSTATE_PARKINGBRAKE		0x80
#define	MSTATE_ECUNRBDETECT		0x100
#define	MSTATE_DEADMANPOWERMASK	0x200
#define	MSTATE_MOTORWHILEBRAKE	0x8000

// Modified 2012/09/17 ... begin
//#define	MSTATE_HBDISABLE		MSTATE_EMERGENCYBRAKE
// end
#define	MSTATE_NRBDDISABLE		MSTATE_HOLDINGBRAKE
#define	MSTATE_PWRDISABLE		0x1df
// Modified 2012/03/05 ... start
#define	MSTATE_PWRDISABLERESCUE	0xd1
// ... end
#define	MSTATE_NRBDISABLE		0xff

#define	POWERMASK_BCCOLENGTH	9

	//WORD	wAtcEbr;		// not use
	BYTE	nDir;			// train direction
	BYTE	nFsbSrc;		//BOOL	bFsb;			// not use
	CTRLSTICK	cs;
	HOLDBRAKE	hb;
} MOVESTATE, *PMOVESTATE;

typedef struct _tagCAREMER {
	WORD	wRep;			// process start flag for each cc
	WORD	wBuf;			// signal buffer for each cc, CCDIB_EMR, CCDIB_FDN
	WORD	wStep;			// process step
} CAREMER, *PCAREMER;

typedef struct _tagCARALARM {
	CAREMER		emer;		// emergency for each cc
	CAREMER		fire;		// fire alarm for each cc
	WORD		wSwDeb;		// signal switch debounce of each cc
#define	DEBOUNCE_SW		3

	WORD		wPaapbTimer;	// buzzer off timer without TCDIB_PAAPB
#define	TIME_PAAPB		5000 / TIME_INTERVAL
#define	TIME_PAAPBAGAIN	18000	// 180000 / TIME_INTERVAL

} CARALARM, *PCARALARM;

enum  {
	EXTSTEP_PRELUDE = 0,
	EXTSTEP_STABILITY,
	EXTSTEP_WAITUNSIVK,
	EXTSTEP_WAITREDUCTION,
	EXTSTEP_WAITESK,
	EXTSTEP_POSTLUDE,
	EXTSTEP_WAITUNESK,
	EXTSTEP_MAX
};

typedef struct _tagEXTSTATE {
	BYTE	nStep;			// EXTSTEP_XXX
	BYTE	nState;
#define	EXTSTATE_REDUCTIONB		1	// by c_ext.Check(), control lcd of each cc
#define	EXTSTATE_REDUCTIONA		2	// on during EXTSTEP_WAITESK, to local service device
#define	EXTSTATE_SCANESCAPE		0x10	// monit lrr from escape reduction

	WORD	wCause;			// extension cause, CCDIB_SIVFR, CCDIB_IVF, CCDIB_IVS, cc.real.aSiv.d.st.test.b.chkx
	WORD	wTimer;			// process timer
	WORD	wChgTimer;		// after this time, monitor CCDIB_LRR
	WORD	wEskMonTimer;	// esk monitor time
} EXTSTATE, *PEXTSTATE;
#define	TIME_EXTSTABILITY		5000 / TIME_INTERVAL
#define	TIME_EXTWAITUNSIVK		3000 / TIME_INTERVAL
#define	TIME_EXTWAITREDUCTION	5000 / TIME_INTERVAL
#define	TIME_EXTWAITESK			3000 / TIME_INTERVAL
#define	TIME_EXTWAITUNESK		3000 / TIME_INTERVAL
#define	TIME_CHGREDUCTION		1000 / TIME_INTERVAL
#define	TIME_ESKMON				3000 / TIME_INTERVAL

typedef struct _tagBATTERY {
	DWORD	v;
	BOOL	bState;			// 1 -> battery voltage is under 70
	BYTE	nState;			// flag of TCDIB_HES, not use ???
#define	BATSTATE_TCDIHES	1
	WORD	wTimer;			// after this time, occer Alarm 223
#define	TIME_BATFAULT	30000 / TIME_INTERVAL
} BATTERY, *PBATTERY;

typedef struct _tagBUSMONIT {
	WORD	wMain;			// main trace bus trace
	WORD	wResv;			// reserve trace bus trace
	WORD	wState;			// alarm bus node
	WORD	wBuf;			// bus alarm buffer
	BYTE	nTcs;
	BYTE	nNull;
	BYTE	nDeb[CAR_MAX];
// Modified 2012/09/25
#define	DEBOUNCE_BUSMONIT	5	// 3

	WORD	wTime;			// process cycle time
	BYTE	nWakeup[8];		// for passenger cabin alarm, this alarm not display at CC down.
#define	COUNT_CCWAKEUP		3

} BUSMONIT;

#define	IsWakeupB(n)	(c_tcRef.real.bm.nWakeup[n] >= COUNT_CCWAKEUP)
#define	IsWakeup(n)		(c_pTcRef->real.bm.nWakeup[n] >= COUNT_CCWAKEUP)

typedef struct _tagLOCALMONIT {
	WORD	wAck;			// respond of local signal device, must receive from all(atc1, atc2, ato) then progress...
	WORD	wResult;		// respond result for display, without bcc check
	WORD	wValidate;		// effect from stx, addr, etx, bcc
	WORD	wTimeClose;		// time out indicate for local device
	WORD	wExpectation;	// if this bit is 1, then expect timeout processing
	BYTE	nCode[LOCALID_SERVICE_LENGTH];	// cabin no.(0x31-0x34) for local service device
	BYTE	nNull;
	BYTE	nDeb[LOCALID_MAX];	// result debounce
} LOCALMONIT;
#define	DEBOUNCE_LOCALMONITA	10	// 3
#define	DEBOUNCE_LOCALMONITB	10
// Modified 2012/09/25
#define	DEBOUNCE_POCKETMONIT	10	// 5	// 3

typedef struct _tagSCENEARMARCH {
	BYTE	nState;
// Modified 2013/11/02... remove this line
//#define	ARMSTATE_NEGATEBCID		1	// hold TCDOB_BCID for write alarm table to mem	//nand
#define	ARMSTATE_LIGHTON		2
#define	ARMSTATE_ALARMON		4
#define	ARMSTATE_BUZZON			8
// Modified 2012/12/25
//#define	ARMSTATE_BUZZSW			0x10
#define	ARMSTATE_LAMPON			0x10
#define	ARMSTATE_BUZZSW			0x20

//#define	ARMSTATE_VALIDDATA		1	// not use
//#define	ARMSTATE_SAVEENV		4	// not use

	BYTE	nEmergency;
	DWORD	dwIndex;		// size of ARM_ARCH(c_armArch) at CScene
	DWORD	dwSign;			// sign index at arm_arch
	DWORD	dwAlarm;		// alarm index at arm_arch
// Modified 2013/11/02
	DWORD	dwScrollIndex;
	WORD	wScrollTimer;
#define	TIME_SCROLL		5000 / TIME_INTERVAL

//	WORD	wLightTimer;	// light alarm timer
//#define	TIME_LIGHTON	7000 / TIME_INTERVAL

//	WORD	wAlarmTimer;	// alarm timer
//#define	TIME_ALARMON	7000 / TIME_INTERVAL

	WORD	wBuzzTimer;		// buzzer hold time
#define	TIME_CRITICALBUZZ	0
#define	TIME_HEAVYBUZZ	2000 / TIME_INTERVAL	// 5000 / TIME_INTERVAL
#define	TIME_LIGHTBUZZ	2000 / TIME_INTERVAL
#define	TIME_ALARMBUZZ	2000 / TIME_INTERVAL

// Modified 2013/11/02
//	WORD	wRemoteRegister;
//	WORD	wRemoteReporter;
	BYTE	nNull;
	BYTE	nSevereID;
	WORD	wSevereCode;
} SCENEARMARCH;

typedef struct _tagSCENELOGARCH {
	DWORD	dwIndex;		// size of driving record
	BYTE	nMainStep;			// BEILSTEP_XXX
	BYTE	nPeriodStep;
	WORD	wMainTimer;			// wait timer for beil excuting
#define	TIME_BEILWAIT	10000 / TIME_INTERVAL

	WORD	wPeriodTimer;
	DWORD	dwTag;			// size of trial driving record
} SCENELOGARCH;

enum  {
	BEILSTEP_NON = 0,
	BEILSTEP_SAVELOG,
	BEILSTEP_MAKEDIRECTORYLIST,
	BEILSTEP_LOADALARM,
	BEILSTEP_APPENDDRV,
	BEILSTEP_SORTBYDAY,
	BEILSTEP_SELDELETE,
	BEILSTEP_SECTQUANTITY,
	BEILSTEP_READSECT,
	BEILSTEP_WRITESECT,
	BEILSTEP_MAX
};

typedef struct _tagDUCTRLINFO {
	BYTE	nCurPage;		// current page
	BYTE	nSetPage;		// open page
	BYTE	nSw;			// switch code from du
	BYTE	nDownloadItem;	// download item
	BYTE	nTiePage;		// insert page to nCurPage
	BYTE	nTerm;
#define	DUTERM_REPEAT	1	// view repeat button at du
#define	DUTERM_SKIP		2	// view continue button
#define	DUTERM_FEED		4	// page inserting

	BYTE	nRoll;			// du page setting index
	BYTE	nSel;			// du key 101-130 -> 1->30
	BYTE	nBkupPage;		// prevent for page changing when repeat alarm key
	BYTE	nAlarmPageCount;
#define	COUNT_ALARMPAGE		2

	WORD	wDoubleCounter;
#define	COUNT_DUDELETE		1000 / TIME_INTERVAL

} DUCTRLINFO;

enum  {
	DIRECTORYSTATE_SELF = 0,
	DIRECTORYSTATE_SELFSUB,
	DIRECTORYSTATE_OPPO,
	DIRECTORYSTATE_OPPOSUB,
	DIRECTORYSTATE_WORK = 8,
	DIRECTORYSTATE_MAX
};

enum  {
	DOWNLOADSTEP_NON = 0,
	DOWNLOADSTEP_READYENTRYBUS,
	DOWNLOADSTEP_ENTRYBUS,
	DOWNLOADSTEP_WAITENTRYBUS,
	DOWNLOADSTEP_DOWNLOADING,
	DOWNLOADSTEP_READYEXITBUS,
	DOWNLOADSTEP_MAX
};

typedef struct _tagEACHSIDEENTRY {
	BYTE		nDirectoryState;	// 0
	BYTE		nDownloadStep;		// 1
#define	DOWNLOADSTEP_WAITRESPOND	0x80
#define	DOWNLOADSTEP_ONLY			0xf

	BYTE		nMasterDuty;		// 2
	BYTE		nTargetDuty;		// 3
	BYTE		nDownloadMethod;	// 4
#define	DOWNLOADMETHOD_SELF			0xff
#define	DOWNLOADMETHOD_OPPO			0xfe

	BYTE		nNull;				// 5
	DWORD		dwSelectedDay;		// 6-9
	DWORD		dwSelectedTime;		// 10-13
	DWORD		dwTotalSize;		// 14-17
	DWORD		dwRecvSize;			// 18-21
	BYTE		nCounter;			// 22
#define	COUNT_WAITOPPODIR	10		// 1sec

	BYTE		nRespCounter;		// 23
#define	COUNT_COMMFAULT		10
#define	COUNT_DOWNLOADSYNC	10

	WORD		wSyncTimer;			// 24-25
#define	TIME_BUSBEMPTYMONIT		1000 / TIME_INTERVAL
#define	TIME_WAITBUSBRESPOND	1000 / TIME_INTERVAL

	WORD		wBusBMonitor;		// 26-27
#define	TIME_BUSBMONIT			1500 / TIME_INTERVAL

	WORD		wReadError;			// 28-29
	WORD		wWriteError;		// 30-31
	WORD		wFrameError;		// 32-33
} EACHSIDEENTRY;

enum  {
	PERTMSG_ACK = 0,	// using by SendPert
	PERTMSG_NEW,
	PERTMSG_AGAIN,
	PERTMSG_MAX
};

typedef struct _tagCONDITION {
	BYTE		nCurrentAtc;
	BYTE		nAtcSkipCount;
	BYTE		nActiveAtc;
	BYTE		nAichDsv;
	BYTE		nAichBat;
	BYTE		nAtcWi;			// atc wheel index
	WORD		wReleaseTimer;
// Modified 2012/12/25 because trial
#define	TIME_RELEASE	1000 / TIME_INTERVAL
//#define	TIME_RELEASE	30000 / TIME_INTERVAL	// 5min -> 300,000 / TIME_INVERVAL
//#define	TIME_RELEASE	30000	// 5min -> 300,000 / TIME_INVERVAL

	WORD		wTimerSetWait;
#define	WAIT_TIMERSET	90000 / TIME_INTERVAL

	WORD		wEnvSetWait;
#define	WAIT_ENVSET		5000 / TIME_INTERVAL

	WORD		wTrainNo;
} TCONDITION;

typedef struct _tagALARMLISTINFO {
	WORD	wMaxFiles;		// directory length of nand/archives
	WORD	wCurrentFile;	// display file id on alarm page
#define	ALARMFILE_CURRENT	0xffff

	WORD	wMaxPages;		// alarm max page, alarm / 16(display row)
	WORD	wCurrentPage;	// alarm current page
	BYTE	nListID;
#define	ALARMLIST_COLLECTION	1	// all alarm list
#define	ALARMLIST_PRESENT		2	// current alarm list

	BYTE	nProtect;
#define	PROTECT_PAGE			1

} ALARMLISTINFO;

// BEGIN TCREFERENCE { ====================================================
typedef struct _tagTCREFERENCEREAL {
	BYTE		nDuty;
	BYTE		nProperty;
#define	PROPERTY_ATLABORATORY		1
#define	PROPERTY_DISABLEBACKUP		2
#define	PROPERTY_DISABLESWWATCHDOG	4
#define	PROPERTY_ENABLEBUS			8
#define	PROPERTY_LOGMASKATFILESAVE	0x10
#define	PROPERTY_LOGMASKATDOWNLOAD	0x20
#define	PROPERTY_USEEXTERNALMEM		0x40
#define	PROPERTY_PARTNER			0x80
#define	PROPERTY_LOGMASK	(PROPERTY_LOGMASKATFILESAVE | PROPERTY_LOGMASKATDOWNLOAD)

	BYTE		nState;
#define	STATE_ENABLEOVERHAUL	1
#define	STATE_BEENUSB			2
#define	STATE_PROGRESSING		4
#define	STATE_DOWNLOADING		8
#define	STATE_TRIAL				0x10
#define	STATE_NEEDTIMESET		0x20
#define	STATE_VALIDENVF			0x40
#define	STATE_SAVEENVF			0x80

	BYTE		nObligation;
#define	OBLIGATION_ACTIVE	1
#define	OBLIGATION_HEAD		2
#define	OBLIGATION_MAIN		4
//#define	OBLIGATION_TOP		(OBLIGATION_ACTIVE | OBLIGATION_HEAD | OBLIGATION_MAIN)
//#define	OBLIGATION_CTRL		(OBLIGATION_ACTIVE | OBLIGATION_HEAD)

	// Modified 2013/11/02
	BYTE		nCurtains;
#define	CURTAIN_NEGATEBCID		1
#define	CURTAIN_WRITEENV		0x10
#define	CURTAIN_WRITELOG		0x20
#define	CURTAIN_CONDITION		0xf0

	BYTE		nNull;

	// Inserted 2012/01/16
	WORD		wInterTimerLength;

	CARFORMATION	cf;
	CARID		cid;
	BYTE		nAddr;		// 1 ~ 4
	BYTE		nActiveBus;
#define	ACTIVEBUS_A		1
#define	ACTIVEBUS_B		2
#define	ACTIVEBUS_ONLY	(ACTIVEBUS_A | ACTIVEBUS_B)
#define	INHIBITBUS_B	0x80

	TDIRSTATE	dir;
	HCRINFO		hcr;
	BUSCYCLE	cycle;
	REVOLT		revolt;
	OPERATESTATE	op;
	INSPECTSTATE	insp;
	POWERSTATE	power;
	PWMBACKUP	pwmbk;
	ATOCMD		atoCmd;
	MCDSTICK	mcDrv;
#define	TBE_BRAKEMAX		-1024
#define	ATOTBE_MAX			10000
#define	ATOTBE_COAST		0
#define	ATOTBE_MIN			-10000
	INSTRUCTA	rescPwr;

	SPEEDCONDITION	speed;
#define	SPEEDTERM_HOLDBRAKE		1500

	INTERGRAL	intg;
	DEVICESTATE	dev;
	DMANSTATE	dead;
	DOORSTATE	door;
	PANTOFLOW	ptflow;
	MOVESTATE	ms;
	CARALARM	ca;
	EXTSTATE	ext;
	BATTERY		bat;
	float		cps;
	BUSMONIT	bm;
	LOCALMONIT	lm;
	BYTE		nLocalDataType[LOCALID_MAX];
	WORD		wTraceDev;
	WORD		wSivTrace;
	WORD		wV3fTrace;
	WORD		wEcuTrace;
	WORD		wSivTraceClear;
	WORD		wV3fTraceClear;
	WORD		wEcuTraceClear;
	WORD		wDevTimerSet;
#define	COUNT_DEVTIMERSET	4;

	// DEVTIME		loginTime;
	SCENEARMARCH	scArm;
	SCENELOGARCH	scLog;
	DUCTRLINFO		du;				// 8 byte
	EACHSIDEENTRY	es;
	TCONDITION		cond;
	ALARMLISTINFO	ai;

#define	SIZE_BUSFAULT	14
	BYTE			nBusFailCounter[SIZE_BUSFAULT];

// Modified 2013/02/18 cause 3.
	WORD			wTerminalBackupTimer;
#define	TIME_TERMINALBACKUP		2000 / TIME_INTERVAL

#define	SIZE_TERMINALCODEBACKUP	10
	BYTE			nTerminalCodeBackup[SIZE_TERMINALCODEBACKUP];
#define	SIZE_VALIDTERMINALCODE	7

} TCREFERENCEREAL;

#define	SIZE_TCREFERENCE	656
typedef struct _tagTCREFERENCE {
	TCREFERENCEREAL	real;
	BYTE	dummy[SIZE_TCREFERENCE - sizeof(TCREFERENCEREAL)];
} TCREFERENCE, *PTCREFERENCE;

// END TCREFERENCE { ======================================================

typedef struct _CCBUSCYCLE {
	BOOL	bOpen;
	BYTE	nNull;
	WORD	wTime;
	WORD	wOver;
	WORD	wOverLampOnTimer;
} CCBUSCYCLE;

typedef struct _tagWHEELDIA {
	WORD	wV;
	BYTE	nCur;
	BYTE	nDeb;
#define	DEBOUNCE_WHEELDIA	5
} WHEELDIA;

#include <sys/time.h>

typedef struct _tagCALCSPEEDTRIGGER {
	BOOL	bOpen;
	BYTE	nNull;
	uint64_t	clock;
	struct timeval	stv;
	struct timeval	etv;
} SPEEDTRIGGER;

#define	LENGTH_AXEL		4
#define	CYCLE_PULSE		5

typedef struct _tagCALCSPEEDELEM {
	WORD	wPi;
	WORD	wPulse[LENGTH_AXEL][CYCLE_PULSE];
} SPEEDELEM;

typedef struct _tagCALCSPEEDRESULT {
	DWORD	dwSpeed[LENGTH_AXEL];
} SPEEDRESULT;

typedef struct _tagCURRENTIDC {
	WORD	wAvr;
	WORD	wLength;
	DWORD	dwSum;
} CURRENTIDC;

typedef struct _tagAVERAGEIDC {
	CURRENTIDC	p;
	CURRENTIDC	n;
} AVERAGEIDC;

typedef struct _tagAVERAGEPOWER {
	WORD	wSync;
	WORD	wLength;
	uint64_t	clock;
	double	p;
	double	n;
} AVERAGEPOWER;

typedef struct _tagPOCKETTIMER {
	BYTE	nDevTimer;
	BYTE	nNull;
	WORD	w20ms;
	WORD	w50ms;
	WORD	w100ms;
	WORD	w200ms;
} POCKETTIMER;
#define	INTERVAL_POCK20MS	10
#define	INTERVAL_POCK50MS	4
#define	INTERVAL_POCK100MS	2
#define	INTERVAL_POCK200MS	2

typedef struct _tagLOGINFO {
	BYTE	nState;
#define	STATELOG_SCAN	1

	BYTE	nCounter;
#define	CYCLE_TICKER	50	// 10 * 50 = 500ms

	WORD	wCurTime;
	WORD	wMaxTime;
	WORD	wTimer;
} LOGINFO;

typedef struct _tagOPREF {
	BYTE	nBus;
	BYTE	nPiH;
	BYTE	nPiL;
	BYTE	nNull;
} OPREF;

// BEGIN CCREFERENCE { ====================================================
typedef struct _tagCCREFERENCEREAL {
	WORD		wProperty;
	CARINFO		car;
	CCBUSCYCLE	cycle;
	PWMARCH		pwm;
	SPEEDTRIGGER	spt;
	SPEEDELEM	spe;
	DWORD		dwSpeed[LENGTH_AXEL];
	double		dbBase[LENGTH_AXEL];
	WHEELDIA	dia[LENGTH_AXEL];
	AVERAGEPOWER	sivp;
	AVERAGEPOWER	v3fp;
	AVERAGEIDC	idc;
	AVERAGEIDC	bkidc;
	POCKETTIMER	pt;
	LOGINFO		log;
	OPREF		opr;
	float		cps;

	DWORD		dwRecvOrderID;
	WORD		wActiveBus;
#define	ACTIVEBUS_MAIN		1
#define	ACTIVEBUS_RESV		2

	WORD		wActiveDev;
	WORD		wOut[3];
	WORD		wKxOut;
#define	KXOUTS	((1 << (CCDOB_K1L - 16)) | (1 << (CCDOB_K2L - 16)) | (1 << (CCDOB_K3L - 16)))

	WORD		wEskMonitor;
#define	TIME_MONITORESK		3000 / TIME_INTERVAL

	BYTE		nDoorFlow;
	BYTE		nCallID;
	WORD		wDevDeb[DEVID_MAX];
	WORD		wDoorDeb[8];
} CCREFERENCEREAL;

#define	SIZE_CCREFERENCE	512
typedef struct _tagCCREFERENCE {
	CCREFERENCEREAL	real;
	BYTE	dummy[SIZE_CCREFERENCE - sizeof(CCREFERENCEREAL)];
} CCREFERENCE, *PCCREFERENCE;

// END CCREFERENCE { ======================================================

#pragma pack()

#define	BATVOLTAGE_NORMAL	79
#define	BATVOLTAGE_WARNING	77
#define	BATVOLTAGE_FAULT	70
#define	BATVOLTAGE_LGCRANGE	156
#define	BATVOLTAGE_PSCRANGE	4095

enum  {
	LP_OFF = 0,
	LP_ON,
	LP_FLASH
};

#define	BUSFAIL_NON		0
#define	BUSFAIL_MAIN	1
#define	BUSFAIL_RESV	2

#define	TDIRRES_NON							0
#define	TDIRRES_DEBOUNCING					1
#define	TDIRRES_EXCHANGE					2
#define	TDIRRES_UNKNOWNID					3
#define	TDIRRES_EXCHANGE0					4
#define	TDIRRES_EXCHANGETOHEAD				5
#define	TDIRRES_EXCHANGETOHEADWITHLOADENV	6
#define	TDIRRES_EXCHANGETOTAIL				7
#define	TDIRRES_BUSERROR					8

#define	OWN_SPACE		c_tcRef.real.nDuty
#define	OWN_SPACEA		c_pTcRef->real.nDuty
#define	PROCSTEP(n)		c_panel.StepLamp(n);
#define	PROCALARM(n)	c_panel.AlarmLamp(n);
#define	RECTIME(n)\
	do {\
		c_opTimes[n].cur = ClockCycles();\
		c_opTimes[n].bUse = TRUE;\
	} while(0)

#define	TOEND(p)\
	do {\
		while (*p != NULL)	++ p;\
	} while(0)

#define	PTYPE(name)	#name, sizeof(name)

#define	HEADCCBYLENGTH	((8 - c_tcRef.real.cf.nLength) >> 1)
#define	HEADCCBYLENGTHA	((8 - c_pTcRef->real.cf.nLength) >> 1)

enum  {
	TTIME_BEGIN = 0,
	TTIME_TCOPEN,		// begin broadcast
	TTIME_TCOPENED,		// end broadcast
	TTIME_BUSSEND,
	TTIME_BUSSENDED,
	TTIME_LOCALSEND,
	TTIME_LOCALSENDED,
	TTIME_LOCALCLOSE,
	TTIME_PROCBEGIN,
	TTIME_PROCEND,
	TTIME_TICKERBEGIN,
	TTIME_TICKEREND,
	TTIME_BUSRESPOND,
	TTIME_BUSRESPONDB,
	TTIME_BUSRESPONDC,
	TTIME_BUSRESPONDD,
	TTIME_BUSRESPOND1,
	TTIME_BUSRESPOND2,
	TTIME_BUSRESPOND3,
	TTIME_BUSRESPOND4,
	TTIME_BUSRESPOND5,
	TTIME_BUSRESPOND6,
	TTIME_BUSRESPOND7,
	TTIME_BUSRESPOND8,
	TTIME_BUSCLOSE,
	TTIME_BUSTIMEOUT,
	TTIME_TCCLOSE,
	TTIME_TCUPDATE,
	TTIME_CYCLEEND,
	TTIME_PRESENT,
	TTIME_PRESENTED,
	TTIME_TIMERBEGIN,
	TTIME_TIMEREND,
	TTIME_MAXSHEET
};

enum  {
	CTIME_BEGIN = 0,
	CTIME_DEVICESEND,
	CTIME_DEVICESENDED,
	CTIME_V3FSEND,
	CTIME_V3FSENDED,
	CTIME_ECUSEND,
	CTIME_ECUSENDED,
	CTIME_DCUSEND,
	CTIME_DCUSENDED,
	CTIME_CMSBSEND,
	CTIME_CMSBSENDED,
	CTIME_OPENDEV,
	CTIME_CLOSEV3F,
	CTIME_CLOSESIV,
	CTIME_CLOSEECU,
	CTIME_CLOSEDCU,
	CTIME_CLOSECMS,
	CTIME_BUSRECVOTHER,
	CTIME_BUSSENDA,
	CTIME_BUSSENDEDA,
	CTIME_BUSSENDB,
	CTIME_BUSSENDEDB,
	CTIME_CYCLEEND,
	CTIME_TICKERBEGIN,
	CTIME_TICKEREND,
	CTIME_BUSPRESENT,
	CTIME_BUSPRESENTED,
	CTIME_MAXSHEET
};

#endif	/* TRAINARCH_H_ */
