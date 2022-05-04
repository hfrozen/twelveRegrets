/*
 * SivInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	SIVINFO_H_
#define	SIVINFO_H_

// follow "ESEM_023_TCMS-SIV I/F (1118)"

#pragma pack(1)

// const WORD	addr = 0x7303;
// BYTE	nFlow -> 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
// Request, cc->siv
/*typedef union _tagSIVRTEXT {
	struct {				// SDR
		PARCEL(	state,		// 0
					spr7, spr6,	spr5, spr4,
					spr3, spr2,
					pand,	// Panto Down
					lors );	// Lockout Reset
		PARCEL(	test,		// 1
					tdc,	// trace data clear
					spr6, spr5, spr4, spr3, spr2, spr1,
					sivsot );	// Start of Test
		DEVTIME time;		// 2-7
	} st;
	struct {				// TDR
		WORD	wIndex;		// sequential index no. (inc. from 0)
							// trace length = 10 -> 3626 * 10 = 36260byte
							// wBlock has 0 to 362 (0 ~ 0x16a)
		BYTE	nSpr;
	} tr;
} SIVRTEXT;
*/

typedef struct _tagSRSIV {
	PARCEL(	state,		// 1
				spr7, spr6,	spr5, spr4,
				spr3, spr2,
				pand,	// Panto Down
				lors );	// Lockout Reset
	PARCEL(	test,		// 2
				tdc,	// trace data clear
				spr6, spr5, spr4, spr3, spr2, spr1,
				sivsot );	// Start of Test
	DEVTIME time;		// 3-8
} SRSIV;

typedef struct _tagTRSIV {
	WORD	wIndex;		// sequential index no. (inc. from 0)
						// trace length = 10 -> 3626 * 10 = 36260byte
						// wBlock has 0 to 362 (0 ~ 0x16a)
	BYTE	nSpr;
} TRSIV;

typedef union _tagSIVRTEXT {
	SRSIV	st;
	TRSIV	tr;
} SIVRTEXT;

typedef struct _tagSIVRINFO {
	LWORD	wAddr;		// = 0x7303
	BYTE	nFlow;		//
	SIVRTEXT	d;
} SIVRINFO, *PSIVRINFO;

#define	SIZE_SIVSR	(sizeof(SRSIV) + 3)	//sizeof(SIVRINFO)
#define	SIZE_SIVTR	(sizeof(TRSIV) + 3)	//SIVRLENG - 5

// control, tc->cc
typedef struct _tagSIVCINFO {
	WORD	wPanto;			// 0-1
	WORD	wLors;			// 2-3
	WORD	wSivsot;		// 4-5
	WORD	wTraceClear;	// 6-7
	WORD	wTracing;		// 8-9
} SIVCINFO, *PSIVCINFO;

