/**
 * @file	RtdInfo.h
 * @brief
 * @details
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "../DuoTypes.h"
#include "../../../../Common/VarUnion.h"

#define	LADD_RTD	0xff40
#pragma pack(push, 1)

// sync, 1152000, 50ms
// BYTE	cFlow -> 0x20=SDR, 0x30=SDA
// SDR, tcms->rtd

//	54			// trouble send
//	0000-0008	// 8block
//	0000-0000
//	8 * 64 byte	//

//	53			// logbook send
//	0000-0000	// block index
//	0000-0000	// block length
//	512 byte	// text

// 65535(block index 16bit) / 7200(1 hour sample at 0.5sec) = 9 hour / 735
// 65535 / 36000(1 hour sample at 0.1sec) = 1 hour / 29535(49min)

// filename					: 20byte
// 20161231235959(235959)	: (drive time)
// 01234567890123 456789
// filesize					: 10byte
// spare					: 2byte
// 한 파일에 32바이트가 필요하면 512바이트 / 32바이트 = 16 파일이 표현된다.

//#if	!defined(SIZE_LOGBOOK)
//#define	SIZE_LOGBOOK	496
//#endif

/*
	(RTDBINFO)
	RTDBINFO ---+-	wAddr
	|512		+-	cCtrl
				+-	RTDBTEXT ---+-	c[509]
					|t-509		+- 	SBRTD    ---+-	cFlow
									s			+-	RTDPACK  ---+-	ob  --------+-	main
													|pack-508	|				+-	head
																|				+-	active
																+-	cItem	------	NON, STATE, LOGBOOK, TROUBLE, TRIAL, TRACE, TEST, DIRECTORY, FILE...
																+-	RTDPAD    --+-	RTDSPADE   -+-	dwTotal
																	|pad-506	|	|sp			+-	dwIndex
																				|				+-	wLength
																				|				+-	RTDHOE   ---+-	c[496]
																				|					|hoe-496	+-	ENTRYSHAPE[]
																				|								|	|f
																				|								+-	ENTRYHEAD --+-	ENTRYSHAPE
																				|								|	|h			+-	c[]
																				|								+-	USBDLCMD  --+-	cItem
																				|									|ucmd		+-	dwIndex
																				+-	RTDTYPIC  --+- 	wLength
																					|tp			+- 	RTDTINE  ---+-	RTDSTATE|stat
																									|tin-504	+-	logbook
																												+-	trouble
	24 hour = 172,800 dtb cycle
	172,800 * 504 = 87,091,200 bytes
	87,091,200 / 496 = 175,588 rtd cycle
	2Mbps 에서 512바이트틀 주고 받는 시간을 5ms쯤으로 보고...
	175,588 * 5ms ~= 878초 = 15분 남짓... 길다..
	이렇게 하면 다른 프로세스에 영향을 줄 수도 있으므로
	Arteriam()에서 한다고 보면 10ms 인터벌임으로 시간은 두배가 된다.
	175588 = 0x2ade4
	// 위 설명은 맞지 않다.
*/

typedef struct _tagRTDSTATE {
	__BC(	stat,
				modm,	1,	// 1:동작 / 0:정지
				txfw,	1,	// 1:전송 실패 - wifi
				cpm,	1,	// 1:동작 / 0:정지
				fail,	1,	// 1:고장 / 0:정상
				txfl,	1,	// 1:전송 실패 - lte
				tv,		1,	// 1:time valid
				sp,		2);
	__BC(	serv,
				wifi,	1,	// 1:server connect
				lte1,	1,	//
				lte2,	1,
				lte3,	1,
				sp,		4);
	DEVTIME	devt;
} RTDSTATE, *PRTDSTATE;

typedef struct _tagRTDSTATEA {
	__BC(	stat,
				modm,	1,	// 1:동작 / 0:정지
				txfw,	1,	// 1:전송 실패 - wifi
				cpm,	1,	// 1:동작 / 0:정지
				fail,	1,	// 1:고장 / 0:정상
				txfl,	1,	// 1:전송 실패 - lte
				tv,		1,	// 1:time valid
				sp,		2);
	__BC(	serv,
				wifi,	1,	// 1:server connect
				lte1,	1,	//
				lte2,	1,
				lte3,	1,
				sp,		4);
	DBFTIME	dbft;
} RTDSTATEA, *PRTDSTATEA;
//#define	RTD_TROUBLEBLOCK	(sizeof(LOGBOOK) / sizeof(TROUBLEARCH))
//typedef struct _tagWAKEUPNOTE {
//	ENVIRONS	env;			c[0] ~
//	CAREERSA	crr;			c[100] ~
//	CMJOGA		cmj;			c[300] ~
//} WAKEUPNOTE;
//
typedef union _tagRTDTINE {
	RTDSTATE	stat;
	//WAKEUPNOTE	note;
	//LOGBOOK		logbook;
	//TROUBLEARCH	troble[RTD_TROUBLEBLOCK];
	BYTE		c[504];
} RTDTINE;

typedef struct _tagRTDFRESH {	// 506bytes
	WORD		wLength;		// block length(ex:trouble block...)
	RTDTINE		tin;			// 504bytes
} RTDTYPIC, *PRTDTYPIC;

								//  00000000011111111112222222
								//  12345678901234567890123456
