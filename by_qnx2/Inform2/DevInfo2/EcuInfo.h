/**
 * @file	EcuInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_ECU		0xff10

#pragma pack(push, 1)

// sync, 38400, 50ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA, 0x21=TDR, 0x31=TDA
// SDR, tcms->ecu
typedef struct _tagSRECU {
	BYTE	cFlow;			// 0
	__BC(	cmd,			// 1
				b,		1,		// brake
				p,		1,		// power
				dc,		1,		// door close
				atom,	1,		// ato mode
				tset,	1,		// calender set
				btr,	1,		// brake test request
				astr,	1,		// anti-skid test request
				// 171221
				//sp,		1);
				cprs,	1);		// = TUDIB_CPRS
	//BYTE	cSpr2;			// 2
	__BC(	trc,			// 2, TCMS가 조작하는 명령, ECU는 spare부분이다.
				clr,	1,		// trace data clear
				spr1,	3,
				upl,	1,		// trace data upload
				spr5,	3);
	DEVTIME	devt;			// 3~8
} SRECU;

typedef union _tagECURTEXT {
	SRECU	s;
	BYTE	c[sizeof(SRECU)];
} ECURTEXT;

typedef struct _tagECURINFO {
	WORD	wAddr;			// 0xff10
	BYTE	cCtrl;			// 0x13
	ECURTEXT	t;
} ECURINFO, *PECURINFO;

typedef struct _tagECURINFOEX {
	WORD	wAddr;
	BYTE	cCtrl;
	ECURTEXT	t;
	__BC(	trc,			// 2, TCMS가 조작하는 명령, ECU는 spare부분이다.
				clr,	1,	// trace data clear
				spr1,	3,
				upl,	1,	// trace data upload
				spr5,	3);
	WORD	wTraceIndex;
} ECURINFOEX, *pECURIFOEX;

#define	SIZE_ECUSR		sizeof(ECURINFO)

// SD, ecu->tcms
typedef struct _tagSAECU {
	BYTE	cFlow;			// 0
	__BC(	ref,			// 1
				b,		1,
				p,		1,
				fsb,	1,
				hb,		1,
				eb,		1,
				sp,		2,
				slide,	1);
	BYTE	cBe;			// 2, brake effort, 0~100%(0~0xff)
	BYTE	cAsp;			// 3, 0~10bar(0~0xff), 1bar = 1.01972kg/cm^2
	BYTE	cLw;			// 4, 20~75t(0~0xff)
	BYTE	cBed;			// 5, demand, 0~120kN(0~0xff)
	BYTE	cBea;			// 6, attain, 0~120kN(0~0xff)
	BYTE	cEp;			// 7, 0~120kN(0~0xff)
	BYTE	cBcp;			// 8, 0~10bar(0~0xff), 1bar = 1.01972kg/cm^2
	BYTE	cSpr;			// 9
	__BC(	evt,			// 10
				min,	1,
				maj,	1,
				sp,		2,
				ast,	1,		// anti-skid testing
				astr,	1,		// anti-skid test result, 1(OK)/0(NG)
				bt,		1,		// brake testing
				btr,	1);		// brake test result, 1(OK)/0(NG)
	__BC(	fa,				// 11
				dvfr,	1,
				dvff,	1,
				emvf,	1,
				//lwsf,	1,		// 171114	lwf,	1,
				lwf,	1,
				//171221
				//rbedf,	1,
				//lwf,	1,		// 171114
				//sp,		1,
				//pbuf,	1);
				sp,		4);
	__BC(	fb,				// 12
				a4as,	1,
				a3as,	1,
				a2as,	1,
				a1as,	1,
				tpsf,	1,
				cpsf,	1,
				sp,		2);
	__BC(	fc,				// 13
				rbeaf,	1,
				tllf,	1,
				bcf,	1,
				ebrf,	1,
				sp,		4);
	__BC(	fd,				// 14
				alwf,	1,
				mb04f,	1,
				eb01f,	1,
				eb03f,	1,
				cb07f,	1,
				sp,		1,
				r1hf,	1,
				r2hf,	1);
	__BC(	fe,				// 15
				r5hf,	1,
				r6hf,	1,
				cmvf,	1,
				vmvf,	1,
				cvpsf,	1,
				rpsf,	1,
				cif,	1,
				lrpd,	1);
	//BYTE cSpr16[14];		// 16~29
	//BYTE cSpr16[13];		// 16~28
	// 171019
	//BYTE	cSpr16[9];		// 16~24
	BYTE	cSpr16[7];		// 16~22
	WORD	wBlindCnt;		// 23~24
	DBFTIME	dbft;			// 25~28
	__BC(	trc,			// 29, TCMS가 조작하는 부분, ECU에서는 spare부분이다.
				leng,	4,
				spr,	4);
} SAECU;

typedef union _tagECUATEXT {
	SAECU	s;
	BYTE	c[sizeof(SAECU)];
} ECUATEXT;

typedef struct _tagECUAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	ECUATEXT	t;
} ECUAINFO, *PECUAINFO;

#define	SIZE_ECUSA		sizeof(ECUAINFO)
//#define	SIZE_ECUSAO		(sizeof(SAECU) + 3)

typedef struct _tagECUFLAP {
	ECURINFO	r;
	ECUAINFO	a;
} ECUFLAP, *PECUFLAP;

typedef struct _tagECUFLAPEX {
	ECURINFOEX	r;
	ECUAINFO	a;
} ECUFLAPEX, *PECUFLAPEX;

// ===== 6 car
// 200902
//SDR6	tcms->ecu
typedef struct _tagSRECU2 {
	BYTE	cFlow;			// 0
	__BC(	cmd,			// 1
				b,		1,		// brake
				p,		1,		// power
				dc,		1,		// door close
				atom,	1,		// ato mode
				tset,	1,		// calender set
				btr,	1,		// brake test request
				astr,	1,		// anti-skid test request
				// 171221
				//sp,		1);
				cprs,	1);		// = TUDIB_CPRS
	__BC(	trc,			// 2, TCMS가 조작하는 명령, ECU는 spare부분이다.
				clr,	1,		// trace data clear
				fsb,	1,
				spr2,	2,
				upl,	1,		// trace data upload
				spr5,	3);
	DEVTIME	devt;			// 3~8
} SRECU2;

typedef union _tagECURTEXT2 {
	SRECU2	s;
	BYTE	c[sizeof(SRECU2)];
} ECURTEXT2;

typedef struct _tagECURINFO2 {
	WORD	wAddr;			// 0xff10
	BYTE	cCtrl;			// 0x13
	ECURTEXT2	t;
} ECURINFO2, *PECURINFO2;

// SD6	ecu->tcms
typedef struct _tagSAECU2 {
	BYTE	cFlow;			// 0
	__BC(	ref,			// 1
				b,		1,
				p,		1,
				fsb,	1,
				hb,		1,
				eb,		1,
				sp,		2,
				slide,	1);
	BYTE	cBe;			// 2, brake effort, 0~102%(0~0xff), 255 * 0.4 = 102
	BYTE	cAsp[4];		// 3~6, 0~765kPa(0~0xff), 255 * 3 = 765
	BYTE	cLw;			// 7, 20~96.5t(0~0xff). 255 * 0.3 + 20 = 96.5
	BYTE	cBed;			// 8, demand, 0~127.5kN(0~0xff), 255 * 0.5 = 127.5
	BYTE	cBea;			// 9, attain, 0~127.5kN(0~0xff), 255 * 0.5 = 127.5
	BYTE	cEp;			// 10, 0~127.5kN(0~0xff), 255 * 0.5 = 127.5
	BYTE	cBcp;			// 11, 0~1020kPa(0~0xff), 255 * 4 = 1020, 1bar = 1.01972kg/cm^2
	BYTE	cSpr;			// 12
	__BC(	evt,			// 13
				minf,	1,
				majf,	1,
				sp,		2,
				ast,	1,		// anti-skid testing
				astr,	1,		// anti-skid test result, 1(OK)/0(NG)
				bt,		1,		// brake testing
				btr,	1);		// brake test result, 1(OK)/0(NG)
	__BC(	fa,				// 14
				dvfr,	1,
				dvff,	1,
				emvf,	1,
				lwf,	1,
				sp,		4);
	__BC(	fb,				// 15
				a4as,	1,
				a3as,	1,
				a2as,	1,
				a1as,	1,
				ecuf,	1,
				bcpsf,	1,
				sp,		2);
	__BC(	fc,				// 16
				rbeaf,	1,
				tllf,	1,
				bcf,	1,
				ebrf,	1,
				sp,		4);
	__BC(	fd,				// 17
				alwf,	1,
				as4psf,	1,
				as3psf,	1,
				as2psf,	1,
				as1psf,	1,
				sp5,	3);
	__BC(	fe,				// 18
				r5hf,	1,
				r6hf,	1,
				sp2,	2,
				acpsf,	1,
				sp5,	3);
	BYTE	cSpr16[4];		// 19~22
	WORD	wBlindCnt;		// 23~24
	DBFTIME	dbft;			// 25~28
	__BC(	trc,			// 29, TCMS가 조작하는 부분, ECU에서는 spare부분이다.
				leng,	4,
				spr,	4);
} SAECU2;

typedef union _tagECUATEXT2 {
	SAECU2	s;
	BYTE	c[sizeof(SAECU2)];
} ECUATEXT2;

typedef struct _tagECUAINFO2 {
	WORD	wAddr;
	BYTE	cCtrl;
	ECUATEXT2	t;
} ECUAINFO2, *PECUAINFO2;

#define	SIZE_ECUSA2		sizeof(ECUAINFO2)

typedef struct _tagECUFLAP2 {
	ECURINFO2	r;
	ECUAINFO2	a;
} ECUFLAP2, *PECUFLAP2;

typedef struct _tagECUFLAPEX2 {
	ECURINFOEX	r;
	ECUAINFO2	a;
} ECUFLAPEX2, *PECUFLAPEX2;

typedef union _tagSAECUS {
	SAECU	co;				// -----
	SAECU2	ct;
} SAECUS, *PSAECUS;

typedef union _tagECUFLAPS {
	ECUFLAP		co;		// -----
	ECUFLAP2	ct;
} ECUFLAPS, *PECUFLAPS;

typedef union _tagECUFLAPEXS {
	ECUFLAPEX	co;			// -----
	ECUFLAPEX2	ct;
} ECUFLAPEXS, *PECUFLAPEXS;

#pragma pack(pop)

#define	SIZE_ECUTRACE		101
#define	SIZE_PREECUTRACE	50
#define	SIZE_SUFECUTRACE	(SIZE_ECUTRACE - SIZE_PREECUTRACE)
