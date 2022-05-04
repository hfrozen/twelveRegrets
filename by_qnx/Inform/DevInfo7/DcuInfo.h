/*
 * DcuInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	DCUINFO_H_
#define	DCUINFO_H_

#pragma pack(1)

// SDR, CC->dcu
// const WORD	addr = 0x7309;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagDCURINFO {
	BYTE	nStx;
	LWORD	wAddr;		// = 0x7309
	BYTE	nFlow;		// 0
	BYTE	nDcua;		// 1
	BYTE	nSpr2;		// 2
	PARCEL(	test,		// 3
				spr7, spr6, spr5,
				aoac,	// door mode auto open/auto close
				spr3, spr2,
				tsr,	// Test Start Request
				tdc );	// TDC
	DEVTIME time;		// 4-9
	BYTE	nSpr10;		// 10
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} DCURINFO, *PDCURINFO;

typedef struct _tagDCUCINFO {	// TC->CC
	// Modified 2013/01/10
	// BYTE	nSpr2;		// 0
	BYTE	nMode;		// 0
	WORD	wTsr;		// 1-2
	WORD	wTdc;		// 3-4
	BYTE	nSpr10;		// 5
} DCUCINFO, *PDCUCINFO;

// sda, dcu->tcms
// const WORD	addr = 0x7309;
// BYTE	nFlow	= 0x30;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagDCUAINFO {
	BYTE	nStx;
	LWORD	wAddr;		// = 0x7309
	BYTE	nFlow;		// 0
	BYTE	nDcua;		// 1 DCU Address Code
	PARCEL(	test,		// 2
				dcus,	// DCU O.K (1:O.K)
				spr6, srp5, spr4, spr3, spr2,
				ts1, ts0 );	// Test
	PARCEL(	state,		// 3
				spr7,
				eeh,	// External emergency handle-
				eha,	// Emergency handle activeted-
				ods,	// Obstacle Detection Status-
				fap,	// Fault Present
				dfo,	// Door Fully Open
				diso,	// door by pass	// DOOR ISOLATION-
				dncs );	// Door Not Closed Status
	BYTE	nSpr4;		// 4 <-- new
	PARCEL(	fail,		// 5
				spr7, spr6,	spr5,
				cdcsf,	// Close-DCS Failure
				cdlsf,	// Close-DLS Failure
				dmcf,	// Door motor circuit failure
				dcuf2,	// DCU minor failure
				dcuf1 );	// DCU Major failure
	PARCEL(	err,		// 6
				doc,	// DOC (Door Open Command) Failure
				dcc,	// DCC (Door Close Command) Failure
				svl,	// Supply Volts Low	(V<70)
				svh,	// Supply Volts	High (V>110)
				dfoo,	// Door failed to open because obstructtion
				dfco,	// Door failed to close because obstructtion
				odcsf,	// Open-DCS Failure
				odlsf );	// Open-DLS Failure
	BYTE	nRomv;		// 7
	BYTE	nNtd;		// 8
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} DCUAINFO, *PDCUAINFO;

// tdr, tcms->DCU
// addr low - addr high - flow - text.... - crc low - crc high
// const WORD	addr = 0x7309;
// BYTE	nFlow	= 0x21;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagtagTRDCUINFO {
	BYTE	nCode;
	BYTE	nAddr;
	BYTE	nBlock;
} TRDCUINFO, *PTRDCUINFO;

// tda, ATC->tcms
// addr low - addr high - flow - text.... - crc low - crc high
// const WORD	addr = 0x7309;
// BYTE	nFlow	= 0x31;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagtagTADCUINFO {
	BYTE	nCode;
	BYTE	nAddr;
	BYTE	nBlock;
	BYTE	nText[79];
} TADCUINFO, *PTADCUINFO;

#pragma pack()

#endif	/* DCUINFO_H_ */
