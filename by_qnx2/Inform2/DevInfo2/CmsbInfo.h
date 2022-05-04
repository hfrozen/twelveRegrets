/**
 * @file	CmsbInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2020-02-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_CMSB		0x7309

#pragma pack(push, 1)

typedef struct _tagSRCMSB {
	BYTE	cFlow;			// 0
	BYTE	nSpr1;			// 1
	__BC(	cmd,			// 2
				cmsb,	1,
				cmg1,	1,
				cmg2,	1,
				cmg3,	1,
				spr4,	4);
	BYTE	cDelay;			// 3, 0~-x3c -> 0~60sec
	DEVTIME	devt;			// 4~9
	BYTE	nSpr10;			// 10
} SRCMSB;

typedef union _tagCMSBRTEXT {
	SRCMSB	s;
	BYTE	c[sizeof(SRCMSB)];
} CMSBRTEXT;

typedef struct _tagCMSBRINFO {
	WORD	wAddr;
	BYTE	cCtrl;			// 0x13
	CMSBRTEXT	t;
} CMSBRINFO, *PCMSBRINFO;

#define	SIZE_CMSBSR		sizeof(CMSBRINFO)

// SD, cmsb->tcms
typedef struct _tagSACMSB {
	BYTE	cFlow;			// 0
	__BC(	stat,			// 1
				invon,	1,
				spr2,	2,
				bypass,	1,
				spr4,	4);
	__BC(	f,				// 2
				ot,		1,	// over temp.
				spr1,	1,
				lv,		1,	// low voltage
				ov,		1,	// over voltage
				hot,	1,	// heatsink over temp.
				ooc,	1,	// output over current
				ool,	1,	// output over load
				os,		1);	// output short
	WORD		wDlv;		// 3-4, dc link voltage, 0~999V
	BYTE		cOc;		// 5, output current, 9~99mA
	BYTE		cSpr6[2];	// 6-7
} SACMSB;

typedef union _tagCMSBATEXT {
	SACMSB	s;
	BYTE	c[sizeof(SACMSB)];
} CMSBATEXT;

typedef struct _tagCMSBAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	CMSBATEXT	t;
} CMSBAINFO, *PCMSBAINFO;

#define	SIZE_CMSBSA		sizeof(CMSBAINFO)

typedef struct _tagCMSBFLAP {
	CMSBRINFO	r;
	CMSBAINFO	a;
} CMSBFLAP, *PCMSBFLAP;

#pragma pack(pop)
