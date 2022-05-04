/*
 * Internal.h
 *
 *  Created on: 2010. 10. XX
 *      Author: Che
 */

#ifndef	INTERNAL_H_
#define	INTERNAL_H_

#include <Define.h>

#include <Archive.h>
#include <AtcInfo.h>
#include <AtoTwcInfo.h>
#include <SivInfo.h>
#include <V3fInfo.h>
#include <EcuInfo.h>
#include <CmsbInfo.h>

/*
SIZE_ATCSR is 15
SIZE_ATCSA is 18		= 33 * 1728 = 57,024
SIZE_ATOSR is 31
SIZE_ATOSA is 26		= 57 * 1728 = 98,496
SIZE_SIVSR is 11
SIZE_SIVSA is 28		= 39 * 1728 = 67,392
SIZE_ECUSR is 14
SIZE_ECUSA is 33		= 47 * 1728 = 81,216
SIZE_CMSBSR is 17
SIZE_CMSBSA is 11		= 28 * 1728 = 48,384
*/

#pragma	pack(1)

#define	MAX_TICKERSAMPLE	(3600 * 48 * 2)	// 0.5sec * 2day	10	// 0.1sec * 2day
#define	MAX_TICKERFILES		200
#define	MAX_TICKERINDEXOFFILE	(MAX_TICKERSAMPLE / MAX_TICKERFILES)	// 1728

typedef struct _tagBBATC {
	BYTE	r[SIZE_ATCSR];
	BYTE	a[SIZE_ATCSA];
} BBATC;

typedef union _tagTBBATC {	// struct of file "/tmp/atc"
	BBATC	f;
	BYTE	n[sizeof(BBATC)];
} TBATC;

typedef struct _tagBBATO {
	BYTE	r[SIZE_ATOSR];
	BYTE	a[SIZE_ATOSA];
} BBATO;

typedef union _tagTBATO {
	BBATO	f;
	BYTE	n[sizeof(BBATO)];
} TBATO;

typedef struct _tagBBSIV {
	BYTE	r[SIZE_SIVSR];
	BYTE	a[SIZE_SIVSA];
} BBSIV;

typedef union _tagTBSIV {
	BBSIV	f;
	BYTE	n[sizeof(BBSIV)];
} TBSIV;

typedef struct _tagBBV3F {
	BYTE	r[SIZE_V3FSR];
	BYTE	a[SIZE_V3FSA];
} BBV3F;

typedef union _tagTBV3F {
	BBV3F	f;
	BYTE	n[sizeof(BBV3F)];
} TBV3F;

typedef struct _tagBBECU {
	BYTE	r[SIZE_ECUSR];
	BYTE	a[SIZE_ECUSA];
} BBECU;

typedef union _tagTBECU {
	BBECU	f;
	BYTE	n[sizeof(BBECU)];
} TBECU;

typedef struct _tagBBCMSB {
	BYTE	r[sizeof(CMSBRINFO)];
	BYTE	a[sizeof(CMSBAINFO)];
} BBCMSB;

typedef union _tagTBCMSB {
	BBCMSB	f;
	BYTE	n[sizeof(BBCMSB)];
} TBCMSB;

#pragma	pack()

// for socket
#define	NAME_GATE	(PSZ)"/Gate"
#define	NAME_SERV	(PSZ)"/Post"
#define	SIZE_SMSG	8186
#define	SIZE_RTIME	28		// "\tVARS:MSG:00-00-00 00:00:00"
#define	SIZE_GENERALBUF	SIZE_SMSG - SIZE_RTIME
#define	MSG_MAX		16

typedef struct _tagSERVERMSG {
	WORD	wID;
	DWORD	dwLength;
	BYTE	nBuf[SIZE_SMSG];
} SMSG, *PSMSG;

#define	MSG_CONTC	2
#define	MSG_CONCC	3
#define	MSG_ACCEPT	4
#define	MSG_SEND	6
#define	MSG_RECEIVE	8
#define	MSG_CLOSE	10

typedef struct _tagDIRINFO {
	int		quantity;
	long long	size;
} DIRINFO, *PDIRINFO;

typedef union _tagELEMENT {
	DEVTIME		dt;
	struct {
		WORD	length;
		DWORD	size;
		DWORD	free;
	} s;
} ELEMENT;

