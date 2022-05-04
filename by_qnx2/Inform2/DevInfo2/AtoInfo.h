/**
 * @file	AtoInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#pragma pack(push, 1)

// aync, 19200, 500ms
// SDR, tcms->ato
typedef struct _tagSRATO {
	BYTE	cSeqCnt;		// 0, sequence counter, inc. every cycle
	BYTE	cChgCnt;		// 1, update counter
	__BC(	valid,			// 2,
				weight,	1,	// train weight valid
				sp,		6,
				slip,	1);	// slip/slide
	__BW(	proper,			// 3-4, proper no
				u32,	8,
				u10,	8);
				//u102,	4,
				//u103,	4,	// highest nibble is 103
				//u10l,	8);	// lowest nibble is 100
				//u100,	4,	// lowest nibble is 100
				//u101,	4);
	BYTE	cWeight;		// 5, 100~610t -> 0~255, 2t/1
	__BC(	fdev,			// 6
				siv,	1,
				v3f,	1,
				ecu,	1,
				trs,	1,
				roml,	1,	// reserve on main line
				// 170920
				bkup,	1,	// liu2 control, 		mrps,	1,
				psov,	1,	// passenger overload,	dcu,	1,
				inhibit,	1);	// ato inhibit,		dman,	1); ecuf*2 || vvvf*2 || ccf*2 || nrbd || sb || eb
	BYTE	cReserve1;		// 7
	__BC(	ts,				// 8
				sp,		4,
				leng,	4);	// train length
	BYTE	cReserve2[2];	// 9-10
} SRATO;

typedef union _tagATORTEXT {
	SRATO	s;
	BYTE	c[sizeof(SRATO)];
} ATORTEXT;

typedef struct _tagATORINFO {
	ATORTEXT	t;
	WORD		crc;
} ATORINFO, *PATORINFO;

#define	SIZE_ATOSR		sizeof(ATORINFO)

#define	ATO_WEIGHTOFFSET	100
// 180123
//#define	ATO_BLINDCODE		0x37

// SDA, ato->tcms
typedef struct _tagSAATO {
	BYTE	cSeqCnt;		// 0
	BYTE	cChgCnt;		// 1
	__BC(	valid,			// 2												(0x79)
				dsno,	1,	// destination station no.
				nsno,	1,	// next station no.
				ndo,	1,	// next door opening
				dcw,	1,	// door close warning
				psdc,	1,	// psd close
				effort,	1,	// tebe
				sp,		2);
	BYTE	cVer;			// 3												(0x04)
	BYTE	cNextSt;		// 4, next station code								(0x12)
	BYTE	cReserve1;		// 5, cCtrl로 사용한다.								(0x13)
	BYTE	cDestSt;		// 6, destination code								(0x0b)
	__BC(	door,			// 7												(0x02)
				ln,		1,	// right door on next
				rn,		1,	// left door on next
				dcw,	1,
				sp,		5);
	__BW(	notr,			// 8-9, number on the rail							(0x9903)
				u102,	4,	// lower nibble is 102
				u103,	4,	// higher nibble is 103
				u100,	4,	// lower nibble is 100
				u101,	4);	// higher nibble is 101
	__BC(	fail,			// 10												(0x00)
				mmilk,	1,	// mmi link
				atplk,	1,	// atp link
				ovsp,	1,	// out of stop position
				eb,		1,	// emergency braking
				sp,		4);
	__BC(	note,			// 11												(0x83)
				pdt,	1,	// pdt atc ok
				atom,	1,	// ato mode
				ldr,	1,	// left door release
				rdr,	1,	// right door release
				hb,		1,	// hold braking
				sp,		2,
				psdc,	1);	// all psd closed and locked
	BYTE	cWheelID;		// 12, wheel size index, 0(860)~16(780), 5mm/1bit	(0x01)
	BYTE	cEffort;		// 13, 0(0)~100%(200), 0.5%/1bit					(0x6d)
	BYTE	cEffType;		// 14, 0:coast, 1:brake, 2:traction, 0xff:no atc	(0x01)
	BYTE	cSpeed;			// 15, atc speed, 0~255Km/h							(0x31)
	__BC(	mode,			// 16, ato mode										(0x04)
				ar,		1,	// auto recurrence
				fa,		1,	// full auto
				aut,	1,	// auto
				manu,	1,	// manual
				yard,	1,
				sp,		3);
	BYTE	cReserve2;		// 17												(0x00)
} SAATO;

typedef union _tagATOATEXT {
	SAATO	s;
	BYTE	c[sizeof(SAATO)];
} ATOATEXT;

typedef struct _tagATOAINFO {
	ATOATEXT	t;
	WORD		crc;
} ATOAINFO, *PATOAINFO;

#define	SIZE_ATOSA		sizeof(ATOAINFO)

typedef struct _tagATOFLAP {
	ATORINFO	r;
	ATOAINFO	a;
} ATOFLAP, *PATOFLAP;

#pragma pack(pop)