// Acknowledge, siv->tcms
typedef struct _tagSASIV {
	BYTE	nTrace;		// 1, -1, Number of Failure Trace data(0-10) BCD DATA
	PARCEL(	test,		// 2
				spr7,
				tsw2,	// Gate Output Test
				tsw1,	// Sequence Test
				spr4, spr3,
				chkx,	// Chk On Signal
				spr1,
				ivkx );	// Ivk On Signal
	PARCEL(	state,		// 3
				ssmk,	// Siv Standart Movement
				spr6, spr5,
				lovd,	// Output Voltage fault	// elvr,	// Input Low Voltage
				spr3,
				dcov,	// eovr,	// Input Over Voltage
				isoc,	// is300,	// Input Over Current
				spr0 );
	PARCEL(	vol,		// 4
				spr7, spr6,
				sivkax,	// siv ivk contactor fault	// fclv,	// Input Fc Low Voltage
				fcov,	// Input Fc Over Voltage
				spr3,
				iaoc,	// siv Input over-current	// elv,	// INV Input Low Voltage
				sivkx,	// Contactor fault	// eov,	// INV Input Over voltage
				spr0 );
	PARCEL(	cur,		// 5
				stx,	// Gate On Command
				spr6,
				chkax,	// Chk Answer
				lock,	// stop cause siv heavy fault	// sivflo,	// LockOut
				spr3,
				iocw,	// INV W Phase Output Over Current
				iocv,	// INV V Phase Output Over Current
				iocu );	// INV U Phase Output Over Current
	PARCEL(	over,		// 6
				acov,	// Output Over-voltage	// ovdox,	// Output Over Voltage(S/W)
				thuf,	// heat pipe u phase overheat	// ovdo,	// Output Over Voltage(H/W)
				aclv,	// Output Low Voltage
				spr4,
				osc,	// Current Limit
				spr2, spr1,
				ool );	// Over Load
	PARCEL(	inst,		// 7
				sivtc,	// Test Complete
				sivte,	// Test Excuting
				spr5, spr4, spr3,
				thwf,	// heat pipe w phase overheat	// hpotm,	// Heat-pipe Base Plate Over Thermal
				trof,	// Test Result Of OF
				treo );	// Test Result Of EO
	PARCEL(	ack,		// 8
				chkf,	// Chk On Answer Abnormal
				spr6,
				ivkax,	// IVK Answer
				ivkf,	// IVK Answer Abnormal
				spr3, spr2,
				sivkf,	// sivk contactor fault	// glvd,	// Gate PS Low Voltage
				spr0 );
	PARCEL(	low,		// 9
				thvf,	// heat pipe v phase overheat	// cp1lvd,	// Control PS Low Voltage1
				bcoc,	// Control PS Low Voltage2
				bcov,	// Control PS Low Voltage3
				spr4, spr3,
				stf,	// fcvlvsx,// FC Voltage Not Establish
				spr1, spr0 );
	BYTE	nEo;		// 10, Output Voltage							AC 0-750V (0x00-0xfa) 3.0V/bit
	BYTE	nIo;		// 11, Output Current,							AC 0-1250A (0x00-0xfa) 5.0A/bit
	BYTE	nOf;		// 12, output Frequency,						0-127.5Hz (0x00-0xfa) 0.5Hz/bit
	BYTE	nFcv;		// 13, FC Voltage,								DC 0-3000V (0x00-0xfa) 12.0V/bit
	BYTE	nEd;		// 14, Input Voltage,							DC 0-3000V (0x00-0xfa) 12.0V/bit
	BYTE	nIs;		// 15, Input Current,							DC 0-750A (0x00-0xfa) 3.0A/bit
	BYTE	nUdveo;		// 16, Upper Design Value of Output Voltage,	AC 0-750V (0x00-0xfa) 3.0V/bit
	BYTE	nLdveo;		// 17, Low Design Value of Output Voltage,		AC 0-750V (0x00-0xfa) 3.0V/bit
	BYTE	nUdvof;		// 18, Upper Design Value of Output Frequency,	0-127.5Hz (0x00-0xfa) 0.5Hz/bit
	BYTE	nLdvof;		// 19, Low Design Value of Output Frequency,	0-127.5Hz (0x00-0xfa) 0.5Hz/bit
	BYTE	nHpbpu;		// 20, heat-Pipe Base Plate Temperature U phase,0-250C  (0x00-0xfa) 1.0c/bit
	BYTE	nHpbpv;		// 21, heat-Pipe Base Plate Temperature V phase,0-250C  (0x00-0xfa) 1.0c/bit
	BYTE	nHpbpw;		// 22, heat-Pipe Base Plate Temperature W phase,0-250C  (0x00-0xfa) 1.0c/bit
	BYTE	nSpr23[3];	// 23,24,25
} SASIV;

#define	TRACEBLENGTH_SIV	100

typedef struct _tagTASIV {
	WORD	wIndex;
	BYTE	nText[TRACEBLENGTH_SIV];
} TASIV;

