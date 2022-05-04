/*
 * EcuInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	ECUINFO_H_
#define	ECUINFO_H_

// follow "ESEM_022_TCMS-ECU I/F(1222)"

#pragma pack(1)

// const WORD	addr = 0x7302
// BYTE	nFlow				SDR		SDA
//	status					0x20	0x20
//	status & time set		0x21	0x21
//	status & test			0x22	0x22
//	status & trace clear	0x23	0x23
//	trace data				0x30	0x30

#define	ECUFLOW_SD			0x20
#define	ECUFLOW_TSET		0x21
#define	ECUFLOW_TEST		0x22
#define	ECUFLOW_TCLR		0x23
#define	ECUFLOW_TDTA		0x30

// Request
/*typedef union _tagECURTEXT {
	struct {		// SDR,	CC->ecu
		PARCEL(	set,		// 3
					rqt,	// request of test
					tit3, tit2, tit1, tit0,	// test item
					spr2,
					cals,	// calender set
					rtdc );	// request of trace data clear
		BYTE	nAsoc;		// 4, AS pressure of the other car
		DEVTIME	time;		// 5-10
	} st;
	struct {
		BYTE	nTrace;		// 0 to 4
		BYTE	nIndex;		// 0 to 40 (41block * 2 = 82sample, 22byte/sample)
	} tr;
} ECURTEXT;
*/

typedef struct _tagSRECU {
	PARCEL(	set,		// 1
				rqt,	// request of test
				tit3, tit2, tit1, tit0,	// test item
				spr2,
				cals,	// calender set
				rtdc );	// request of trace data clear
	BYTE	nAsoc;		// 2, AS pressure of the other car
	DEVTIME	time;		// 3-8
} SRECU;

typedef struct _tagTRECU {
	BYTE	nTrace;		// 0 to 4
	BYTE	nIndex;		// 0 to 40 (41block * 2 = 82sample, 22byte/sample)
} TRECU;

typedef union _tagECURTEXT {
	SRECU	st;
	TRECU	tr;
} ECURTEXT;

typedef struct _tagECURINFO {
	LWORD	wAddr;		// = 0x7302
	BYTE	nFlow;		//
	_TV_	vTbeL;		// 0-1,
	PARCEL(	inst,		// 2
				pecuf,	// paired ecu failure
				msb,	// maximum service brake
				adc,	// door close
				hb,		// hold brake
				fsb,	// full service brake
				mwb,	// motoring while braking
				p,		// powering selection
				b );	// braking selection
	ECURTEXT	d;
} ECURINFO, *PECURINFO;

#define	SIZE_ECUSR	(sizeof(SRECU) + 6)	//sizeof(ECURINFO)
#define	SIZE_ECUTR	(sizeof(TRECU) + 6)	//ECURLENG - 6

typedef struct _tagECUCINFO {	// TC->CC
	_TV_	vTbeL;		// 0-1, little-endian
	PARCEL(	inst,		// 2
				pecuf,	// paired ecu failure
				msb,	// maximum service brake
				adc,	// door close
				hb,		// hold brake
				fsb,	// full service brake
				mwb,	// motoring while braking
				p,		// powering selection
				b );	// braking selection
	WORD	wPecuf;		// 3-4, fail ecu flag
	WORD	wMsb;		// 5-6
	PARCEL(	set,		// 7
				rqt,	// request of test
				tit3, tit2, tit1, tit0,	// test item
				spr2,
				cals,	// calender set
				rtdc );	// request of trace data clear
	WORD	wRqt;		// 8-9
	WORD	wCals;		// 10-11
	WORD	wTraceClear;	// 12-13
	BYTE	nAsoc[8];	// 14-21, AS pressure of the other car
	DEVTIME	time;		// 22-23-24-25-26-27
	WORD	wTracing;	// 28-29
} ECUCINFO, *PECUCINFO;

// Acknowledge
typedef struct _tagECUACOMMON {
	PARCEL(	fail,		// 1, -1
				buf,	// pneumatic brake failure
				sde,	// self diagnosis error
				ramf,	// ram check error
				rbedf,	// abnormality of regen. brake effort demand
				lwf,	// abnormality of load weight
				emvf,	// electromagnetic valve failure
				dvff,	// dump valve failure (front bogie)
				dvrf );	// dump valve failure (rear bogie)
	PARCEL(	failsig,	// 2
				rbapsf,	// rear bogie AS press. sensor failure
				fbapsf,	// front bogie AS press. sensor failure
				bpsf,	// BC press. sensor failure
				apsf,	// AC press. sensor failure
				a1as, a2as, a3as, a4as );	// axle abnormal speed signal
	PARCEL(	abnor,		// 3
				spr7, spr6, spr5, spr4,
				abrc,	// anormallity of air brake command <-- spr3
				bcf,	// abnormality of brake command
				tllf,	// abnormality of train line logic
				rbeaf ); // abnormality of regen. brake effort achieved
	PARCEL(	axle,		// 4
				spr7, spr6,
				fbasp,	// front bogie AS puncture
				rbasp,	// rear bogie AS puncture
				ss1a, ss2a, ss3a, ss4a );	// skid signal of axle
	_TV_	vTbeL;		// 5-6, little-endian
	PARCEL(	inst,		// 7
				pecuf,	// paired ecu failure
				msb,	// maximum service brake
				dc,		// door close
				hb,		// hold brake
				fsb,	// full service brake
				mwb,	// motoring while braking
				p,		// powering command
				b );	// braking command
	PARCEL(	set,		// 8
				oot,	// operation test
				tc,		// test completion
				spr5, spr4,
				nrbd,
				cpr,
				nnrbd,
				tdc );	// trace data clear
} ECUACOMMON;

