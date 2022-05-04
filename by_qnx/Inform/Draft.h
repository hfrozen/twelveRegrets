/*
 * Draft.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	DRAFT_H_
#define	DRAFT_H_

#include "../Cmn/Define.h"
#include "TrainArch.h"
#include "PcioInfo.h"
#include "Archive.h"

// link to TC
#include "AtcInfo.h"
#include "AtoTwcInfo.h"
#include "InterInfo.h"

// link to CC
#include "V3fInfo.h"
#include "EcuInfo.h"
#include "SivInfo.h"
#include "DcuInfo.h"
#include "CmsbInfo.h"

#define	SIZE_FILESBYDAY		30
#define	SIZE_FILESUNDERDAY	100

#include "DuInfo.h"

/*
	WORD		wEnableNode;	// (1 << enabled cc), 0:not respond at cc
							// reference to only tc
	WORD		wBadNode;	// (1 << bad cc)
	BYTE		nMode;		// reserve
*/

#pragma pack(1)

typedef struct _tagAUXCMD {
	BYTE	nNest;
	WORD	wLocali;
} AUXCMD;

typedef struct _tagDEVERROR {
	BYTE	nState;
	WORD	wCount;
} DEVERROR;

typedef struct _tagLCFIRM {
	ATCRINFO	rAtc[2];
	ATCAINFO	aAtcBuff[2];
	ATCAINFO	aAtc[2];
	ATORINFO	rAto;
	ATOAINFO	aAto;
	CSCAINFO	aCsc;		// 13
	PISAINFO	aPis;		// 14
	TRSAINFO	aTrs;		// 15
	HTCAINFO	aHtc;		// 16
	PSDAINFO	aPsd;		// 17
} LCFIRM, *PLCFIRM;

typedef struct _tagTCDIRECTORY {
	BYTE		nDuty;
	BYTE		nLength;
	DWORD		dwItem[SIZE_FILESBYDAY];
} TCDIRECTORY;

typedef struct _tagTCDOWNLOAD {
	BYTE		nDuty;
	BYTE		nTotal;
	WORD		wYear;
	BYTE		nMonth;
	BYTE		nDay;
	BYTE		nAck;
	BYTE		nNull;
	SHORT		iRespond;
	WORD		wProgress;
} TCDOWNLOAD;

typedef union _tagTCINFORMITEM {
	ENVARCHEXTENSION	env;
	TCDIRECTORY	dir;
	TCDOWNLOAD	dl;
} TCINFORMITEM;

enum  {
	TCINFORM_NON,
	TCINFORM_ENV,
	TCINFORM_DIR,
	TCINFORM_DOWNLOAD,
	TCINFORM_MAX
};

typedef struct _tagTCEACH {
	WORD		wId;
	TCINFORMITEM	info;
} TCEACH;

