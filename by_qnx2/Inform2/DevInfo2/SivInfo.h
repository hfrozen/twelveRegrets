/**
 * @file	SivInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_SIV		0xff90

#pragma pack(push, 1)

// sync, 38400, 50ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA, 0x21=TDR, 0x31=TDA
// SDR, tcms->siv
typedef struct _tagSRSIV {
	BYTE	cFlow;			// 0
	__BC(	cmd,			// 1
				tsr,	1,	// test start request (voltage/frequency)
				etr9,	1,	// extension, siv9 off
				etr5,	1,	//			, siv5 off
				etr0,	1,	//			, siv0 off
				sp,		1,
				esk,	1,	// esk on
				extsr,	1,	// test start request
				tdc,	1);	// trace data clear request
	BYTE	cCid;			// 2, 0-5-9 siv car no.
	DEVTIME	devt;			// 3~8
	BYTE	cSpr[2];		// 9~10
} SRSIV;

// TDR
typedef struct _tagTRSIV {
	BYTE	cFlow;			// 0
	BYTE	cChapID;		// 1, data id, ~ trc.b.leng
	WORD	wPageID;		// 2~3, 100바이트씩 끊은...
	BYTE	cSpr;
} TRSIV;

typedef union _tagSIVRTEXT {
	SRSIV	s;
	BYTE	c[sizeof(SRSIV)];
	TRSIV	t;
} SIVRTEXT;

typedef struct _tagSIVRINFO {
	WORD	wAddr;			// 0xff90
	BYTE	cCtrl;			// 0x13
	SIVRTEXT	t;
} SIVRINFO, *PSIVRINFO;

#define SIZE_SIVSR		sizeof(SIVRINFO)

// ===== 10 car =====
// SD siv->tcms
typedef struct _tagSASIV {
	BYTE	cFlow;			// 0
	__BC(	tst,			// 1
				st,		2,	// 01:testing, 10:test end
				vng,	1,	// voltage test result NG
				vok,	1,	// voltage test result OK
				fng,	1,	// frequency test result NG
				fok,	1,	// frequency test result OK
				sp,		1,
				sts,	1);
	__BC(	sta,			// 2
				sp0,	1,
				off9,	1,	// siv 9 off
				off5,	1,	// siv 5 off
				off0,	1,	// siv 0 off
				cttf,	1,	// contactor fail
				gfux2,	1,
				gfvy2,	1,
				gfwz2,	1);
	__BC(	stb,			// 3
				lk,		1,
				chk,	1,
				sivk,	1,
				sp,		2,
				start,	1,
				pcd,	1,
				ivs,	1);
	__BC(	stc,			// 4
				ivoc,	1,
				acoc,	1,
				psft,	1,
				bcoc,	1,
				gfux1,	1,
				gfvy1,	1,
				gfwz1,	1,
				thf,	1);
	__BC(	std,			// 5
				dcov,	1,
				fcov,	1,
				mmocd,	1,	//180511, unft,	1,
				acov,	1,
				acuv,	1,
				bcov,	1,
				ovld,	1,
				mjf,	1);
	BYTE	cEs;			// 6, line voltage				0~3000V / 0~0xfa
	BYTE	cIs;			// 7, invertor input current	0~500A / 0~0xfa
	BYTE	cVo;			// 8, output voltage			0~750V / 0~0xfa
	BYTE	cIo;			// 9, output current			0~750A / 0~0x2ee's LSB	// 0~0xfa
	BYTE	cFo;			// 10, output frequency			0~125Hz / 0~0xfa
	BYTE	cVs;			// 11, invertor input voltage	0~3000V / 0~0xfa
	__BC(	trc,			// 12
				cok,	2,
				sp,		1,
				col,	1,
				leng,	4);
	__BC(	ste,			// 13
				igdf,	1,
				llvf,	1,	// 171114
				sp,		2,
				iom,	4);	// output current				0~750A / 0~0x2ee's MSB 4bit
				//sp,		6);	// 171114
	BYTE	cBatV;			// 170731, cSpr14;			// 14
} SASIV;

// SDEX siv->tcms
typedef struct _tagSASIVEX {
	BYTE	cFlow;			// 0
	__BC(	tst,			// 1
				st,		2,	// 01:testing, 10:test end
				vng,	1,	// voltage test result NG
				vok,	1,	// voltage test result OK
				fng,	1,	// frequency test result NG
				fok,	1,	// frequency test result OK
				sp,		1,
				sts,	1);
	__BC(	sta,			// 2
				sp0,	1,
				off9,	1,	// siv 9 off
				off5,	1,	// siv 5 off
				off0,	1,	// siv 0 off
				cttf,	1,	// contactor fail
				gfux2,	1,
				gfvy2,	1,
				gfwz2,	1);
	__BC(	stb,			// 3
				lk,		1,
				chk,	1,
				sivk,	1,
				sp,		2,
				start,	1,
				pcd,	1,
				ivs,	1);
	__BC(	stc,			// 4
				ivoc,	1,
				acoc,	1,
				psft,	1,
				bcoc,	1,
				gfux1,	1,
				gfvy1,	1,
				gfwz1,	1,
				thf,	1);
	__BC(	std,			// 5
				dcov,	1,
				fcov,	1,
				mmocd,	1,	//180511, unft,	1,
				acov,	1,
				acuv,	1,
				bcov,	1,
				ovld,	1,
				mjf,	1);
	BYTE	cEs;			// 6, line voltage				0~3000V / 0~0xfa
	BYTE	cIs;			// 7, invertor input current	0~500A / 0~0xfa
	BYTE	cVo;			// 8, output voltage			0~750V / 0~0xfa
	BYTE	cIo;			// 9, output current			0~750A / 0~0x2ee's LSB	// 0~0xfa
	BYTE	cFo;			// 10, output frequency			0~125Hz / 0~0xfa
	BYTE	cVs;			// 11, invertor input voltage	0~3000V / 0~0xfa
	__BC(	trc,			// 12
				cok,	2,
				sp,		1,
				col,	1,
				leng,	4);
	__BC(	ste,			// 13
				igdf,	1,
				llvf,	1,	// 171114
				sp,		2,
				iom,	4);	// output current				0~750A / 0~0x2ee's MSB 4bit
				//sp,		6);	// 171114
	BYTE	cBatV;			// 170731, cSpr14;			// 14
	// 180626
	WORD	wVer;			// 15~16
	WORD	wSpr[4];		// 17~18/19~20/21~22/23~24
} SASIVEX;

typedef union _tagSIVATEXT {
	SASIV	s;
	BYTE	c[sizeof(SASIV)];
} SIVATEXT;

typedef union _tagSIVATEXTEX {
	SASIVEX	s;
	BYTE	c[sizeof(SASIVEX)];
} SIVATEXTEX;

typedef struct _tagSIVAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	SIVATEXT	t;
} SIVAINFO, *PSIVAINFO;

typedef struct _tagSIVAINFOEX {
	WORD	wAddr;
	BYTE	cCtrl;
	SIVATEXTEX	t;
} SIVAINFOEX, *PSIVAINFOEX;

// DIAGNOSTIC
//typedef struct _tagSIVADETECTIVE {
//	DBFTIME		dbft;
//	WORD		wStatus;
//	SIVAINFO	d;
//} SIVADET, *PSIVADET;
//
// DIAGNOSTIC
typedef struct _tagSIVADETECTIVEEX {
	DBFTIME		dbft;
	WORD		wStatus;
	SIVAINFOEX	d;
} SIVADETEX, *PSIVADETEX;

// TD
typedef struct TASIV {
	BYTE	cFlow;			// 0
	BYTE	cChapID;		// 1, data id, ~ trc.b.leng
	WORD	wPageID;		// 2~3, 100바이트씩 끊은...
	BYTE	c[SIZE_TRACETEXT];
} TASIV;

//typedef union _tagSIVATEXTTRC {
//	SASIV	s;
//	BYTE	c[sizeof(SASIV)];
//	TASIV	t;
//} SIVATEXTTRC;
//
typedef union _tagSIVATEXTTRCEX {
	SASIVEX	s;
	BYTE	c[sizeof(SASIV)];
	TASIV	t;
} SIVATEXTTRCEX;

//typedef struct _tagSIVAINFOTRC {
//	WORD	wAddr;
//	BYTE	cCtrl;
//	SIVATEXTTRC	tt;
//} SIVAINFOTRC, *PSIVAINFOTRC;
//
typedef struct _tagSIVAINFOTRCEX {
	WORD	wAddr;
	BYTE	cCtrl;
	SIVATEXTTRCEX	tt;
} SIVAINFOTRCEX, *PSIVAINFOTRCEX;

//#define	SIZE_SIVSA		sizeof(SIVAINFO)
//#define	SIZE_SIVSAO		(sizeof(SASIV) + 3)

//typedef struct _tagSIVFLAP {
//	SIVRINFO	r;
//	SIVAINFO	a;
//} SIVFLAP, *PSIVFLAP;
//
typedef struct _tagSIVFLAPEX {
	SIVRINFO	r;
	SIVAINFOEX	a;
} SIVFLAPEX, *PSIVFLAPEX;

#define	SIZE_SIVFLAPEX	sizeof(SIVFLAPEX)

//typedef struct _tagSIVFLAPTR {
//	SIVRINFO	r;
//	SIVAINFOTRC	ta;
//} SIVFLAPTRC, *PSIVFLAPTRC;
//
typedef struct _tagSIVFLAPTRCEX {
	SIVRINFO		r;
	SIVAINFOTRCEX	ta;
} SIVFLAPTRCEX, *PSIVFLAPTRCEX;

// ===== 6 car
// SD6 siv->tcms
typedef struct _tagSASIV2 {
	BYTE	cFlow;			// 0
	__BC(	tst,			// 1
				st,		2,	// 01:testing, 10:test end
				vng,	1,	// voltage test result NG
				vok,	1,	// voltage test result OK
				fng,	1,	// frequency test result NG
				fok,	1,	// frequency test result OK
				sp,		1,
				sts,	1);
	__BC(	sta,			// 2
				off9,	1,	// siv 9 off
				off5,	1,	// siv 5 off
				off0,	1,	// siv 0 off
				cttf,	1,	// contactor fail
				inov,	1,
				inoc,	1,
				chpfov,	1,
				chpfuv,	1);
	__BC(	stb,			// 3
				lk,		1,
				chk,	1,
				sivk,	1,
				bcchk,	1,
				bcik,	1,
				sp,		1,
				start,	1,
				ivs,	1);
	__BC(	stc,			// 4
				chpov,	1,
				chpoc,	1,
				invcov,	1,
				dcoc,	1,
				invoc,	1,
				acov,	1,
				acuv,	1,
				lovd,	1);
	__BC(	std,			// 5
				acub,	1,
				acoc,	1,
				acol,	1,
				bcnoc,	1,
				bcoc,	1,
				bcov,	1,
				sp,		2);
	__BC(	ste,			// 6
				chkf,	1,
				lkf,	1,
				sivkf,	1,
				sp,		3,
				stf,	1,
				igdf,	1);
	BYTE	cEs;			// 7, line voltage				0~3000V / 0~0xfa
	BYTE	cIs;			// 8, invertor input current	0~500A / 0~0xfa
	BYTE	cVo;			// 9, output voltage			0~750V / 0~0xfa
	WORD	wIo;			// 10~11, output current, big endian,	0~750A / 0~0x2ee
	BYTE	cFo;			// 12, output frequency			0~125Hz / 0~0xfa
	BYTE	cVs;			// 13, invertor input voltage	0~3000V / 0~0xfa
	__BC(	stf,			// 14
				gdfc1,	1,
				gdfc2,	1,
				gdfru,	1,
				gdfrv,	1,
				gdfbu,	1,
				gdfbv,	1,
				sp,		2);
	__BC(	stg,			// 15
				gdfu,	1,
				gdfv,	1,
				gdfw,	1,
				sp,		3,
				thf,	1,
				cplv,	1);
	__BC(	trc,			// 16
				cok,	2,
				sp,		1,
				col,	1,
				leng,	4);
	BYTE	cBatV;			// 17
	WORD	wVer;			// 18~19
	BYTE	cSpr[5];		// 20~24
} SASIV2;

typedef union _tagSIVATEXT6 {
	SASIV2	s;
	BYTE	c[sizeof(SASIV2)];
} SIVATEXT2;

typedef struct _tagSIVAINFO2 {
	WORD	wAddr;
	BYTE	cCtrl;
	SIVATEXT2	t;
} SIVAINFO2, *PSIVAINFO2;

// DIAGNOSTIC
typedef struct _tagSIVADETECTIVE2 {
	DBFTIME		dbft;
	WORD		wStatus;
	SIVAINFO2	d;
} SIVADET2, *PSIVADET2;

typedef union _tagSIVATEXTTRC2 {
	SASIV2	s;
	BYTE	c[sizeof(SASIV2)];
	TASIV	t;
} SIVATEXTTRC2;

typedef struct _tagSIVAINFOTRC2 {
	WORD	wAddr;
	BYTE	cCtrl;
	SIVATEXTTRC2	tt;
} SIVAINFOTRC2, *PSIVAINFOTRC2;

typedef struct _tagSIVFLAP2 {
	SIVRINFO	r;
	SIVAINFO2	a;
} SIVFLAP2, *PSIVFLAP2;

#define	SIZE_SIVFLAP2	sizeof(SIVFLAP2)

typedef struct _tagSIVFLAPTR2 {
	SIVRINFO		r;
	SIVAINFOTRC2	ta;
} SIVFLAPTRC2, *PSIVFLAPTRC2;

#pragma pack(pop)

//#if (SIZE_SIVFLAPEX != SIZE_SIVFLAP2)
//#error	sivflap size is different!!!
//#endif

#define	LOW_LINEVOLTAGE		900
#define	LOW_BATVOLTAGE		85
#define	SIZE_SIVDETECT		101
#define	SIZE_PRESIVDETECT	50
#define	SIZE_SUFSIVDETECT	(SIZE_SIVDETECT - SIZE_PRESIVDETECT)