typedef struct _tagSAECU {
	ECUACOMMON	cm;		// 1-8
	BYTE	nAsp;		// 9, AS pressure,					50-560kPa(0x00-0xff)2kPa/bit, 0.51~5.71Kg/cm^2
	BYTE	nWt;		// 10, load weight,					0-102%   (0x00-0xff)0.4%/bit
	BYTE	nBed;		// 11, regen. brake effort demand	0-102%   (0x00-0xff)0.4%/bit
	BYTE	nBea;		// 12, regen. brake effort achieved	0-102%   (0x00-0xff)0.4%/bit
	BYTE	nAbrc;		// 13, air brake reducing command	0-10.2V  (0x00-0xff)0.04V/bit
	BYTE	nAcp;		// 14, AC pressure,					0-765kPa (0x00-0xff)3kPa/bit
	BYTE	nBcp;		// 15, BC pressure,					0-765kPa (0x00-0xff)3kPa/bit, 0~7.8Kg/cm^2
	BYTE	nTrace;		// 16, Number of Failure Trace data(0-5) BIT form
	BYTE	nMrp;		// 17, main compressor pressure (0.0 ~ 10.0kg/cm2)
	BYTE	nTestResult;	// 18
	BWORD	wUdvwt;		// 19-20, design value of load weight	0-102%	(0x00-0xff)
	BWORD	wUdvbed;	// 21-22, design value of regen. brake effort demand	0-102%	(0x00-0xff)
	BWORD	wUdvacp;	// 23-24, design value of AC pressur	0-765kPa	(0x00-0xff)
	BWORD	wUdvbcp;	// 25-26, design value of BC pressur	0-765kPa	(0x00-0xff)
	BYTE	nSpr26[4];	// 27-30
} SAECU;

typedef struct _tagECUSTIME {
	BYTE	nFault;		// 0
	BYTE	nYear;		// 1
	BYTE	nMonth;		// 2
	BYTE	nDay;		// 3
	BYTE	nHour;		// 4
	BYTE	nMinute;	// 5
	BYTE	nSecond;	// 6
} ECUTSTIME;

typedef struct _tagECUSTEXT {
	ECUACOMMON	cm;		// 0-7
	BYTE	nAsp[3];	// 8-10
	BYTE	nWt;		// 11, load weight,					0-102%   (0x00-0xff)0.4%/bit
	BYTE	nBed;		// 12, regen. brake effort demand	0-102%   (0x00-0xff)0.4%/bit
	BYTE	nBea;		// 13, regen. brake effort achieved	0-102%   (0x00-0xff)0.4%/bit
	BYTE	nAbrc;		// 14, air brake reducing command	0-10.2V  (0x00-0xff)0.04V/bit
	BYTE	nAcp;		// 15, AC pressure,					0-765kPa (0x00-0xff)3kPa/bit
	BYTE	nBcp;		// 16, BC pressure,					0-765kPa (0x00-0xff)3kPa/bit, 0~7.8Kg/cm^2
	BYTE	nAxle[4];	// 17-20, axle speed				0-127.5Km/h	(0x00-0xff)
	BYTE	nSpeed;		// 21, standard axle speed				0-127.5Km/h	(0x00-0xff)
} ECUTSTEXT, *PECUTSTEXT;	// 22byte

typedef union _tagECUTRFORM {
	ECUTSTIME	tm;
	ECUTSTEXT	t;		// 0-21
} ECUTRFORM;		// 22byte

#define	ECUTRACEMARKLENGTH_EMPTY	5
#define	ECUTRACEMARKLENGTH_END		10

typedef struct _tagECUTRACEDA {
	ECUTRFORM	f;		// 22byte
	ECUTSTEXT	s;		// 22byte
} ECUTRACEA;

typedef union _tagTAECU {
	ECUTRACEA	t;
	BYTE		n[44];
} TAECU;		// 44byte

typedef union _tagECUATEXT {
	SAECU	st;
	TAECU	tr;
} ECUATEXT;

typedef struct _tagECUAINFO {
	LWORD	wAddr;		// = 0x7302
	BYTE	nFlow;		//
	ECUATEXT	d;
} ECUAINFO, *PECUAINFO;

#define	SIZE_ECUSA	(sizeof(SAECU) + 3)
#define	SIZE_ECUTA	(sizeof(TAECU) + 3)

#define	LENGTH_SAMPLINGECU	82

typedef struct _tagECUSTABLE {
	ECUTSTEXT	sd[LENGTH_SAMPLINGECU];
} ECUSTABLE, *PECUSTABLE;	// 1804byte

#define	MAXBLOCK_ECU	41	// 44 * 41 = 1804 byte
#define	MAXTRACELENGTH_ECU	5

typedef struct _tagECUCHASE {
	TRACEARCH	ta;
	union {
		BYTE	s[MAXTRACELENGTH_ECU][MAXBLOCK_ECU][sizeof(TAECU)];
		BYTE	c[MAXTRACELENGTH_ECU * MAXBLOCK_ECU][sizeof(TAECU)];
		ECUSTABLE	t[MAXTRACELENGTH_ECU];
	} d;
} ECUCHASE, *PECUCHASE;

#pragma pack()

#endif	/* ECUINFO_H_ */
