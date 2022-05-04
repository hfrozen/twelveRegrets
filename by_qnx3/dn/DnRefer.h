/**
 * @file	DnRefer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "../Common/Refer.h"

#define	DTQ_STR				"/dtq"			// wd & tdc connect queue
#define	DTPACK_STR			"/dtpk"
#define DNRESULT_FILE		"/tmp/downres.txt"

typedef struct _tagDNMEASURINGITEMAS {
	MSRTIMEDW	sc;
	MSRTIMEDW	dn;
} DNMSRITEM;

typedef struct _tagDTBAIL {
	WORD		wCmd;
	WORD		wState;
	WORD		wResult;
	DNMSRITEM	mi;
	WORD		wRatioT;		// rate per thousand
} DTBAIL;

typedef struct _tagDTPACK {
	DTBAIL		bail;
	_ENV		env;
} DTPACK, *PDTPACK;

enum enDNCMD {
	DNCMD_DOWNLOAD = 0,
	DNCMD_ABORT
};

enum enWDSTATE {
	DNSTATE_MOUNTING = 0,
	DNSTATE_SEIZEUSB,
	DNSTATE_DOWNLOADING,
	DNSTATE_NOTHINGMORE
};

enum enDNRESULT {
	DNRESULT_SUCCESS = 1,
	DNRESULT_NOFILES,
	DNRESULT_CANNOTMAKEDIR1,
	DNRESULT_CANNOTMAKEDIR2,
	DNRESULT_CANNOTMAKEDIR3,
	DNRESULT_FAIL1,
	DNRESULT_FAIL2
};
