/**
 * @file	FduInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2020-02-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_FDU		0x0860

#pragma pack(push, 1)

typedef struct _tagSRFDU {
	BYTE	cFlow;			// 0
	DEVTIME	devt;			//1~6
	WORD	wSeq;			//7-8
	__BW(	no,				// 9-10, train no
				u102,	4,
				u103,	4,
				u100,	4,
				u101,	4);
	BYTE	cCode;			// 11
	BYTE	cCurrSt;		// 12, current station
	BYTE	cNextSt;		// 13, next station
	BYTE	cDestSt;		// 14, destination station
	__BC(	req,			// 15
				rpb,	1,	// release parking brake request
				spr3,	3,
				drr,	1,	// detection return request
				spr2,	2,
				selft,	1);	// self test request
	__BC(	fd,				// 16, fire detected
				c0,		1,
				c1,		1,
				c2,		1,
				c3,		1,
				c4,		1,
				c5,		1,
				c6,		1,
				c7,		1);
	BYTE	cSpr17[4];		// 17~20
} SRFDU;

typedef union _tagFDURTEXT {
	SRFDU	s;
	BYTE	c[sizeof(SRFDU)];
} FDURTEXT;

typedef struct _tagFDURINFO {
	WORD	wAddr;
	BYTE	cCtrl;			// 0x13
	FDURTEXT	t;
} FDURINFO, *PFDURINFO;

#define	SIZE_FDUSR		sizeof(FDURINFO)

// SD, fdu->tcms
typedef struct _tagSAFDU {
	BYTE	cFlow;			// 0
	WORD	wSeq;			// 1-2
	__BW(	det,			// 3-4, fire detect
				c0,		1,
				c1,		1,
				c2,		1,
				c3,		1,
				c4,		1,
				c5,		1,
				c6,		1,
				c7,		1,
				c8,		1,
				c9,		1,
				spr,	6);
	__BC(	uf,				// 5, unit fault
				cl,		1,	// left fault
				cr,		1,	// right fault
				sp6,	6);
	__BC(	selft,			// 6
				sp,		4,
				prog,	2,
				ng,		1,
				ok,		1);
	__BC(	stat,			// 7
				mt1,	1,	// left master
				mt2,	1,	// right master
				sp2,	2,
				frr,	1,	// fire release ready
				sp5,	2,
				pms,	1);	// park mode status
	BYTE	susage[4];		// 8-11, sensor use
	BYTE	sstate[8];		// 12-19, sensor state, 00:normal, 01:warning, 10: fire, 11: fault
	BYTE	sclean[4];		// 20-23, sensor cleaning
	BYTE	cTemp[10][3];	// 24-53
	BYTE	cSmoke[10][3];	// 54-83
	__BC(	curs,			// 84
				fd,		1,	// fire detect
				sp1,	1,
				dc,		1,	// detector check
				sp5,	5);
	__BC(	fl,				// 85, light trouble
				sw,		1,	// switch fault
				d,		1,	// detector fault
				sp6,	6);
	__BC(	fs1,			// 86, severe trouble
				intemp,	1,	// internal temp.
				ad,		1,	// all detector fault
				mem,	1,	// memory fault
				cs,		1,	// check sum fault
				sp4,	4);
	__BC(	fs2,			// 87, severe trouble
				im,		1,	// interface module fault
				diom,	1,
				adm,	1,
				cpum,	1,
				sp3,	3,
				fds,	1);
	WORD	wVer;			// 88-89
	WORD	wSpr90;			// 90-91
} SAFDU;

typedef union _tagFDUATEXT {
	SAFDU	s;
	BYTE	c[sizeof(SAFDU)];
} FDUATEXT;

typedef struct _tagFDUAINFO {
	WORD	wAddr;
	BYTE	cCtrl;
	FDUATEXT	t;
} FDUAINFO, *PFDUAINFO;

#define	SIZE_FDUSA		sizeof(FDUAINFO)

typedef struct _tagFDUFLAP {
	FDURINFO	r;
	FDUAINFO	a;
} FDUFLAP, *PFDUFLAP;

// since FDU SDA is too long to relocate to recip, only important parts are selected
typedef struct _tagSAFDUAB {	// abbreviation
	BYTE	cFlow;			// 0
	__BW(	det,			// 1-2, fire detect
				c0,		1,
				c1,		1,
				c2,		1,
				c3,		1,
				c4,		1,
				c5,		1,
				c6,		1,
				c7,		1,
				c8,		1,
				c9,		1,
				spr,	6);
	__BC(	uf,				// 3, unit fault
				cl,		1,	// left fdu fault
				cr,		1,	// right fdu fault
				//sp6,	6);
				sp2,	2,
				cid,	4);
	__BC(	selft,			// 4
				sp,		4,
				prog,	2,
				ng,		1,
				ok,		1);
	__BC(	stat,			// 5
				mt1,	1,	// left master
				mt2,	1,	// right master
				sp2,	2,
				frr,	1,	// fire release ready
				sp5,	2,
				pms,	1);	// park mode status
	BYTE	susage[4];		// 6-9, sensor use
	BYTE	sstate[8];		// 10-17, sensor state, 00:normal, 01:warning, 10: fire, 11: fault
	BYTE	sclean[4];		// 18-21, sensor cleaning
	__BC(	curs,			// 22
				fd,		1,	// fire detect
				sp1,	1,
				dc,		1,	// detector check
				sp5,	5);
	__BC(	fl,				// 23, light trouble
				sw,		1,	// switch fault
				d,		1,	// detector fault
				sp6,	6);
	__BC(	fs1,			// 24, severe trouble
				intemp,	1,	// internal temp.
				ad,		1,	// all detector fault
				mem,	1,	// memory fault
				cs,		1,	// check sum fault
				sp4,	4);
	__BC(	fs2,			// 25, severe trouble
				im,		1,	// interface module fault
				diom,	1,
				adm,	1,
				cpum,	1,
				sp3,	3,
				fds,	1);
	WORD	wVer;			// 26-27
	WORD	wRecover;		// 28-29
} SAFDUAB;

typedef union _tagFDUATEXTAB {
	SAFDUAB	s;
	BYTE	c[sizeof(SAFDUAB)];
} FDUATEXTAB;

typedef struct _tagFDUAINFOAB {
	WORD	wAddr;
	BYTE	cCtrl;
	FDUATEXTAB	t;
} FDUAINFOAB, *PFDUAINFOAB;

#define	SIZE_FDUSAAB		sizeof(FDUAINFOAB)

typedef struct _tagFDUFLAPAB {
	FDURINFO	r;
	FDUAINFOAB	a;
} FDUFLAPAB, *PFDUFLAPAB;

#pragma pack(pop)
