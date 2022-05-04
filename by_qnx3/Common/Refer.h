/**
 * @file	Refer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once

#define	SIZE_OWNERBUFF			256
#define	UNKNOWNOWNER			"The truth untold"

// task times
#define	TPERIOD_BASE			20		//10
#define	REAL2PERIOD(x)			(x / TPERIOD_BASE)
#define	TPERIOD_ERRORLAMP		REAL2PERIOD(500)

// task priorities
#define	PRIORITY_PERIODBUS		30		// where ???
#define	PRIORITY_INTRBUS		29

#define	PRIORITY_TAG			26
#define	PRIORITY_PERIODTUNE		24

#define	PRIORITY_PERIODPORTA	23
#define	PRIORITY_PERIODPORTB	22
#define	PRIORITY_PERIODPORTC	22
#define	PRIORITY_PERIODASYNC	22
#define	PRIORITY_INTRPORTB		21
#define	PRIORITY_INTRPORTC		21
#define	PRIORITY_INTRASYNC		21
#define	PRIORITY_PERIODFIO		20

#define	PRIORITY_TASKCMD		16
#define	PRIORITY_POST			16
#define	PRIORITY_MXINTR			16
#define	PRIORITY_DEFAULT		10

#include "DefTypes.h"

typedef struct _tagLINEFABRIC {
	union {
		int		fd;		// such as /dev/ser1...
		WORD	wAddr;
	} subject;
	int		iCh;
	bool	bAddrMatch;
	DWORD	dwBps;
	struct {
		WORD	wRespont;
		WORD	wCycle;
	} time;
} LFABRIC, *PLFABRIC;

#define	WORKING_PATH	"/h/m"
#define	STORAGE_PATH	"/h/sd"
#define	ARCHIVE_PATH	"/h/sd/arc"
#define	ANOTHER_PATH	"/h/sd/not"
#define	LOG_PATH		"/h/sd/log"
#define	TEMP_PATH		"/tmp"
#define	USB_PATHA		"/dos/c"
#define	USB_PATHB		"/dos/c/arc"
#define	USB_PATHC		"/dos/c/not"
#define	CMP_FILE		"/tmp/cmp"

#define	ENVFILE_NAME	"/h/sd/env"

#define	CURTOD_SIGN		'c'
#define	ZIPTOD_SIGN		'a'			// wait upload to net or download to usb
#define	UPETOD_SIGN		'n'			// unloaded, wait download to usb
#define	LOG_SIGN		'l'

#define	MAX_TASKCMD		4096	//256

#define	DESTIP_DEFAULT		"193.168.0.3"
#define	DESTIP_TITLE		"DIP="		// server ip
#define	TRAINO_TITLE		"TNO="		// train no

#pragma pack(push, 1)

typedef struct _tagTASKCMD {
	WORD	wCmd;
} TASKCMD;

enum enTASKCMD {
	TASKCMD_NON = 0,
	TASKCMD_TIMESET,
	TASKCMD_APPENDTOD,
	TASKCMD_WIDEPING,
	TASKCMD_NARROWPING,
	TASKCMD_UPLOAD,
	TASKCMD_CONNECT,		// 200212
	TASKCMD_REPORT,			// 200212
	TASKCMD_SCANUSB,
	TASKCMD_DOWNLOAD,
	TASKCMD_MAX
};

#define	MAX_DIP		64
typedef struct {
	char	szDip[MAX_DIP][256];
	//char	szUpload[256];
	WORD	wTrNo;
} _ENV;

typedef struct _tagMEASURINGTIMES {
	WORD	wMin;
	WORD	wNor;
	WORD	wMax;
} MSRTIME;

#define	DEFAULT_MINDB		3600.f
typedef struct _tagMEASURINGTIMEDBS {
	double	dbMin;
	double	dbNor;
	double	dbMax;
} MSRTIMEDB, *PMSRTIMEDB;

// 0xffffffff = 4294967725us = 4294sec = 71min
#define	DEFAULT_MINDW		0x7fffffff
typedef struct _tagMEASURINGTIMEDWS {
	DWORD	dwMin;
	DWORD	dwNor;
	DWORD	dwMax;
} MSRTIMEDW, *PMSRTIMEDW;

typedef union _tagBCDT {
	struct {
		BYTE	one :	4;
		BYTE	ten :	4;
	} b;
	BYTE a;
} BCDT;

typedef struct _tagTODTIME {
	BCDT	year;
	BCDT	mon;
	BCDT	day;
	BCDT	hour;
	BCDT	min;
	BCDT	sec;
} TODTBCD, *PTODTBCD;

typedef struct _tagTODTRNO {
	BCDT	th;
	BCDT	to;
} TRNOBCD, *PTRNOBCD;

typedef struct _tagTODDELIMITER {
	TODTBCD	t;
	TRNOBCD	n;
} TODDLM, *PTODDLM;

#define	SIZE_TOD		1408
typedef struct _tagTODREAL {
	BYTE		cCtrl;
	BYTE		cCnt;
	TODDLM		td;
	BYTE		cChars[SIZE_TOD - (sizeof(TODDLM) + 2)];
} TODREAL;

typedef union _tagTODTEXT {
	TODREAL	real;
	BYTE	c[SIZE_TOD];
} TODTEXT;

typedef struct _tagTODFREAL {
	BYTE	cStx;
	TODTEXT	todt;
	BYTE	cEtx;
	BYTE	cBcce;
	BYTE	cBcco;
} TODFREAL;

typedef union _tagTODFRAME {
	TODFREAL	real;
	BYTE		c[sizeof(TODREAL)];
} _TODF;

typedef struct _tagRESPOND {
	BYTE	cStx;
	BYTE	cCtrl;
	BYTE	cCnt;
	BYTE	cSpr4[4];
	BYTE	cVer;
	BYTE	cSpr5[5];
	BYTE	cEtx;
	BYTE	cBcce;
	BYTE	cBcco;
} _RESP;

//typedef struct _tagRESPOND {
//	BYTE	cStx;
//	BYTE	cCtrl;
//	BYTE	cCnt;
//	BYTE	cSpr4[4];
//	BYTE	cVer;
//	BYTE	cSpr;
//	BYTE	cEtx;
//	BYTE	cBcce;
//	BYTE	cBcco;
//} _RESP;
//
typedef union _tagTODTHEX {
	struct {
		DWORD	min		: 6;	// lsb
		DWORD	hour	: 5;
		DWORD	day		: 5;
		DWORD	mon		: 4;
		DWORD	year	: 12;
	} r;
	DWORD	dw;
} TODTHEX, *PPTODTHEX;

typedef union _tagTODFINDEX {
	struct {
		TODTHEX		t;
		WORD	wTrNo;
		WORD	wNull;
	} i;
	QWORD	qw;
} TODFINDEX, *PTODFINDEX;

#pragma pack(pop)

#define	Validity(v)		((v) >= 0)

#define	CHECK_SIZE(limit, obj)\
	enum { SIZE_##obj = sizeof(obj) };\
	struct __tag##obj {\
		char	ch##obj[limit - SIZE_##obj];\
	};

#define	KILL(p)\
	do {\
		if (p != NULL) {\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	KILLR(p)\
	do {\
		if (p != NULL) {\
			if (p->GetRhythm())	p->Release();\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	KILLC(p)\
	do {\
		if (p != NULL) {\
			p->Close();\
			delete p;\
			p = NULL;\
		}\
	} while (0)

#define	MAKEDWORD(low, high)	((((DWORD)high << 16) & 0xffff0000) | (DWORD)low)
#define	HIGHWORD(dw)			((WORD)((dw >> 16) & 0xffff))
#define	LOWWORD(dw)				((WORD)(dw & 0xffff))
#define	MAKEWORD(low, high)		((((WORD)high << 8) & 0xff00) | ((WORD)low & 0xff))
#define	HIGHBYTE(w)				((BYTE)((w >> 8) & 0xff))
#define	LOWBYTE(w)				((BYTE)(w & 0xff))

#define	INCWORD(w)\
	do {\
		if (w < 0xffff) ++ w;\
	} while (0)

#define	INCBYTE(c)\
	do {\
		if (c < 0xff)	++ c;\
	} while (0)
