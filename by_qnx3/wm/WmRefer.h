/**
 * @file	WmRefer.h
 * @brief
 * @details
 * @author
 * @data
 */
#pragma once
#include "../Common/DefTypes.h"

#define	WMQ_STR			"/wmq"		// dm pump queue
#define	PULLQ_STR		"/wtq"		// dm & tdc connect queue

#if	!defined(MAINQ_STR)
#define	MAINQ_STR		"/tdq"		// tdc pump queue
#endif

#define	SIZE_WMMSG		8192
#define	SIZE_WMRTIME	28			// "\tVARS:MSG:00-00-00 00:00:00"
#define	SIZE_GENBUF		SIZE_WMMSG - SIZEWMRTIME
#define	MAX_WMMSG		16

typedef struct _tagWMMSG {
	WORD	wID;
	DWORD	dwLength;
	BYTE	cBuf[SIZE_WMMSG];
} WMMSG, *PWMMSG;

enum enWMMSG {
	WMMSG_CONTS = 1,
	WMMSG_ACCEPT,
	WMMSG_SEND,
	WMMSG_RECEIVE,
	WMMSG_CLOSE
};

enum enTAGS {
	TAG_ACCEPT = 1,
	TAG_LINKWM,
	TAG_UNLINKWM,
	TAG_DEBUGWM
};
