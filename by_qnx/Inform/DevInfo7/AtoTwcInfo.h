/*
 * AtoTwcInfo.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	ATOTWCINFO_H_
#define	ATOTWCINFO_H_

#pragma pack(1)

// BYTE	nFlow -> 0x20=SDR/SD, 0x30=PDTR/PDTD 0x21=TDR/TD
// SDR, tcms->ato
typedef struct _tagSRATO {
	WORD	wTnc;		// 1-2, train no. indication (0-9999) bcd, big-endian
	BYTE	nFno;		// 3, formation no. (0-255), at tcms panel
	BYTE	nTrs0;		// 4
	PARCEL(	fail,		// 5
				ovl,	// passenger overload, 1: psg load ar all car >= 200%
				sp6,
				atc1f,	// atc1 fault
				sivc,	// siv cut, 1: one or more siv cut
				brf1,	// brake 2 fault
				brf0,	// brake 1 fault, 00:normal,
				pwrf1,	// power 2 fault
				pwrf2 );	// power 1 fault, 00:normal
	BYTE	nTrs1;		// 6
	BYTE	nTrs2;		// 7
	PARCEL(	set,		// 8
				tl3, tl2, tl1, tl0,	// train length
				actato,	// activate ato
				caba,	// head-cab active
				atoinh,	// run inhibit at auto, drvl
				tdcl );	// trace data clear
	PARCEL(	mode,		// 9
				aamd,	// atc/ato mode driverless
				aama,	// atc/ato mode automatic
				aamm,	// atc/ato mode manual
				aamy,	// atc/ato mode yard
				aame,	// atc/ato mode emergency
				adc,	// all door close
				dak,	// driverless ack., 1:at run from 1st. station on drvl
				trnih );	// transmission inhibit
#define	ATOMODE_ONLY	0xf8

	_TV_	vTbeB;		// 10-11, actual power/brake commmand, +_10000, big-endian
	_TV_	vAsp;		// 12-13, TCMS actual speed
	_TV_	vBpa;		// 14-15, braking percentage
	_TV_	vPpa;		// 16-17, powering percentage
	DEVTIME	time;		// 18-23
	WORD	wSpr;		// 24-25
	WORD	wChk;		// 26-27
} SRATO;

typedef struct _tagTRATO {
	BYTE	nIndex;
} TRATO;

typedef struct _tagIRATO {
	PARCEL( cmd,		// 1
				sp7,
				req,	// test excution request
				sp5, sp4,
				sp3, sp2, sp1, sp0 );
	BYTE	nItem;		// 2, item no.(BCD)
	BYTE	nSpr3[2];	// 3-4,
	WORD	wChk;		// 5-6
} IRATO;

typedef union _tagATORTEXT {
	SRATO	sr;
	TRATO	tc;
	IRATO	it;
} ATORTEXT;

typedef struct _tagATORINFO {
	LWORD	wAddr;		// = 0xff40
	BYTE	nCtrl;
	BYTE	nFlow;
	ATORTEXT	r;
} ATORINFO, *PATORINFO;

#define	SIZE_ATOSR		(sizeof(SRATO) + 4)
#define	SIZE_ATOTR		(sizeof(TRATO) + 4)
#define	SIZE_ATOIR		(sizeof(IRATO) + 4)

// sda, ato->tcms
typedef struct _tagSAATO {
	BYTE	nPsn;		// 1, present station no.(0-99 binary)
	BYTE	nPstn;		// 2, present station twc no.(0-99 binary)
	BYTE	nNsn;		// 3, next station no.(0-99 binary), to PA
	BYTE	nNstn;		// 4, next station twc no.(0-99 binary), to PA
	WORD	wTtn;		// 5-6, twc train number binary, big-endian, to PA
	BYTE	nDsn;		// 7, destination station no., to PA
	PARCEL(	info,		// 8
				tcd,	// twc carrier detect
				dar,	// driver ack request, to monit
				dcw,	// door closed warning, to PA
				ddns,	// door direction next station, to PA
				td3, td2, td1, td0 );	// trace data
	PARCEL(	arm,		// 9
				itm,	// invalid twc message
				aaso,	// atc1 atc2 switch over
				tcf,	// twc carrier fault
				stsr,	// spurious twc system reset
				hf,		// ato/twc hardware fault
				utd,	// unexpected twc destination
				atcf2,	// atc2 communication fault
				atcf1 );	// atc1 communication fault
	PARCEL(	state,		// 10
				atc1c,	// atc1 control
				atc2c,	// atc2 control
				stb,	// send train berth, 1:last stop pos.
				idft,	// inhibit driving from tcms
				nm,		// normal mode
				rm,		// recovery mode
				ator,	// ato ready
				ccf );	// communication check fault
	PARCEL(	fail,		// 11
				itaf,	// invalid twc address fault
				spc1f,	// speed probe ch.1 fault
				pssmf,	// programmed station stop marker fault
				scef,	// ato tcms speed correspond ence fault
				spr3,	// automatic turn back fault
				tdbf,	// track data base fault
				orf,	// over run fault
				srf );	// short run fault
	PARCEL( psm,		// 12
				anmto,	// ato no motion timeout
				spc2f,	// speed probe ch.2 fault
				f1,
				f2,
				f3,
				f4,
				f5,
				f6 );
	PARCEL( pss,		// 13
				sp7, sp6, sp5, sp4,
				ipm,	// invalid pss marker
				upm,	// unexpected pss marker
				utd,	// unexpected twc destination
				sp0 );
	_TV_	vTbeB;		// 14-15, big-endian
	_TV_	vPwbB;		// 16-17, powering when braking
	BYTE	nFsc;		// 18, fixed speed code (0-255) binary
	WORD	wSpr;		// 19-20
	WORD	wChk;		// 21-22
} SAATO;

#define	TRACEBLENGTH_ATO	100	//140

typedef struct _tagTAATO {
	BYTE	nIndex;
	BYTE	nText[TRACEBLENGTH_ATO];
} TAATO;

typedef struct _tagIAATO {
	BYTE	nState;		// 1, 0:Initial, 1:Excuting, 2:End
#define	ATOINSPSTATE_INIT		0
#define	ATOINSPSTATE_EXCUTING	1
#define	ATOINSPSTATE_END		2

	PARCEL( nstsf,		// 2
				sp7,
				twc,	// no wayside twc carrier not satisfied
				motion,	// no motion not satisfied
				cmd,	// no wayside command not satisfied,
				manu,	// atc manual position not satisfied,
				fsb,	// master controller FSB position not satisfied
				cond,	// condition not fulfilled
				sp0 );
	BYTE	nItem;		// 3, Excuting Item
	PARCEL( res,		// 4
				sp7, sp6, sp5, sp4,
				sp3, sp2,
				ok,
				ng );
	BYTE	nSubItem;	// 5
	BYTE	nSpr6[2];	// 6-7
	WORD	wChk;
} IAATO;

typedef union _tagATOATEXT {
	SAATO	st;
	TAATO	tr;
	IAATO	ip;
} ATOATEXT;

typedef struct _tagATOAINFO {
	LWORD	wAddr;		// = 0xff40
	BYTE	nCtrl;
	BYTE	nFlow;
	ATOATEXT	d;
} ATOAINFO, *PATOAINFO;

#define	SIZE_ATOSA		(sizeof(SAATO) + 4)
#define	SIZE_ATOTA		(sizeof(TAATO) + 4)
#define	SIZE_ATOIA		(sizeof(IAATO) + 4)

typedef struct _tagATOSTEXT {
	BYTE	n[28];
} ATOSTEXT;

#define	LENGTH_SAMPLINGATO	30

typedef struct _tagATOSTABLE {
	ATOSTEXT	sd[LENGTH_SAMPLINGATO];
} ATOSTABLE, *PATOSTABLE;	// 840byte

#define	MAXBLOCK_ATO	68	// 840 * 8 = 6720byte
#define	MAXTRACELENGTH_ATO	8

typedef struct _tagATOCHASE {
	TRACEARCH	ta;
	BYTE		d[MAXBLOCK_ATO][TRACEBLENGTH_ATO];
} ATOCHASE, *PATOCHASE;

#pragma pack()

#endif	/* ATOINFO_H_ */