typedef struct _tagBUCKET {
	BYTE	nMainAck;
	BYTE	nPeriodAck;
	WORD	wState;		//
#define	BEILSTATE_BEENMEM		1
#define	BEILSTATE_BEENUSB		2
#define	BEILSTATE_TC1			8
#define	BEILSTATE_LIU2			0x10
#define	BEILSTATE_CMDEXIT		0x80

	WORD	wTicker;
#define	BEILTICKER_ATCR			1
#define	BEILTICKER_ATCA			2
#define	BEILTICKER_ATOR			4
#define	BEILTICKER_ATOA			8
#define	BEILTICKER_SIVR			0x10
#define	BEILTICKER_SIVA			0x20
#define	BEILTICKER_V3FR			0x40
#define	BEILTICKER_V3FA			0x80
#define	BEILTICKER_ECUR			0x100
#define	BEILTICKER_ECUA			0x200
#define	BEILTICKER_CMSBR		0x400
#define	BEILTICKER_CMSBA		0x800
#define	BEILTICKER_DRV			0x4000
#define	BEILTICKER_ENV			0x8000

	SHORT	iRespond;	//
	WORD	wProgress;	//
	WORD	wParam1;	//
	ELEMENT	src;		//
	ELEMENT	tar;		//
	WORD	wFileID;	// use download by bus, same to TCPERT.cont.text.nFileID
	DWORD	dwOffset;	// use download by bus, same to TCPERT.cont.text.dwOffset
	DRV_ARCH	drv;
//	WORD	wInhibit;
//#define	INHIBIT_TICKER	1

	TBATC	atc;
	TBATO	ato;
	TBSIV	siv;
	TBV3F	v3f;
	TBECU	ecu;
	TBCMSB	cmsb;
} BUCKET, *PBUCKET;

enum  {
	BEILACK_NON = 0,
	BEILACK_WORKING,
	BEILACK_WORKEND,
	BEILACK_MAX
};

#define	BEIL_MAX		16

#define	NAME_BEIL	(PSZ)"/Beil"
#define	NAME_BUCKET	(PSZ)"/Bucket"
#define	USB_PATH		"/hd1"

#define	DRV_FILE		"oper"
#define	ARM_FILE		"arm"
#define	BDRV_FILE		"bkop"
#define	BARM_FILE		"bkarm"

#define	ENV_PATHT		"/tmp/env"
#define	DRV_PATHT		"/tmp/oper"
#define	ARM_PATHT		"/tmp/arm"
#define	ARM_PATHET		"/tmp/armed"
#define	ARM_PATHCT		"/tmp/carm"
#define	TRIAL_PATHT		"/tmp/trial"
#define	INSP_PATHT		"/tmp/insp"
#define	TATC_PATHT		"/tmp/atc"
#define	TATO_PATHT		"/tmp/ato"
#define	TSIV_PATHT		"/tmp/siv"
#define	TV3F_PATHT		"/tmp/v3f"
#define	TECU_PATHT		"/tmp/ecu"
#define	DIR_PATHT		"/tmp/dir"
#define	SECT_PATHT		"/tmp/sect"

#define	ARCH_PATHN		"/nand/archives"
#define	ENV_PATHN		"/nand/env"

#define	MEM_PATHE		"/nfd"
#define	ENV_PATHE		"/nfd/env"
#define	ARCH_PATHE		"/nfd/archives"
#define	CUR_PATHE		"/nfd/archives/current"
#define	DRV_PATHE		"/nfd/archives/current/oper"
#define	BKDRV_PATHE		"/nfd/archives/current/bkop"
#define	BKARM_PATHE		"/nfd/archives/current/bkarm"
#define	TICKER_PATHE	"/nfd/archives/ticker"
#define	TINDEX_PATHE	"/nfd/archives/ticker/index"
#define	TATC_PATHE		"/nfd/archives/ticker/atc"
#define	TATO_PATHE		"/nfd/archives/ticker/ato"
#define	TSIV_PATHE		"/nfd/archives/ticker/siv"
#define	TV3F_PATHE		"/nfd/archives/ticker/v3f"
#define	TECU_PATHE		"/nfd/archives/ticker/ecu"
#define	TCMSB_PATHE		"/nfd/archives/ticker/cmsb"