#define	SIZE_ENTRYNAME	27		// "/h/sd/arc/d991231/l235959	25 char
								// "/h/sd/ins/p170222004211		23 char
typedef struct _tagENTRYSHAPE {
	char		szName[SIZE_ENTRYNAME];
	DWORD		dwSize;
} ENTRYSHAPE, *PENTRYSHAPE;

// for RTD
typedef struct _tagENTRYCONTENT {
	char		szName[SIZE_ENTRYNAME];
	DWORD		dwSize;
	bool		valid;
} ENTRYCONTENT, *PENTRYCONTENT;

#define	INDEX_FILEHEAD		0x80000000
#define	SIZE_RTDCONTEXT		496
#define	SIZE_ENTRYBLOCK		(SIZE_RTDCONTEXT / sizeof(ENTRYSHAPE))	// be 16
#define	SIZE_TROUBLEBLOCK	(SIZE_RTDCONTEXT / sizeof(QWORD))		// 62
#define	SIZE_HEADCONTEXT	(SIZE_RTDCONTEXT - sizeof(ENTRYSHAPE) - sizeof(BYTE))

typedef struct _tagENTRYHEAD {
	ENTRYSHAPE	f;
	BYTE		cNull;				// for even
	BYTE		c[SIZE_HEADCONTEXT];	// 464
} ENTRYHEAD;

#define	USBDIRITEM_DAILY	0		// /dos/c/n2XYY/arc/dYYMMDD		- /lHHmmSS,	- /tHHmmSS
#define	USBDIRITEM_INSPECT	1		// /dos/c/n2XYY/ins				- /pYYMMDDHHmmSS,	- /iYYMMDDHHmmSS,	- /mYYMMDDHHmmSS
typedef struct _tagUSBDIRCMD {		// mm에 주는...
	BYTE	cItem;					// 다운로드하기 전에 USB에 다운로드할 디렉토리를 만드는 명령이다
	DWORD	dwIndex;
} USBDLCMD;

typedef union _tagRTDHOE {
	BYTE		c[SIZE_RTDCONTEXT];		// 496bytes
	ENTRYSHAPE	fs[SIZE_ENTRYBLOCK];	//
	ENTRYHEAD	h;						// 파일 처음 부분을 전송할 때 이름과 같이 보내도록..., 이때 dwIndex는 0x80000000이 or 되어있다.
	USBDLCMD	ucmd;
} RTDHOE, *PRTDHOE;

typedef struct _tagRTDSTORAGE {			// 506bytes
	DWORD		dwTotal;				// 파일 리스트에서는 block index가 아니라 file index이다.
	DWORD		dwIndex;
	WORD		wLength;
	RTDHOE		hoe;
} RTDSPADE, *PRTDSPADE;

#if	!defined(_WIN32)
CHECK_SIZE(506, RTDSPADE)
#endif

typedef union _tagRTDSTANCE {			// 506bytes
	RTDTYPIC	tp;						// 506bytes
	RTDSPADE	sp;						// 506bytes
	BYTE		c[sizeof(RTDSPADE)];
} RTDPAD;

typedef struct _tagRTDCONTENT {			// 508bytes
	__BC	( ob,
				main,	1,
				head,	1,
				active,	1,
				spr,	5);
	BYTE		cItem;
	RTDPAD		pad;					// 506bytes
} RTDPACK, *PRTDPACK;

enum enRTDITEM {
	RTDITEM_NON = 0,
	RTDITEM_CLOSECURRENT,
	RTDITEM_SAVELOGBOOK,
	RTDITEM_SAVETROUBLE,
	RTDITEM_SAVEINSPECT,
	RTDITEM_DESKREPORT,
	RTDITEM_LOADLOGBOOKENTRY,
	RTDITEM_LOADINSPECTENTRY,
	RTDITEM_LOADLOGBOOKLIST,
	RTDITEM_LOADLOGBOOK,
	RTDITEM_LOADINSPECT,
	RTDITEM_LOADSECTIONFIELD,
	// 190523
	RTDITEM_SAVEFULLSDASET,
	RTDITEM_MAX
};

typedef struct _tagSBRTD {				// 509bytes
	BYTE		cFlow;
	RTDPACK		pack;					// 508bytes
} SBRTD;

typedef union _tagRTDBTEXT {			// 509bytes
	SBRTD		s;
	BYTE		c[sizeof(SBRTD)];
} RTDBTEXT;

typedef struct _tagRTDRINFO {
	WORD		wAddr;					// 0xff40
	BYTE		cCtrl;					// 0x13
	RTDBTEXT	t;						// 512 - 3 = 509bytes
} RTDBINFO, *PRTDBINFO;

#define	SIZE_RTDSB	sizeof(RTDBINFO)

//#if ((sizeof(RTDBINFO)) % 512)
//#error SIZE_RTDSB is not 512!!!
//#endif

typedef struct _tagRTDFLAP {
	RTDBINFO	r;
	RTDBINFO	a;
} RTDFLAP, *PRTDFLAP;

#pragma pack(pop)
