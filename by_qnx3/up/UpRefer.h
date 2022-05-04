/**
 * @file	UpRefer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "../Common/Refer.h"

#define	UTQ_STR				"/utq"			// p & tdc connect queue
#define	UTPACK_STR			"/utpk"
#define UPLOADRESULT_FILE	"/tmp/uploadres.txt"

//typedef struct _tagUPMEASURINGITEMAS {
//	MSRTIME	up;
//} UPMSRITEM;
//
typedef struct _tagUPMEASURINGITEMAS {
	MSRTIMEDW	up;
	MSRTIMEDW	cp;
	MSRTIMEDW	rp;
} UPMSRITEM;

typedef struct _tagUTBAIL {
	WORD		wCmd;
	WORD		wResult;
	UPMSRITEM	mi;
	WORD		wCurIP;
	WORD		wRatioT;		// rate per thousand
	WORD		wRepState;		// 200212
} UTBAIL;

typedef struct _tagUTPACK {
	UTBAIL		bail;
	TODTEXT		todt;		// 200212
	_ENV		env;
} UTPACK, *PUTPACK;

enum enUPCMD {
	UPCMD_UPLOAD = 0,
	UPCMD_ABORT
};

enum enUPRESULT {
	UPRESULT_SUCCESS = 1,
	UPRESULT_NOFILES,
	UPRESULT_ABORT,
	UPRESULT_NONIP,
	UPRESULT_FAIL
};

enum enREPSTATE {
	REPST_CONNECTING = 0,
	REPST_CONNECTED,
	REPST_SENDING
};
