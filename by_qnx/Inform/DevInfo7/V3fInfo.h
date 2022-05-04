/*
 * V3fInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	V3FINFO_H_
#define	V3FINFO_H_

// follow "ESEM_024 TCMS-VVVF I/F(0308)"

#pragma pack(1)

// const WORD	addr = 0x7301;
// BYTE	nFlow -> 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
// Request, tcms->siv

typedef struct _tagSRV3F {
	_TV_	vTbeL;		// 1-2, little-endian
	PARCEL(	inst,		// 3
				msb,	// maximum service brake
				fsb,	// full service brake
				mwb,	// motoring while braking
				p,		// powering selection feedback
				b,		// braking selection feedback
				rst,	// reset
				f,		// forward feedback
				r );	// reverse feedback
	PARCEL(	state,		// 4
				pand,	// danto down
				spr6,
				epcor,	// Emergency Power CutOut Relay
				hb,		// hold brake
				spr3, spr2, spr1, spr0 );
	PARCEL(	mode,		// 5
				pp1co,	// 1 motor car CutOut mode
				pp2co,	// 2 motor car CutOut mode
				spr5, spr4,
				fmcco,	// CutOut command
				spr2, spr1, spr0 );
	DEVTIME	time;		// 6-11
	PARCEL(	test,		// 12
				vfsot,	// test
				spr6, spr5, spr4, spr3,
				pdt,
				dayt,	// daily inspection
				mont );	// monthly inspection
	PARCEL(	exec,		// 13
				tdc,	// Trace Data Clear
				spr6, spr5, spr4, spr3, spr2, spr1,
				tmvld );	// Time Valid
} SRV3F;

typedef struct _tagTRV3F {
	WORD	wIndex;		// sequential index no. (inc. from 0)
	BYTE	nSpr;
} TRV3F;

typedef union _tagV3FRTEXT {
	SRV3F	st;
	TRV3F	tr;
} V3FRTEXT;

typedef struct _tagV3FRINFO {
	LWORD	wAddr;		// = 0x7301
	BYTE	nFlow;		//
	V3FRTEXT	d;
} V3FRINFO, *PV3FRINFO;

#define	SIZE_V3FSR	(sizeof(SRV3F) + 3)	//sizeof(V3FRINFO)
#define	SIZE_V3FTR	(sizeof(TRV3F) + 3)	//V3FRLENG - 10

typedef struct _tagV3FCINFO {	// TC->CC
	_TV_	vTbeL;		// 0-1 *, little-endian
	PARCEL(	inst,		// 2 *
				msb,	// maximum service brake
				fsb,	// full service brake
				mwb,	// motoring while braking
				p,		// powering selection feedback
				b,		// braking selection feedback
				rst,	// reset
				f,		// forward feedback
				r );	// reverse feedback
	PARCEL(	state,		// 3 *
				pand,	// danto down
				spr6,
				epcor,	// Emergency Power CutOut Relay
				hb,		// hold brake
				spr3, spr2, spr1, spr0 );
	PARCEL(	mode,		// 4 *	//cutout length		0	1	2	3
				pp1co,	// 1 cutout bit				0	1	0	1
				pp2co,	// 2 cutout bit				0	0	1	1
				spr5,
				rescue,
				fmcco,	// CutOut command
				spr2, spr1, spr0 );
	PARCEL(	test,		// 5 *
				vfsot,	// test
				spr6, spr5, spr4, spr3,
				pdt,
				day,	// daily inspection
				month );// monthly inspection
	WORD	wMsb;		// 6-7 *
	WORD	wRstCmd;	// 8-9 *
	WORD	wPanto;		// 10-11
	WORD	wFmccoCmd;	// 12-13 *
	WORD	wVfsot;		// 14-15
	WORD	wTimeValid;	// 16-17
	WORD	wTraceClear;	// 18-19
	WORD	wTracing;	// 20-21
} V3FCINFO, *PV3FCINFO;

// Acknowledge
typedef struct _tagSAV3F {
	_TV_	vTbeL;		// 1-2, -1, little-endian
	PARCEL(	inst,		// 3
				msb,	// maximum service brake
				fsb,	// full service brake
				mwb,	// motoring while braking
				p,		// Powering selecton feedback
				b,		// Braking selection feedback
				rst,	// reset request
				f,		// forward feedback
				r );	// reverse feedback
	PARCEL(	state,		// 4
				pand,	// danto down
				spr6,
				epcor,	// emergency power cutout relay
				spr4, spr3, spr2, spr1, spr0 );
	PARCEL(	cout,		// 5
				pp1co,	// 1 motor car CutOut mode
				pp2co,	// 2 motor car CutOut mode
				spr5, spr4,
				fmcco,	// CutOut command
				spr2, spr1, spr0 );
	PARCEL(	test,		// 6
				tce1,	// test condition
				tce0,
				tpd,	// test preparation
				whto,	// test ok
				whtn,	// test ng
				mot,	// monthly inspection
				dat,	// daily inspection
				pdt );	// pre-depareture
	BYTE	nItem;		// 7
	BYTE	nVer;		// 8 VVVF Software Version <-- move (9th)
	PARCEL(	req,		// 9
				spr7, spr6, spr5,
				vco,	// Vehicle CutOut
				spr3,
				log,	// data log
				timereq,// timer request
				acoreq );	// Auto CutOut Request
	PARCEL(	trace,		// 10
				al3, al2, al1, al0,
				tr3, tr2, tr1, tr0 );	// Number of Failure Trace data(0-10) HEX DATA
	BYTE	nSpr11;		// 11
	PARCEL(	dedev,		// 12
				pgfd4, pgfd3, pgfd2, pgfd1,	// pulse generator failure detection for MM4
				bcfd,	// chopper failure detection
				hbd,	// hscb no action detection
				ld,		// cck no action detection
				hotf );	// gate signal feedback fault
				//cfd );	// commutation on failure detection
	PARCEL(	deval,		// 13
				bsd,	// backward detection
				l1d,	// fc charge failure
				wdtd,	// watchdog detection
				clvd,	// control power lv detection
				gplvd,	// date power lv detection
				mocd,	// motor over current
				chdf,	// chopper driver fault,	// spr1,
				pud );	// phase unbalance detection
	PARCEL(	deexc,		// 14
				l1da,	// hscb not open detection
				hscbt,	// hscb trip detection
				lgd,	// line ground detection
				fclvd,
				eslvd,	// source low voltage
				ovd1,	// over voltage detection1
				ovd2,	// over voltage detection2
				wsd );	// wheel slip/slide detection
	PARCEL(	gdf,		// 15
				gdfu,
				gdfv,
				gdfw,
				iocd,
				spr3, spr2, spr1, spr0 );
	PARCEL(	oper,		// 16
				fn,		// forward direction
				rn,		// reserve direction
				spr5,
				p,		// powering mode
				b,		// braking mode
				bcv,	// regen. brake current valid
				chgst,
				spr0 );
	PARCEL(	cmd,		// 17
				ppco,	// pulse mode selection
				frwmd,	// tcms failure mode
				imc,	// motor current command
				sqts,	// sequence test switch
				rstreq,	//
				spr2, spr1,
				rstack );	// reset
	PARCEL(	sig,		// 18
				gs,		// gate start
				bchon,	// brake chopper gate start
				spr5,
				ccrar,	// cck closing signal
				spr3,
				hscb,	// hscb closing
				cck,	// cck closing
				spr0 );
	PARCEL(	ack,		// 19
				spr7,
				rsr,	// resetting output
				pr,		// powering active
				bfd,	// braking feed out
				spr3, spr2,
				cdr,	// re-generating brake failure
				dck );	// operation active
	PARCEL(	mode,		// 20
				p1,		// 1 pulse mode
				spr6,
				dp3,	// 3 dash pulse mode
				p3,		// 3 pulse mode
				p5,		// 5 pulse mode
				p9,		// 9 pulse mode
				p15,	// 15 pulse mode
				async );	// async. mode
	BYTE	nEs;		// 21, line voltage,				0-2000V (0x00-0xfa)
	BYTE	nFc;		// 22, filter capacitor voltage,	0-2000V (0x00-0xfa)
	BYTE	nTrqRef;	// 23, torque instruction,			0-2000A (0x00-0xfa)
	BYTE	nIq;		// 24, torque current,				0-2000A (0x00-0xfa)
	BYTE	nIm;		// 25, motor current,				0-1000A (0x00-0xfa)
	BYTE	nAl;		// 26, modulation rate,				0-1.00  (0x00-0xfa)
	BYTE	nMf;		// 27, slip frequency,				0-200Hz (0x00-0xfa)
	BYTE	nBp;		// 28, brake demand,				0-100%  (0x00-0xfa)
	BYTE	nBfc;		// 29, brake feedback torque,		0-100%  (0x00-0xfa)
	BYTE	nVl;		// 30, variable load,				0-100%  (0x00-0xfa)
	BYTE	nTgf1;		// 31, TG pulse1,					0-200Hz (0x00-0xfa)
	BYTE	nTgf2;		// 32, TG pulse2,					0-200Hz (0x00-0xfa)
	BYTE	nTgf3;		// 33, TG pulse3,					0-200Hz (0x00-0xfa)
	BYTE	nTgf4;		// 34, TG pulse4,					0-200Hz (0x00-0xfa)
	BYTE	nHptp;		// 35, heat pipe temperature,		0-250^C (0x00-0xff)
	_TV_	vIdc;		// 36-37, * motor current, big-endian
						// this value be maked CC from motor sensor (LS-MS) *
	BYTE	nSpr37;		// 38
} SAV3F;

#define	TRACEBLENGTH_V3F	100

typedef struct _tagTAV3F {
		WORD	wIndex;		// trace length = 10 -> 10062 * 10 = 100620byte
							// wBlock has 0 to 1006 (hex)
		BYTE	nText[TRACEBLENGTH_V3F];
} TAV3F;

typedef union _tagV3FATEXT {
	SAV3F	st;
	TAV3F	tr;
} V3FATEXT;

typedef struct _tagV3FAINFO {
	LWORD	wAddr;		// = 0x7301
	BYTE	nFlow;		//
	V3FATEXT	d;
} V3FAINFO, *PV3FAINFO;

#define	SIZE_V3FSA	(sizeof(SAV3F) + 3)
#define	SIZE_V3FTA	(sizeof(TAV3F) + 3)

typedef struct _tagSEV3F {
	SAV3F	st;			// 1-38
	double	power;		// 38-45
	double	revival;	// 56-53
	DWORD	dwSpd[4];	// 54-69
	//WORD	vIdcP;		// 38-39, current consumption, big-endian
	//					// this value be maked CC from motor sensor (LS-MS)
	//WORD	vIdcN;		// 40-41, current revival
	//DWORD	dwSpd[4]; 	// 42-57, store average speed value which computed with TG pluses.
} SEV3F;

typedef union _tagV3FETEXT {
	SEV3F	st;
	TAV3F	tr;
} V3FETEXT;

typedef struct _tagV3FEINFO {
	LWORD	wAddr;		// = 0x7301
	BYTE	nFlow;		//
	V3FETEXT	d;
} V3FEINFO, *PV3FEINFO;

// vvvf sampling data struct
typedef struct _tagV3FSTEXT {
	PARCEL(	text0,		// 1
				f,
				r,
				p,
				b,
				hscb,
				cck,
				hbt,	// hscb trip
				spr0 );
	PARCEL(	text1,		// 2
				hscba,
				ccka,
				hbta,
				gsc,
				sqs,
				spr2,
				spr1,
				spr0 );
	BYTE	nSpr2;		// 3
	PARCEL( text3,		// 4
				bpsf,	// battery power fault
				spsf,
				apsf,
				gdfu,	// gate driver fault
				gdfv,
				gdfw,
				chgdf,	// chopper gate driver fault
				spr0 );
	PARCEL(	text4,		// 5
				iocd,	// line over current
				mocd,	// motor over current
				bocd,
				pud,	// motor phase fault
				vllvd,	// line low power detect
				fclvd,	// capacity low power detect
				fcovd,	// capacity over power detect
				pgd );	// pg fault
	PARCEL(	text5,		// 6
				otdu,	// stack over-heating
				otdv,
				otdw,
				hbtf,	// hscb mc fault
				lgd,	// 
				spr2,
				spr1,
				sync );
	BYTE	nSpr6[2];	// 7-8
	LWORD	wLineVoltage;			// 9-10
	LWORD	wFcVoltage;				// 11-12
	LWORD	wDcCurrent;				// 13-14
	LWORD	wActualTorqueCurrent;	// 15-16
	LWORD	wTorqueCurrentCommand;	// 17-18
	LWORD	wActualFluxCommand;		// 19-20
	LWORD	wFluxCurrentCommand;	// 21-22
	LWORD	wModulationIndex;		// 23-24
	LWORD	wIOU;					// 25-26
	LWORD	wIOV;					// 27-28
	LWORD	wIOW;					// 29-30
	LWORD	wMotorCurrent;			// 31-32
	LWORD	wInverterFrequency;		// 33-34
	LWORD	wSlipFrequency;			// 35-36
	LWORD	wLoadWeight;			// 37-38
	LWORD	wAlpha;					// 39-40
	LWORD	wDeltaPwm;				// 41-42
	LWORD	wTorqueEstimation;		// 43-44
	LWORD	wWmSim;					// 45-46
	LWORD	wSpr47[2];				// 47-48-49-50
} V3FSTEXT, *PV3FSTEXT;

#define	LENGTH_SAMPLINGV3F	201

typedef struct _tagV3FSTABLE {
	BYTE	nCode;		// 0, "0x80"
	BYTE	nSpr1[2];	// 1-2
	BYTE	nFault;		// 3
	BYTE	nYear;		// 4
	BYTE	nMonth;		// 5
	BYTE	nDay;		// 6
	BYTE	nSpr7;		// 7
	BYTE	nHour;		// 8
	BYTE	nMinite;	// 9
	BYTE	nSec;		// 10
	BYTE	nSpr11;		// 11
	V3FSTEXT	sd[LENGTH_SAMPLINGV3F];
	BYTE	nNull[38];	// v3f has heap data
} V3FSTABLE, *PV3FSTABLE;	// 10062 + 38 = 10100byte / 1block

#define	MAXBLOCK_V3F	1010	// 10100 * 10 = 101000 byte / 100
#define	MAXTRACELENGTH_V3F	10

typedef struct _tagV3FCHASE {
	TRACEARCH	ta;
	BYTE	d[MAXBLOCK_V3F][TRACEBLENGTH_V3F];
} V3FCHASE, *PV3FCHASE;

#pragma pack()

#endif	/* V3FINFO_H_ */
