/**
 * @file	V3fInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_V3F		0xff20

#pragma pack(push, 1)

// sync, 38400, 50ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA, 0x21=TDR, 0x31=TDA
// SDR, tcms->v3f
typedef struct _tagSRV3F {
	BYTE	cFlow;			// 0
	DEVTIME	devt;			// 1~6
	__BC(	cmd,			// 7
				r,		1,
				f,		1,
				y,		1,
				fms,	1,
				tsr,	1,	// unit test start request
				sp5,	1,
				b,		1,	// 180103	//sp6,	1,	// tex,	1,	// test excute request ???, 안쓴다.
				p,		1);	// 1810103	//sp7,	1);
	BYTE	sp8;			// 8
	__BC(	cst,			// 9
				sb,		1,
				eb,		1,
				atom,	1,
				boost,	1,
				sp,		4);
	__BC(	ref,			// 10
				cn,		4,
				vwc,	1,	// valid wheel code
				sp,		2,
				tdc,	1);	// trace data clear request
	BYTE	cSpr11[6];		// 11~16
	WORD	wWheel;			// 17~18, 7800~8600		0.1mm/bit hi-lo
	BYTE	cSpr19[2];		// 19~20
} SRV3F;

// TDR
typedef struct _tagTRV3F {
	BYTE	cFlow;			// 0
	BYTE	cChapID;		// 1, data id, ~ trc.b.leng
	WORD	wPageID;		// 2~3, 100바이트씩 끊은...
	BYTE	cSpr;
} TRV3F;

typedef union _tagV3FRTEXT {
	SRV3F	s;
	BYTE	c[sizeof(SRV3F)];
	TRV3F	t;
} V3FRTEXT;

typedef struct _tagV3FRINFO {
	WORD	wAddr;			// 0xff20
	BYTE	cCtrl;			// 0x13
	V3FRTEXT	t;
} V3FRINFO, *PV3FRINFO;

#define	SIZE_V3FSR		sizeof(V3FRINFO)

// SD, v3f->tcms
typedef struct _tagSAV3F {
	BYTE	cFlow;			// 0
	__BC(	ref,			// 1
				sqs,	1,
				gate,	1,
				regp,	1,
				pwrp,	1,
				ccos,	1,
				//rstr,	1,	// 171128
				spr,	1,
				act,	1,
				stok,	1);
	__BC(	rsp,			// 2
				tst,	1,
				sp1,	1,
				cdron,	1,
				fout,	1,
				sp4,	1,
				rgoff,	1,
				poff,	1,
				sp7,	1);
	__BC(	cmd,			// 3
				hdr,	1,
				lb2c,	1,
				lb1c,	1,
				b,		1,
				p,		1,
				sp5,	1,
				hcr2,	1,
				hcr1,	1);
	__BC(	det,			// 4
				rev,	1,
				ep,		1,
				wss,	1,
				eb,		1,
				sp4,	1,
				lb2,	1,
				lb1,	1,
				hb,		1);
	__BC(	st,				// 5
				zvr,	1,
				scd,	1,
				etr,	1,
				te,		1,
				nrbd,	1,
				sp,		1,
				ads,	1,
				lsm,	1);
	__BC(	val,			// 6
				sp,		5,
				bdp,	1,
				lwp,	1,
				epwm,	1);
	__BC(	gdf,			// 7
				sp,		1,
				b,		1,
				z,		1,
				w,		1,
				y,		1,
				v,		1,
				x,		1,
				u,		1);
	__BC(	cpt,			// 8
				sp,		2,
				lb1c,	1,
				lb1o,	1,
				lb2c,	1,
				lb2o,	1,
				hbc,	1,
				hbo,	1);
	__BC(	otd,			// 9
				sp,		5,
				w,		1,
				v,		1,
				u,		1);
	BYTE	cSpr10;			// 10
	WORD	wX3sp;			// 11~12, axle 3 speed		0~10000 ->	0~100Km, big endian
	BYTE	cIq;			// 13, torque current		0~0xff ->	0~2000A
	BYTE	cLv;			// 14, variable load		0~0x64 ->	0~100%
	BYTE	cTbe;			// 15, input pwm			0~0x64 ->	0~100%
	BYTE	cBed;			// 16, brake e??? demand	0~0x64 ->	0~100%
	WORD	wApbr;			// 17~18, actual pbr		-10000~10000 ->	-100~100KN, big endian
	WORD	wEs;			// 19~20, line voltage		0~20000 ->	0~2000V, big endian
	WORD	wIdc;			// 21~22, line current		0~20000 ->	0~2000A, big endian
	WORD	wFc;			// 23~24, filter cap. volt.	0~20000 ->	0~2000V, big endian
	__BC(	tst,			// 25
				ti,		4,
				ng,		1,
				ok,		1,
				st,		2);	// 01:ing, 10:end
	BYTE	cSubi;			// 26
	__BC(	fa,				// 27
				pud,	1,
				bocd,	1,
				mocd,	1,
				iocd,	1,
				sp,		1,
				apsf,	1,
				spsf,	1,
				bpsf,	1);
	__BC(	fb,				// 28
				sp0,	1,
				lgd,	1,
				hbtf,	1,
				sp3,	1,
				pgd,	1,
				fcovd,	1,
				fclvd,	1,
				vllvd,	1);
	__BC(	fc,				// 29
				comf,	1,
				sp1,	2,
				bsd,	1,
				pbf,	1,
				frf,	1,
				fcd,	1,
				sp7,	1);
	__BC(	trc,			// 30
				cok,	1,
				sp,		2,
				col,	1,
				leng,	4);
	// 180626
	//BYTE	cSpr31[2];		// 31~32
	WORD		wVer;		// 31~32
} SAV3F;

typedef union _tagV3FATEXT {
	SAV3F	s;
	BYTE	c[sizeof(SAV3F)];
} V3FATEXT;

typedef struct _tagV3FAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	V3FATEXT	t;
} V3FAINFO, *PV3FAINFO;

// TD
typedef struct TAV3F {
	BYTE	cFlow;			// 0
	BYTE	cChapID;		// 1, data id, ~ trc.b.leng
	WORD	wPageID;		// 2~3, 100바이트씩 끊은...
	BYTE	c[SIZE_TRACETEXT];
} TAV3F;

typedef union _tagV3FATEXTTRC {
	SAV3F	s;
	BYTE	c[sizeof(SAV3F)];
	TAV3F	t;
} V3FATEXTTRC;

typedef struct _tagV3FAINFOTRC {
	WORD	wAddr;
	BYTE	cCtrl;
	V3FATEXTTRC	tt;
} V3FAINFOTRC, *PV3FAINFOTRC;

//#define	SIZE_V3FSA		sizeof(V3FAINFO)
//#define	SIZE_V3FSAO		(sizeof(SAV3F) + 3)

typedef struct _tagV3FFLAP {
	V3FRINFO	r;
	V3FAINFO	a;
} V3FFLAP, *PV3FFLAP;

typedef struct _tagV3FFLAPTRC {
	V3FRINFO	r;
	V3FAINFOTRC	ta;
} V3FFLAPTRC, *PV3FFLAPTRC;

#pragma pack(pop)
