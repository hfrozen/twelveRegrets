/**
 * @file	Mm.h
 * @brief	파일 입출력 메시지 큐이름과 크기 등을 정한다.
 * @details	시간 설정이나 파일 쓰기등은 꽤 시간을 필요로 한다.
 *			ts에서 직접 이런 일들을 하게되면 원래의 제어/감시를 수행하는데 지연이 생겨 곤란하게 되므로 mm에서 수행하도록 한다.
 * @author	Che-follo
 * @date	2015-11-XX
 */
#pragma once

#include "DuoTypes.h"
#include "Docu.h"

#define	MTQ_STR			"/mtq"		// mm 과 ts 사이의 큐 이름
#define	MMQ_STR			"/mmq"		// 주기적인 메시지를 위한 mm 내부 큐, period
#define	MAQ_STR			"/maq"		// 비 주기적인 메시지를 위한 mm 내부 큐, occasion
#define	MAX_MMMSG		256

#define	MAX_FILELIST	8192		// 운행 기록 디렉토리내 파일의 최대치
typedef struct _tagFILELIST {
	WORD		wLength;
	ENTRYSHAPE	list[MAX_FILELIST];
} FILELIST, *PFILELIST;

typedef struct _tagLOGLIST {
	WORD			wListNumber;
	ENTRYCONTENT	list[MAX_FILELIST];
} LOGLIST, *PLOGLIST;

#pragma	pack(push, 1)

typedef struct _tagFILEACCESST {
	WORD	wCur;
	WORD	wMax;
} FILEACCESST;

typedef struct _tagAIDMSG {
	WORD	wCmd;
} AIDMSG;

enum enAIDCMDINDEX {
	AIDCMD_NON = 0,
	AIDCMD_TIMESETBYUSER,
	// 171212
	AIDCMD_CLOSECURRENT,		// c_bCurDirExist를 false로 하여 새로운 디렉토리를 만들도록 한다.
	AIDCMD_CULLREQUIREDDOC,		// 3
	AIDCMD_SAVEENVIRONS,		// 4
	AIDCMD_SAVECAREERS,			// 5
	AIDCMD_APPENDLOGBOOK,		// 6
	AIDCMD_APPENDTROUBLE,		// 7
	AIDCMD_SAVEINSPECT,			// 8
	AIDCMD_SAVESIVDETECT,		// 9
	AIDCMD_SAVEECUTRACE,		// 10
	AIDCMD_ERASEECUTRACE,		// 11
	AIDCMD_ARRAGEECUTRACE,		// 12
	AIDCMD_SAVESIVTRACESPHERE,	// 13
	AIDCMD_SAVEV3FTRACESPHERE,	// 14
	AIDCMD_SAVEECUTRACESPHERE,	// 15
	AIDCMD_EXTRACTLOGBOOKENTRY,	// 16
	AIDCMD_EXTRACTINSPECTENTRY,	// 17
	AIDCMD_DESTINATIONONUSB,	// 18
	AIDCMD_EXTRACTLOGBOOKLIST,	// 19
	AIDCMD_COPYLOGBOOK,			// 20
	AIDCMD_EXTRACTLOGBOOK,		// 21
	AIDCMD_EXTRACTINSPECT,		// 22
	AIDCMD_INSERTIONLOGBOOK,	// 23
	AIDCMD_INSERTIONINSPECT,	// 24
	AIDCMD_DOWNLOADCANCEL,		// 25
	AIDCMD_USBSCAN,				// 26
	//AIDCMD_NOTUSED27,			// 27
	AIDCMD_TIMESETBYRTD,		// 27
	AIDCMD_NOTUSED28,			// 28
	AIDCMD_SENDEDBYWIRELESS,	// 29, 마지막 무선 보고를 날짜로서 남기고 구간 적산 거리, 적산 전력등을 초기화 하여 다시 증가시킨다.
	AIDCMD_COMPRESSION,			// 30
	AIDCMD_APPENDLOGBOOKTOCPM,	// 31
	AIDCMD_SENDEDBYWIRELESSOK,	// 32
	AIDCMD_CREATENEWFILE,		// 33	일정 시간마다 운행정보 및 고장 정보를 LOG 서버에 보내기 위해 신규 파일 생성함
	AIDCMD_APPENDLOGBOOKTOLTE,	// 34
	AIDCMD_NEWSHEET,			// 35
	AIDCMD_MAX
};

#define	BULK_STR		"/bulk"

typedef union _tagBULKTEXT {
	RTDPAD		pad;
	BYTE		c[sizeof(RTDPAD)];
} BULKTEXT;

