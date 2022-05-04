/**
 * @file	SmRefer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "../Common/Refer.h"

#define	STQ_STR			"/stq"			// sm & tdc connect queue
#define	STPACK_STR		"/stpk"

//#define	SIZE_TOD		1280			// train operation data
//// 1280 * 3600 = 4,608,000(hour) * 24 = 110,592,000(day) * 30 = 3,317,760,000
// Collect for 5min
// 1280 * 60 * 5 = 384,000bytes
// Number of files in 31days
// 24(hour) * 12(60/5min) = 288(/day),	288 * 31 = 8928(/31day)

//#define	SIZE_TOD		1408			// train operation data
// 1408 * 5(min) * 60(sec) = 422,400 bytes -> zip -> =~~ 400kb
// file length = 31(day) * 24(hour) * 60(min) / 5 = 8928
// total size = 8928 * 400kb = 3.5712gb
// 1408 * 3600 = 4,608,000(hour) * 24 = 110,592,000(day) * 30 = 3,317,760,000
// Collect for 5min
// 1280 * 60 * 5 = 384,000bytes
// Number of files in 31days
// 24(hour) * 12(60/5min) = 288(/day),	288 * 31 = 8928(/31day)

//typedef struct _tagSMMEASURINGITEMS {
//	MSRTIME	wrt;
//	MSRTIME	zip;
//} SMMSRITEM;
//
typedef struct _tagSMMEASURINGITEMS {
	MSRTIMEDW	wrt;		// include file align & file zip time
	MSRTIMEDW	zip;
} SMMSRITEM;

typedef struct _tagSTBAIL {
	WORD		wStatus;
	WORD		wResult;
	SMMSRITEM	mi;
	WORD		wRecvErr[2];
} STBAIL;

typedef struct _tagSTPACKEX {			// sm & tdc shared memory
	STBAIL		bail;
	TODTEXT		todt;
	_ENV		env;
} STPACK, *PSTPACK;

enum enSMSTATUS {
	SMSTATUS_APPEND = 0,
	SMSTATUS_ALIGN
};

enum enSLABERROR {
	PACKERR_NON = 0,
	PACKERR_CANNOTWRITETOD = 1
};

//# find . -name "c*" -exec zip {}.zip {} \;
