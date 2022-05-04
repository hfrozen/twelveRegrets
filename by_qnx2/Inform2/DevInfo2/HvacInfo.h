/**
 * @file	HvacInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_HVAC		0xff70

#pragma pack(push, 1)

// sync, 38400, 200ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA, 0x21=TDR, 0x31=TDA
// SDR, tcms->hvac
typedef struct _tagSRHVAC {
	BYTE	cFlow;			// 0
	__BC(	req,			// 1
				fire,	1,
				// 201028
				//ground,	1,		// at 200
				apdk1,	1,		// at 24
				full,	1,
				head,	1,		// head car
				lrr,	1,		// load reduction request
				lor,	1,		// load off request
				esk,	1,
				dow,	1);		// door open warning
	__BC(	ctrl,			// 2
				cmd,	4,		// refer. to pio.h
				ef,		1,		// exhaust fan manual on
				// 201028
				//apdk,	1,		// apdk manual on
				apdk2,	1,
				elff,	1,		// end line flow fan on
				lff,	1);		// line flow fan on
	__BC(	tst,			// 3
				item,	4,		// 1:heat test, 2: cool test // 1:PDT,	2:daily,	3:monthly
				// 201028
				//spr,	2,
				cl0,	1,
				cl1,	1,
				tsr,	1,		// test start request
				tsp,	1);		// test stop request, 2초간 유지.
	__BC(	ref,			// 4
				cno,	4,		// car no
				spr,	1,
				stv,	1,		// 170726, 별로 의미가 없다. 항상 1로 유지한다.	// set temp. valid
				vnt1,	1,		// ventialtion 1
				vnt2,	1);
	BYTE	cOutDoorTemp;	// 5, 실외 온도 값(-128 ~ +127, 유효 범위 -30 ~ +50)
	BYTE	cInDoorTemp;	// 6, 실내 설정 온도 값
	BYTE	cNextSt;		// 7
	BYTE	cDestSt;		// 8
	BYTE	cSpr9;			// 9
	DEVTIME	devt;			// 10~15
} SRHVAC;

typedef union _tagHVACRTEXT {
	SRHVAC	s;
	BYTE	c[sizeof(SRHVAC)];
} HVACRTEXT;

typedef struct _tagHVACRINFO {
	WORD	wAddr;			// 0xff70
	BYTE	cCtrl;			// 0x13
	HVACRTEXT	t;
} HVACRINFO, *PHVACRINFO;

#define	SIZE_HVACSR		sizeof(HVACRINFO)

// SD, hvac->tcms
typedef struct _tagSAHVAC {
	BYTE	cFlow;			// 0
	__BC(	cam,			// 1
				maint,	1,
				stop,	1,
				aut,	1,
				manu,	1,
				spr,	4);
	__BC(	test,			// 2
				item,	4,
				seq,	2,	// 0:init,	1:run,	2:end,	3:
				ng,		1,
				ok,		1);
	__BC(	sta,			// 3
				u1hc,	1,
				u2v,	1,
				u1v,	1,
				aut,	1,
				off,	1,
				h3,		1,
				h2,		1,
				h1,		1);
	__BC(	stb,			// 4
				u2tok,	1,
				u1tok,	1,
				spr,	2,
				lr,		1,
				u2fc,	1,
				u1fc,	1,
				u2hc,	1);
	__BC(	stc,			// 5
				apdk,	1,
				lffk2,	1,
				lffk1,	1,
				vf,		1,
				d380,	1,
				opm,	1,
				htok,	1,
				spr,	1);
	__BC(	fa,				// 6
				u1d1hp,	1,
				u1d1lp,	1,
				u2eol,	1,
				u1eol,	1,
				u2c2ol,	1,
				u2c1ol,	1,
				u1c2ol,	1,
				u1c1ol,	1);
	__BC(	fb,				// 7
				vfk,	1,
				sp1,	1,
				u2d2hp,	1,
				u2d2lp,	1,
				u2d1hp,	1,
				u2d1lp,	1,
				u1d2hp,	1,
				u1d2lp,	1);
	__BC(	fc,				// 8
				cmk3,	1,
				efk1,	1,
				cmk2,	1,
				cmk1,	1,
				ts4,	1,
				ts3,	1,
				ts2,	1,
				ts1,	1);
	__BC(	fd,				// 9
				sp,		2,
				lffk2,	1,
				lffk1,	1,
				rhek2,	1,
				rhek1,	1,
				efk2,	1,
				cmk4,	1);
	__BC(	fe,				// 10
				p24,	1,
				vdp,	1,
				spr,	2,
				co2m2,	1,
				co2m1,	1,
				apdk,	1,
				// 171114
				//spr1,	1);
				apdkf,	1);	// 171114
	__BC(	ff,				// 11
				vf1do,	1,
				vf1dc,	1,
				vf2do,	1,
				vf2dc,	1,
				spr,	3,
				hvac,	1);
	BYTE	cTemp;			// 12
	BYTE	cHumi;			// 13
	BYTE	cCO2[2];		// 14-15
	BYTE	cSpr16[2];		// 16-17
	BYTE	cVer;			// 18
} SAHVAC;

typedef union _tagHVACATEXT {
	SAHVAC	s;
	BYTE	c[sizeof(SAHVAC)];
} HVACATEXT;

typedef struct _tagHVACAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	HVACATEXT	t;
} HVACAINFO, *PHVACAINFO;

#define	SIZE_HVACSA		sizeof(HVACAINFO)
//#define	SIZE_HVACSAO	(sizeof(SAHVAC) + 3)

typedef struct _tagHVACFLAP {
	HVACRINFO	r;
	HVACAINFO	a;
} HVACFLAP, *PHVACFLAP;

#pragma pack(pop)