typedef struct _tagTCFIRMREAL {
	LWORD		wAddr;		// 1:tc1-liu1, 2:tc1-liu2,
							// 3:tc0-liu1, 3:tc0-liu2
							// 4-:each cc
	DWORD		dwOrderID;	// like system byte of secs, always > 0
	BYTE		nFlowNo;	// 1:master-main	2:master-reserve
							// 3:slave -main	4:slave -reserve	c_nDuty + 1
	BYTE		nHcrInfo;	// master tc, same follow
							// 1 -> TC1-LIU1, 2 -> TC1-LIU2, 3 -> TC0-LIU1, 4 -> TC0-LIU2
							// 0x10 -> NOACTIVE
	BYTE		nRealAddr;	// used to distinguish between LIU1, LIU2
	BYTE		nCallAddr;
	TBTIME		time;
	BYTE		nIssue;
#define	ISSUE_TIMERSET					1
#define	ISSUE_ENVIROMENTSET				2
#define	ISSUE_DIRECTORYREQUEST			4
#define	ISSUE_DOWNLOAD					8
#define	ISSUE_BEENUSB					0x10
#define	ISSUE_KILLBUSREQUEST			0x20
#define	ISSUE_KILLDOWNLOAD				0x40

	BYTE		nHint;
#define	HINT_DELAYATC		1
#define	HINT_PARTNER		0x80

	BYTE		nSlaveCtrl;
#define	SLAVETCCTRL_ENABLEOVERHAUL			1
#define	SLAVECCCTRL_DISABLEDEVICEERRORCOUNT	0x10
#define	SLAVECCCTRL_INVALIDOUT				0x20

	BYTE		nNull;
	WORD		wVersion;
	BYTE		nThumbWheel[3];
	BYTE		nDia[16];
	V3FCINFO	cV3f;		// 1
	ECUCINFO	cEcu;		// 2
	SIVCINFO	cSiv;		// 3
	DCUCINFO	cDcu;		// 4
	CMSBCINFO	cCmsb;		// 5
	ATCRINFO	rAtc;		// 6
	ATORINFO	rAto;		// 8
	CMMRINFO	rCmm;		// 12
	BYTE		nDrvlCmd;
#define	DRVLCMD_DMI		1	// TCDIB_DM
#define	DRVLCMD_KUP		2	// Atc kup
#define	DRVLCMD_ING		4	// driverless mode
#define	DRVLCMD_KDN		8	// Atc kdn
#define	DRVLCMD_KDNSETTLE	0x10

	DEVERROR	eAtcA;
	DEVERROR	eAtcB;
	DEVERROR	eAto;
	DEVERROR	eCsc;
	DEVERROR	ePis;
	DEVERROR	eTrs;
	DEVERROR	eHtc;
	DEVERROR	ePsd;
//	MAINDEV		iAtc;
//	MAINDEV		iAto;
//	SERVICEDEV	fCsc;
//	SERVICEDEV	fPis;
//	SERVICEDEV	fTrs;
//	SERVICEDEV	fHtc;
//	SERVICEDEV	fPsd;
	BYTE		nInput[SIZE_TCDI];
	BYTE		nOutput[SIZE_TCDO];
	SHORT		vInput[SIZE_TCAI];
	BYTE		nOrder[LENGTH_CC][SIZE_CCDO];	// output command to each cc,
												// 8 [Len:8][DIO data:4],
												// cc Digital Out Mapping,
												// ref PcioInfo.
	PWMARCH		pwm;
	AUXCMD		acmd;
#define	SIZE_HEADBK		24
	BYTE		ducia[SIZE_HEADBK];
#define	SIZE_NORMALBK	20
	BYTE		ducib[SIZE_NORMALBK];
	//ENVARCH		env;
	TCEACH		each;
	MCDSTICK	mcDrv;
	BYTE		nRealOutput[SIZE_TCDO];
	ARM_ARCH	arm;
} TCFIRMREAL;

#define	SIZE_FIRM		510
typedef struct _tagTCFIRM {
	TCFIRMREAL	real;
	BYTE	dummy[SIZE_FIRM - sizeof(TCFIRMREAL)];
} TCFIRM, *PTCFIRM;

typedef struct _tagPERTDIR {
	WORD		wLength;
	DWORD		dwTotalSize;
	DWORD		dwFile[SIZE_FILESUNDERDAY];
} PERTDIR;

#define	SIZE_MATETEXT	(SIZE_FIRM - (sizeof(DWORD) * 2 + sizeof(WORD) * 2 + sizeof(BYTE) * 4))
//									dwOrderID, dwOffset
//														wAddr, wSize
//																	nFlowNo, nHcrInfo, nDirectoryID, nFileID
typedef struct _tagPERTFILE {
	BYTE		nDirectoryID;	// 0 - 99
	BYTE		nFileID;		// 0:drv, 1:arm, 2:bkop, 3:bkarm
	DWORD		dwOffset;
	WORD		wSize;
	BYTE		nText[SIZE_MATETEXT];
} PERTFILE;

typedef union _tagPERTINFO {
	PERTDIR		dir;
	PERTFILE	text;
} PERTINFO;

typedef struct _tagTCPERT {
	LWORD		wAddr;		// 1:tc1-liu1, 2:tc1-liu2,
							// 3:tc0-liu1, 3:tc0-liu2
							// 4-:each cc
	DWORD		dwOrderID;	// like system byte of secs, always > 0
	BYTE		nFlowNo;	// 1:master-main	2:master-reserve
							// 3:slave -main	4:slave -reserve	c_nDuty + 1
	BYTE		nHcrInfo;	// master tc, same follow
							// 1 -> TC1-LIU1, 2 -> TC1-LIU2, 3 -> TC0-LIU1, 4 -> TC0-LIU2
							// 0x10 -> NOACTIVE
							// 0x80 -> drive record dump, directory
	PERTINFO	cont;
} TCPERT, *PTCPERT;

typedef struct _tagCCFIRMREAL {
	LWORD		wAddr;
	DWORD		dwOrderID;
	WORD		wVersion;
	BYTE		nNull;
	BYTE		nCallAddr;
	TBTIME		time;
	BYTE		nThumbWheel[3];
	V3FEINFO	eV3f;		// 1
	ECUAINFO	aEcu;		// 2
	SIVEINFO	eSiv;		// 3
	CMSBAINFO	aCmsb;		// 5
	DCUAINFO	aDcu[2][4];	// 4
	BYTE		nInput[SIZE_CCDI];
	BYTE		nOutput[SIZE_CCDO];
	PWMARCH		pwm;		// 6
	V3FRINFO	rV3f;
	ECURINFO	rEcu;
	SIVRINFO	rSiv;
	CMSBRINFO	rCmsb;
	WORD		wDoorErrorCount[2][4];
	WORD		wCmsbErrorCount;
} CCFIRMREAL;

