/**
 * @file	TrsInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_TRS		0x0460

#pragma pack(push, 1)

// sync, 38400, 300ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA
// SDR, tcms->trs
typedef struct _tagSRTRS {
	BYTE	cFlow;			// 0
	BYTE	cCode;			// 1, Trs code
	__BW(	no,				// 2-3, train no
				u102,	4,
				u103,	4,
				u100,	4,
				u101,	4);
	BYTE	cSpr[2];		// 4-5
} SRTRS;

typedef union _tagTRSRTEXT {
	SRTRS	s;
	BYTE	c[sizeof(SRTRS)];
} TRSRTEXT;

typedef struct _tagTRSRINFO {
	WORD	wAddr;			// 0x0460
	BYTE	cCtrl;			// 0x13
	TRSRTEXT	t;
} TRSRINFO, *PTRSRINFO;

#define	SIZE_TRSSR	sizeof(TRSRINFO)

// SDA, trs->tcms
typedef struct _tagSATRS {
	BYTE	cFlow;			// 0
	__BW(	no,				// 1-2, train no
				u102,	4,
				u103,	4,
				u100,	4,
				u101,	4);
	__BC(	fail,			// 3
				rosc,	1,	// receive osc.
				rpwr,	1,	// receive pwr.
				tosc,	1,	// trans. osc.
				tpwr,	1,	// trans. pwr.
				chm,	1,
				chy,	1,
				chc,	1,
				che,	1);
	BYTE	cChgCode;		// 4
	BYTE	cVer;			// 5
	BYTE	cSpr;			// 6
} SATRS;

typedef union _tagTRSATEXT {
	SATRS	s;
	BYTE	c[sizeof(SATRS)];
} TRSATEXT;

typedef struct _tagTRSAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	TRSATEXT	t;
} TRSAINFO, *PTRSAINFO;

#define	SIZE_TRSSA	sizeof(TRSAINFO)

typedef struct _tagTRSFLAP {
	TRSRINFO	r;
	TRSAINFO	a;
} TRSFLAP, *PTRSFLAP;

#pragma pack(pop)
