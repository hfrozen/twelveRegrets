/*
 * InterInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	INTERINFO_H_
#define	INTERINFO_H_

#pragma pack(1)

// ***** ALL *****
// SDR, tcms->htc, pisc, trs, rf, csc
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagCMMRINFO {
	BYTE	nStx;
	LWORD	wAddr;		//
	BYTE	nFlow;		// 0
	BYTE	nSpr1;		// 1
	BYTE	nCnn;		// 2, CAB no. 0x31-0x34
	BYTE	nPrst;		// 3, the present station code(0x00 - 0xFF)
	HALFB (	nsc, ta, ln );	// 4, next station code
	BYTE	nNext;		// 5, next station code
	BYTE	nDest;		// 6, destination station code
	WORD	wTno;		// 7-8, bcd, big-endian
	PARCEL(	info,		// 9
				spr7,
				nsddl,	// next station door direction left
				nsddr,	// next station door direction right
				spr4, spr3, spr2,
				rts,	// ozonizer run
				dcw );	// door close warning
	PARCEL(	inst,		// 10
				spr7,
				domaa,	// auto open / auto close
				domam,	// auto open / manual close
				dommm,	// manual open / manual close
				dme,	// emergency mode
				dmr,	// driverless mode
				dmm,	// manual mode
				dma );	// auto mode
	PARCEL(	cmd,		// 11
				zvr,	// zero velocity relay
				hcr,	// head car relay
				siv0,	// tc0 sivk input is high
				siv1,	// tc1 sivk input is high
				emc,	// emergency call
				frd,	// fire detection
				dol,	// door open left
				dor );	// door open right
	PARCEL(	req,		// 12
				esk,	// esk on
				hvts,	// cool test start
				htts,	// heat test start
				lrr,	// load reduction
				lpo,	// lamp on
				siva,	// siv active
				rvs,	// drive stick position is neutral
				fwd );	//
	BYTE	temp[16];	// 13-28, room avrerage temperature
	DEVTIME	time;		// 29-34
	BYTE	nSpr2;		// 35
	BYTE	nPW[8];		// 36-43, passenger weight
	BYTE	nEm;		// 44, emergency interphone call from x
	BYTE	nFr;		// 45, fire from x
	WORD	wStsd;		// 46-47, distance between stations
	BYTE	nAtcSpeed;	// 48
	BYTE	nSivV;		// 49
	BYTE	nSivI;		// 50
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} CMMRINFO, *PCMMRINFO;

// ***** PISC *****
// SDA, pisc->tcms
// const WORD	addr = 0x7306;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagPISAINFO {
	BYTE	nStx;
	LWORD	wAddr;		//
	BYTE	nFlow;		// 0
	BYTE	nSpr1;		// 1
	BYTE	nActive;	// 2, Active 1-0x31 2-0x32. Backup 1-0x33 2-0x34
	BYTE	nCsc;		// 3, the current station code(0x00 - 0xFF)
	HALFB (	nsc, ln, ta );	// 4, next station code
	BYTE	nNsc;		// 5, next station code
	BYTE	nDsc;		// 6, destination station code
	PARCEL(	state,		// 7
				srf0,
				srf1,
				slf0,
				slf1,
				cf0,
				cf1,
				paf0,
				paf1 );
	PARCEL(	pa,			// 8
				f0,
				f7,
				f6,
				f5,
				f4,
				f3,
				f2,
				f1 );
	PARCEL( emp1,		// 9
				f0,
				f7,
				f6,
				f5,
				f4,
				f3,
				f2,
				f1 );
	PARCEL(	emp2,		// 10
				f0,
				f7,
				f6,
				f5,
				f4,
				f3,
				f2,
				f1 );
	BYTE	nSpr11;		// 11
	PARCEL(	fdd,		// 12
				spr7, spr6, spr5, spr4,
				spr3, spr2,
				f2,
				f1 );
	BYTE	nSpr13[8];	// 13-14-15-16:17-18-19-20
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} PISAINFO, *PPISAINFO;

// ***** HTC *****
// SDA, htc->tcms
// const WORD	addr = 0x7306;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagHTCAINFO {
	BYTE	nStx;
	LWORD	wAddr;		//
	BYTE	nFlow;		// 0
	BYTE	nSpr1;		// 1
	BYTE	nSpr2;		// 2
	BYTE	nCsc;		// 3, the current station code(0x00 - 0xFF)
	HALFB (	nsc, ln, ta );	// 4 next station code (LineNum/TWC Address)
	BYTE	nNsc;		// 5, next station code
	BYTE	nDsc;		// 6, destination station code
	PARCEL(	info,		// 7
				spr7, spr6, spr5, spr4,
				rst,	// reseting
				eskr,	// esk ready
				hvcf2,	// hvc2 fault
				hvcf1 );	// hvc1 fault
	PARCEL(	fail,		// 8
				hcc0,
				hcc7,
				hcc6,
				hcc5,
				hcc4,
				hcc3,
				hcc2,
				hcc1 );	//
	BYTE	temp[16];	// 9-24
	PARCEL(	test,		// 25
				end,
				exc,
				spr5, spr4,
				tst4, tst3, tst2, tst1 );
	BYTE	nHeatArm[3];	// 26-28, HeatAlarm 0..2
	BYTE	nSenArm;	// 29
	BYTE	nCoolArm[2]; // 30-31, CoolAlarm 0..1
	BYTE	nFanArm[2];	// 32-33, Fan Alarm 0..1
	BYTE	nExhaustArm; // 34
	BYTE	nEdamArm;	// 35
	BYTE	nFdamArm;	// 36
	BYTE	nSpr27[4];	// 37-38-39-40
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} HTCAINFO, *PHTCAINFO;

// ***** TRS *****
// SDA, trs->tcms
// const WORD	addr = 0x7306;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagTRSAINFO {
	BYTE	nStx;
	LWORD	wAddr;		//
	BYTE	nFlow;		// 0
	BYTE	nSpr1;		// 1
	WORD	wTno;		// 2-3, bcd, big-endian
	PARCEL(	info,		// 4
				spr7, spr6, spr5, spr4,
				atnf,	// wireless antena fault	// tpf,	// wireless tx power fault
				rapf,	// wireless radio part		// tof,	// wireless tx osc. fault
				tcrpf,	// tcrp controller fault,	// risf,	// wireless rx power fault
				tcif );	// tci controller fault	// rof );	// wireless rx osc. fault
	BYTE	nSpr5[2];	// 5-6
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} TRSAINFO, *PTRSAINFO;

// ***** PSD-RF *****
// SDA, psd_rf -> tcms
// const WORD	addr = 0x7307;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagPSDAINFO {
	BYTE	nStx;
	LWORD	wAddr;		//
	BYTE	nFlow;
	BYTE	nSpr1;		// 1
	BYTE	nSpr2;		// 2
	BYTE	nCsc;		// 3, the current station code(0x00 - 0xFF)
	HALFB (	nsc, ln, ta );	// 4, next station code
	BYTE	nNsc;		// 5, next station code
	BYTE	nDsc;		// 6, destination station code
	PARCEL(	info,		// 7
				spr7, spr6, spr5, spr4,
				psdf,	// psd fault
				psdc,	// psd all doors close
				psdo,	// psd all doors open
				wpsdf );	// wayside psd control fault
	BYTE	nSpr8[13];		// 8-20
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} PSDAINFO, *PPSDAINFO;

// ***** CSC *****
// SDA, csc->tcms
// const WORD	addr = 0x7308;
// BYTE	nFlow = 0x20;	// 0x20=SDR, 0x21=TDR, 0x30=SD, 0x31=TD
typedef struct _tagCSCAINFO {
	BYTE	nStx;
	LWORD	wAddr;		//
	BYTE	nFlow;		// 0
	BYTE	nSpr1[2];		// 1-2
	BYTE	nPsc;		// 3, the present station code(0x00 - 0xFF)
	HALFB (	nsc, ln, ta );	// 4, next station code
	BYTE	nNsc;		// 5, next station code
	BYTE	nDsc;		// 6, destination station code
	BYTE	nSpr2;		// 7
	WORD	wTno;		// 8-9, bcd, big-endian
	BYTE	nSpr3[10];	// 10-19
	BYTE	nEtx;
	BYTE	nBcc1;
	BYTE	nBcc2;
} CSCAINFO, *PCSCAINFO;

#pragma pack()

#endif	/* INTERINFO_H_ */