typedef struct _tagCCFIRM {
	CCFIRMREAL	real;
	BYTE	dummy[SIZE_FIRM - sizeof(CCFIRMREAL)];
} CCFIRM, *PCCFIRM;

typedef struct _tagTCDOZ {
	TCFIRM		tcs[LENGTH_TC];
	CCFIRM		ccs[LENGTH_CC];
} TCDOZ, *PTCDOZ;

typedef struct _tagCCDOZ {
	TCFIRM		tc;
	CCFIRM		cc;
} CCDOZ, *PCCDOZ;

#pragma pack()

#define	FIRMERROR_ADDR		4
#define	FIRMERROR_FAULT		3
#define	FIRMERROR_UNNORMAL	2
#define	FIRMERROR_NON		0

// for TC
#define	GETTBIT(n, id)	((c_pDoz->tcs[n].real.nInput[id >> 3]) & (1 << (id & 7)))
#define	SETTBIT(n, id)	c_pDoz->tcs[n].real.nOutput[id >> 3] |= (1 << (id & 7))
#define	CLRTBIT(n, id)	c_pDoz->tcs[n].real.nOutput[id >> 3] &= ~(1 << (id & 7))
#define	TOGTBIT(n, id)	c_pDoz->tcs[n].real.nOutput[id >> 3] ^= (1 << (id & 7))
#define	FBTBIT(n, id)	((c_pDoz->tcs[n].real.nOutput[id >> 3]) & (1 << (id & 7)))

#define	SETOBIT(n, b, id)	c_pDoz->tcs[n].real.nOrder[b][id >> 3] |= (1 << (id & 7))
#define	CLROBIT(n, b, id)	c_pDoz->tcs[n].real.nOrder[b][id >> 3] &= ~(1 << (id & 7))
#define	TOGOBIT(n, b, id)	c_pDoz->tcs[n].real.nOrder[b][id >> 3] ^= (1 << (id & 7))
#define	FBOBIT(n, b, id)	(c_pDoz->tcs[n].real.nOrder[b][id >> 3] & (1 << (id & 7)))

#define	READTBYTE(n, add)		c_pDoz->tcs[n].real.nInput[add]
#define	WRITETBYTE(n, add, d)	c_pDoz->tcs[n].real.nOutput[add] = d
#define	FEEDTBYTE(n, add)		c_pDoz->tcs[n].real.nOutput[add]
#define	WRITEOBYTE(n, b, add, d)	c_pDoz->tcs[n].real.nOrder[b][add] = d

#define	GETNBIT(n, id)	(c_pDoz->tcs[n].real.acmd.nNest & (1 << id))
#define	SETNBIT(n, id)	c_pDoz->tcs[n].real.acmd.nNest |= (1 << id)
#define	CLRNBIT(n, id)	c_pDoz->tcs[n].real.acmd.nNest &= ~(1 << id)

#define	READANALOG(n, add)			c_pDoz->tcs[n].real.vInput[add]
#define	WRITEANALOG(n, b, add, w)	c_pDoz->tcs[n].real.vOutput[b][add] = w

#define	GETCBIT(n, id)		((c_pDoz->ccs[n].real.nInput[id >> 3]) & (1 << (id & 7)))
#define	READCBYTE(n, add)	c_pDoz->ccs[n].real.nInput[add]
#define	FEEDCBIT(n, id)		((c_pDoz->ccs[n].real.nOutput[id >> 3]) & (1 << (id & 7)))
#define	FEEDCBYTE(n, add)	c_pDoz->ccs[n].real.nOutput[add]

// for CC
#define	SETCIBITC(id)	c_pDoz->cc.real.nInput[id >> 3] |= (1 << (id & 7));
#define	CLRCIBITC(id)	c_pDoz->cc.real.nInput[id >> 3] &= ~(1 << (id & 7));

#define	GETCOBITC(id)	(c_pDoz->cc.real.nOutput[id >> 3] & (1 << (id & 7)))
#define	SETCOBITC(id)	c_pDoz->cc.real.nOutput[id >> 3] |= (1 << (id & 7));
#define	CLRCOBITC(id)	c_pDoz->cc.real.nOutput[id >> 3] &= ~(1 << (id & 7));

#endif	/* DRAFT_H_ */