enum  {
	BEILCMD_NON = 0,
	BEILCMD_RTC,
	BEILCMD_SEARCHUSB,
	BEILCMD_WRITEENV,
	BEILCMD_LOGBOOKTOMEM,			// must be respond
	BEILCMD_LOGBOOKTOUSB,			//BEILCMD_DOWNLOAD,
	BEILCMD_LOGBOOKTOUSBNDELETE,	//BEILCMD_DOWNLOADDELETE,
	BEILCMD_INSPECTTOUSB,
	BEILCMD_TRACETOUSB,
	BEILCMD_TRIALTOUSB,
	BEILCMD_DELETEARCHIVES,
	BEILCMD_TICKER,
	BEILCMD_MAKEDIRECTORYLIST,		// must be respond
	BEILCMD_LOADALARM,				// must be respond
	BEILCMD_APPENDDRV,
	BEILCMD_SELTOUSB,
	BEILCMD_SELTODELETE,
	BEILCMD_SECTQUANTITY,
	BEILCMD_READSECT,
	BEILCMD_WRITESECT,
	BEILCMD_MAX
};

enum  {
	FILEWORK_EXITDOWNLOAD = 5,				// 230
	FILEWORK_NOTFINDUSB = 4,				// 229
	FILEWORK_ANYDATA = 3,					// 228
	FILEWORK_END = 2,						// 227
	FILEWORK_WORKING = 1,					// 226
	FILEERROR_NON = 0,						// 225
	FILEERROR_CANNOTREADENVFILE = -1,		// 224*
	FILEERROR_CANNOTWRITEENVFILE = -2,		// 223*
	FILEERROR_CANNOTREADINSPFILE = -3,		// 222*
	FILEERROR_CANNOTCREATEINSPFILE = -4,	// 221*
	FILEERROR_CANNOTCREATERACEFILE = -5,	// 220*
	FILEERROR_CANNOTGETFREESIZE = -6,		// 219*
	FILEERROR_CANNOTRENAMEDIR = -7,			// 218*, change msg !!!!!
	FILEERROR_CANNOTCREATETARGETDIR = -8,	// 217*
	FILEERROR_CANNOTCREATEBASEDIR = -9,		// 216*
	FILEERROR_CANNOTREMOVEFILE = -10,		// 215*
	FILEERROR_CANNOTREMOVEDIR = -11,		// 214*
	FILEERROR_CYCLEOVERDELETE = -12,		// 213*
	FILEERROR_CANNOTCREATEDIR = -13,		// 212*
	FILEERROR_CANNOTFOUNDSRC = -14,			// 211*
	FILEERROR_CANNOTOPENDEST = -15,			// 210*
	FILEERROR_CANNOTREADSRC = -16,			// 209*
	FILEERROR_CANNOTWRITEDEST = -17,		// 208*
	FILEERROR_CANNOTOPENDRV = -18,			// 207*
	FILEERROR_CANNOTOPENARM = -19,			// 206*
	FILEERROR_CANNOTOPENDIR = -20,			// 205*, change msg!!!!!
	FILEERROR_CANNOTFOUNDDATA = -21,		// 204*, change msg!!!!!
	FILEERROR_NOTFOUNDUSB = -22,			// 203*, change msg!!!
	FILEERROR_CANNOTCREATEARM = -23,		// 202*
	FILEERROR_NOTENOUGHSPACE = -24,			// 201*
	FILEERROR_EXITCMD = -25,				// 200
	FILEERROR_FAILRESERVEBUS = -26,			// 199
	FILEERROR_FAILOPPOSITE = -27,			// 198
	FILEERROR_NOTFOUNDANYFILEATBUS = -28,	// 197 해당 날짜는 비어있습니다.
	FILEERROR_CANNOTREADANYFILEATBUS = -29,	// 196 해당 날짜를 읽을 수 없습니다
	FILEERROR_MAX = -100
};

#define	RESET_BEILACK() {\
 c_pBucket->nMainAck = BEILACK_NON;\
 c_pBucket->wState &= ~BEILSTATE_CMDEXIT;\
 c_pBucket->iRespond = FILEERROR_NON;\
 c_pBucket->wProgress = 0;\
}

#define	RESET_BEILACKA(p) {\
 p->nMainAck = BEILACK_NON;\
 p->wState &= ~BEILSTATE_CMDEXIT;\
 p->iRespond = FILEERROR_NON;\
 p->wProgress = 0;\
}

#define	IsExitCmd()	(c_pBucket->wState & BEILSTATE_CMDEXIT)

#define	FILEMSG_BEGIN	225

#endif	/* INTERNAL_H_ */