typedef union _tagSIVATEXT {
	SASIV	st;
	TASIV	tr;
} SIVATEXT;

typedef struct _tagSIVAINFO {
	LWORD	wAddr;		// = 0x7303
	BYTE	nFlow;		//
	SIVATEXT	d;
} SIVAINFO, *PSIVAINFO;

#define	SIZE_SIVSA	(sizeof(SASIV) + 3)
#define	SIZE_SIVTA	(sizeof(TASIV) + 3)

typedef struct _tagSESIV {
	SASIV	st;
	double	power;
} SESIV;

typedef union _tagSIVETEXT {
	SESIV	st;
	TASIV	tr;
} SIVETEXT;

typedef struct _tagSIVEINFO {
	LWORD	wAddr;
	BYTE	nFlow;
	SIVETEXT	d;
} SIVEINFO, *PSIVEINFO;

// SIV sampling data struct
typedef struct _tagSIVSTEXT {
	WORD	wEs;		// 1-2
	WORD	wIs;		// 3-4
	WORD	wFcv;		// 5-6
	WORD	wEo;		// 7-8
	WORD	wIo;		// 9-10
	BYTE	nSpr11;		// 11
	PARCEL(	text12,		// 12
				bcov,
				spr6,
				spr5,
				dcov,
				fcov,
				aclv,
				acov,
				spr0 );
	PARCEL(	text13,		// 13
				ool,
				spr6,
				ooc,	// output over current
				thf,
				spr3,
				iaoc,
				spr1,
				lovd );
	PARCEL(	text14,		// 14
				spr7,
				isoc,
				stf,
				ivkf,
				sivkf,
				chkf,
				bcoc,
				osc );
	PARCEL(	text15,		// 15
				spr7, spr6, spr5, spr4,
				spr3, spr2, spr1,
				lockout );
	PARCEL(	text16,		// 16
				spr7, spr6, spr5, spr4,
				spr3,
				sivax,
				ivkax,
				chkax );
	BYTE	nSpr17[2];	// 17-18
} SIVSTEXT, *PSIVSTEXT;

#define	LENGTH_SAMPLINGSIV	201

typedef struct _tagSIVSTABLE {
	BYTE	nCode;		// 0
#define	FAULTCODE_ACOV		1
#define	FAULTCODE_ACLV		2
#define	FAULTCODE_FCOV		3
#define	FAULTCODE_DCOV		4
#define	FAULTCODE_BCOV		5
#define	FAULTCODE_LOVD		6
#define	FAULTCODE_IAOC		7
#define	FAULTCODE_THF		8
#define	FAULTCODE_OOC		9
#define	FAULTCODE_OOL		0x10
#define	FAULTCODE_OSC		0x20
#define	FAULTCODE_BCOC		0x30
#define	FAULTCODE_CHKF		0x40
#define	FAULTCODE_SIVKF		0x50
#define	FAULTCODE_IVKF		0x60
#define	FAULTCODE_STF		0x70
#define	FAULTCODE_ISOC		0x80

	BYTE	nYear;		// 1
	BYTE	nMonth;		// 2
	BYTE	nDay;		// 3
	BYTE	nHour;		// 4
	BYTE	nMinute;	// 5
	BYTE	nSpr6;		// 6
	BYTE	nSpr7;		// 7
	SIVSTEXT	sd[LENGTH_SAMPLINGSIV];
} SIVSTABLE, *PSIVSTABLE;	// 3626byte

#define	MAXBLOCK_SIV	363	// 3626 * 10 = 36260 -> 36300 byte(0x8da4)
#define	MAXTRACELENGTH_SIV	10

typedef struct _tagSIVCHASE {
	TRACEARCH	ta;
	BYTE	d[MAXBLOCK_SIV][TRACEBLENGTH_SIV];
} SIVCHASE, *PSIVCHASE;

#pragma pack()

#endif	/* SIVINFO_H_*/