#define	SIZE_BULKTINE	sizeof(RTDTINE)
typedef struct _tagBULKMODULE {
	bool		bBegin;
	bool		bEnd;
	short		iResult;
	WORD		wSeq;			// mm은 운행 기록의 wSeq를 여기에 담아두고 ts는 그 변화를 지켜보며 mm이 동작하고 있는지 검사한다.
	BULKTEXT	bt;
} BULKMODULE;

enum enBPINDEX {
	BPID_PERIOD,		// 운행 기록이나 고장 정보를 주기적으로 저장할 때...
	BPID_OCCASN,		// 운행 기록이나 고장 정보를 참조할 때...
	BPID_MAX
};

typedef struct _tagFILEATS {
	FILEACCESST	logw;
	FILEACCESST	trbw;
	FILEACCESST	rd;
	FILEACCESST	wr;
} FILEATS;

typedef struct _tagBULKPACK {
	WORD		wMmVer;
	WORD		wState;
	int64_t		iUsbSize;
	uint64_t	uiCurSize;			// 메모리에서 usb로 바로 옮길 때 옮기는 양를 알 방법이 없다, 일단 이 변수로 알 수 있도록 한다.
	FILEATS		fts;
	BYTE		cEcuTraceLength;
	// 180308
	//BYTE		cNull;
	BYTE		cCullState;			// 211편성에서 누적 거리와 전력량이 리셋되는...
									// 이 변수를 통해 CFlux::CullRequiredDoc()의 결과를 살펴본다.
	LOGBOOKHEAD	lbHead;
	BULKMODULE	m[BPID_MAX];
	V3FBRKCNTU	bcu;
	// 171107
	WORD		wOldLoadTime;
	//WORD		wOldTroubleMap;
	//OLDTROUBLES	ot;
} BULKPACK, *PBULKPACK;

#pragma	pack(pop)

enum enBULKSTATE {
	BULKSTATE_CULLREQUIREDDOC,	// 1
	BULKSTATE_TIMESETTING,		// 2
	BULKSTATE_LOGAREA,			// 4
	BULKSTATE_MOUNTING,			// 8
	BULKSTATE_FILEWORK,			// 0x10
	BULKSTATE_ARRANGEECUTRACE,	// 0x20
	BULKSTATE_ERASEECUTRACE,	// 0x40
	BULKSTATE_SEIZEUSB,			// 0x80
	BULKSTATE_OLDTROUBLE,		// 0x100
	BULKSTATE_INTCMS,			// 0x200
	// 181011
	//BULKSTATE_DELETEBAD,		// 0x400
	BULKSTATE_INRTD = 15,
	BULKSTATE_MAX
};

enum enBULKFILEID {
	BULKFID_HEAD,
	BULKFID_FILEEND,
	BULKFID_DIREND
};

enum enBULKRES {
	BULKRES_OK = 1,
	BULKRES_ERRNON = 0,
	BULKRES_ERRCANNOTCREATETROUBLEPATHTMP = -1,
	BULKRES_ERRCANNOTCREATECURRENTDIR = -2,
	BULKRES_ERRCANNOTRENAMEDIR = -3,
	BULKRES_ERRCANNOTREMOVEFILE = -4,
	BULKRES_ERRCANNOTFOUNDSRC = -5,
	BULKRES_ERRCANNOTKNOWNSIZESRC = -6,
	BULKRES_ERRCANNOTREADSRC = -7,
	BULKRES_ERRCANNOTOPENDEST = -8,
	BULKRES_ERRCANNOTWRITEDEST = -9,
	BULKRES_ERRCANNOTMAKEDIR = -10,
	BULKRES_ERRCANNOTMAKEDIRONUSB = -11,
	BULKRES_ERRUNKNOWN = -100
};

// 180308
enum enCULLERR {
	CULLERR_CANNOTOPENENV = 0,			// 1
	CULLERR_UNMATCHSIZEENV,				// 2
	CULLERR_NOTDEFINED,					// 4
	CULLERR_WRONGLOGBOOKVERSION,		// 8
	CULLERR_CANNOTOPENLOGBOOK,			// 10
	CULLERR_UNMATCHSIZELOGBOOK,			// 20
	CULLERR_CANNOTFOUNDLATELYLOGBOOK,	// 40
	CULLERR_COUNDNOTFOUNDLOGBOOK,		// 80
	CULLERR_MAX
};

enum enTX_ERROR
{
	LOG_NO_ERROR = 0,
	LOG_DIR_NOT_EXIST,
	LOG_COMP_PASS,
	LOG_COMP_ERROR
};

