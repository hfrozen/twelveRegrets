/**
 * @file	PaInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"
#include "AisInfo.h"

#define	LADD_PAU		0x0160

#pragma pack(push, 1)

// sync, 38400, 300ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA
// SDR, tcms->pau
typedef struct _tagSRPAU {
	SRAIS	ais;			// 0~20
	BYTE	cSpr[2];		// 21-22
} SRPAU;

typedef union _tagPAURTEXT {
	SRPAU	s;
	BYTE	c[sizeof(SRPAU)];
} PAURTEXT;

typedef struct _tagPAURINFO {
	WORD	wAddr;			// 0x0160
	BYTE	cCtrl;			// 0x13
	PAURTEXT	t;
} PAURINFO, *PPAURINFO;

#define	SIZE_PAUSR		sizeof(PAURINFO)

// SDA, pau->tcms
typedef struct _tagSAPAU {
	BYTE	cFlow;			// 0
	__BC(	alarm,			// 1
				spau,	1,
				scob,	1,
				ssobr,	1,
				ssobl,	1,
				mpau,	1,
				mcob,	1,
				msobr,	1,
				msobl,	1);
	__BW(	ampf,			// 2-3
				c7,		1,
				c6,		1,
				c5,		1,
				c4,		1,
				c3,		1,
				c2,		1,
				c1,		1,
				c0,		1,
				c9,		1,
				c8,		1,
				sp,		6);
	__BC(	fail,			// 4
				pa,		1,
				bkup,	1,
				update,	1,
				sp,		5);
	BYTE	cVer;			// 5
	BYTE	cSpr;			// 6
} SAPAU;

typedef union _tagPAUATEXT {
	SAPAU	s;
	BYTE	c[sizeof(SAPAU)];
} PAUATEXT;

typedef struct _tagPAUAINFO {
	WORD	wAddr;
	BYTE	cCtrl;			// 0x13
	PAUATEXT	t;
} PAUAINFO, *PPAUAINFO;

#define	SIZE_PAUSA	sizeof(PAUAINFO)

typedef struct _tagPAUFLAP {
	PAURINFO	r;
	PAUAINFO	a;
} PAUFLAP, *PPAUFLAP;

#pragma pack(pop)
