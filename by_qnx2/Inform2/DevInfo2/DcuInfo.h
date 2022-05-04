/**
 * @file	DcuInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_DCUL		0x0170		//	0x0270, 0x0370, 0x0470
#define	LADD_DCUR		0x1170		//	0x1270, 0x1370, 0x1470

#pragma pack(push, 1)

// sync, 38400, 100ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA
// SDR, tcms->dcu
typedef struct _tagSRDCU {
	BYTE	cFlow;			// 0
	BYTE	cCid;			// 1
	__BC(	ref,			// 2
				tst,	1,	// test start request
				spr1,	3,
				dlt,	1,	// door lamp test
				dcw,	1,
				dow,	1,	// door open warning
				spr7,	1);
	DEVTIME	devt;			// 3~8
} SRDCU;

typedef union _tagDCURTEXT {
	SRDCU	s;
	BYTE	c[sizeof(SRDCU)];
} DCURTEXT;

typedef struct _tagDCURINFO {
	WORD	wAddr;			// 0x0170, 0x0270, 0x0370, 0x0470, 0x1170, 0x1270, 0x1370, 0x1470
	BYTE	cCtrl;			// 0x13
	DCURTEXT	t;
} DCURINFO, *PDCURINFO;

#define	SIZE_DCUSR		sizeof(DCURINFO)

// SD, dcu->tcms
typedef struct _tagSADCU {
	BYTE	cFlow;			// 0
	__BC(	ref,			// 1
				tst,	2,	// 01:testing, 11:test end
				rng,	1,	// test result NG
				rok,	1,	// test result OK
				sp,		3,
				dok,	1);	// dcu OK
	__BC(	stat,			// 2
				nc,		1,	// not closed
				isol,	1,	// isolation(cut) status, bypass switch
				fullo,	1,	// fully opened
				f,		1,	// fault
				od,		1,	// obstacle detect
				eed,	1,	// eed(internal) handle
				ead,	1,	// ead(external) handle
				odbps,	1);	// obstacle detect bypass switch
	__BC(	pf,				// 3
				majf,	1,
				minf,	1,
				mcf,	1,	// motor circuit
				dlsf,	1,
				dcs1f,	1,
				dcs2f,	1,
				dcu2f,	1,
				dcu2r,	1);
	__BC(	ff,				// 4, function failure
				unl,	1,	// unexpected unlocking
				lock,	1,
				obsc,	1,	// close fail by obstruction
				obso,	1,	// open
				sp,		4);
	BYTE	cVer;			// 5
} SADCU;

typedef union _tagDCUATEXT {
	SADCU	s;
	BYTE	c[sizeof(SADCU)];
} DCUATEXT;

typedef struct _tagDCUAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	DCUATEXT	t;
} DCUAINFO, *PDCUAINFO;

#define	SIZE_DCUSA		sizeof(DCUAINFO)
//#define	SIZE_DCUSAO		(sizeof(SADCU) + 3)

typedef struct _tagDCUFLAP {
	DCURINFO	r;
	DCUAINFO	a;
} DCUFLAP, *PDCUFLAP;

#pragma pack(pop)
