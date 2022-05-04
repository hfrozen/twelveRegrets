/**
 * @file	BmsInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2020-02-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_BMS		0xff30

#pragma pack(push, 1)

typedef struct _tagSRBMS {
	BYTE	cFlow;			// 0
	DEVTIME	devt;			// 1~6
	WORD	wSeq;			// 7-8
	__BW(	no,				// 9-10, train no
				u102,	4,
				u103,	4,
				u100,	4,
				u101,	4);
	BYTE	nSpr[2];		// 11-12
} SRBMS;

typedef union _tagBMSRTEXT {
	SRBMS	s;
	BYTE	c[sizeof(SRBMS)];
} BMSRTEXT;

typedef struct _tagBMSRINFO {
	WORD	wAddr;
	BYTE	cCtrl;			// 0x13
	BMSRTEXT	t;
} BMSRINFO, *PBMSRINFO;

#define	SIZE_BMSSR		sizeof(BMSRINFO)

// SD, bms->tcms
typedef struct _tagSABMSAB {
	BYTE	cFlow;			// 0
	WORD	wSeq;			// 1-2
	BYTE	cCurV;			// 3
	WORD	wCurI;			// 4-5, little
	WORD	wVer;			// 6-7
	BYTE	cChgRate;		// 8, 0~0x64 -> 0~100%
	BYTE	nSpr9;			// 9
	__BC(	stat,			// 10
				chgc,	1,	// charge contactor,	0:open / 1:close,	status
				disc,	1,	// discharge contactor,	0:open / 1:close,	status
				chgg,	1,	// charging,			0:off / 1:on,		status
				disg,	1,	// discharging,			0:off / 1:on,		status
				chgcf,	1,	// charge contactor fault,			light
				discf,	1,	// discharge contactor fault,		light
				pdcf,	1,	// predischarge contactor fault,	light
				clcf,	1);	// charge low contactor fault,		light
	BYTE	nSpr11[2];		// 11-12
	__BC(	fc,				// 13
				cov,	1,	// cell over voltage,				severe
				cuv,	1,	// cell under voltage,				severe
				pov,	1,	// battery pack over voltage,		light
				puv,	1,	// battery pack under voltage,		light
				occ,	1,	// over current at charging,		severe
				ocd,	1,	// over current at discharging,		severe
				ci,		1,	// cell imbalance,					severe
				scf,	1);	// sbms communication fault,		light
	__BC(	fe,				// 14
				lvf,	1,	// low voltage fault,				light
				pot,	1,	// pack over temp.,					light
				put,	1,	// pack under temp.,				light
				pdc,	1,	// predischarge contactor,	0:open / 1:close,	status
				lcc,	1,	// low charge contactor,	0:open / 1:close,	status
				tsf,	1,	// temp. sensor fault,				light
				cof,	1,	// current offset fault,			light
				shf,	1);	// output short,					light
	BYTE	cMod1Temp[2];	// 15-16, temp[2]. at module1, 0~211 => -60กษ ~ 151กษ
	BYTE	cMod2Temp[2];	// 17-18, temp[2]. at module2, 0~211 => -60กษ ~ 151กษ
	BYTE	cSpr19[2];		// 19-20
} SABMSAB;

typedef union _tagBMSATEXTAB {
	SABMSAB	s;
	BYTE	c[sizeof(SABMSAB)];
} BMSATEXTAB;

typedef struct _tagBMSAINFOAB {
	WORD	wAddr;
	BYTE	cCtrl;
	BMSATEXTAB	t;
} BMSAINFOAB, *PBMSAINFOAB;

#define	SIZE_BMSSAAB		sizeof(BMSAINFOAB)

typedef struct _tagBMSFLAPAB {
	BMSRINFO	r;
	BMSAINFOAB	a;
} BMSFLAPAB, *PBMSFLAPAB;

//==================================
typedef struct _tagBMSCVS {
	//BYTE	cMod1CV[25];	// 21-45, cell voltage at module1, 20mV/bit, 5.1V max
	//BYTE	cMod2CV[25];	// 46-70
	//BYTE	cCVS[2][25];
	BYTE	cCVS[50];
} BMSCVS, *PBMSCVS;

typedef struct _tagSABMS {
	SABMSAB	s;
	BMSCVS	v;
	BYTE	cSpr71[4];		// 71-74
} SABMS;

typedef union _tagBMSATEXTEX {
	SABMS	se;
	BYTE	c[sizeof(SABMS)];
} BMSATEXT;

typedef struct _tagBMSAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	BMSATEXT	t;
} BMSAINFO, *PBMSAINFO;

#define	SIZE_BMSSA	sizeof(BMSAINFO)

typedef struct _tagBMSFLAP {
	BMSRINFO	r;
	BMSAINFO	a;
} BMSFLAP, *PBMSFLAP;

#pragma pack(pop)