/*
	BULKPACK
	+-	wCmd
	+-	iResult
	+-	info ---+-	lbHead ---	title[12], wVer, wProperNo, cLength, cRate, cDummy[14];		// 32
				+-	bt   ---+-	c[506]
							+-	RTDPAD    --+-	RTDSPADE   -+-	dwTotal
								|pad-506	|	|sp			+-	dwIndex
											|				+-	wLength
											|				+-	RTDHOE   ---+-	c[496]
											|					|hoe-496	+-	ENTRYSHAPE[]
											|								|	|f
											|								+-	ENTRYHEAD --+-	ENTRYSHAPE
											|									|h			+-	c[]
											+-	RTDTYPIC  --+- 	wLength
												|tp			+- 	RTDTINE  ---+-	RTDSTATE|stat
																|tin-504	+-	logbook
																			+-	trouble
*/

// temporary file
#define	PATHTMP					"/tmp"
#define	ENVIRONS_PATHTMP		"/tmp/envi"
#define	CAREERS_PATHTMP			"/tmp/career"
#define	TRACE_PATHTMP			"/tmp/b"
#define	INSPECT_PATHTMP			"/tmp/insp"
#define	ECUTRACE_PATHTMP		"/tmp/ecut"				// ecu 트레이스 데이터 단위를 기록하기위한 파일
#define	ECUTRACEALL_PATHTMP		"/tmp/ecutall"			// ecu 트레이스 데이터들을 마스터에 주기위한 파일
#define	SIVDETECT_PATHTMP		"/tmp/sivd"
#define	OLDTROUBLE_PATHTMP		"/tmp/oldt"				// oldt0..11
#define	OLDTROUBLEEND_PATHTMP	"/tmp/oend"
#define	LOG_PATHTMP				"/tmp/note"

// storage
#define	ENVIRONS_PATHA			"/h/m/envi"
#define	CAREERS_PATHA			"/h/m/career"
#define	CMJOG_PATHA				"/h/m/cmjog"

#define	CAREERS_PATHB			"/h/sd/care"
#define	CMJOG_PATHB				"/h/sd/cmj"
#define	V3FBCNT_PATHB			"/h/sd/vbc"

#define	ENVIRONS_PATH			"/h/sd/envi"
#define	CAREERS_PATH			"/h/sd/career"
#define	CMJOG_PATH				"/h/sd/cmjog"
#define	SECTDOSS_PATH			"/h/sd/sectds"
#define	STORAGE_PATH			"/h/sd"
#define	ARCHIVE_PATH			"/h/sd/arc"
#define	INSPECT_PATH			"/h/sd/ins"
#define	TRACE_PATH				"/h/sd/trc"
#define	LOG_PATH				"/h/sd/log"
#define	LOG_NAME				"/h/sd/log/log"
//#define	LOG_PATHTMP				"/tmp/note"		//"/h/sd/note"
#define CPM_PATH				"/h/usb"
#define	SENDHISTORY_PATH		"/h/sd/his"
#define	ARCHIVELTE_PATH			"/h/sd/lte"
#define	WIFISERVER_IP			"194.168.0.230"
#define WIFISERVER_IDPW			"rtdftp:rtdftpuserspassword"

#define	SIVTRACESPHERE_NAME		"tsiv"		// tsivx_xx
#define	V3FTRACESPHERE_NAME		"tv3f"		// tv3fx_xx
#define	ECUTRACESPHERE_NAME		"tecu"		// tecux_xx

#define	LOGDIR_SIGN				'd'
#define	LOGBOOK_SIGN			'l'
#define	TROUBLE_SIGN			't'
#define	TRACESIV_SIGN			's'
#define	TRACEV3F_SIGN			'v'
#define	TRACEECU_SIGN			'e'
#define	TRIAL_SIGN				'r'
#define	PDTINSPECT_SIGN			'p'
#define	DAILYINSPECT_SIGN		'i'		// 'd'는 운행 기록 디렉토리 이름으로 쓴다.
#define	MONTHLYINSPECT_SIGN		'm'

#define	USB_PATH				"/dos/c"
#define	USBARCH_NAME			"/arch"
#define	USBINSP_NAME			"/insp"
#define	USBTRACE_NAME			"/trace"

#define	LIMIT_PDT				100
#define	LIMIT_DAILY				50
#define	LIMIT_MONTHLY			30

#define	LIMIT_INSPECT			(LIMIT_PDT + LIMIT_DAILY + LIMIT_MONTHLY)
#define	LIMIT_LOGBOOK			LIMIT_INSPECT

#define	_DEBUG_DOWNLOAD_
